/*
 * Copyright 2024 Olympus Veran Technologies.
 * Do not reproduce without written permission.
 * All rights reserved.
 *
 * Created by Jordan Deitsch.
 */

#include "HHDMI.h"
#include "xil_printf.h"
#include <xstatus.h>

/************************** Device Instance Definitions *****************************/
ADV7511Device HdmiInst;

/************************** Variable Definitions *****************************/
u32 HDMI_INTERRUPT_VECTOR = 0;

/************************** Internal Definitions *****************************/
static int HHDMI_DisplaySetup(ADV7511Device *HdmiInstPtr);
static int HHDMI_GetConnectionStatus(ADV7511Device *HdmiInstPtr);
static int HHDMI_ReadEdid(ADV7511Device *HdmiInstPtr);
static bool HHDMI_GetHPDStatus(ADV7511Device *HdmiInstPtr);
static bool HHDMI_GetMonSenseStatus(ADV7511Device *HdmiInstPtr);
static bool HHDMI_GetIntrStatus(ADV7511Device *HdmiInstPtr, InterruptId IntrId);
static int HHDMI_EnableIntr(ADV7511Device *HdmiInstPtr, InterruptId IntrId, bool Enable);
static int HHDMI_ClearIntr(ADV7511Device *HdmiInstPtr, InterruptId IntrId);
static int HHDMI_DisableAllInterrupts(ADV7511Device *HdmiInstPtr);
static int HHDMI_ClearAllInterrupts(ADV7511Device *HdmiInstPtr);
static int HHDMI_PowerInit(ADV7511Device *HdmiInstPtr, bool Enable);
static int HHDMI_ConfigureDevice(ADV7511Device *HdmiInstPtr);

static int HHDMI_GetReg(ADV7511Device *HdmiInstPtr, const u8 RegAddr, u8 *ValuePtr);
static int HHDMI_SetReg(ADV7511Device *HdmiInstPtr, const u8 RegAddr, const u8 Value);
static int HHDMI_GetEdidPacket(ADV7511Device *HdmiInstPtr);
static int HHDMI_ParseEdid(ADV7511Device *HdmiInstPtr);


/*
 * Interrupt handlers for HDMI driver events
 */
void HDMI_IntrHandler(void *CallBackRef)
{
    if(NULL == CallBackRef){
        return;
    }
    
    HDMI_INTERRUPT_VECTOR = 1;
}


// Initialize HDMI driver
int HHDMI_Init(ADV7511Device *HdmiInstPtr, IicBus *I2cBusPtr, u8 Address)
{
    if((HdmiInstPtr == NULL) || (I2cBusPtr == NULL)){
        return XST_FAILURE;
    }
    
    HdmiInstPtr->IicBusPtr = I2cBusPtr;
    HdmiInstPtr->Address = Address >> 1;    // Shift 1 bit for 7-bit address
    HdmiInstPtr->EdidAddress = HDMI_EDID_ADDR >> 1;
    memset(HdmiInstPtr->EdidData, 0, HDMI_EDID_NUM_BYTES);

    int Status = XST_SUCCESS;
    u8 RegValue = 0;
    
    Status = HHDMI_GetReg(HdmiInstPtr, HDMI_MAIN_CHIP_REV_REG, &RegValue);
	if(Status == XST_SUCCESS) {
		HdmiInstPtr->IicPresent = TRUE;
        xil_printf("HDMI Driver Present: ID 0x%x\r\n", RegValue);
	}
	else if(Status == IIC_TIMEOUT_ERROR) {
		HdmiInstPtr->IicPresent = FALSE;
        xil_printf("HDMI Driver Not Present...\r\n");
		return XST_FAILURE; // Return failure, HDMI driver not connecting
	}
	else {
		HdmiInstPtr->IicPresent = FALSE;
        xil_printf("FAILED HDMI Driver Setup...\r\n");
		return Status;
	}

    // Initialize ADV7511 with power OFF
    HHDMI_PowerInit(HdmiInstPtr, false);

    // Disable all interrupts, and clear any active interrupts
    HHDMI_DisableAllInterrupts(HdmiInstPtr);
    HHDMI_ClearAllInterrupts(HdmiInstPtr);

    // Setup interrupt handler
    Status = HINTC_SetupNewInterrupt(&IntrCtrInst, HDMI_INTR_ID, HDMI_IntrHandler, HdmiInstPtr);
    if(Status != XST_SUCCESS){
        return XST_FAILURE;
    }
    
    // Enable HPD and Monitor Sense interrupt
    HHDMI_EnableIntr(HdmiInstPtr, e_Intr_Hpd, true);
    HHDMI_EnableIntr(HdmiInstPtr, e_Intr_MonSense, true);

    // Check connection status on initialization to verify display present on power-up
    msleep(HDMI_DELAY_MSEC);
    xil_printf("Checking for display at power-on...\r\n");
    HHDMI_DisplaySetup(HdmiInstPtr);

    return XST_SUCCESS;
}

