# main window
##############
proc test.Quit {} {
    init:SaveOptions 0
    exit
}
proc test.RtpSupported {} {return "4.1.0.3"}
proc test.SnmpSupported {} {return "1"}
proc test.SecSupported {} {return "4.1.0.3"}
proc test.H450Supported {} {return "4.1.0.3"}
proc test.LdapSupported {} {return ""}
proc test.AllocationDebug {} {return 0}


# calls
########
proc Call.SetOutgoingParams {callInfo} {}
proc Call.Dial {callInfo aliases} {Window close .newcall}
proc Call.Drop {callInfo channelIn channelOut reason} {}
proc Call.SendAlerting {callInfo} {}
proc Call.SendCallProceeding {callInfo} {}
proc Call.SendConnect {callInfo} {}
proc Call.SendBRQ {callInfo desiredBandwidth} {}
proc Call.SendStatusInquiry {callInfo} {}
proc Call.SendProgress {callInfo} {}
proc Call.SendNotify {callInfo} {}
proc Call.SendFacility {callInfo Aliases} {}
proc Call.IncompleteAddress {callInfo} {Window close .incall}
proc Call.SendAdditionalAddr {callInfo isARJ args} {Window close .redial}
proc Call.CancelNewCall {callInfo} {
    Window close .newcall
    .test.msg.list delete end
}



# faststart
############
proc Call.OpenOutgoingFaststart {callInfo} {
    Window open .faststart $callInfo 0 "g711Alaw64k g711Ulaw64k g728 g7231"
}
proc Call.SetOutgoingFaststart {callInfo} {Window close .faststart}
proc Call.ApproveFaststart {callInfo} {}


# channels
###########
proc Channel.OpenOutgoingWindow {callInfo} {}
proc Channel.ConnectOutgoing {callInfo args} {}
proc Channel.DisplayChannelList {args} {}
proc Channel.ResponseForOLC {channelHandle acceptOrReject} {}
proc Channel.ResponseForCLC {channelHandle acceptOrReject} {}
proc Channel.Drop {channelHandle1 channelHandle2 dropReason} {}

# tools
########
proc Log.FetchFilter {filter} {}
proc Log.SetFilter {levelChar onoff filter} {}
proc Log.Update {catchIt} {}
proc Options.SetStaticFields {} {}
proc Options.GetLocalIP {} {return "127.0.0.1"}
proc Status.Display {} {
    status:Clean
    for {set i 0} {$i < 30} {incr i} {status:InsertLine TEST TEST$i [expr $i*2] [expr $i*3] [expr $i*4]}
    status:InsertLine {}   TIMER {} [clock seconds] {}
    status:DrawGraphs
}
proc sec:notifyModeChange { mode } {}



# wrapped API functions
########################
proc api:cm:GetGenParam {param} {return 0}
proc api:cm:CallCreate {} {}
proc api:cm:CallDial {handle} {}
proc api:cm:CallProceeding {handle display useruser} {}
proc api:cm:CallAccept {handle display useruser} {}
proc api:cm:CallDrop {handle UserUser} {}
proc api:cm:CallAccept {handle} {}
proc api:cm:CallAnswer {handle display useruser} {}
proc api:cm:CallSetParam {handle CallParam bool str alias index} {}
proc api:cm:CallStatusInquiry {handle display} {}
proc api:cm:CallSendProgress {handle} {}
proc api:cm:CallSendNotify {handle notification display} {}
proc api:cm:CallSendInformation {handle display} {}
proc api:cm:CallConnectControl {handle} {}
proc api:cm:CallCapResp {handle response} {}
proc api:cm:CallSendAdditionalAddress {handle AdditionalDigits} {}
proc api:cm:CallIncompleteAddress {handle AddressComplete DisplayInfo} {}
proc api:cm:ChannelCreate {CallHandle} {}
proc api:cm:ChannelOpen {CallHandle ChannelHandle} {}
proc api:cm:ChannelAnswer {ChannelHandle Response} {}
proc api:cm:ChannelDrop {ChannelHandle} {}
proc api:cm:ChannelReplyDrop {ChannelHandle Response} {}
proc api:cm:ChannelClose {ChannelHandle} {}
proc api:cm:ChannelGetParam {ChannelHandle ParamType} {}
proc api:app:GetDataTypes {} {return {a b ccgvgjcf}}
proc api:app:GetChannelList {callHandle} {}
proc api:cm:GetValTree {} {return 0}
proc api:cm:GetRASConfigurationHandle {} {return 0}
proc api:cm:Vt2Alias { arg1 arg2 } {return ""}
proc api:cm:GetLocalRASAddress {} {return "127.0.0.1:1719"}
proc api:cm:GetLocalCallSignalAddress {} {return "127.0.0.1:1720"}
proc api:cm:GetLocalAnnexEAddress {} {return "127.0.0.1:1719"}
proc api:pvt:GetByPath { arg1 arg2 arg3 } {return -1}


# manual H245
##############
proc Call.H245SendMsg {CallInfo ChannelHandle MessageType} {}

# VTCL functions
#################
proc vTclWindow. {base} {}
proc vTclWindow.test {base} {test:create}
proc vTclWindow.redial {base} {redial:create 1}
proc vTclWindow.incall {base} {incall:create}
proc vTclWindow.call {args} {call:create .call "Call information"}
proc vTclWindow.inchan {args} {inchan:create}
proc vTclWindow.capability {args} {capability:create}
proc vTclWindow.log {base} {log:create}
proc vTclWindow.status {base} {status:create}
proc vTclWindow.console {base} {console:create}
proc vTclWindow.facility {args} {facility:create "1 0x0"}
proc vTclWindow.faststartIn {args} {faststart:create "1 0x0" 1 "g711Alaw64k g711Ulaw64k g728 g7231 h261VideoCapability h263VideoCapability t120 h224" ""}
proc vTclWindow.faststartOut {args} {faststart:create "1 0x0" 0 "g711Alaw64k g711Ulaw64k g728 g7231 h261VideoCapability h263VideoCapability t120 h224" ""}
proc vTclWindow.manualRAS {args} {manualRAS:create}


test:updateGui
after 1000 "update;wm withdraw .;wm geometry .test $app(test,size);wm deiconify .test;update;notebook:refresh test"

