/*
 * Copyright 2024 Olympus Veran Technologies.
 * Do not reproduce without written permission.
 * All rights reserved.
 *
 * Created by Jordan Deitsch.
 */

#include "HINTC.h"

XIntc IntrCtrInst;
u32 StubInterruptCounter = 0;

int HINTC_InitializeInterruptSystem(XIntc 	*IntcInstPtr,	u32	IntrCtrAddr)
{
	int Status;

	// Initialize the interrupt controller driver so that it's ready to use.
	Status = XIntc_Initialize(IntcInstPtr, IntrCtrAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller so interrupts are enabled for all
	 * devices that cause interrupts. Specify real mode so that the System
	 * Monitor/ACD device can cause interrupts through the interrupt
	 * controller.
	 */
	Status = XIntc_Start(IntcInstPtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Initialize the exception table.
	Xil_ExceptionInit();

	// Register the interrupt controller handler with the exception table.
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
								(Xil_ExceptionHandler) XIntc_InterruptHandler,
								IntcInstPtr);

	// Enable exceptions.
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

int HINTC_SetupPeripheralInterrupt(XIntc 	*IntcInstPtr,
								IicBus 		**IicBusList, 	u16 NumIicDrivers)
{
	for (int i = 0; i < NumIicDrivers; i++){
		// Set TX, RX and Status handlers for IIC bus instances
		XIic_SetSendHandler((*(IicBusList + i))->IicInstPtr, (*(IicBusList + i))->IicInstPtr, (XIic_Handler) HIIC_SendHandler);
		XIic_SetRecvHandler((*(IicBusList + i))->IicInstPtr, (*(IicBusList + i))->IicInstPtr, (XIic_Handler) HIIC_ReceiveHandler);
		XIic_SetStatusHandler((*(IicBusList + i))->IicInstPtr, (*(IicBusList + i))->IicInstPtr, (XIic_StatusHandler) HIIC_StatusHandler);

		// Set interrupt handler for IIC bus instances
		XIntc_Connect(IntcInstPtr, (*(IicBusList + i))->IntrId, (XInterruptHandler) XIic_InterruptHandler, (void *) (*(IicBusList + i))->IicInstPtr);
	}

	for (int i = 0; i < NumIicDrivers; i++){
		XIntc_Enable(IntcInstPtr, (*(IicBusList + i))->IntrId);
	}

	// Initialize the exception table.
	Xil_ExceptionInit();

	// Register the interrupt controller handler with the exception table.
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
								(Xil_ExceptionHandler) XIntc_InterruptHandler,
								IntcInstPtr);

	// Enable exceptions.
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

int HINTC_SetupNewInterrupt(XIntc 	*IntcInstPtr,
							u8 		InterId,
							XInterruptHandler IntrHandler,
					  	  	void 	*CallBackRef)
{
	// Connect interrupt handler with interrupt ID
	XIntc_Connect(IntcInstPtr,
				InterId,
				IntrHandler,
				CallBackRef);

	// Enable interrupt ID
	XIntc_Enable(IntcInstPtr, InterId);

	// Initialize the exception table.
	Xil_ExceptionInit();

	// Register the interrupt controller handler with the exception table.
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler) XIntc_InterruptHandler,
				IntcInstPtr);

	// Enable exceptions.
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

void HINTC_StubIntrHandler(void *CallBackRef)
{
	StubInterruptCounter++;
	if(NULL == CallBackRef)
	{
		return;
	}
}