int HHDMI_ConnectionEvent(ADV7511Device *HdmiInstPtr)
{
    if(NULL == HdmiInstPtr){
		return XST_FAILURE;
	}

    u8 RegValue = 0;

    msleep(HDMI_DELAY_MSEC);
    xil_printf("Connection Event Detected...\r\n");

    // Check status of HPD interrupt, if no interrupt then clear interrupts
    if(HHDMI_GetIntrStatus(HdmiInstPtr, e_Intr_Hpd) == false){
        HHDMI_GetReg(HdmiInstPtr, HDMI_MAIN_INTR_STATUS_REG, &RegValue);
        xil_printf(" > Invalid interrupt: 0x%x\r\n", RegValue);
        HHDMI_ClearAllInterrupts(HdmiInstPtr);
        return XST_SUCCESS;
    }

    return HHDMI_DisplaySetup(HdmiInstPtr);
}

static int HHDMI_DisplaySetup(ADV7511Device *HdmiInstPtr)
{
    int Status = XST_SUCCESS;
    
    if(NULL == HdmiInstPtr){
		return XST_FAILURE;
	}

    // Check connection status of HPD and Monitor Sense for valid display connection
    Status = HHDMI_GetConnectionStatus(HdmiInstPtr);
    if((Status != XST_SUCCESS) || (HdmiInstPtr->DisplayPresent == false)){
        return Status;
    }

    // Enable chip power and read EDID of display
    Status = HHDMI_ReadEdid(HdmiInstPtr);
    if(Status != XST_SUCCESS){
        return Status;
    }

    // Set remaining configuration registers
    Status = HHDMI_ConfigureDevice(HdmiInstPtr);
    if(Status != XST_SUCCESS){
        return Status;
    }
    
    return XST_SUCCESS;
}

static int HHDMI_GetConnectionStatus(ADV7511Device *HdmiInstPtr)
{
    if(NULL == HdmiInstPtr){
		return XST_FAILURE;
	}

    int TimeoutCounter;

    HdmiInstPtr->DisplayPresent = false;

    // Check status of HPD, if not connected disable power and clear interrupts
    if(HHDMI_GetHPDStatus(HdmiInstPtr) == false){
        HHDMI_PowerInit(HdmiInstPtr, false);
        HHDMI_ClearAllInterrupts(HdmiInstPtr);
        return XST_SUCCESS;
    }

    // Wait for Monitor Sennse interrupt or timeout
    TimeoutCounter = HDMI_MONSENSE_TIMEOUT_COUNTER;
    while(TimeoutCounter > 0)
    {
        if(HHDMI_GetIntrStatus(HdmiInstPtr, e_Intr_MonSense)){
                break;
        }
        
        msleep(HDMI_INTR_TIMEOUT_MSEC);
        TimeoutCounter--;
    }

    // Clear interrupts regardless of timeout counter state
    HHDMI_ClearAllInterrupts(HdmiInstPtr);

    // Check status of Monitor Sense, if termination not valid disable power and clear interrupts
    if(HHDMI_GetMonSenseStatus(HdmiInstPtr) == false){
        HHDMI_PowerInit(HdmiInstPtr, false);
        return XST_SUCCESS;
    }

    HdmiInstPtr->DisplayPresent = true;
    
    return XST_SUCCESS;
}

