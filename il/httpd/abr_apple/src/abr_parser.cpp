#include <string.h>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_loops.hpp>

#include "abr_interfaces.h"
#include "abr_player.h"


using namespace std;
using namespace boost::spirit::classic;

////////////////////////////////////////////////////////////////////////////////////
bool Player::ParseUrl(URL* mUrl, string str)
{
    parse_info<const char*> info;

    rule<> scheme_p = (+alpha_p);
    rule<> scheme_a = (scheme_p >> str_p("://"))[assign_a(mUrl->mSchemeName)];

    rule<> ipv6_p = ch_p('[') >> +(+xdigit_p >> ch_p(':') >> !ch_p(':')) >> +xdigit_p >> ch_p(']');
    rule<> ipv4_p = +(+digit_p >> ch_p('.')) >> +digit_p;
    rule<> domain_p = alpha_p >> +(+alnum_p >> ch_p('.')) >> +alpha_p;

    rule<> port_a = ch_p(':') >> (+digit_p)[assign_a(mUrl->mPortNumber)];
    rule<> host_a = (ipv6_p | ipv4_p | domain_p)[assign_a(mUrl->mDomainName)][assign_a(mUrl->mPortNumber, "80")] >> !port_a;

    rule<> name_p = +(alnum_p | ch_p('.') | ch_p('-') | ch_p('_'));
    rule<> root_a = str_p("./") | ch_p('/')[assign_a(mUrl->mRoot)][assign_a(mUrl->mPath, "")][assign_a(mUrl->mProgram, "")];
    rule<> path_a = !root_a >> (+(name_p >> ch_p('/')))[assign_a(mUrl->mPath)];

    rule<> program_a = name_p[assign_a(mUrl->mProgram)];
    rule<> url_a = !(!scheme_a >> host_a[assign_a(mUrl->mRoot, "/")] >> !port_a) >> !root_a >> !path_a >> !program_a;

    info = parse(str.c_str(), str.c_str() + str.size(), url_a, space_p);

    return info.full;
}

////////////////////////////////////////////////////////////////////////////////////
int Player::ParseHttpHeader(const char* uParserBegin, const char* uParserEnd)
{

    parse_info<const char*> mInfo;

    rule<> text_p = +print_p;
    rule<> endl_p = !ch_p('\r') >> ch_p('\n');
    rule<> version_a = (str_p("HTTP/") >> +digit_p >> ch_p('.') >> +digit_p)[assign_a(mHttp.mVersion)];
    rule<> status_a = +space_p >> uint_p[assign_a(mHttp.mStatus)];
    rule<> content_type_a = str_p("Content-Type:") >> !(+space_p) >> text_p[assign_a(mHttp.mContentType)];
    rule<> content_length_a = str_p("Content-Length:") >> !(+space_p) >> uint_p[assign_a(mHttp.mContentLength)];

    rule<> header_a = ((version_a >>status_a) | content_type_a | content_length_a | text_p ) >> !text_p  >>endl_p;

    do
    {
        mInfo = parse(uParserBegin, uParserEnd, header_a, nothing_p);
        mHttp.mHeaderLength += mInfo.stop - uParserBegin;
        uParserBegin = mInfo.stop;
    } while (mInfo.hit);

    //////////////////////////////////////////
    mInfo = parse(uParserBegin, uParserBegin + 2, (eol_p));

    if (mInfo.full)
    {
        mHttp.mHeaderLength += mInfo.stop - uParserBegin;
        uParserBegin = mInfo.stop;
        mHttp.mReplyLength = mHttp.mHeaderLength + mHttp.mContentLength;
    }

    return (uParserEnd - uParserBegin);
}

