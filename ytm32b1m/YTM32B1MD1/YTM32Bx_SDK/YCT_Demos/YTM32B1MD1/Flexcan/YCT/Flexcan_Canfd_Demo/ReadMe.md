# Flexcan_Canfd_Demo
---

## Function Description

This example demonstrates the usage of FlexCAN in CAN FD mode and its self-test procedure:

- Short CAN1_H to CAN2_H and CAN1_L to CAN2_L on the board to perform self-test
- Nominal baud rate: 500Kbps, Data baud rate: 2000Kbps
- CAN1 uses Mailbox-1 to transmit CAN-FD frames with ID 0x002, and Mailbox-0 to receive CAN-FD frames with ID 0x003
- CAN2 uses Mailbox-1 to transmit CAN-FD frames with ID 0x003, and Mailbox-0 to receive CAN-FD frames with ID 0x002
- Both CAN1 and CAN2 print the content of received messages in real-time
- The self-test concludes after CAN1 transmits 10 messages
 
---

## Hardware Requirements

+ **EVB**: YTM32B1MD1-EVB-Q100
+ **Debugger**: J-LINK v9.2 or later version

---

## Software Requirements

+ **SDK**      : YTM32B1MD1_1_4_0 or later version
+ **GCC**      : GNU GCC v10.3 or later version
+ **Keil/MDK** : ARM Keil/MDK v5.36 or later version
+ **IAR***     : IAR v8.4 or later version
+ **VS Code**  : Microsoft Visual Studio Code IDE v1.76 or later version(GNU GCC toolchain, CMake v3.25, ninja, Cortex-Debug extensions installation required)
+ **J-LINK** : SEGGER JLINK v6.96 or later version, YTM32B1MD1 patch(support package) installation required.
+ **Ozone**  : SEGGER Ozone v3.28 or later version.
  
---