static int HHDMI_ReadEdid(ADV7511Device *HdmiInstPtr)
{
    if(NULL == HdmiInstPtr){
		return XST_FAILURE;
	}

    int Status;
    int TimeoutCounter;
    
    // Enable power and initialize start-up registers
    HHDMI_PowerInit(HdmiInstPtr, true);

    // Enable EDID_Ready interrupt and wait for interrupt or timeout
    HHDMI_EnableIntr(HdmiInstPtr, e_Intr_EdidReady, true);
    TimeoutCounter = HDMI_EDID_TIMEOUT_COUNTER;
    while(TimeoutCounter > 0)
    {
        if(HHDMI_GetIntrStatus(HdmiInstPtr, e_Intr_EdidReady)){
            break;
        }
        
        msleep(HDMI_INTR_TIMEOUT_MSEC);
        TimeoutCounter--;
    }

    // Clear interrupts regardless of timeout counter state
    HHDMI_ClearAllInterrupts(HdmiInstPtr);

    // If timeout expires, disable power and return
    if(TimeoutCounter == 0){
        HHDMI_PowerInit(HdmiInstPtr, false);
        return XST_FAILURE;
    }

    // Read EDID registers from ADV7511
    Status = HHDMI_GetEdidPacket(HdmiInstPtr);
    if(Status != XST_SUCCESS){
        return Status;
    }

    Status = HHDMI_ParseEdid(HdmiInstPtr);
    if(Status != XST_SUCCESS){
        return Status;
    }

    return XST_SUCCESS;
}

static bool HHDMI_GetHPDStatus(ADV7511Device *HdmiInstPtr)
{
    if(NULL == HdmiInstPtr){
		return false;
	}

    // Get state of HPD
    u8 RegValue = 0;
    HHDMI_GetReg(HdmiInstPtr, HDMI_MAIN_HPD_STATE_REG, &RegValue);
    if(RegValue & HPD_STATE_MASK){
        xil_printf(" > HPD state: CONNECTED\r\n");
        return true;
    }
    else {
        xil_printf(" > HPD state: DISCONNECTED\r\n");
        return false;
    }
}

static bool HHDMI_GetMonSenseStatus(ADV7511Device *HdmiInstPtr)
{
    if(NULL == HdmiInstPtr){
		return false;
	}

    // Get state of Monitor Sense
    u8 RegValue = 0;
    HHDMI_GetReg(HdmiInstPtr, HDMI_MAIN_HPD_STATE_REG, &RegValue);
    if(RegValue & MONITOR_STATE_MASK){
        xil_printf(" > Clock term: GOOD\r\n");
        return true;
    }
    else {
        xil_printf(" > Clock term: BAD\r\n");
        return false;
    }
}

static bool HHDMI_GetIntrStatus(ADV7511Device *HdmiInstPtr, InterruptId IntrId)
{
    if(NULL == HdmiInstPtr){
		return XST_FAILURE;
	}

    u8 MaskValue = 0;
    switch(IntrId) {
    case e_Intr_Hpd:
        MaskValue = INTR_STATUS_HPD_MASK;
        break;
    case e_Intr_MonSense:
        MaskValue = INTR_STATUS_MON_SENSE_MASK;
        break;
    case e_Intr_EdidReady:
        MaskValue = INTR_STATUS_EDID_READY_MASK;
        break; 
    default:
        return XST_FAILURE;    
    }

    u8 RegValue = 0;
    HHDMI_GetReg(HdmiInstPtr, HDMI_MAIN_INTR_STATUS_REG, &RegValue);
    
    if(RegValue & MaskValue){
        return true;
    }

    return false;
}

