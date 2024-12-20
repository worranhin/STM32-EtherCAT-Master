# STM32-EtherCAT-Master

这是一个使用 STM32 的 EtherCAT 主站项目。

> [!WARNING]
> 项目未完成，持续开发中

## 通讯格式定义

与上位机的通讯通过 UART 实现，波特率为 115200，一帧数据由帧头+帧数据组成。

一帧数据的格式如下:

| 数据描述     | 数据长度 | 说明                             |
| ------------ | -------- | -------------------------------- |
| 头字节       | 1        | 帧头，固定为 0xA4                |
| 命令字节     | 1        | 见下文描述                       |
| 数据长度     | 1        | 描述数据长度                     |
| 帧头校验字节 | 1        | checksum 加和校验                |
| 数据         | x        | x 取决于数据长度字节             |
| 数据校验字节 | 0~1      | 若数据长度为 0, 则该字节不会发送 |

### 命令说明

#### 获取状态

上位机发送 4 bytes

| 数据域 | 说明     | 数据 |
| ------ | -------- | ---- |
| 0      | 头字节   | 0xA4 |
| 1      | 命令字节 | 0x01 |
| 2      | 数据长度 | 0x00 |
| 3      | 帧头校验 | 0xA5 |

下位机回复 6 bytes

| 数据域 | 说明       | 数据    |
| ------ | ---------- | ------- |
| 0      | 头字节     | 0xA4    |
| 1      | 命令字节   | 0x01    |
| 2      | 数据长度   | 0x01    |
| 3      | 帧头校验   | 0xA6    |
| 4      | 状态码     | uint8_t |
| 5      | 数据校验和 | uint8_t |

#### 位置控制

上位机发送 9 bytes

| 数据域 | 说明                     | 数据    |
| ------ | ------------------------ | ------- |
| 0      | 头字节                   | 0xA4    |
| 1      | 命令字节                 | 0x11    |
| 2      | 数据长度                 | 0x04    |
| 3      | 帧头校验                 | 0xB9    |
| 4~7    | 目标位置(4=高位, 7=低位) | int32_t |
| 8      | 数据校验和               | uint8_t |

## 线程间共享资源使用情况

### ethTxBuffMutex

文件 | 函数 | 操作
--- | --- | ---
eth.c | ethSend() | Acquire
freertos.c | HAL_ETH_TxCpltCallback | Release

### ethRxCpltSemaphore

文件 | 函数 | 操作
--- | --- | ---
nicdrv.c | ecx_waitinframe() | Acquire
nicdrv.c | ecx_srconfirm() | Acquire
freertos.c | HAL_ETH_RxCpltCallback() | Release
freertos.c | HAL_ETH_ErrorCallback() | Release

### rxBufferPool

文件 | 函数 | 操作
--- | --- | ---
freertos.c | HAL_ETH_RxAllocateCallback() | Alloc
eth.c | ethRxBufferFree() | Free

### ethTxQueue

内容物：`ETH_AppBuff*` 

## Credits

This project uses the following open source library:
- SOEM (GNU General Public License version 2 with exception)
- LwIP modified by ST 
- FreeRTOS modified by ST (BSD 3-Clause license)
