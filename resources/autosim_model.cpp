//****************************************************************************
//
//  Project:        AutoSimETS
//  File:           autosim_model.cpp
//  Version:        
//  Description:    Boilerplate Source File for MFX Closed Loop Model
//  Author:         
//
//  Copyright (C) 2005 Pi Technology 
//  This document (program) contains proprietary information
//  which is the property of Pi Technology               
//  The contents of this document (program) must not be copied
//  or disclosed to a third party without the prior agreement 
//  of an authorised officer of Pi Technology              
//
//****************************************************************************

//******** Includes *********
#include "stdafx.h"
#include "autosim_model.h"
#include "autosim_SerialComms.h"
#include "viossd.h"
#include "protocols.h"
#include "msgtypes.h"
#include "dllnames.h"
#include "can.h"
#include "j1850.h"
//******** These macros can be changed *********

//#define ASAP3_ENABLE

// Define macros for the model variable pointers.
#define ENGINE_SPEED   (*pENGINE_SPEED)
#define ENGINE_ANGLE   (*pENGINE_ANGLE)
#define TPS_SENSOR1    (*pTPS_SENSOR1)
#define TPS_SENSOR2    (*pTPS_SENSOR2)
#define HGS_SENSOR1    (*pHGS_SENSOR1)
#define HGS_SENSOR2    (*pHGS_SENSOR2)
#define IMAP_SENSOR1   (*pIMAP_SENSOR1)
#define GEAR_SENSOR    (*pGEAR_SENSOR)
#define O2_SENSOR1     (*pO2_SENSOR1)
#define O2_SENSOR2     (*pO2_SENSOR2)

// Define rich-lean states
#define O2_SENSOR_OPEN	(0)
#define O2_SENSOR_LEAN	(1)
#define O2_SENSOR_LTOR	(2)
#define O2_SENSOR_RICH  (3)
#define O2_SENSOR_RTOL  (4)


//******** These macros must not be changed *********

// create linker references to include MFX DLL functions
#pragma comment(linker, "/include:_InitialiseInstance")
#pragma comment(linker, "/include:_StartInstance")
#pragma comment(linker, "/include:_IdleInstance")
#pragma comment(linker, "/include:_StopInstance")
#pragma comment(linker, "/include:_GetNumMFXProperty")
#pragma comment(linker, "/include:_GetMFXProperty")
#pragma comment(linker, "/include:_SetMFXProperty")

//******** Static Structures *********

//******** Static Prototypes *********

//******** Static Variables *********

// Define model variable pointers.
static CModelVar* pENGINE_SPEED;
static CModelVar* pENGINE_ANGLE;
static CModelVar* pTPS_SENSOR1;
static CModelVar* pTPS_SENSOR2;
static CModelVar* pHGS_SENSOR1;
static CModelVar* pHGS_SENSOR2;
static CModelVar* pIMAP_SENSOR1;
static CModelVar* pGEAR_SENSOR;
static CModelVar* pO2_SENSOR1;
static CModelVar* pO2_SENSOR2;


//******** Global Variables *********

// this is the default setting for the closed loop model frame time
DWORD gdwMFXCLFrameTime = 1000UL;

// this is the default setting for model variables in use
BOOL gbMFXCLInUse = TRUE;

// this is the default setting for a dummy rack configuration
BOOL gbMFXCLNoRack = FALSE;
SSlotConfiguration gMFXCLDummyRackConfiguration[MAX_PISA_SLOTS];

// gpIOS_Inputs is the pointer to the IOS hardware input region
__declspec(dllimport) SimInputs* gpIOS_Inputs;

// gpIOS_Outputs is the pointer to the IOS hardware output region
__declspec(dllimport) SimOutputs* gpIOS_Outputs;

// gpIOS_TimingData is the pointer to the IOS timing data
__declspec(dllimport) IOSSDumDataTag* gpIOS_TimingData;

