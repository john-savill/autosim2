#include "stdafx.h"
#include "autosim_SerialComms.h"
#include "viossd.h"
#include "Protocols.h"
#include "MsgTypes.h"
#include "DllNames.h"
#include "Can.h"
#include "j1850.h"

#include <map>
using namespace std;
#pragma warning(disable:4786 4100)

typedef void (*MSGDECODER_FN) (BYTE* ,int &);
struct DECODER_PARAM
{
    MSGDECODER_FN pFn;
    CRxMessage *pRx;
};
map<unsigned long, DECODER_PARAM> g_RXMsgDecoderMap;


static CBranch *pCANVar;
CProtocolSupport CANBUS (CAN_1);
CBranch CAN_Branch ("CAN Network", &CANBUS);
CBranch NoBranch_TX ("Unallocated Messages", &CAN_Branch);
CBranch SNodes_Branch ("Simulated Nodes", &CAN_Branch);
CBranch NonSNodes_Branch ("Real Nodes", &CAN_Branch);

CModelVar HMI_120_TX_PERIOD   ("HMI_120_TX_PERIOD",     "",     10,     0,    1000,  DC_DIGITAL,     IFWRITE,    0,  pCANVar);
CModelVar HMI_120_TX_TIMER    ("HMI_120_TX_TIMER",      "",      0,     0,    1000,  DC_DIGITAL,     IFRDWR,     0,  pCANVar);
 
CModelVar ABS_450_TX_PERIOD   ("ABS_450_TX_PERIOD",     "",     50,     0,    1000,  DC_DIGITAL,     IFWRITE,    0,  pCANVar);
CModelVar ABS_450_TX_TIMER    ("ABS_450_TX_TIMER",      "",      0,     0,    1000,  DC_DIGITAL,     IFRDWR,     0,  pCANVar);

CModelVar ABS_451_TX_PERIOD   ("ABS_451_TX_PERIOD",     "",     10,     0,    1000,  DC_DIGITAL,     IFWRITE,    0,  pCANVar);
CModelVar ABS_451_TX_TIMER    ("ABS_451_TX_TIMER",      "",      0,     0,    1000,  DC_DIGITAL,     IFRDWR,     0,  pCANVar);

CBranch EMS_Branch ("EMS", &NonSNodes_Branch);
    CBranch EMS_RX ("RX Messages", &EMS_Branch);
        CRxMessage  EMS_160_RX("EMS_160_RX", "EMS_160_RX(0x160)",pCANVar ,&EMS_RX);
                CModelVar  CC_Status       ("CruiseCtrlStatus",  "", 0,    0,   2, DC_DIGITAL, IFRDWR, 0, pCANVar );
                CModelVar  CC_DeactReason  ("CCDeactiveReason",  "", 0,    0,  12, DC_DIGITAL, IFRDWR, 0, pCANVar );

CBranch HIL_Branch ("HIL", &SNodes_Branch);
    CBranch HIL_TX ("TX Messages", &HIL_Branch);
        CTxMessage  HMI_120_TX("HMI_120_TX", HMI_120_Tx_TrigFn, "HMI_120_TX(0x120)",pCANVar ,&HIL_TX);
                CModelVar  CC_OnOffState   ("CC OnOff State",    "", 0,    0,   3, DC_DIGITAL, IFWRITE, 0, pCANVar );
                CModelVar  CC_SetMinusState("CC SetMinus State", "", 0,    0,   3, DC_DIGITAL, IFWRITE, 0, pCANVar );
                CModelVar  CC_ResPlusState ("CC ResPlus State",  "", 0,    0,   3, DC_DIGITAL, IFWRITE, 0, pCANVar );
                CModelVar  CC_OnOff        ("CruiseCtrlOnOff",   "", 0,    0,   1, DC_BUTTON,  IFWRITE, 0, pCANVar );
                CModelVar  CC_SetMinus     ("CruiseCtrlSetMinus","", 0,    0,   1, DC_BUTTON,  IFWRITE, 0, pCANVar );
                CModelVar  CC_ResPlus      ("CruiseCtrlResPlus", "", 0,    0,   1, DC_BUTTON,  IFWRITE, 0, pCANVar );
                CModelVar  CC_UseButtons   ("CC Use Buttons",    "", 1,    0,   1, DC_DIGITAL, IFWRITE, 0, pCANVar );
        CTxMessage  ABS_450_TX("ABS_450_TX", ABS_450_Tx_TrigFn, "ABS_450_TX(0x450)",pCANVar ,&HIL_TX);
                CModelVar  BrakePressure   ("BrakePressure",     "", 0,    0, 128, DC_DIGITAL, IFWRITE, 0, pCANVar );
                CModelVar  BrakeSwStatus   ("BrakeSwStatus",     "", 1,    0,   3, DC_DIGITAL, IFWRITE, 0, pCANVar );
        CTxMessage  ABS_451_TX("ABS_451_TX", ABS_451_Tx_TrigFn, "ABS_451_TX(0x450)",pCANVar ,&HIL_TX);
                CModelVar  WheelSpdFront   ("WheelSpeedFront",   "", 0,    0, 500, DC_DIGITAL, IFREAD,  0, pCANVar );
                CModelVar  WSFrontStatus   ("WSFrontStatus",     "", 1,    0,   3, DC_DIGITAL, IFWRITE, 0, pCANVar );
                CModelVar  WSRearStatus    ("WSRearStatus",      "", 1,    0,   3, DC_DIGITAL, IFWRITE, 0, pCANVar );
                CModelVar  WSDiffFminusR   ("WSDiffFminusR",     "", 0, -500, 500, DC_DIGITAL, IFWRITE, 0, pCANVar );


