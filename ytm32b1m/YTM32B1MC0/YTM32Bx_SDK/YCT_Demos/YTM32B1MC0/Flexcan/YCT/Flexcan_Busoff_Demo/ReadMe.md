# Flexcan Bus off Code

## Project Name: Flexcan_Busoff_Demo

---


## Function description

The application is a demo of the use of the Flexcan on the YTM32B1MC0 FlexCAN1 channel to manual recovery Bus off. The manual recovery use reset can1 by deinitialization and initialization
- **Flexcan**: 
  - Baud rate(nominal) is 500Kbps
  - Use **TX_MAILBOX** defined mailbox number(mailbox-1) to send a standard frame message with id = **TX_MSG_ID**(0x003) every 1s with polling, once send the message the LED5 will be toggled.
  - Use **RX_STD_MAILBOX** defined mailbox number(mailbox-0) to receive a message id = **RX_STD_MSG_ID**(0x002) with can0 event interupt(FLEXCAN_EVENT_RX_COMPLETE), once received the message the LED6 will be toggled
  - Use **RX_EXT_MAILBOX** defined mailbox number(mailbox-2) to receive a message id = **RX_EXTMSG_ID**(0x70012) with can0 event interupt(FLEXCAN_EVENT_RX_COMPLETE), once received the message the LED6 will be toggled
  - Use Bus off error callback
  - Use quick recover mode and slow recover mode to manual recovery Bus off
  - Try the quick recover mode every 100ms with tmr interrupt counting , once try the LED7 will be toggled
  - Try the slow recover mode every 10s with tmr interrupt counting, once try the LED7 will be toggled
  - Try the quick recover after 10 times change to slow recover
  - Recover successful will printf send ok use can0 event interupt(FLEXCAN_EVENT_TX_COMPLETE)
  - If send  message return error, the demo end

---


## HW requirement
+ **EVB**: YTM32B1M-EVB-Q64
+ **Debugger**: J-LINK v9.2 or later with YUNTU YTM32B1MC0 patch(support package) installed.
+ **Connect**: 
  - UART via on board UART-USB converter
  - J12: connect PIN1 to PIN2(Can_SLEEP_EN)
  - J12: connect PIN3 to PIN4(Can_Txd0)
  - J12: connect PIN5 to PIN6(Can_Rxd0)
  - Connect CAN tool CANH&CANL line to board CAN1 chananel CANH&CANL(CAN in evb)

---

## SW requirement

+ **Config & Code Generation Tool**
  + **SDK**:       Yuntu Config Tool(YCT) v2.6.3 or later version
+ **Compiler Toolchain**
  + **GCC***     : GNU GCC v10.3 or later version
  + **Keil/MDK***: ARM Keil/MDK v5.35 or later version
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