// gIOS_Initialisation is the pointer to the IOS hardware initialisation region
__declspec(dllimport) SInitialisation gIOS_Initialisation;

CModelVar imap_angle          ("IMAP sample point",           "deg",   530,  1,  720, DC_DIGITAL, IFWRITE);
CModelVar imap_min_volt       ("IMAP min voltage",            "V",       1,  0,    5, DC_DIGITAL, IFWRITE);
CModelVar imap_max_volt       ("IMAP max voltage",            "V",       4,  0,    5, DC_DIGITAL, IFWRITE);
CModelVar tps_volt            ("Throttle pos voltage",        "V",     0.5,  0,    5, DC_DIGITAL, IFWRITE);
CModelVar hgs_volt            ("Twist grip voltage",          "V",     0.5,  0,    5, DC_DIGITAL, IFWRITE);
CModelVar o2sens1_active      ("LAMBDAV1 activated",          "N/A",     0,  0,    1, DC_BUTTON,  IFWRITE);
CModelVar o2sens1_min_volt    ("LAMBDAV1 min voltage",        "V",   0.05f,  0,    1, DC_DIGITAL, IFWRITE);
CModelVar o2sens1_max_volt    ("LAMBDAV1 max voltage",        "V",   0.95f,  0,    1, DC_DIGITAL, IFWRITE);
CModelVar o2sens1_rich_time   ("LAMBDAV1 rich time",          "msec",  200,  0, 5000, DC_DIGITAL, IFWRITE);
CModelVar o2sens1_lean_time   ("LAMBDAV1 lean time",          "msec",  200,  0, 5000, DC_DIGITAL, IFWRITE);
CModelVar o2sens1_lrsw_time   ("LAMBDAV1 lr switch time",     "msec",   50,  0, 1000, DC_DIGITAL, IFWRITE);
CModelVar o2sens1_rlsw_time   ("LAMBDAV1 rl switch time",     "msec",   50,  0, 1000, DC_DIGITAL, IFWRITE);
CModelVar o2sens2_active      ("LAMBDAV2 activated",          "N/A",     0,  0,    1, DC_BUTTON,  IFWRITE);
CModelVar o2sens2_min_volt    ("LAMBDAV2 min voltage",        "V",   0.05f,  0,    1, DC_DIGITAL, IFWRITE);
CModelVar o2sens2_max_volt    ("LAMBDAV2 max voltage",        "V",   0.95f,  0,    1, DC_DIGITAL, IFWRITE);
CModelVar o2sens2_rich_time   ("LAMBDAV2 rich time",          "msec",  200,  0, 5000, DC_DIGITAL, IFWRITE);
CModelVar o2sens2_lean_time   ("LAMBDAV2 lean time",          "msec",  200,  0, 5000, DC_DIGITAL, IFWRITE);
CModelVar o2sens2_lrsw_time   ("LAMBDAV2 lr switch time",     "msec",   50,  0, 1000, DC_DIGITAL, IFWRITE);
CModelVar o2sens2_rlsw_time   ("LAMBDAV2 rl switch time",     "msec",   50,  0, 1000, DC_DIGITAL, IFWRITE);
CModelVar gear_number         ("GEAR number",                 "abs",     0,  0,    6, DC_DIGITAL, IFWRITE);
CModelVar gear_override       ("GEAR override",               "N/A",     0,  0,    1, DC_BUTTON,  IFWRITE);
CModelVar gear_override_val   ("GEAR override value",         "V",    0.25,  0,    5, DC_DIGITAL, IFWRITE);
CModelVar gear_1_ratio        ("GEAR 1 ratio",                "abs", 12.00,  0,   20, DC_DIGITAL, IFWRITE);
CModelVar gear_2_ratio        ("GEAR 2 ratio",                "abs",  9.00,  0,   20, DC_DIGITAL, IFWRITE);
CModelVar gear_3_ratio        ("GEAR 3 ratio",                "abs",  8.00,  0,   20, DC_DIGITAL, IFWRITE);
CModelVar gear_4_ratio        ("GEAR 4 ratio",                "abs",  7.00,  0,   20, DC_DIGITAL, IFWRITE);
CModelVar gear_5_ratio        ("GEAR 5 ratio",                "abs",  6.00,  0,   20, DC_DIGITAL, IFWRITE);
CModelVar gear_6_ratio        ("GEAR 6 ratio",                "abs",  5.00,  0,   20, DC_DIGITAL, IFWRITE);
CModelVar gear_ratio          ("GEAR RATIO",                  "abs",  5.00,  0,   20, DC_DIGITAL, IFREAD);
CModelVar neutral_voltage     ("NEUTRAL volts",               "V",    0.25,  0,    5, DC_DIGITAL, IFWRITE);
CModelVar gear_1_voltage      ("GEAR 1 volts",                "V",    1.00,  0,    5, DC_DIGITAL, IFWRITE);
CModelVar gear_2_voltage      ("GEAR 2 volts",                "V",    1.75,  0,    5, DC_DIGITAL, IFWRITE);
CModelVar gear_3_voltage      ("GEAR 3 volts",                "V",    2.50,  0,    5, DC_DIGITAL, IFWRITE);
CModelVar gear_4_voltage      ("GEAR 4 volts",                "V",    3.25,  0,    5, DC_DIGITAL, IFWRITE);
CModelVar gear_5_voltage      ("GEAR 5 volts",                "V",    4.00,  0,    5, DC_DIGITAL, IFWRITE);
CModelVar gear_6_voltage      ("GEAR 6 volts",                "V",    4.75,  0,    5, DC_DIGITAL, IFWRITE);
CModelVar tyre_circumference  ("TYRE circumference",          "mm",   2000,  0, 5000, DC_DIGITAL, IFWRITE);
CModelVar veh_spd_override    ("VEHSPD override",             "N/A",     0,  0,    1, DC_BUTTON,  IFWRITE);
CModelVar veh_spd_override_val("VEHSPD override val",         "km/h", 0.00,  0,  500, DC_DIGITAL, IFWRITE);
CModelVar vehicle_speed       ("VEHICLE SPEED",               "km/h", 0.00,  0,  500, DC_DIGITAL, IFREAD);



