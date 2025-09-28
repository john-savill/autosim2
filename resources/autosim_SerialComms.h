//MODEL_VERSION is 1144.374900

#ifndef autosim_SerialComms_h
#define autosim_SerialComms_h

#ifdef _PIAUTOSIM_RAPID_CDK 
	#include "CtrlDefs.h"
	#include "SimLib.h"
	#include "ProtocolSupport.h"
#else 
	#include "ctrldefs.h"
	#include "CModVar.h"
	#include "ProtocolSupport.h"
	#include "RxMessage.h"
	#include "TxMessage.h"
	#include "Branch.h"
	#include "RefModelVar.h"
	#include "TriggerVar.h"
#endif //_PIAUTOSIM_RAPID_CDK 

extern CProtocolSupport CANBUS;
extern CBranch CAN_Branch;
extern CBranch SNodes_Branch;
extern CBranch NonSNodes_Branch;
extern CBranch NoBranch_TX;

extern CBranch NoBranch_TX;

extern CBranch EMS_Branch;
	extern CBranch EMS_RX;
		extern CRxMessage  EMS_160_RX;
				extern CModelVar CC_Status;
				extern CModelVar CC_DeactReason;

extern CBranch HIL_Branch; //simulated 
	extern CBranch HIL_TX;
		extern CTxMessage  HMI_120_TX;
				extern CModelVar CC_OnOff;
				extern CModelVar CC_SetMinus;
				extern CModelVar CC_ResPlus;
		extern CTxMessage  ABS_450_TX;
				extern CModelVar  BrakePressure;
				extern CModelVar  BrakeSwStatus;
		extern CTxMessage  ABS_451_TX;
				extern CModelVar  WheelSpdFront;
				extern CModelVar  WSDiffFminusR;
				extern CModelVar  WSFrontStatus;
				extern CModelVar  WSRearStatus;



extern void CANdbModelLoad(void);


extern void CANdbModelProcessMessage(ProtocolIDs ProtocolID,unsigned long MessageID,unsigned char* pMessageData,int  MessageLength,CRxMessage*& RxMessage,unsigned short& nUpdateCount);

//Packing functions
void PackData(unsigned __int64 n64bitData,BYTE BitPosition, BYTE nLength, BOOL IsIntel, BYTE *pTxMessage);

extern void SC_ModelIterate(void);

// trigger function declarations
extern void HMI_120_Tx_TrigFn(const CTriggerVar *pTrigger);
extern void ABS_450_Tx_TrigFn(const CTriggerVar *pTrigger);
extern void ABS_451_Tx_TrigFn(const CTriggerVar *pTrigger);
extern void DecodeEMS_160_Rx(BYTE* RxMessage, int & MessageLength);

#endif // autosim_SerialComms__h



//Message Names resolution reference

//=======================================


//Signal Names resolution reference

//=======================================



//Signals that are not fully supported due to CModvar float limitation

//===================================================================