// Periodic message timer
int hmi_120_tx_timer = 0;
int abs_450_tx_timer = 0;
int abs_451_tx_timer = 0;


void SC_ModelIterate(void)
{

	// Periodically transmit HMI 120 message
    hmi_120_tx_timer += 1;

    if (hmi_120_tx_timer >= HMI_120_TX_PERIOD)
    {
        HMI_120_TX.SetTrigger();
        hmi_120_tx_timer = 0;
    }

	HMI_120_TX_TIMER = hmi_120_tx_timer;

	// Periodically transmit ABS 450 message
    abs_450_tx_timer += 1;

    if (abs_450_tx_timer >= ABS_450_TX_PERIOD)
    {
        ABS_450_TX.SetTrigger();
        abs_450_tx_timer = 0;
    }

	ABS_450_TX_TIMER = abs_450_tx_timer;

	// Periodically transmit ABS 451 message
    abs_451_tx_timer += 1;

    if (abs_451_tx_timer >= ABS_451_TX_PERIOD)
    {
        ABS_451_TX.SetTrigger();
        abs_451_tx_timer = 0;
    }

	ABS_451_TX_TIMER = abs_451_tx_timer;

}

void CANdbModelLoad(void)
{
    DECODER_PARAM DecoderParam;

    _MFXSetCANXTDMode(CAN_1, true);
    DecoderParam.pFn = DecodeEMS_160_Rx;
    DecoderParam.pRx = &EMS_160_RX;
    g_RXMsgDecoderMap.insert(map<unsigned long, DECODER_PARAM>::value_type(0x160, DecoderParam));
}

void CANdbModelProcessMessage(ProtocolIDs ProtocolID,unsigned long MessageID,unsigned char* pMessageData,int  MessageLength,CRxMessage*& RxMessage,unsigned short& nUpdateCount)
{
    if  (ProtocolID == CAN_1 || ProtocolID == CAN_2)
    {
        map <unsigned long, DECODER_PARAM>::iterator it;
        it = g_RXMsgDecoderMap.find(MessageID);
        if(it != g_RXMsgDecoderMap.end())
        {
            MSGDECODER_FN MsgDecoderFn =  (*it).second.pFn;
            MsgDecoderFn(pMessageData, MessageLength);
            RxMessage = (*it).second.pRx;
        }
    }

}


// trigger function definitions

void HMI_120_Tx_TrigFn(const CTriggerVar* /*pTrigger*/)
{
    HRESULT res;
    BYTE TXMessage[8];
    memset(TXMessage,0,8);
	BYTE varCC_OnOff;
    BYTE varCC_SM;
    BYTE varCC_RP;

	if (CC_UseButtons > 0)
	{
		varCC_OnOff = CC_OnOff;
	    varCC_SM = CC_SetMinus;
		varCC_RP = CC_ResPlus;
	}
	else
	{
		varCC_OnOff = CC_OnOffState;
	    varCC_SM = CC_SetMinusState;
		varCC_RP = CC_ResPlusState;
	}

    TXMessage[1] = varCC_OnOff << 6;
    TXMessage[3] = (varCC_SM << 4) + (varCC_RP << 6);

    res = _MFXTransmitMessage(CAN_1, 0x120, TXMessage, 8, &HMI_120_TX, 1);

    if (FAILED(res))
    {
        if (HRESULT_CODE(res) == CAN_ERR_TX_BUFFER_FULL)
        {
            _MFXPutMessage(PI_MSG_USER_INFO, " HMI_120_TX Message Error! CAN_1 transmit buffer is full");
        }
        else
        {
            _MFXPutMessage(PI_MSG_USER_INFO, " HMI_120_TX Message Error! Don't know what went wrong");
        }
    }
}