//******** Global Procedures *********

//****************************************************************************
//  Function:       InitialiseDummyRack
//  Purpose:        Get the model into a clean state
//  Returns:        nothing
//  Pre-condition:  
//  Post-condition:                        
//  Notes:          This is called once before the model is started.
//****************************************************************************
void InitialiseDummyRack(void)
{
    // firstly, define an empty rack
    for (int i = 0; i < MAX_PISA_SLOTS; i++)
    {
        gMFXCLDummyRackConfiguration[i].BoardID = EUnknownBoardID;
    }
}

//****************************************************************************
//  Function:       Model Load
//  Purpose:        Get the model into a clean state
//  Returns:        None
//  Pre-condition:  None
//  Post-condition: None
//  Notes:          This is called once after the model is loaded
//****************************************************************************
void    ModelLoad(void)
{
#ifdef ASAP3_ENABLE
    // Call special ModelLoad function
    if (ASAP3ModelLoad())
    {
        // Special function indicated not to run normal ModelLoad code
        return;
    }
#endif


    // This code must be supplied for each model.

    // Initialise the model variable pointers.
	pENGINE_SPEED = CModelVar::FindModelVar("S15-PWG:CHAN0:RPM");
	pENGINE_ANGLE = CModelVar::FindModelVar("S13-EVT:D3:CHAN31[POSITION]:START");
	pTPS_SENSOR1  = CModelVar::FindModelVar("S04-AOT:D0V:CHAN00");
	pTPS_SENSOR2  = CModelVar::FindModelVar("S04-AOT:D0V:CHAN01");
	pHGS_SENSOR1  = CModelVar::FindModelVar("S04-AOT:D0V:CHAN02");
	pHGS_SENSOR2  = CModelVar::FindModelVar("S04-AOT:D0V:CHAN03");
	pIMAP_SENSOR1 = CModelVar::FindModelVar("S04-AOT:D1V:CHAN04");
	pGEAR_SENSOR  = CModelVar::FindModelVar("S04-AOT:D1V:CHAN06");
	pO2_SENSOR1   = CModelVar::FindModelVar("S05-AOT:D3V:CHAN13");
	pO2_SENSOR2   = CModelVar::FindModelVar("S05-AOT:D3V:CHAN14");

    CANdbModelLoad();
}

