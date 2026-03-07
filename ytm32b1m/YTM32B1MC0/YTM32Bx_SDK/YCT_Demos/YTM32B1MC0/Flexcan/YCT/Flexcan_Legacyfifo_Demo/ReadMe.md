# Flexcan Legacyfifo Example Code

## Project Name: Flexcan_Legacyfifo_Demo

---

## Function description

The application is a demonstration of the use of the Flexcan_legacyfifo on the YTM32B1MC0 FlexCAN1 channel.
- **Flexcan_Legacyfifo_Demo**: 
  - Use **TX_MAILBOX** defined mailbox number(mailbox-8) to send a standard frame message with id = **TX_MSG_ID**(0x003) every 100ms.
  - Use legacy rx fifo to receive some message id match with legacy rx fifo filter table variable ***rxFifoAccId***.
  - Baud rate(nominal) is 500Kbps, 8 Rx FIFO Filters.
  - Connect CAN tool CANH&CANL line to board CAN chananel CANH&CANL(J7).

  ```C
  const uint32_t rxFifoAccId[CAN0_RX_FIFO_FILTER_COUNT] = {
    0x0C1001F0U,
    0x081002F0U,
    0x0C0803A5U,
    0x0C1004F0U,
    0x0C1005E0U,
    0x0C1006F3U,
    0x0C1007F0U,
    0x0C10080FU,
  };
  ```

 - Each match id message received will be print via UART.
 - If received message count up to 10, the whole demo routine will be stopped.
 - Note: **RX_LEGACY_FIFO** is mailbox-id of legacy fifo must be defined as 0
---

## HW requirement
+ **EVB**: YTM32B1MC0-EVB-Q64
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
