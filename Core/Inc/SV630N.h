/*
 * SV630N.h
 *
 *  Created on: Dec 8, 2024
 *      Author: Administrator
 */

#ifndef INC_SV630N_H_
#define INC_SV630N_H_

#include "osal.h"
#include "ethercat.h"

#define SV630N_MANUFATURER_ID 1048576
#define SV630N_PRODUCT_ID 786706

typedef enum {
  probe1_enable = 0,      // 探针1使能, 0=不使能, 1=使能
  probe1_triggerMode = 1, // 0=单次触发, 1=连续触发
  probe1_triggerSignal = 2, // 0=DI输入信号, 1=Z信号（零点），使能后不能更改
  probe1_risingEnable = 4,  // 0=上升沿不锁存, 1=上升沿锁存
  probe1_fallingEnable = 5, // 0=下降沿不锁存, 1=下降沿锁存
  probe2_enable = 8,        // 探针2使能, 0=不使能, 1=使能
  probe2_triggerMode = 9,   // 0=单次触发, 1=连续触发
  probe2_triggerSignal = 10, // 0=DI输入信号, 1=Z信号（零点），使能后不能更改
  probe2_risingEnable = 12,  // 0=上升沿不锁存, 1=上升沿锁存
  probe2_fallingEnable = 13, // 0=下降沿不锁存, 1=下降沿锁存
} ProbeModeBits;

typedef struct PACKED {
  uint16 controlword;   // 6040h 控制字
  int32 targetPostion;  // 607Ah 目标位置
  uint16 probeFunction; // 60B8h 探针功能配置
  uint32 forceDoOutput; // 60FEh.01 物理输出,
} SV630N_Outputs_1701;  // 1701h PDO 映射配置

typedef struct PACKED {
  uint16 errorCode;                    // 603Fh 错误码
  uint16 statusWord;                   // 6041h 状态字
  int32 positionFeedback;             // 6064h 位置反馈，指令单位
  int16 torqueFeedback;               // 6077h 实际转矩
  int32 positionDeviation;            // 60F4h 位置偏差，指令单位
  uint16 probeStatus;                  // 60B9h 探针状态
  int32 probe1RisingPositionFeedback; // 60BAh 探针1上升沿位置值
  int32 probe2RisingPositionFeedback; // 60BCh 探针2上升沿位置值
  uint32 DIStatus;                     // 60FDh DI状态
} SV630N_Inputs_1B01;                 // 1B01h

// 默认的 PDO 映射配置
#define SV630N_Outputs SV630N_Outputs_1701
#define SV630N_Inputs SV630N_Inputs_1B01

int SV630N_Setup(uint16 slave);

#endif /* INC_SV630N_H_ */