//****************************************************************************
//  Function:       ModelStart
//  Purpose:        Get the model into a clean state
//  Returns:        nothing
//  Pre-condition:  
//  Post-condition:                        
//  Notes:          This is called once before the model is started.
//****************************************************************************
void    ModelStart(void)
{
    // This code must be supplied for each model.
	// Initialise any file-scope variables here.
}

//****************************************************************************
//  Function:       ModelIterate
//  Purpose:        Runs an iteration of the model
//  Returns:        nothing
//  Pre-condition:
//  Post-condition:                        
//  Notes:          This is called every model timer tick.
//****************************************************************************
void    ModelIterate(void)
{
	static int o2sens1_state = O2_SENSOR_OPEN;
	static int o2sens2_state = O2_SENSOR_OPEN;
    static int o2sens1_count = 0;
    static int o2sens2_count = 0;

	/* For non-zero engine speed, simulate IMAP sensor reading
	 * with an approximate sine wave across the engine cycle,
	 * with minimum imap_min_volt at imap_angle, and maximum
	 * imap_max_volt 360 degrees before and after. Thus when
	 * imap_angle matches the ECU map sampling point, the ECU
	 * reading of MAP1 should be imap_min_volt and its reading
	 * of MAP4COMP should be imap_max_volt. When engine speed
	 * is negligible, the output is set to imap_max_volt, which
	 * should map onto barometric air pressure.
	 */
	if (ENGINE_SPEED < 0.1)
	{
		IMAP_SENSOR1 = imap_max_volt;
	}
	else
	{
		double output = 0;
		double amplitude   = (imap_max_volt - imap_min_volt) / 2;
		double zero_offset = (imap_max_volt + imap_min_volt) / 2;
		int relative_angle = ((ENGINE_ANGLE + 180) - imap_angle);

		relative_angle = relative_angle % 720;
		if (relative_angle < 0)   relative_angle += 720;

		// Approximate sin(2x) using Bhaskara I formula
		if (relative_angle < 360)
		{
			output = zero_offset -
				((amplitude * 4 * relative_angle * (360 - relative_angle)) / 
				 (162000 - (relative_angle * (360 - relative_angle))));
		}
		else
		{
			relative_angle = 720 - relative_angle;
			output = zero_offset +
				((amplitude * 4 * relative_angle * (360 - relative_angle)) / 
				 (162000 - (relative_angle * (360 - relative_angle))));
		}

		IMAP_SENSOR1 = output;
	}

	TPS_SENSOR1 = tps_volt;
	TPS_SENSOR2 = 5 - tps_volt;
	HGS_SENSOR1 = hgs_volt;
	/* For TVS, we have second sensor voltage expected to be half of first sensor */
	HGS_SENSOR2 = hgs_volt/2;

	if (o2sens1_active > 0)
	{
		if (o2sens1_state == O2_SENSOR_OPEN)
		{
			o2sens1_state = O2_SENSOR_LEAN;
			o2sens1_count = 0;
			O2_SENSOR1 = o2sens1_min_volt;
		}
		else if (o2sens1_state == O2_SENSOR_LEAN)
		{
			O2_SENSOR1 = o2sens1_min_volt;
			o2sens1_count++;
			if (o2sens1_count >= o2sens1_lean_time)
			{
				o2sens1_state = O2_SENSOR_LTOR;
				o2sens1_count = 0;
			}
		}
		else if (o2sens1_state == O2_SENSOR_LTOR)
		{
			o2sens1_count++;
			O2_SENSOR1 = ((o2sens1_min_volt * (o2sens1_lrsw_time - o2sens1_count))
				       +  (o2sens1_max_volt *  o2sens1_count)) / o2sens1_lrsw_time;

			if (o2sens1_count >= o2sens1_lrsw_time)
			{
				o2sens1_state = O2_SENSOR_RICH;
				o2sens1_count = 0;
			}
		}
		else if (o2sens1_state == O2_SENSOR_RICH)
		{
			O2_SENSOR1 = o2sens1_max_volt;
			o2sens1_count++;
			if (o2sens1_count >= o2sens1_rich_time)
			{
				o2sens1_state = O2_SENSOR_RTOL;
				o2sens1_count = 0;
			}
		}
		else
		{
			o2sens1_count++;
			O2_SENSOR1 = ((o2sens1_max_volt * (o2sens1_rlsw_time - o2sens1_count))
				       +  (o2sens1_min_volt *  o2sens1_count)) / o2sens1_rlsw_time;

			if (o2sens1_count >= o2sens1_rlsw_time)
			{
				o2sens1_state = O2_SENSOR_LEAN;
				o2sens1_count = 0;
			}
		}
	}
	else
	{
		o2sens1_state = O2_SENSOR_OPEN;
		// Leave O2_SENSOR1 value unwritten
		o2sens1_count = 0;
	}

	if (o2sens2_active > 0)
	{
		if (o2sens2_state == O2_SENSOR_OPEN)
		{
			o2sens2_state = O2_SENSOR_LEAN;
			o2sens2_count = 0;
			O2_SENSOR2 = o2sens2_min_volt;
		}
		else if (o2sens2_state == O2_SENSOR_LEAN)
		{
			O2_SENSOR2 = o2sens2_min_volt;
			o2sens2_count++;
			if (o2sens2_count >= o2sens2_lean_time)
			{
				o2sens2_state = O2_SENSOR_LTOR;
				o2sens2_count = 0;
			}
		}
		else if (o2sens2_state == O2_SENSOR_LTOR)
		{
			o2sens2_count++;
			O2_SENSOR2 = ((o2sens2_min_volt * (o2sens2_lrsw_time - o2sens2_count))
				       +  (o2sens2_max_volt *  o2sens2_count)) / o2sens2_lrsw_time;

			if (o2sens2_count >= o2sens2_lrsw_time)
			{
				o2sens2_state = O2_SENSOR_RICH;
				o2sens2_count = 0;
			}
		}
		else if (o2sens2_state == O2_SENSOR_RICH)
		{
			O2_SENSOR2 = o2sens2_max_volt;
			o2sens2_count++;
			if (o2sens2_count >= o2sens2_rich_time)
			{
				o2sens2_state = O2_SENSOR_RTOL;
				o2sens2_count = 0;
			}
		}
		else
		{
			o2sens2_count++;
			O2_SENSOR2 = ((o2sens2_max_volt * (o2sens2_rlsw_time - o2sens2_count))
				       +  (o2sens2_min_volt *  o2sens2_count)) / o2sens2_rlsw_time;

			if (o2sens2_count >= o2sens2_rlsw_time)
			{
				o2sens2_state = O2_SENSOR_LEAN;
				o2sens2_count = 0;
			}
		}
	}
	else
	{
		o2sens2_state = O2_SENSOR_OPEN;
		// Leave O2_SENSOR2 value unwritten
		o2sens2_count = 0;
	}

	if (gear_override > 0)
	{
		GEAR_SENSOR = gear_override_val;
		vehicle_speed = veh_spd_override_val;
	}
	else
	{
		BYTE gear_num = (BYTE)gear_number;
		
		switch (gear_num)
		{
			case 0:
			default:
				GEAR_SENSOR = neutral_voltage;
				gear_ratio = 0.0;
				break;
			case 1:
				GEAR_SENSOR = gear_1_voltage;
				gear_ratio = gear_1_ratio;
				break;
			case 2:
				GEAR_SENSOR = gear_2_voltage;
				gear_ratio = gear_2_ratio;
				break;
			case 3:
				GEAR_SENSOR = gear_3_voltage;
				gear_ratio = gear_3_ratio;
				break;
			case 4:
				GEAR_SENSOR = gear_4_voltage;
				gear_ratio = gear_4_ratio;
				break;
			case 5:
				GEAR_SENSOR = gear_5_voltage;
				gear_ratio = gear_5_ratio;
				break;
			case 6:
				GEAR_SENSOR = gear_6_voltage;
				gear_ratio = gear_6_ratio;
				break;
		}

		if (veh_spd_override > 0)
		{
			vehicle_speed = veh_spd_override_val;
		}
		else if (gear_ratio < 1.0)
		{
			vehicle_speed = 0.0;
		}
		else
		{
			vehicle_speed = (tyre_circumference * ENGINE_SPEED * 3.0) / (gear_ratio * 50000.0);
		}
		WheelSpdFront = vehicle_speed;
	}

    SC_ModelIterate();

}

