/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    eth.h
 * @brief   This file contains all the function prototypes for
 *          the eth.c file
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ETH_H__
#define __ETH_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

#include "lan8720.h"

/* USER CODE END Includes */

extern ETH_HandleTypeDef heth;

/* USER CODE BEGIN Private defines */

extern ETH_TxPacketConfigTypeDef TxConfig;

typedef struct {
  ETH_BufferTypeDef AppBuff;
  uint8_t buffer[ETH_RX_BUF_SIZE] __ALIGNED(32);
} ETH_AppBuff;

typedef struct {
  ETH_BufferTypeDef* head;
  ETH_BufferTypeDef* tail;
  unsigned int len;
} RxBufferList;

typedef struct {
  uint8_t dest_mac[6];
  uint8_t src_mac[6];
  uint8_t type[2];
  uint8_t payload[100];
} ethernet_frame_t;

/* USER CODE END Private defines */

void MX_ETH_Init(void);

/* USER CODE BEGIN Prototypes */

void ETH_StartLink(void);
void ETH_ConstructEthernetFrame(ethernet_frame_t* frame,
                                uint8_t* dest_mac,
                                uint8_t* src_mac,
                                uint8_t* type,
                                uint8_t* payload,
                                uint16_t payload_len);
int ethSend(void* pBuff, int len);
void ethSendProcess(ETH_AppBuff* appBuff, uint32_t timeout);
int ethReceive(void** pPacket);
int ethRxListPush(ETH_BufferTypeDef* pBuff);
int ethRxListPop(ETH_BufferTypeDef** pBuff);
void ethRxBufferFree(void* pBuff);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __ETH_H__ */

