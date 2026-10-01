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
#include <stdbool.h>


/************************** Constant Definitions *****************************/
#define TCA9548_DEVICE_ADDR       (0xE8)  // 0xE8 = 8'b1110_1000
#define TCA9548_THDDAT_TIME_NSEC  (500)

typedef struct TCA9548Device
{
	IicBus *IicBusPtr; 	// Pointer to I2C bus
	u8 Address; 		// Device address
    bool IicPresent;
    u8 SwitchPosition;
} TCA9548Device;


/************************** Peripheral Device Declarations *****************************/
extern TCA9548Device TCA9548Inst;

/************************** Function Declarations *****************************/
int HTCA9548_Init(TCA9548Device *TCA9548InstPtr, IicBus *I2cBusPtr, u8 Address);
int HTCA9548_SwitchSel(TCA9548Device *TCA9548InstPtr, u8 SwitchSel);


#endif