//****************************************************************************
//  Function:       ModelStop
//  Purpose:        Get the model into a clean state
//  Returns:        nothing
//  Pre-condition:  The memory must have been already allocated
//  Post-condition:                        
//  Notes:          This is called when the model is stopped.
//****************************************************************************
void    ModelStop(void)
{
    // This code must be supplied for each model.
}

//****************************************************************************
//  Function:       ModelUnload
//  Purpose:        Get the model into a clean state
//  Returns:        None
//  Pre-condition:  The memory must have been already allocated
//  Post-condition: None                       
//  Notes:          This is called when the model is unloaded.
//****************************************************************************
void    ModelUnload(void)
{
#ifdef ASAP3_ENABLE
    if (ASAP3ModelUnload())
    {
        // Special function indicated not to run normal model unload code
        return;
    }
#endif

    // This code must be supplied for each model.
}

//****************************************************************************
//  Function:       ModelProcessMessage
//  Purpose:        Process received serial comms messages
//  Returns:        None
//  Pre-condition:  None
//  Post-condition: None                       
//  Notes:          This is called each time a serial comms message is received.
//                  Leave function empty if the model does not contain serial
//                  comms.
//****************************************************************************

// Here we disable warnings about unreferenced formal parameters
#pragma warning(disable: 4100)

void    ModelProcessMessage
(
    ProtocolIDs     ProtocolID,     // ID of protocol
    unsigned long   MessageID,      // Message ID - may not apply for this protocol
    unsigned char*  pMessageData,   // Message Data
    int             MessageLength,  // Number of bytes in message data
    CRxMessage*&    RxMessage,      // Output: the rx message that has been identified
    unsigned short& nUpdateCount    // Output: the number to update the rx message count by
)
{
#ifdef ASAP3_ENABLE
    // Call special ModelProcessMessage function
    if (ASAP3ModelProcessMessage(ProtocolID, MessageID, pMessageData, MessageLength, RxMessage, nUpdateCount))
    {
        // Message was processed by special function - assume no further action required
        return;
    }
#endif

    // This code must be supplied if the model supports serial comms.
    CANdbModelProcessMessage(ProtocolID,
                             MessageID,
                             pMessageData,
                             MessageLength,
                             RxMessage,
                             nUpdateCount);

}

// Re-enable the unreferenced formal parameters warning
#pragma warning(default: 4100)

