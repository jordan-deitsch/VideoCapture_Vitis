/*
 * Copyright 2024 Olympus Veran Technologies.
 * Do not reproduce without written permission.
 * All rights reserved.
 *
 * Created by Jordan Deitsch.
 */

#include "HIIC.h"
#include <stdio.h>

/************************** Device Instance Definitions *****************************/
IicBus	IicBusInstHdmi;

XIic    XIicList[MAX_IIC_PERIPHERALS];
IicBus 	*IicBusList[NUM_IIC_DRIVERS];
static  int IicListId = 0;

/************************** Variable Definitions *****************************/
static volatile u8 IicTransmitComplete = 0;
static volatile u8 IicReceiveComplete = 0;
static int IicTimeoutNumber = 0;

static u8 IicAddress = 0;

/************************** Static Function Definitions *****************************/
static void HIIC_SetTimingParam(XIic *XIicPtr, u32 RegOffset, u32 TimeNsec);


/*
 * Interrupt handlers for all IIC bus transactions
 */
void HIIC_SendHandler(XIic *XIicPtr, int Event)
{
	IicTransmitComplete = 1;
}


void HIIC_ReceiveHandler(XIic *XIicPtr, int Event)
{
	IicReceiveComplete = 1;
}


void HIIC_StatusHandler(XIic *XIicPtr, int Event)
{
	// Add status handler logic as needed
}

int HIIC_Setup() {
	int Status;

	Status = HIIC_Init(&IicBusInstHdmi, IIC_HDMI_DEVICE_ADDR, IIC_HDMI_INTR_ID, IIC_HDMI_PERIPHERALS);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	IicBusList[0] = &IicBusInstHdmi;

	return XST_SUCCESS;
}

/*
 * Initialization function to configure the IIC bus
 * and set the parameters of the associated IicBus struct
 */
