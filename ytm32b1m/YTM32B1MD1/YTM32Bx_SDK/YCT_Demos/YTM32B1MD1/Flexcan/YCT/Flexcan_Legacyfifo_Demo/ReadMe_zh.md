# Flexcan_Legacyfifo_Demo

---

## 功能描述

该示例演示了 FlexCAN Legacy FIFO 的使用。

- 短接板子上的 CAN1_H 与 CAN2_H，CAN1_L 与 CAN2_L 来完成自测试
- 波特率（标称）为 500Kbps，8 个 RX FIFO 过滤器。
- CAN1 配置如下
  - 用邮箱-8 发送 ID 为 0x0C1001F0 的扩展帧
  - 用 Legacy FIFO 接收 CAN 报文
- CAN2 配置如下
  - 用邮箱-8 发送 ID 为 0x081002F0 的扩展帧
  - 用 Legacy FIFO 接收 CAN 报文
- Legacy FIFO 配置如下：

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

- CAN1 与 CAN2 均会打印当前收到的报文内容
- 当 CAN1 发送完 10 条报文后，结束自测试

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