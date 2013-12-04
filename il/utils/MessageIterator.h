/// @file
/// @brief Message Iterator header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_MESSAGE_ITERATOR_H_
#define _L4L7_MESSAGE_ITERATOR_H_

#include <iterator>

#include <ace/Message_Block.h>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/utility/enable_if.hpp>

#include <utils/UtilsCommon.h>

DECL_L4L7_UTILS_NS

///////////////////////////////////////////////////////////////////////////////

template<class MessageBlockType>
class MessageIter
{
  private:
    struct enabler { };
    
  public:
    typedef typename boost::is_const<MessageBlockType>                   _const;
    typedef MessageIter<MessageBlockType>                                _self;
    typedef std::forward_iterator_tag                                    iterator_category;
    typedef char                                                         value_type;
    typedef std::ptrdiff_t                                               difference_type;
    typedef typename boost::mpl::if_<_const, const char *, char *>::type pointer;
    typedef typename boost::mpl::if_<_const, const char&, char&>::type   reference;
    
    MessageIter(void)
        : mb_(0),
          ptr_(0),
          end_(0)
    {
    }

    explicit MessageIter(MessageBlockType *mb)
        : mb_(mb),
          ptr_(mb->rd_ptr()),
          end_(mb->wr_ptr())
    {
    }

    MessageIter(MessageBlockType *mb, pointer p)
        : mb_(mb),
          ptr_(p),
          end_(mb->wr_ptr())
    {
    }

    /// @note Allows the conversion of mutable iterator to const iterator but not the other way around
    template<class OtherMessageBlockType>
    MessageIter(const MessageIter<OtherMessageBlockType>& other, typename boost::enable_if<boost::is_convertible<OtherMessageBlockType *, MessageBlockType *>, enabler>::type = enabler())
        : mb_(other.mb_),
          ptr_(other.ptr_),
          end_(other.end_)
    {
    }
    
    MessageBlockType *block(void) const { return mb_; }
    pointer ptr(void) const { return ptr_; }

    reference operator*() const { return *ptr_; }
    pointer operator->() const { return ptr_; }

    _self& operator++()
    {
        if (++ptr_ == end_)
        {
            MessageBlockType *cont = mb_->cont();
            if (cont)
            {
                mb_ = cont;
                ptr_ = mb_->rd_ptr();
                end_ = mb_->wr_ptr();
            }
        }
        return *this;
    }

    _self operator++(int)
    {
        _self tmp = *this;
        ++(*this);
        return tmp;
    }

    bool operator==(const _self& other) const { return (this->mb_ == other.mb_) && (this->ptr_ == other.ptr_); }
    bool operator!=(const _self& other) const { return !(*this == other); }
    
  private:
    template<class> friend class MessageIter;

    MessageBlockType *mb_;
    pointer ptr_;
    pointer end_;
};

///////////////////////////////////////////////////////////////////////////////

typedef MessageIter<ACE_Message_Block> MessageIterator;
typedef MessageIter<const ACE_Message_Block> MessageConstIterator;

///////////////////////////////////////////////////////////////////////////////

inline MessageIterator MessageBegin(ACE_Message_Block *mb)
{
    return MessageIterator(mb);
}

inline MessageConstIterator MessageBegin(const ACE_Message_Block *mb)
{
    return MessageConstIterator(mb);
}

inline MessageIterator MessageEnd(ACE_Message_Block *mb)
{
    while (mb->cont())
        mb = mb->cont();

    return MessageIterator(mb, mb->wr_ptr());
}

inline MessageConstIterator MessageEnd(const ACE_Message_Block *mb)
{
    while (mb->cont())
        mb = mb->cont();

    return MessageConstIterator(mb, mb->wr_ptr());
}

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_UTILS_NS

#endif