int HIIC_Init(IicBus *IicBusPtr,
			u32 IicDeviceAddr,
			u8 IntrId,
			u8 NumPeriphs)
{
	int Status;

	XIic_Config *ConfigPtr = NULL;	// Pointer to configuration data

	// Initialize the IIC driver so that it is ready to use.
	ConfigPtr = XIic_LookupConfig(IicDeviceAddr);
	if (ConfigPtr == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	Status = XIic_CfgInitialize(&XIicList[IicListId], ConfigPtr, ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Initialize device list entry
	IicBusPtr->DeviceAddr = IicDeviceAddr;
	IicBusPtr->IntrId = IntrId;
	IicBusPtr->NumPeriphs = NumPeriphs;
	IicBusPtr->IicInstPtr = &XIicList[IicListId];
	IicListId++;

	// Get default IIC parameters
//	u32 RegValue = 0x0;
//	RegValue = XIic_ReadReg(IicBusPtr->IicInstPtr->BaseAddress, IIC_REG_TSUSTA);
//	RegValue = XIic_ReadReg(IicBusPtr->IicInstPtr->BaseAddress, IIC_REG_TSUSTO);
//	RegValue = XIic_ReadReg(IicBusPtr->IicInstPtr->BaseAddress, IIC_REG_THDSTA);
//	RegValue = XIic_ReadReg(IicBusPtr->IicInstPtr->BaseAddress, IIC_REG_TSUDAT);
//	RegValue = XIic_ReadReg(IicBusPtr->IicInstPtr->BaseAddress, IIC_REG_TBUF);
//	RegValue = XIic_ReadReg(IicBusPtr->IicInstPtr->BaseAddress, IIC_REG_THIGH);
//	RegValue = XIic_ReadReg(IicBusPtr->IicInstPtr->BaseAddress, IIC_REG_TLOW);
//	RegValue = XIic_ReadReg(IicBusPtr->IicInstPtr->BaseAddress, IIC_REG_THDDAT);

	// Set IIC Bus Timing Parameters
	HIIC_SetTimingParam(IicBusPtr->IicInstPtr, IIC_REG_THDDAT, IIC_THDDAT_DEFAULT_NSEC);

	return XST_SUCCESS;
}

void HIIC_ClearList()
{
	IicListId = 0;
	memset(XIicList, 0, sizeof(XIicList));
}

/*
 * Standard function called to initiate any IIC write transfer
 */
int HIIC_WriteData(IicBus *IicBusPtr,
			u8 IicSlaveAddr,
			u8 *TxMsgPtr,
			int ByteCount,
			u32 DataHoldNsec)
{
	int Status;
	int IicTimeoutCounter = 0;
	int IicMaxTimeout = IIC_TIMEOUT_PER_BYTE * ByteCount;

	// Enable timeout counter at start of IIC transaction
	//MPCI_WriteReg(IIC_MONITOR_BASE_ADDR, IIC_MONITOR_CTRL_REG_OFFSET, 0x1);

	IicAddress = IicSlaveAddr;

	//HIIC_SetTimingParam(XIicPtr, IIC_REG_THDDAT, DataHoldNsec);

	// Set the defaults.
	IicTransmitComplete = 0;

	// Set address of IIC slave
	XIic_SetAddress(IicBusPtr->IicInstPtr, XII_ADDR_TO_SEND_TYPE, IicSlaveAddr);

	//MPCI_WriteReg(FPGA_IDAQ_TEST_BASE, FPGA_IDAQ_IIC_TEST_MESSAGE, Iic_Write_Start);
	//MPCI_WriteReg(FPGA_IDAQ_TEST_BASE, FPGA_IDAQ_IIC_TEST_DEVICE, (u32)IicSlaveAddr);

	// Start the IIC device.
	Status = XIic_Start(IicBusPtr->IicInstPtr);
	if (Status != XST_SUCCESS) {
		// Disable timeout counter at end of IIC transaction
		//MPCI_WriteReg(IIC_MONITOR_BASE_ADDR, IIC_MONITOR_CTRL_REG_OFFSET, 0x0);
		return Status;
	}

	//MPCI_WriteReg(FPGA_IDAQ_TEST_BASE, FPGA_IDAQ_IIC_TEST_MESSAGE, Iic_Write_MasterSend);

	IicBusPtr->IicInstPtr->Options = 0x0;

	// Send the Data.
	Status = XIic_MasterSend(IicBusPtr->IicInstPtr, TxMsgPtr, ByteCount);
	if (Status != XST_SUCCESS) {
		// Disable timeout counter at end of IIC transaction
		//MPCI_WriteReg(IIC_MONITOR_BASE_ADDR, IIC_MONITOR_CTRL_REG_OFFSET, 0x0);
		return Status;
	}

	//MPCI_WriteReg(FPGA_IDAQ_TEST_BASE, FPGA_IDAQ_IIC_TEST_MESSAGE, Iic_Write_Transmit);

	// Wait till data is transmitted.
	while ((IicTransmitComplete == 0) || (XIic_IsIicBusy(IicBusPtr->IicInstPtr) == TRUE)) {

		if((++IicTimeoutCounter) > IicMaxTimeout) {
			XIic_Stop(IicBusPtr->IicInstPtr);
			IicTimeoutNumber++;

			// Repeat IIC transaction attempt for maximum allowable timeouts, then return XST_FAILURE
			if(IicTimeoutNumber > IIC_MAX_TIMEOUT) {
				IicTimeoutNumber = 0;
				// Disable timeout counter at end of IIC transaction
				//MPCI_WriteReg(IIC_MONITOR_BASE_ADDR, IIC_MONITOR_CTRL_REG_OFFSET, 0x0);
				return IIC_TIMEOUT_ERROR;
			}
			return HIIC_WriteData(IicBusPtr, IicSlaveAddr, TxMsgPtr, ByteCount, DataHoldNsec);
		}
	}

	// Write finished successfully, reset timeout number and stop the IIC device.
	IicTimeoutNumber = 0;
	
	//MPCI_WriteReg(FPGA_IDAQ_TEST_BASE, FPGA_IDAQ_IIC_TEST_MESSAGE, Iic_Write_Stop);

	XIic_Stop(IicBusPtr->IicInstPtr);

	//MPCI_WriteReg(FPGA_IDAQ_TEST_BASE, FPGA_IDAQ_IIC_TEST_MESSAGE, 0);
	//MPCI_WriteReg(FPGA_IDAQ_TEST_BASE, FPGA_IDAQ_IIC_TEST_DEVICE, 0);

	// Disable timeout counter at end of IIC transaction
	//MPCI_WriteReg(IIC_MONITOR_BASE_ADDR, IIC_MONITOR_CTRL_REG_OFFSET, 0x0);
	return XST_SUCCESS;
}


/*
 * Standard function called to initiate any IIC read transfer
 *
 * Read transfers start with a write (master send) to prepare end-point device
 * and include a repeat start condition to pass control to the end-point for reading
 */
int HIIC_ReadData(IicBus *IicBusPtr,
			u8 IicSlaveAddr,
			u8 ReadRegAddr,
			u8 *RxMsgPtr,
			int ByteCount,
			u32 DataHoldNsec)
{
	int Status;
	int IicTimeoutCounter = 0;
	int IicMaxTimeout = IIC_TIMEOUT_PER_BYTE * ByteCount;

	// Enable timeout counter at start of IIC transaction
	//MPCI_WriteReg(IIC_MONITOR_BASE_ADDR, IIC_MONITOR_CTRL_REG_OFFSET, 0x1);

	IicAddress = IicSlaveAddr;

	//HIIC_SetTimingParam(XIicPtr, IIC_REG_THDDAT, DataHoldNsec);

	// Set the defaults.
	IicTransmitComplete = 0;
	IicReceiveComplete = 0;

	// Set address of IIC slave
	XIic_SetAddress(IicBusPtr->IicInstPtr, XII_ADDR_TO_SEND_TYPE, IicSlaveAddr);

	//MPCI_WriteReg(FPGA_IDAQ_TEST_BASE, FPGA_IDAQ_IIC_TEST_MESSAGE, Iic_Read_Start);
	//MPCI_WriteReg(FPGA_IDAQ_TEST_BASE, FPGA_IDAQ_IIC_TEST_DEVICE, (u32)IicSlaveAddr);
	

	// Start the IIC device.
	Status = XIic_Start(IicBusPtr->IicInstPtr);
	if (Status != XST_SUCCESS) {
		// Disable timeout counter at end of IIC transaction
		//MPCI_WriteReg(IIC_MONITOR_BASE_ADDR, IIC_MONITOR_CTRL_REG_OFFSET, 0x0);
		return XST_FAILURE;
	}

	//MPCI_WriteReg(FPGA_IDAQ_TEST_BASE, FPGA_IDAQ_IIC_TEST_MESSAGE, Iic_Read_MasterSend);

	// Set the Repeated Start option, needed for read operations
	IicBusPtr->IicInstPtr->Options = XII_REPEATED_START_OPTION;
	// Transmit the register address to start read from
	Status = XIic_MasterSend(IicBusPtr->IicInstPtr, &ReadRegAddr, 1);
	if (Status != XST_SUCCESS) {
		// Disable timeout counter at end of IIC transaction
		//MPCI_WriteReg(IIC_MONITOR_BASE_ADDR, IIC_MONITOR_CTRL_REG_OFFSET, 0x0);
		return Status;
	}

	//MPCI_WriteReg(FPGA_IDAQ_TEST_BASE, FPGA_IDAQ_IIC_TEST_MESSAGE, Iic_Read_Transmit);

	// Wait till all the data is transmitted.
	while (IicTransmitComplete == 0) {

		if((++IicTimeoutCounter) > IicMaxTimeout) {
			XIic_Stop(IicBusPtr->IicInstPtr);
			IicTimeoutNumber++;

			// Repeat IIC transaction attempt for maximum allowable timeouts, then return XST_FAILURE
			if(IicTimeoutNumber > IIC_MAX_TIMEOUT) {
				IicTimeoutNumber = 0;
				// Disable timeout counter at end of IIC transaction
				//MPCI_WriteReg(IIC_MONITOR_BASE_ADDR, IIC_MONITOR_CTRL_REG_OFFSET, 0x0);
				return IIC_TIMEOUT_ERROR;
			}
			return HIIC_ReadData(IicBusPtr, IicSlaveAddr, ReadRegAddr, RxMsgPtr, ByteCount, DataHoldNsec);
		}
	}

	//MPCI_WriteReg(FPGA_IDAQ_TEST_BASE, FPGA_IDAQ_IIC_TEST_MESSAGE, Iic_Read_MasterRecv);

	IicBusPtr->IicInstPtr->Options = 0x0;
	// Receive the Data.
	Status = XIic_MasterRecv(IicBusPtr->IicInstPtr, RxMsgPtr, ByteCount);
	if (Status != XST_SUCCESS) {
		// Disable timeout counter at end of IIC transaction
		//MPCI_WriteReg(IIC_MONITOR_BASE_ADDR, IIC_MONITOR_CTRL_REG_OFFSET, 0x0);
		return Status;
	}

	//MPCI_WriteReg(FPGA_IDAQ_TEST_BASE, FPGA_IDAQ_IIC_TEST_MESSAGE, Iic_Read_Receive);

	// Wait till all the data is received, reset timeout counter.
	IicTimeoutCounter = 0;
	while ((IicReceiveComplete == 0) || (XIic_IsIicBusy(IicBusPtr->IicInstPtr) == TRUE)) {

		if((++IicTimeoutCounter) > IicMaxTimeout) {
			XIic_Stop(IicBusPtr->IicInstPtr);
			IicTimeoutNumber++;

			// Repeat IIC transaction attempt for maximum allowable timeouts, then return XST_FAILURE
			if(IicTimeoutNumber > IIC_MAX_TIMEOUT) {
				IicTimeoutNumber = 0;
				// Disable timeout counter at end of IIC transaction
				//MPCI_WriteReg(IIC_MONITOR_BASE_ADDR, IIC_MONITOR_CTRL_REG_OFFSET, 0x0);
				return IIC_TIMEOUT_ERROR;
			}
			return HIIC_ReadData(IicBusPtr, IicSlaveAddr, ReadRegAddr, RxMsgPtr, ByteCount, DataHoldNsec);
		}
	}

	// Read finished successfully, reset timeout number and stop the IIC device.
	IicTimeoutNumber = 0;

	//MPCI_WriteReg(FPGA_IDAQ_TEST_BASE, FPGA_IDAQ_IIC_TEST_MESSAGE, Iic_Read_Stop);

	XIic_Stop(IicBusPtr->IicInstPtr);

	//MPCI_WriteReg(FPGA_IDAQ_TEST_BASE, FPGA_IDAQ_IIC_TEST_MESSAGE, 0);
	//MPCI_WriteReg(FPGA_IDAQ_TEST_BASE, FPGA_IDAQ_IIC_TEST_DEVICE, 0);

	// Disable timeout counter at end of IIC transaction
	//MPCI_WriteReg(IIC_MONITOR_BASE_ADDR, IIC_MONITOR_CTRL_REG_OFFSET, 0x0);
	return XST_SUCCESS;
}

static void HIIC_SetTimingParam(XIic *XIicPtr, u32 RegOffset, u32 TimeNsec)
{
	u32 NumCycles = (TimeNsec / IIC_AXI_CLOCK_PERIOD) + 1;
	XIic_WriteReg(XIicPtr->BaseAddress, RegOffset, NumCycles);
}