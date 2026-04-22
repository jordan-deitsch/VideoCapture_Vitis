#include "HHDMI.h"
#include "HINTC.h"
#include "HIIC.h"
#include "HVTC.h"
#include "HVTPG.h"
#include "xil_printf.h"
#include <stdbool.h>

int main()
{
    int Status = 0;
    
    Status = HINTC_InitializeInterruptSystem(&IntrCtrInst, INTC_DEVICE_ADDR);
	if (Status != XST_SUCCESS) {
		return Status;
	}

    Status = HIIC_Setup();
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = HINTC_SetupPeripheralInterrupt(&IntrCtrInst, IicBusList, NUM_IIC_DRIVERS);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = HVTC_Init(&TimingControllerInst);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = HVTPG_Init(&PatternGenInst);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = HHDMI_Init(&HdmiInst, &IicBusInstHdmi, HDMI_MAIN_ADDR);
	if (Status != XST_SUCCESS) {
		return Status;
	}
	
	xil_printf("Finished initializing\r\n");

	/*
	 * VTC Testing
	 */
	
	// HVTC_EnableController(&TimingControllerInst, true);
	// HVTPG_ConfigureFrame(&PatternGenInst, e_HorizontalRamp);
	// HVTPG_GenerateFrame(&PatternGenInst);

	// u32 RegOffset = 0x0;
	// u32 SetValue = 0x0;
	// u32 GetValue = 0x0;
	
	// while(1)
	// {
	// 	GetValue = HVTC_GetReg(RegOffset);
	// 	HVTC_SetReg(RegOffset, SetValue);
	// }

	bool GenVideo = false;
	int UpdateTime = 5;
	int BackgroundPattern = e_SolidRed;
	
	while(1)
	{
		if(HDMI_INTERRUPT_VECTOR != 0)
		{
			HHDMI_ConnectionEvent(&HdmiInst);

			if(HdmiInst.DisplayPresent)
			{
				HVTC_EnableController(&TimingControllerInst, true);
				HVTPG_ConfigureFrame(&PatternGenInst, BackgroundPattern);
				HVTPG_GenerateContinuous(&PatternGenInst);
				GenVideo = true;
			}
			else
			{
				HVTC_EnableController(&TimingControllerInst, false);
				GenVideo = false;
			}
		}

		if(GenVideo)
		{
			HVTPG_MovingTarget(&PatternGenInst, BackgroundPattern);
			msleep(UpdateTime);
		}	
	}
    
  return 0;
}