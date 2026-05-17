# Flexcan_Busoff_Demo

---

## 功能描述

该示例演示了如何通过初始化和去初始化 FlexCAN 手动恢复Bus Off错误。

- 波特率（标称）为 500Kbps
- 使用 **TX_MAILBOX** 定义的邮箱号（邮箱-1）以轮询方式每 1 秒发送一个 ID = **TX_MSG_ID**(0x003) 的标准帧消息，一旦发送消息，LED5 将被切换。
- 使用 **RX_STD_MAILBOX** 定义的邮箱号（邮箱-0）接收 ID = **RX_STD_MSG_ID**(0x002) 的消息，使用 can2 事件中断（FLEXCAN_EVENT_RX_COMPLETE），一旦接收到消息，LED6 将被切换
- 使用 **RX_EXT_MAILBOX** 定义的邮箱号（邮箱-2）接收 ID = **RX_EXTMSG_ID**(0x70012) 的消息，使用 can2 事件中断（FLEXCAN_EVENT_RX_COMPLETE），一旦接收到消息，LED6 将被切换
- 使用总线关闭错误回调
- 使用快速恢复模式和慢速恢复模式手动恢复总线关闭
- 每 100ms 尝试快速恢复模式，使用 tmr 中断计数，一旦尝试，LED7 将被切换
- 每 10s 尝试慢速恢复模式，使用 tmr 中断计数，一旦尝试，LED7 将被切换
- 尝试快速恢复 10 次后切换到慢速恢复
- 恢复成功将 printf 发送 ok 使用 can2 事件中断（FLEXCAN_EVENT_TX_COMPLETE）
- 如果发送消息返回错误，则演示结束

+ **连接**:
  - 通过板载 UART-USB 转换器连接 UART
  - J16: 将 PIN1 连接到 PIN2（Can_SLEEP_EN）
  - J16: 将 PIN3 连接到 PIN4（Can_Txd0）
  - J16: 将 PIN5 连接到 PIN6（Can_Rxd0）
  - 将 CAN 工具 CANH&CANL 线连接到板载 CAN1 通道 CANH&CANL（EVB 中的 CAN）

---

## 硬件需求

+ **开发板**: YTM32B1MD1-EVB-Q100
+ **调试器**: J-LINK v9.2 或更高版本

---

## 软件需求

+ **SDK**     : YTM32B1MD1_1_4_0 或更高版本
+ **GCC**     : GNU GCC v10.3 或更高版本
+ **Keil/MDK**: ARM Keil/MDK v5.36 或更高版本
+ **IAR**     : IAR v8.40 或更高版本
+ **VS Code** : Microsoft Visual Studio Code IDE v1.76 或更高版本（GNU GCC 工具链，CMake v3.25，ninja，Cortex-Debug 扩展安装）
+ **J-LINK**  : SEGGER JLINK v6.96 或更高版本，需要安装 YTM32B1MD1 补丁（支持包）
+ **Ozone**   : SEGGER Ozone v3.28 或更高版本

---