void ABS_450_Tx_TrigFn(const CTriggerVar* /*pTrigger*/)
{
    HRESULT res;
    BYTE TXMessage[8];
    memset(TXMessage,0,8);

    BYTE varBrakeSwStatus = BrakeSwStatus;
    TXMessage[1] = varBrakeSwStatus << 6;

    double varBrakePressure = BrakePressure;
	if (varBrakePressure > 127.0)
	{
		varBrakePressure = 255.0;
	}
	else
	{
        varBrakePressure = varBrakePressure * 2.0;
	}
    TXMessage[2] = (BYTE)(varBrakePressure);

    res = _MFXTransmitMessage(CAN_1, 0x450, TXMessage, 8, &ABS_450_TX, 1);

    if (FAILED(res))
    {
        if (HRESULT_CODE(res) == CAN_ERR_TX_BUFFER_FULL)
        {
            _MFXPutMessage(PI_MSG_USER_INFO, " ABS_450_TX Message Error! CAN_1 transmit buffer is full");
        }
        else
        {
            _MFXPutMessage(PI_MSG_USER_INFO, " ABS_450_TX Message Error! Don't know what went wrong");
        }
    }
}

void ABS_451_Tx_TrigFn(const CTriggerVar* /*pTrigger*/)
{
    HRESULT res;
    BYTE TXMessage[8];
    memset(TXMessage,0,8);

    double varWheelSpdF = WheelSpdFront;
	double varWSDelta   = WSDiffFminusR;
    BYTE varWSFStatus = WSFrontStatus;
    BYTE varWSRStatus = WSRearStatus;

	// Calculate rear wheel speed
    double varWheelSpdR = varWheelSpdF - varWSDelta;

	// Clip to range
	if (varWheelSpdR < 0.0)
	{
		varWheelSpdR = 0.0;
	}
	if (varWheelSpdR > 500.0)
	{
		varWheelSpdR = 500.0;
	}

	// Convert to raw
	if (varWheelSpdF > 460.0)
	{
	    // DBC file has max 460, so treat anything above that as invalid
		varWheelSpdF = 8191.0;
	}
	else
	{
	    varWheelSpdF = varWheelSpdF * 160.0 / 9.0;
	}

	if (varWheelSpdR > 460.0)
	{
		varWheelSpdR = 8191.0;
	}
	else
	{
	    varWheelSpdR = varWheelSpdR * 160.0 / 9.0;
	}
	unsigned int wsf = (int)varWheelSpdF;
	unsigned int wsr = (int)varWheelSpdR;

    TXMessage[1] = (BYTE)((wsr & 0x000F) << 4);
    TXMessage[2] = (BYTE)((wsr & 0x0FF0) >> 4);
    TXMessage[3] = (BYTE)((wsr & 0x1000) >> 12) + (BYTE)((wsf & 0x7F) << 1);
    TXMessage[4] = (BYTE)((wsf & 0x1F80) >> 7)  + (BYTE)(varWSRStatus << 6);
    TXMessage[5] = varWSFStatus;

    res = _MFXTransmitMessage(CAN_1, 0x451, TXMessage, 8, &ABS_451_TX, 1);

    if (FAILED(res))
    {
        if (HRESULT_CODE(res) == CAN_ERR_TX_BUFFER_FULL)
        {
            _MFXPutMessage(PI_MSG_USER_INFO, " ABS_451_TX Message Error! CAN_1 transmit buffer is full");
        }
        else
        {
            _MFXPutMessage(PI_MSG_USER_INFO, " ABS_451_TX Message Error! Don't know what went wrong");
        }
    }
}

void DecodeEMS_160_Rx(BYTE* RxMessage, int & MessageLength)
{
    BYTE RXMessage[8];
    memcpy(RXMessage, RxMessage, MessageLength);

    CC_Status = (RXMessage[2] >> 1) & 7;
    CC_DeactReason = RXMessage[1] >> 4;

}
