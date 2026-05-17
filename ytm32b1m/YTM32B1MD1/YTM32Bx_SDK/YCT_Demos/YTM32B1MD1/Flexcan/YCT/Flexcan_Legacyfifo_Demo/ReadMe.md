# Flexcan_Legacyfifo_Demo

---

## Function Description

This example demonstrates the usage of FlexCAN's Legacy FIFO functionality:

- Short CAN1_H to CAN2_H and CAN1_L to CAN2_L on the board to perform self-test
- Nominal baud rate: 500Kbps, with 8 RX FIFO filters configured
- CAN1 configuration:
    - Uses Mailbox-8 to transmit extended frames with ID 0x0C1001F0
    - Uses Legacy FIFO for CAN message reception
- CAN2 configuration:
    - Uses Mailbox-8 to transmit extended frames with ID 0x081002F0
    - Uses Legacy FIFO for CAN message reception
- Legacy FIFO configuration:

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