////////////////////////////////////////////////////////////////////////////////////
int Player::ParseManifest(const char* uParserBegin, const char* uParserEnd)
{
    int uProgramID = 0;
    int tag;
    string url;

    static const int TAG_NONE = 0;
    static const int TAG_EXT_X_STREAM_INF = 2;
    static const int TAG_EXTINF = 5;
    //static const int TAG_EXTM3U = 1;
    //static const int TAG_EXT_X_TARGETDURATION = 3;
    //static const int TAG_EXT_X_MEDIA_SEQUENCE = 4;
    //static const int TAG_EXT_X_ENDLIST = 6;


    VariantDescriptor uVariantDescriptor;
    FragmentDescriptor uFragmentDescriptor;

    parse_info<const char*> mInfo;

    rule<> text_p = +print_p;
    rule<> endl_p = !ch_p('\r') >> ch_p('\n');

    rule<> extm3u_p = str_p("#EXTM3U");
    rule<> ext_x_stream_inf_p = str_p("#EXT-X-STREAM-INF:");

    rule<> program_id_a = str_p("PROGRAM-ID=") >> uint_p[assign_a(uProgramID)];
    rule<> bandwidth_a = str_p("BANDWIDTH=") >> uint_p[assign_a(uVariantDescriptor.mCompressionRate)];
    rule<> url_a = text_p[assign_a(mUrl)];

    rule<> ext_x_target_duration_a = str_p("#EXT-X-TARGETDURATION:") >> uint_p[assign_a(mTargetDuration)] >> !(ch_p('.') >> +digit_p);
    rule<> ext_x_media_sequence_a = str_p("#EXT-X-MEDIA-SEQUENCE:") >> uint_p[assign_a(mMediaSequence)];
    rule<> ext_x_media_endlist_a = str_p("#EXT-X-ENDLIST")[assign_a(mEndList, true)];
    rule<> extinf_a =  str_p("#EXTINF:")>>uint_p[assign_a(uFragmentDescriptor.mDuration)]>>ch_p(',')>>endl_p>>text_p[assign_a(uFragmentDescriptor.mUrl)];
    rule<> ext_x_stream_inf_a = ext_x_stream_inf_p >> program_id_a >> ch_p(',') >> bandwidth_a >> !text_p >> endl_p >> url_a;


    rule<> manifset_a = !(extinf_a[assign_a(tag, TAG_EXTINF)] | ext_x_stream_inf_a[assign_a(tag, TAG_EXT_X_STREAM_INF)] | extm3u_p | ext_x_target_duration_a | ext_x_media_sequence_a | ext_x_media_endlist_a) >> !text_p >> endl_p;


    do
    {
        tag = TAG_NONE;
        mInfo = parse(uParserBegin, uParserEnd, manifset_a, nothing_p);
        uParserBegin = mInfo.stop;

        if (tag == TAG_EXTINF)
        {
            if (mMediaSequence > mFragmentLast)
            {
                uFragmentDescriptor.mNo = mMediaSequence;
                uFragmentDescriptor.mExpiredBefore = mTimeNow;
                uFragmentDescriptor.mExpiredAfter = mTimeNow + mTargetDuration;  //AY Fix it

                mFragment.push_back(uFragmentDescriptor);
                mFragmentLast = mMediaSequence;
                mFragmentsAvailable += 1;
            }
            mMediaSequence++;
        }

        else if (tag == TAG_EXT_X_STREAM_INF)
        {
            if (mCfg.mProgramID == 0)
            {
                mCfg.mProgramID = uProgramID;
            }
            if (mCfg.mProgramID == uProgramID)
            {
                if (mCfg.mBitrateMin <= uVariantDescriptor.mCompressionRate && uVariantDescriptor.mCompressionRate <= mCfg.mBitrateMax)
                {
                    uVariantDescriptor.mUrl  = mCfg.mUrl;
                    if (ParseUrl(&uVariantDescriptor.mUrl, mUrl))
                    {
                        mVariant.push_back(uVariantDescriptor);
                    }
                    else FatalError(string("Can't Parse Variant url:") + mUrl + "\n");
                }
            }
        }

    } while (mInfo.hit);
    return (uParserEnd - uParserBegin);
}

////////////////////////////////////////////////////////////////////////////////////
int Player::ParseFragment(const char* uParserBegin, const char* uParserEnd)
{
    return 0;
}