static int HHDMI_EnableIntr(ADV7511Device *HdmiInstPtr, InterruptId IntrId, bool Enable)
{
    if(NULL == HdmiInstPtr){
		return XST_FAILURE;
	}

    u8 MaskValue = 0;
    switch(IntrId) {
    case e_Intr_Hpd:
        MaskValue = INTR_CTRL_HPD_MASK;
        break;
    case e_Intr_MonSense:
        MaskValue = INTR_CTRL_MON_SENSE_MASK;
        break;
    case e_Intr_EdidReady:
        MaskValue = INTR_CTRL_EDID_READY_MASK;
        break; 
    default:
        return XST_FAILURE;    
    }
    
    u8 RegValue = 0;
    HHDMI_GetReg(HdmiInstPtr, HDMI_MAIN_INTR_CTRL_REG, &RegValue);
    if(Enable){
        RegValue = RegValue | MaskValue;
    }
    else {
        RegValue = RegValue & (~MaskValue);
    }

    return HHDMI_SetReg(HdmiInstPtr, HDMI_MAIN_INTR_CTRL_REG, RegValue);
}

static int HHDMI_ClearIntr(ADV7511Device *HdmiInstPtr, InterruptId IntrId)
{
    if(NULL == HdmiInstPtr){
		return XST_FAILURE;
	}

    u8 MaskValue = 0;
    switch(IntrId) {
    case e_Intr_Hpd:
        MaskValue = INTR_STATUS_HPD_MASK;
        break;
    case e_Intr_MonSense:
        MaskValue = INTR_STATUS_MON_SENSE_MASK;
        break;
    case e_Intr_EdidReady:
        MaskValue = INTR_STATUS_EDID_READY_MASK;
        break; 
    default:
        return XST_FAILURE;    
    }

    u8 RegValue = 0;
    HHDMI_GetReg(HdmiInstPtr, HDMI_MAIN_INTR_STATUS_REG, &RegValue);

    // Clear active interrupt with write to the status register
    if(RegValue & MaskValue){
        HHDMI_SetReg(HdmiInstPtr, HDMI_MAIN_INTR_STATUS_REG, MaskValue);
    }

    // If all active interrupts are cleared, clear interrupt vector
    if(RegValue == 0){
        HDMI_INTERRUPT_VECTOR = 0;
    }
    
    return XST_SUCCESS;
}

static int HHDMI_DisableAllInterrupts(ADV7511Device *HdmiInstPtr)
{
    // Disable all interrupts, and clear any active interrupts
    for(int i=0; i<e_NumInterrupts; i++)
    {
        HHDMI_EnableIntr(HdmiInstPtr,i, false);
    }

    return XST_SUCCESS;
}

static int HHDMI_ClearAllInterrupts(ADV7511Device *HdmiInstPtr)
{
    u8 RegValue = 0;
    HHDMI_SetReg(HdmiInstPtr, HDMI_MAIN_INTR_STATUS_REG, 0xFF);
    HHDMI_GetReg(HdmiInstPtr, HDMI_MAIN_INTR_STATUS_REG, &RegValue);

    // If all active interrupts are cleared, clear interrupt vector
    if(RegValue != 0){
        return XST_FAILURE;
    }
    
    HDMI_INTERRUPT_VECTOR = 0;
    return XST_SUCCESS;
}

