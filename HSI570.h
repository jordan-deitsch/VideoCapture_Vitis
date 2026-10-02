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
#include "HTCA9548.h"
#include "HIIC.h"
#include "sleep.h"
#include "xil_types.h"
#include "xil_printf.h"
#include <stdbool.h>
#include <xstatus.h>


/************************** Constant Definitions *****************************/
#define SI570_DEVICE_ADDR       (0x5D)
#define SI570_THDDAT_TIME_NSEC  (500)
#define SI570_NUM_CONFIG_REGS   (6)

// SI570 Registers
#define SI570_DIVIDERS_REG              (0x07)
    #define SI570_DIVIDERS_HSDIV_MASK  (0xE0)
    #define SI570_DIVIDERS_N1_6_2_MASK  (0x1F)
    #define SI570_DIVIDERS_HSDIV_OFFSET  (5)
    #define SI570_DIVIDERS_N1_6_2_OFFSET  (2)

#define SI570_RFREQ_37_32_REG           (0x08)
    #define SI570_DIVIDERS_N1_1_0_MASK  (0xC0)
    #define SI570_RFREQ_37_32_MASK      (0x3F)
    #define SI570_DIVIDERS_N1_1_0_OFFSET  (6)

#define SI570_RFREQ_31_24_REG           (0x09)
#define SI570_RFREQ_23_16_REG           (0x0A)
#define SI570_RFREQ_15_8_REG            (0x0B)
#define SI570_RFREQ_7_0_REG             (0x0C)
    #define SI570_RFREQ_31_0_MASK       (0xFF)

#define SI570_RESET_MEM_CTRL_REG        (0x87)  // Register 135
    #define SI570_RESET_REG_MASK        (0x80)
    #define SI570_NEW_FREQ_MASK         (0x40)
    #define SI570_FREEZE_M_MASK         (0x20)
    #define SI570_FREEZE_VCADC_MASK     (0x10)
    #define SI570_RECALL_MASK           (0x01)
    
#define SI570_FREEZE_DCO_REG            (0x89)  // Register 137
    #define SI570_FREEZE_DCO_MASK       (0x10)



typedef struct SI570FreqSettings
{
    float OutputFreq;
    u8 HSDIV;
    u8 N1;
    u64 RFREQ;
}SI570FreqSettings;
typedef struct SI570Device
{
	IicBus *IicBusPtr; 	// Pointer to I2C bus
	u8 Address; 		// Device address
    bool IicPresent;
    TCA9548SwitchPosition SwitchPos;
    SI570FreqSettings *CurrentSettings;
    float FreqXTAL;
} SI570Device;

typedef enum
{
    e_SI570_297_MHz,
    e_SI570_296p7_MHz,
    e_SI570_TOTAL_PRESETS
}SI570PresetFrequencies;


/************************** Peripheral Device Declarations *****************************/
extern SI570Device SI570Inst;

/************************** Function Declarations *****************************/
int HSI570_Init(SI570Device *SI570InstPtr, IicBus *I2cBusPtr, u8 Address);
int HSI570_SetFrequency(SI570Device *SI570InstPtr, SI570PresetFrequencies FreqSel);


#endif