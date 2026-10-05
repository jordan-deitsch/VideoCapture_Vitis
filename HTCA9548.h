/*
 * Copyright 2024 Olympus Veran Technologies.
 * Do not reproduce without written permission.
 * All rights reserved.
 *
 * Created by Jordan Deitsch.
 */

#ifndef HTCA9548_H
#define HTCA9548_H


/***************************** Include Files *********************************/
#include "HIIC.h"
#include "sleep.h"
#include "xil_types.h"
#include "xil_printf.h"
#include <stdbool.h>
#include <xstatus.h>


/************************** Constant Definitions *****************************/
#define TCA9548_CLOCKS_DEVICE_ADDR  (0x74)  // 0x74 = 7'b111_0100 [1, 1, 1, 0, A2, A1, A0]
#define TCA9548_SFP_DEVICE_ADDR     (0x75)  // 0x75 = 7'b111_0101 [1, 1, 1, 0, A2, A1, A0]
#define TCA9548_THDDAT_TIME_NSEC    (500)
#define TCA9548_SWITCH_DELAY_USEC   (100)
#define TXA9548_MAX_OUTPUTS         (8)     // Max of 8 switch outputs

typedef enum
{
    e_Switch_CLK_EEPROM,
    e_Switch_CLK_NC_1,
    e_Switch_CLK_NC_2,
    e_Switch_CLK_SI570,
    e_Switch_CLK_SI5328,
    e_Switch_CLK_PMBUS,
    e_Switch_CLK_NC_6,
    e_Switch_CLK_NC_7,
}TCA9548SwitchPosition_Clocks;

typedef enum
{
    e_Switch_SFP_FMC_HPC0,
    e_Switch_SFP_PMBUS,
    e_Switch_SFP_NC_2,
    e_Switch_SFP_NC_3,
    e_Switch_SFP_SFP3,
    e_Switch_SFP_SFP2,
    e_Switch_SFP_SFP1,
    e_Switch_SFP_SFP0,
}TCA9548SwitchPosition_SFP;

typedef struct TCA9548Device
{
	IicBus *IicBusPtr; 	// Pointer to I2C bus
	u8 Address; 		// Device address
    bool IicPresent;
    int SwitchPosition;
} TCA9548Device;


/************************** Peripheral Device Declarations *****************************/
extern TCA9548Device TCA9548Inst_Clocks;
extern TCA9548Device TCA9548Inst_SFP;

/************************** Function Declarations *****************************/
int HTCA9548_Setup();
int HTCA9548_SwitchSel(TCA9548Device *TCA9548InstPtr, int SwitchPos);


#endif