static int HHDMI_PowerInit(ADV7511Device *HdmiInstPtr, bool Enable)
{
    if(NULL == HdmiInstPtr){
		return XST_FAILURE;
	}

    u8 RegValue = 0;
    HHDMI_GetReg(HdmiInstPtr, HDMI_MAIN_POWER_REG, &RegValue);

    if(Enable == true){
        RegValue = RegValue & (~POWER_DOWN_MASK);
        HHDMI_SetReg(HdmiInstPtr, HDMI_MAIN_POWER_REG, RegValue);
        msleep(HDMI_DELAY_MSEC);

        // Set start-up configuration registers after power-on
        HHDMI_SetReg(HdmiInstPtr, 0x98, 0x03);  // Set for proper operation
        HHDMI_SetReg(HdmiInstPtr, 0x9A, 0xE0);  // Set for proper operation
        HHDMI_SetReg(HdmiInstPtr, 0x9C, 0x30);  // Set for proper operation
        HHDMI_SetReg(HdmiInstPtr, 0x9D, 0x61);  // Set for CLK divide (none) and proper operation
        HHDMI_SetReg(HdmiInstPtr, 0xA2, 0xA4);  // Set for proper operation
        HHDMI_SetReg(HdmiInstPtr, 0xA3, 0xA4);  // Set for proper operation
        HHDMI_SetReg(HdmiInstPtr, 0xE0, 0xD0);  // Set for proper operation
        HHDMI_SetReg(HdmiInstPtr, 0xF9, 0x00);  // Set to non-conflicting IIC address (0x00)

        HHDMI_GetReg(HdmiInstPtr, HDMI_MAIN_POWER_REG, &RegValue);
        xil_printf(" > Power: ON\r\n");
    }
    else {
        RegValue = RegValue | POWER_DOWN_MASK;
        HHDMI_SetReg(HdmiInstPtr, HDMI_MAIN_POWER_REG, RegValue);
        msleep(HDMI_DELAY_MSEC);
        
        HHDMI_GetReg(HdmiInstPtr, HDMI_MAIN_POWER_REG, &RegValue);
        xil_printf(" > Power: OFF\r\n");
    }

    return XST_SUCCESS;
}

static int HHDMI_ConfigureDevice(ADV7511Device *HdmiInstPtr)
{
    if(NULL == HdmiInstPtr){
		return XST_FAILURE;
	}

    HHDMI_SetReg(HdmiInstPtr, 0x15, 0x01);  // Input video format: 16-bit YCbCr 4:2:2, separate syncs
    HHDMI_SetReg(HdmiInstPtr, 0x16, 0x3C);  // Output format: 8'b0011_1100: 4:4:4, 8-bit, style 3, no DDR
    HHDMI_SetReg(HdmiInstPtr, 0x17, 0x00);  // Sync Polarity: VSync and Hsync both high polarity
    HHDMI_SetReg(HdmiInstPtr, 0x48, 0x08);  // Input justification: 8'b0000_1000: right justified 
    HHDMI_SetReg(HdmiInstPtr, 0xBA, 0x60);  // Clock Delay: 8'b0110_0000: no delay
    HHDMI_SetReg(HdmiInstPtr, 0xD0, 0x30);  // Sync Pulse: default settings, no DDR, input ID = 1

    // Set AVI Packet info
    HHDMI_SetReg(HdmiInstPtr, 0x55, 0x52);  // AVI InfoFrame: 8'b0101_0010;
    
    return XST_SUCCESS;
}

static int HHDMI_GetReg(ADV7511Device *HdmiInstPtr, const u8 RegAddr, u8 *ValuePtr)
{
    if((HdmiInstPtr == NULL) || (ValuePtr == NULL)){
        return XST_FAILURE;
    }
    
    return HIIC_ReadData(HdmiInstPtr->IicBusPtr, HdmiInstPtr->Address, RegAddr, ValuePtr, 1, HDMI_THDDAT_TIME_NSEC);
}

