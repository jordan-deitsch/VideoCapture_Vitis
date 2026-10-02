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
#define TCA9548_DEVICE_ADDR       (0x74)  // 0x74 = 7'b111_0100 [1, 1, 1, 0, A2, A1, A0]
#define TCA9548_THDDAT_TIME_NSEC  (500)
#define TCA9548_SWITCH_DELAY_USEC (100)

typedef enum
{
    e_Switch_EEPROM,
    e_Switch_NC_1,
    e_Switch_NC_2,
    e_Switch_SI570,
    e_Switch_SI5328,
    e_Switch_PMBUS,
    e_Switch_NC_6,
    e_Switch_NC_7,
    e_Switch_DEFAULT    // Set to default all 0 (no output enabled_)
}TCA9548SwitchPosition;

typedef struct TCA9548Device
{
	IicBus *IicBusPtr; 	// Pointer to I2C bus
	u8 Address; 		// Device address
    bool IicPresent;
    TCA9548SwitchPosition SwitchPosition;
} TCA9548Device;


/************************** Peripheral Device Declarations *****************************/
extern TCA9548Device TCA9548Inst;

/************************** Function Declarations *****************************/
int HTCA9548_Init(TCA9548Device *TCA9548InstPtr, IicBus *I2cBusPtr, u8 Address);
int HTCA9548_SwitchSel(TCA9548Device *TCA9548InstPtr, TCA9548SwitchPosition SwitchPos);


#endif