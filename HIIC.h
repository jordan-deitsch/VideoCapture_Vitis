/*
 * Copyright 2024 Olympus Veran Technologies.
 * Do not reproduce without written permission.
 * All rights reserved.
 *
 * Created by Jordan Deitsch.
 */

#ifndef HI2C_H
#define HI2C_H

#include "xparameters.h"
#include "xiic.h"
#include "xintc.h"


#define NUM_IIC_DRIVERS			(1)
#define MAX_IIC_PERIPHERALS 	(1)
#define IIC_TIMEOUT_PER_BYTE	(5000)
#define IIC_DEFAULT_STATUS_REG	(0x00C0)
#define IIC_MAX_TIMEOUT			(3)
#define IIC_TIMEOUT_ERROR		(2)		// IIC timeout error code
#define IIC_AXI_CLOCK_PERIOD	(8)		// period of AXI clock in nsec
#define IIC_TIMEOUT_ERROR		(2)		// IIC timeout error code
#define IIC_THDDAT_DEFAULT_NSEC	(1500)	// default Data Hold Time in nsec

// Device IDs for AXI-IIC devices
#define IIC_HDMI_DEVICE_ADDR	(XPAR_AXI_IIC_HDMI_BASEADDR)

// Interrupt IDs for AXI-IIC devices
#define IIC_HDMI_INTR_ID 		(XPAR_FABRIC_AXI_IIC_HDMI_INTR)

#define IIC_HDMI_PERIPHERALS	(1)


#define IIC_REG_TSUSTA 	(0x128)
#define IIC_REG_TSUSTO	(0x12C)
#define IIC_REG_THDSTA	(0x130)
#define IIC_REG_TSUDAT	(0x134)
#define IIC_REG_TBUF	(0x138)
#define IIC_REG_THIGH	(0x13C)
#define IIC_REG_TLOW	(0x140)
#define IIC_REG_THDDAT	(0x144)

typedef enum
{
	Iic_Write_Start = 0x1,
	Iic_Write_MasterSend,
	Iic_Write_Transmit,
	Iic_Write_Stop,
	Iic_Read_Start = 0x10,
	Iic_Read_MasterSend,
	Iic_Read_Transmit,
	Iic_Read_MasterRecv,
	Iic_Read_Receive,
	Iic_Read_Stop
} IicDebugMessages;

typedef struct IicBus
{
	u32 DeviceAddr;			// Device ID of IIC bus controller
	u8 IntrId;				// Interrupt ID of IIC controller
	u8 NumPeriphs;			// Number of peripheral devices connected to IIC bus
	XIic* IicInstPtr;		// Pointer to IIC device instance
} IicBus;


extern	IicBus	IicBusInstHdmi;
extern	IicBus	*IicBusList[];


// Interrupt Handler functions
void HIIC_SendHandler(XIic *XIicPtr, int Event);
void HIIC_ReceiveHandler(XIic *XIicPtr, int Event);
void HIIC_StatusHandler(XIic *XIicPtr, int Event);

// Standard Operation functions
int HIIC_Setup();
int HIIC_Init(IicBus *IicBusPtr,
			u32 IicDeviceAddr,
			u8 IntrId,
			u8 NumPeriphs);
int HIIC_WriteData(IicBus *IicBusPtr,
			u8 IicSlaveAddr,
			u8 *TxMsgPtr,
			int ByteCount,
			u32 DataHoldNsec);
int HIIC_ReadData(IicBus *IicBusPtr,
			u8 IicSlaveAddr,
			u8 ReadRegAddr,
			u8 *RxMsgPtr,
			int ByteCount,
			u32 DataHoldNsec);

void HIIC_ClearList();

#endif