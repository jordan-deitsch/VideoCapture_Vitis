/*
 * Copyright 2024 Olympus Veran Technologies.
 * Do not reproduce without written permission.
 * All rights reserved.
 *
 * Created by Jordan Deitsch.
 */

#ifndef HSI570_H
#define HSI570_H


/***************************** Include Files *********************************/
#include "HIIC.h"
#include "sleep.h"
#include "xil_types.h"
#include <stdbool.h>


/************************** Constant Definitions *****************************/
#define SI570_DEVICE_ADDR       (0xBA)  // 0xBA = 8'b1011_1010

#define SI570_THDDAT_TIME_NSEC  (500)

// SI570 Registers
#define HDMI_MAIN_INTR_STATUS_REG       (0x96)
    #define INTR_STATUS_HPD_MASK        (0x80)
    #define INTR_STATUS_MON_SENSE_MASK  (0x40)
    #define INTR_STATUS_EDID_READY_MASK (0x04)

typedef struct SI570Device
{
	IicBus *IicBusPtr; 	// Pointer to I2C bus
	u8 Address; 		// Device address
    bool IicPresent;
} SI570Device;


/************************** Peripheral Device Declarations *****************************/
extern SI570Device SI570Inst;

/************************** Function Declarations *****************************/
int HSI570_Init(SI570Device *SI570InstPtr, IicBus *I2cBusPtr, u8 Address);


#endif