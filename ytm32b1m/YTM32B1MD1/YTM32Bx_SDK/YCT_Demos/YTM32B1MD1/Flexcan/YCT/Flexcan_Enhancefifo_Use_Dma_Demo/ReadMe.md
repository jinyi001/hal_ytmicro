# Flexcan Enhancefifo Example Code

## Project Name: Flexcan_Enhancefifo_Use_Dma_Demo

---

## Function description

This example demonstrates the usage of FlexCAN Enhance Fifo use DMA and self-test.

- Short connect CAN0_H with CAN1_H, and CAN0_L with CAN1_L on the board to complete self-test
- Baud rate: 500Kbps for arbitration field, 2Mbps for data field
- CAN0 configuration as follows
  - Use mailbox-1 to alternately send standard frame with ID 0x4FE and extended frame with ID 0x0CF003FE
  - Use Enhance FIFO to receive standard frame with ID 0x200 and extended frame with ID 0x180F1000, see example for details
- CAN1 configuration as follows
  - Use mailbox-1 to alternately send standard frame with ID 0x200 and extended frame with ID 0x180F1000
  - Use mailbox-1 to receive standard frame with ID 0x4FE
  - Use mailbox-2 to receive extended frame with ID 0x0CF003FE  
- Both CAN0 and CAN1 will print the content of currently received messages
- Self-test ends after CAN1 sends 11 messages
---

## HW requirement
+ **EVB**: YTM32B1MD1-EVB-Q100
+ **Debugger**: J-LINK v9.2 or later with YUNTU YTM32B1MD1 patch(support package) installed.
+ **Connect**: 
  - J11: connect PIN1 to PIN2(CAN0_STB)
  - J11: connect PIN3 to PIN4(CAN0_TXD)
  - J11: connect PIN5 to PIN6(CAN0_RXD)
  - J6: connect PIN1 to PIN2(CAN1_STB)
  - J6: connect PIN3 to PIN4(CAN1_TXD)
  - J6: connect PIN5 to PIN6(CAN1_RXD)
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
    + **J-LINK***              : SEGGER JLINK v6.96 or later version, YUNTU YTM32B1MD1 patch(support package) installation required.
    + **Ozone Debugger tool**  : SEGGER Ozone v3.28 or later version.

    ***Note***: * **YTM32B1MD1** patch install required which can be obtained via **YCT** or contact ***YUNTU sales/FAE team***.
---

## Setup

+ ***Step1***: Open the project.yct with the YCT tool, config the toolchain per your required(by default, the VS code + GCC are used)
+ ***Step2***: Generate the codes, you can change the configurations if needed(to adapt to your own HW design), if so, please ensure all the modifications are OK with no any errors exist before generating the code.
+ ***Step3***: Open the generated project with VS code/IAR/Keil IDE per your selected.
+ ***Step4***: Compile the project, power on the EVB and download it to the EVB via the available debugger, such as a J-LINK debugger probe.
+ ***Step5***: At last, enjoy the demo example.
---

1. Config alt value of PTE_12 Pin with UART2_TX as TX pin and PTD_17 with UART2_RX as RX pin.
2. Enable and config UART2_CLK and FlexCAN0_CLK, FlexCAN1_CLK.
3. Enable and config Uart and Utility Print module.
4. Enable and config Flexcan enhance fifo mode.
5. Short connect CAN0 and CAN1 CANH&CANL.
---