static int HHDMI_SetReg(ADV7511Device *HdmiInstPtr, const u8 RegAddr, const u8 Value)
{
	if(NULL == HdmiInstPtr){
		return XST_FAILURE;
	}

	int NumBytes = 2;
	u8 MsgData[NumBytes];

	//Setup Data packet for write
	MsgData[0] = RegAddr;
	MsgData[1] = Value;
    
    return HIIC_WriteData(HdmiInstPtr->IicBusPtr, HdmiInstPtr->Address, MsgData, NumBytes, HDMI_THDDAT_TIME_NSEC);
}

static int HHDMI_GetEdidPacket(ADV7511Device *HdmiInstPtr)
{
    if(NULL == HdmiInstPtr){
		return XST_FAILURE;
	}

    u8 RegAddr = 0x00;

    return HIIC_ReadData(HdmiInstPtr->IicBusPtr, HdmiInstPtr->EdidAddress, RegAddr, HdmiInstPtr->EdidData, HDMI_EDID_NUM_BYTES, HDMI_THDDAT_TIME_NSEC);
}

static int HHDMI_ParseEdid(ADV7511Device *HdmiInstPtr)
{
    HdmiInstPtr->MonitorParams.IsDigital = ((HdmiInstPtr->EdidAddress + 20) & 0x80) >> 7;
    HdmiInstPtr->MonitorParams.BitDepth = ((HdmiInstPtr->EdidAddress + 20) & 0x70) >> 4;
    HdmiInstPtr->MonitorParams.VideoInterface = ((HdmiInstPtr->EdidAddress + 20) & 0x0F);
    HdmiInstPtr->MonitorParams.HScreenSize = ((HdmiInstPtr->EdidAddress + 21) & 0xFF);
    HdmiInstPtr->MonitorParams.VScreenSize = ((HdmiInstPtr->EdidAddress + 22) & 0xFF);
    HdmiInstPtr->MonitorParams.DisplayGamma = ((HdmiInstPtr->EdidAddress + 23) & 0xFF);
    HdmiInstPtr->MonitorParams.DisplayType = ((HdmiInstPtr->EdidAddress + 24) & 0x18) >> 3;
    HdmiInstPtr->MonitorParams.TimingBitmap[0] = (HdmiInstPtr->EdidAddress + 35);
    HdmiInstPtr->MonitorParams.TimingBitmap[1] = (HdmiInstPtr->EdidAddress + 36);
    HdmiInstPtr->MonitorParams.TimingBitmap[2] = (HdmiInstPtr->EdidAddress + 37);
    HdmiInstPtr->MonitorParams.Resolution = (HdmiInstPtr->EdidAddress + 38);
    HdmiInstPtr->MonitorParams.AspectRatio =  ((HdmiInstPtr->EdidAddress + 39) & 0xC0) >> 6;
    HdmiInstPtr->MonitorParams.VerticalFreq =  ((HdmiInstPtr->EdidAddress + 39) & 0x3F);
    
    /*
    xil_printf(" > Video Interface: 0x%x\r\n", HdmiInstPtr->MonitorParams.VideoInterface);
    xil_printf(" > Screen Size: %d x %d\r\n", HdmiInstPtr->MonitorParams.VScreenSize, 
        HdmiInstPtr->MonitorParams.HScreenSize);
    xil_printf(" > Supported Timing: 0x%x-%x-%x\r\n", HdmiInstPtr->MonitorParams.TimingBitmap[0],
        HdmiInstPtr->MonitorParams.TimingBitmap[1],
        HdmiInstPtr->MonitorParams.TimingBitmap[2]);
    xil_printf(" > Display Type: 0x%x\r\n", HdmiInstPtr->MonitorParams.DisplayType);
    xil_printf(" > Resolution: 0x%x\r\n", HdmiInstPtr->MonitorParams.Resolution);
    xil_printf(" > Aspect Ratio: 0x%x\r\n", HdmiInstPtr->MonitorParams.AspectRatio);
    xil_printf(" > Vertical Frequency: 0x%x\r\n", HdmiInstPtr->MonitorParams.VerticalFreq);
    */
    
    return XST_SUCCESS;
}