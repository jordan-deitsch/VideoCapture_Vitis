/*
 * Copyright 2024 Olympus Veran Technologies.
 * Do not reproduce without written permission.
 * All rights reserved.
 *
 * Created by Jordan Deitsch.
 */

#ifndef HHDMI_H
#define HHDMI_H


/***************************** Include Files *********************************/
#include "HIIC.h"
#include "HINTC.h"
#include "sleep.h"
#include "xil_types.h"
#include <stdbool.h>


/************************** Constant Definitions *****************************/
#define HDMI_MAIN_ADDR		            (0x72)  // 0x72 id PD = LOW, 0x7A if PD = HIGH
#define HDMI_EDID_ADDR                  (0x7E)  // Default, can be programmed as needed
#define HDMI_EDID_NUM_BYTES             (256)

#define HDMI_INTR_ID                    (2)
#define HDMI_THDDAT_TIME_NSEC           (500)
#define HDMI_DELAY_MSEC                 (200)
#define HDMI_MONSENSE_TIMEOUT_COUNTER   (20)
#define HDMI_EDID_TIMEOUT_COUNTER       (50)
#define HDMI_INTR_TIMEOUT_MSEC          (10)

// ADV7511 Main Map Registers
#define HDMI_MAIN_CHIP_REV_REG          (0x00)

#define HDMI_MAIN_POWER_REG             (0x41)
    #define POWER_DOWN_MASK             (0x40)

#define HDMI_MAIN_HPD_STATE_REG         (0x42)
    #define HPD_STATE_MASK              (0x40)
    #define MONITOR_STATE_MASK          (0x20)

#define HDMI_MAIN_INTR_CTRL_REG         (0x94)
    #define INTR_CTRL_HPD_MASK          (0x80)
    #define INTR_CTRL_MON_SENSE_MASK    (0x40)
    #define INTR_CTRL_EDID_READY_MASK   (0x04)

#define HDMI_MAIN_INTR_STATUS_REG       (0x96)
    #define INTR_STATUS_HPD_MASK        (0x80)
    #define INTR_STATUS_MON_SENSE_MASK  (0x40)
    #define INTR_STATUS_EDID_READY_MASK (0x04)

#define HDMI_MAIN_HPD_CTRL_REG          (0xD6)


typedef enum{
    e_Intr_Hpd = 0,
    e_Intr_MonSense,
    e_Intr_EdidReady,
    e_NumInterrupts
}InterruptId;

typedef enum{
    e_800x600x60Hz, // Byte 35, bit 0-7
    e_800x600x56Hz,
    e_640x480x75Hz,
    e_640x480x72Hz,
    e_640x480x67Hz,
    e_640x480x60Hz,
    e_720x400x88Hz,
    e_720x400x70Hz,
    
    e_1280x1024x75Hz, // Byte 36, bit 0-7
    e_1024x768x75Hz,
    e_1024x768x70Hz,
    e_1024x768x60Hz,
    e_1024x768x87Hz_i,
    e_832x624x75Hz,
    e_800x600x75Hz,
    e_800x600x72Hz,

    e_1152x870x75   // Byte 37, bit 7
}TimingMode;

typedef struct{
    u8 IsDigital;
    u8 BitDepth;
    u8 VideoInterface;
    u8 HScreenSize;
    u8 VScreenSize;
    u8 DisplayGamma;
    u8 DisplayType;
    u8 TimingBitmap[3];
    u16 Resolution;
    u8 AspectRatio;
    u8 VerticalFreq;
}DisplayParams;

typedef struct ADV7511Device
{
	IicBus *IicBusPtr; 	// Pointer to I2C bus
	u8 Address; 		// Device address
    u8 EdidAddress;     // EDID address
    bool IicPresent;
    bool DisplayPresent;
    u8 EdidData[HDMI_EDID_NUM_BYTES];
    DisplayParams MonitorParams;
} ADV7511Device;


/************************** Peripheral Device Declarations *****************************/
extern ADV7511Device HdmiInst;
extern u32 HDMI_INTERRUPT_VECTOR;

/************************** Function Declarations *****************************/
int HHDMI_Init(ADV7511Device *HdmiInstPtr, IicBus *I2cBusPtr, u8 Address);
int HHDMI_ConnectionEvent(ADV7511Device *HdmiInstPtr);

void HDMI_IntrHandler(void *CallBackRef);



#endif