# Flexcan Wake Up Example Code

## Project Name: Flexcan_Wakeup_Demo

---

## Function description

This project is flexcan wake up chip demo:
- **Flexcan_Wakeup_Demo**: 
  - Standby mode.
  - Active mode.
1. Config alt value of PTA_9 Pin with LINFlexD2_TX as TX pin and PTA_8 with LINFlexD2_RX as RX pin.
2. Enable and config LINFlexD2_CLK.
3. Enable and config Linflexd Uart and Utility Print module.
4. Config alt value of PTD_5 Pin with LED5, PTD_6 Pin with LED6, PTD_7 Pin with LED7, PTE_6 Pin with CAN1_STB.
5. Enable and config GPIO_CLK.
6. Config alt value of PTC_7 Pin with CAN1_TX, PTC_6 Pin with CAN1_RX.
8. Enable and config FlexCAN1_CLK.
9. Enable and config CAN module.
12. Enable and config POWER module.
13. Use **TX_MAILBOX** defined mailbox number(mailbox-1) to send a standard frame message with id = **TX_MSG_ID**(0x003) every 1s with polling, once send the message success the LED5 will be toggled.
14. Use **RX_STD_MAILBOX** defined mailbox number(mailbox-0) to receive a message id = **RX_STD_MSG_ID**(0x002) with can1 event interupt(FLEXCAN_EVENT_RX_COMPLETE), once received the message the LED6 will be toggled.
15. Use **RX_EXT_MAILBOX** defined mailbox number(mailbox-2) to receive a message id = **RX_EXTMSG_ID**(0x70012) with can1 event interupt(FLEXCAN_EVENT_RX_COMPLETE), once received the message the LED7 will be toggled.
16. User can choose the standby mode by standard can message received. message data[0] == 0x01 will enter standby modeothers data don't care;
18. Any frame can wake up the chip to active mode.
  - The code continues to run after entering the standby statement (wake up from Standby mode).
---

## HW requirement
+ **EVB**: YTM32B1M-EVB-Q64
+ **Debugger**: J-LINK v9.2 or later with YUNTU YTM32B1MC0 patch(support package) installed.

---

## SW requirement

+ **Config & Code Generation Tool**
  + **SDK**:       Yuntu Config Tool(YCT) v2.6.3 or later version
+ **Compiler Toolchain**
  + **GCC***     : GNU GCC v10.3 or later version
  + **Keil/MDK***: ARM Keil/MDK v5.36 or later version
  + **IAR***     : IAR v8.4 or later version
  + **VS Code**  : Microsoft Visual Studio Code IDE v1.76 or later version(C/C++ support, GNU toolchain, CMake v3.25, ninja, Cortex-Debug extensions installation required)
+ **Debugger driver**
    + **J-LINK***              : SEGGER JLINK v6.96 or later version, YUNTU YTM32B1MC0 patch(support package) installation required.
    + **Ozone Debugger tool**  : SEGGER Ozone v3.28 or later version.

    ***Note***: * **YTM32B1MC0** patch install required which can be obtained via **YCT** or contact ***YUNTU sales/FAE team***.

---

## Setup

+ ***Step1***: Open the project.yct with the YCT tool, config the toolchain per your required(by default, the VS code + GCC are used)
+ ***Step2***: Generate the codes, you can change the configurations if needed(to adapt to your own HW design), if so, please ensure all the modifications are OK with no any errors exist before generating the code.
+ ***Step3***: Open the generated project with VS code/IAR/Keil IDE per your selected.
+ ***Step4***: Compile the project, power on the EVB and download it to the EVB via the available debugger, such as a J-LINK debugger probe.
+ ***Step5***: At last, enjoy the demo example.
  
---

---