/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    eth.c
 * @brief   This file provides code for the configuration
 *          of the ETH instances.
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
/* Includes ------------------------------------------------------------------*/
#include "eth.h"
#include "string.h"

ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

ETH_TxPacketConfig TxConfig;

/* USER CODE BEGIN 0 */

#include "oled.h"

static uint8_t txBuffer[ETH_TX_BUF_SIZE];
RxBufferList rxList = {0};

int32_t ETH_PHY_INTERFACE_Init(void);
int32_t ETH_PHY_INTERFACE_DeInit(void);
int32_t ETH_PHY_INTERFACE_ReadReg(uint32_t DevAddr, uint32_t RegAddr, uint32_t *pRegVal);
int32_t ETH_PHY_INTERFACE_WriteReg(uint32_t DevAddr, uint32_t RegAddr, uint32_t RegVal);
int32_t ETH_PHY_INTERFACE_GetTick(void);

lan8720_Object_t lan8720;
lan8720_IOCtx_t lan8720_IOCtx = {ETH_PHY_INTERFACE_Init, ETH_PHY_INTERFACE_DeInit, ETH_PHY_INTERFACE_WriteReg,
                                 ETH_PHY_INTERFACE_ReadReg, ETH_PHY_INTERFACE_GetTick};

/* USER CODE END 0 */

ETH_HandleTypeDef heth;

/* ETH init function */
void MX_ETH_Init(void)
{

    /* USER CODE BEGIN ETH_Init 0 */

    /* USER CODE END ETH_Init 0 */

    static uint8_t MACAddr[6];

    /* USER CODE BEGIN ETH_Init 1 */

    /* USER CODE END ETH_Init 1 */
    heth.Instance = ETH;
    MACAddr[0] = 0x00;
    MACAddr[1] = 0x80;
    MACAddr[2] = 0xE1;
    MACAddr[3] = 0x00;
    MACAddr[4] = 0x00;
    MACAddr[5] = 0x00;
    heth.Init.MACAddr = &MACAddr[0];
    heth.Init.MediaInterface = HAL_ETH_RMII_MODE;
    heth.Init.TxDesc = DMATxDscrTab;
    heth.Init.RxDesc = DMARxDscrTab;
    heth.Init.RxBuffLen = 1536;

    /* USER CODE BEGIN MACADDRESS */

    /* USER CODE END MACADDRESS */

    if (HAL_ETH_Init(&heth) != HAL_OK)
    {
          Error_Handler();
  }

  memset(&TxConfig, 0, sizeof(ETH_TxPacketConfig));
  TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
  TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
  TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;
  /* USER CODE BEGIN ETH_Init 2 */

  /* Set PHY IO functions */
  LAN8720_RegisterBusIO(&lan8720, &lan8720_IOCtx);
  /* Initialize the LAN8742 ETH PHY */
  LAN8720_Init(&lan8720);
  /* Initialize link speed negotiation and start Ethernet peripheral */
  ETH_StartLink();

#ifdef DEBUG_MESSAGE
    printf("ETH init cplt\n");
#endif // DEBUG_MESSAGE

    /* USER CODE END ETH_Init 2 */
}

void HAL_ETH_MspInit(ETH_HandleTypeDef *ethHandle)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (ethHandle->Instance == ETH)
    {
        /* USER CODE BEGIN ETH_MspInit 0 */

        /* USER CODE END ETH_MspInit 0 */
        /* ETH clock enable */
        __HAL_RCC_ETH_CLK_ENABLE();

        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOG_CLK_ENABLE();
        /**ETH GPIO Configuration
        PC1     ------> ETH_MDC
        PA1     ------> ETH_REF_CLK
        PA2     ------> ETH_MDIO
        PA7     ------> ETH_CRS_DV
        PC4     ------> ETH_RXD0
        PC5     ------> ETH_RXD1
        PG11     ------> ETH_TX_EN
        PG13     ------> ETH_TXD0
        PG14     ------> ETH_TXD1
        */
        GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_13 | GPIO_PIN_14;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
        HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

        /* ETH interrupt Init */
        HAL_NVIC_SetPriority(ETH_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(ETH_IRQn);
        /* USER CODE BEGIN ETH_MspInit 1 */

        /* USER CODE END ETH_MspInit 1 */
    }
}

void HAL_ETH_MspDeInit(ETH_HandleTypeDef *ethHandle)
{

    if (ethHandle->Instance == ETH)
    {
        /* USER CODE BEGIN ETH_MspDeInit 0 */

        /* USER CODE END ETH_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_ETH_CLK_DISABLE();

        /**ETH GPIO Configuration
        PC1     ------> ETH_MDC
        PA1     ------> ETH_REF_CLK
        PA2     ------> ETH_MDIO
        PA7     ------> ETH_CRS_DV
        PC4     ------> ETH_RXD0
        PC5     ------> ETH_RXD1
        PG11     ------> ETH_TX_EN
        PG13     ------> ETH_TXD0
        PG14     ------> ETH_TXD1
        */
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5);

        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7);

        HAL_GPIO_DeInit(GPIOG, GPIO_PIN_11 | GPIO_PIN_13 | GPIO_PIN_14);

        /* ETH interrupt Deinit */
        HAL_NVIC_DisableIRQ(ETH_IRQn);
        /* USER CODE BEGIN ETH_MspDeInit 1 */

        /* USER CODE END ETH_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */

int32_t ETH_PHY_INTERFACE_Init(void)
{
    /* Configure the MDIO Clock */
    HAL_ETH_SetMDIOClockRange(&heth);
    return 0;
}
int32_t ETH_PHY_INTERFACE_DeInit(void)
{
    return 0;
}
int32_t ETH_PHY_INTERFACE_ReadReg(uint32_t DevAddr, uint32_t RegAddr, uint32_t *pRegVal)
{
    if (HAL_ETH_ReadPHYRegister(&heth, DevAddr, RegAddr, pRegVal) != HAL_OK)
    {
        return -1;
    }
    return 0;
}
int32_t ETH_PHY_INTERFACE_WriteReg(uint32_t DevAddr, uint32_t RegAddr, uint32_t RegVal)
{
    if (HAL_ETH_WritePHYRegister(&heth, DevAddr, RegAddr, RegVal) != HAL_OK)
    {
        return -1;
    }
    return 0;
}
int32_t ETH_PHY_INTERFACE_GetTick(void)
{
    return HAL_GetTick();
}

void ETH_StartLink(void)
{
    ETH_MACConfigTypeDef MACConf = {0};
    int32_t PHYLinkState = 0U;
    uint32_t linkchanged = 0U, speed = 0U, duplex = 0U;
    PHYLinkState = LAN8720_GetLinkState(&lan8720);
    if (PHYLinkState <= LAN8720_STATUS_LINK_DOWN)
    {
        HAL_ETH_Stop(&heth);
    }
    else if (PHYLinkState > LAN8720_STATUS_LINK_DOWN)
    {
        switch (PHYLinkState)
        {
        case LAN8720_STATUS_100MBITS_FULLDUPLEX:
            duplex = ETH_FULLDUPLEX_MODE;
            speed = ETH_SPEED_100M;
            linkchanged = 1;
            break;
        case LAN8720_STATUS_100MBITS_HALFDUPLEX:
            duplex = ETH_HALFDUPLEX_MODE;
            speed = ETH_SPEED_100M;
            linkchanged = 1;
            break;
        case LAN8720_STATUS_10MBITS_FULLDUPLEX:
            duplex = ETH_FULLDUPLEX_MODE;
            speed = ETH_SPEED_10M;
            linkchanged = 1;
            break;
        case LAN8720_STATUS_10MBITS_HALFDUPLEX:
            duplex = ETH_HALFDUPLEX_MODE;
            speed = ETH_SPEED_10M;
            linkchanged = 1;
            break;
        default:
            break;
        }
        if (linkchanged)
        {
            HAL_ETH_GetMACConfig(&heth, &MACConf);
            MACConf.DuplexMode = duplex;
            MACConf.Speed = speed;
            MACConf.DropTCPIPChecksumErrorPacket = DISABLE;
            MACConf.ForwardRxUndersizedGoodPacket = ENABLE;
            HAL_ETH_SetMACConfig(&heth, &MACConf);
            HAL_ETH_Start_IT(&heth);
        }
    }
}

void ETH_ConstructEthernetFrame(ethernet_frame_t *frame, uint8_t *dest_mac, uint8_t *src_mac, uint8_t *type,
                                uint8_t *payload, uint16_t payload_len)
{
    // Copy the destination MAC address
    memcpy(frame->dest_mac, dest_mac, 6);
    // Copy the source MAC address
    memcpy(frame->src_mac, src_mac, 6);
    // Set the Ethernet type field
    memcpy(frame->type, type, 2);
    // Copy the payload data
    memcpy(frame->payload, payload, payload_len);
}

/**
 * @brief 使用 ETH 发送一个 packet, 若当前 ETH 繁忙, 会等待直到可用 (blocking)
 * @param pBuff: 要发送的数据指针
 * @param len: 要发送的数据长度
 * @retval 0=成功, -1=失败
 */
int ethSend(void *pBuff, int len)
{
    int retval = -1;
    // osSemaphoreAcquire(ethTxCpltSemaphore, osWaitForever);
    memcpy(txBuffer, pBuff, len);
    ETH_BufferTypeDef txBufferDef = {0};
    txBufferDef.buffer = txBuffer;
    txBufferDef.len = len;
    txBufferDef.next = NULL;

    osMutexAcquire(ethTxConfigMutex, osWaitForever);
    TxConfig.TxBuffer = &txBufferDef;
    if (HAL_ETH_Transmit_IT(&heth, &TxConfig) == HAL_OK)
    {
        printf("Eth tx start.\r\n");
        for (uint32_t i = 0; i < txBufferDef.len; i++) {
            printf("%02x ", ((uint8_t*)pBuff)[i]);
        }
        printf("\r\n");
        retval = 0;
    }
    else
    {
        retval = -1;
    }

    osMutexRelease(ethTxConfigMutex);
    return retval;
}

/**
 * @brief 对 ETH Send 线程发送一个 Send 请求，会将待发送的数据复制到内部缓存中
 * 
 * @param pBuff 指向待发送的数据
 * @param len 数据大小
 * @param timeout 超时时间, 0=none blocking
 * @return int 0=成功, -1=失败
 */
int ethSendRequest(void *pBuff, uint32_t len, uint32_t timeout) {
    ETH_AppBuff* appBuff = osMemoryPoolAlloc(txBufferPool, timeout);
    // appBuff->buffer
    if (appBuff) {
        memcpy(appBuff->buffer, pBuff, len);
        appBuff->AppBuff.buffer = appBuff->buffer;
        appBuff->AppBuff.len = len;
        appBuff->AppBuff.next = NULL;
        if (osMessageQueuePut(ethTxQueue, &appBuff, 0, timeout) == osOK) {
            return 0;
        }
    }

    return -1;
}

/**
 * @brief 供 ethSendThread 调用的阻塞发送函数, polling 模式
 * 
 * @param appBuff 
 * @param timeout 
 */
void ethSendProcess(ETH_AppBuff* appBuff, uint32_t timeout) {
    osStatus_t osStat;
    HAL_StatusTypeDef halStat;
    osStat = osMutexAcquire(ethTxConfigMutex, timeout);
    assert_param(osStat == osOK);
    TxConfig.TxBuffer = &appBuff->AppBuff;
    halStat = HAL_ETH_Transmit(&heth, &TxConfig, timeout);
    assert_param(halStat == HAL_OK);
    halStat = HAL_ETH_ReleaseTxPacket(&heth);
    assert_param(halStat == HAL_OK);

#ifdef DEBUG_MESSAGE
    // uint32_t len = appBuff->AppBuff.len;
    // uint8_t* pdata = appBuff->AppBuff.buffer;
    // printf("Transmit:\n");
    // for (uint32_t i = 0; i < len; i++) {
    //     printf("%02x ", *pdata++);
    // }
    // printf("\n");
#endif // DEBUG_MESSAGE

    osStat = osMemoryPoolFree(txBufferPool, appBuff);
    assert_param(osStat == osOK);
    osStat = osMutexRelease(ethTxConfigMutex);
    assert_param(osStat == osOK);
}

/**
 * @brief 接收数据
 *
 * @param pPacket 用于接收 packet 的指针,将被设置为指向 packet
 * @return int 返回 packet 的长度, 若失败返回 -1
 */
int ethReceive(void **pPacket)
{
    ETH_BufferTypeDef *pBuff;
    if (ethRxListPop(&pBuff) != 0)
    {
        return -1;
    }

    *pPacket = pBuff->buffer;
    return pBuff->len;
}

/**
 * @brief 对 ETH Receive 线程发送一个 Receive 请求，若成功，将数据复制到 pBuffer 指向的空间中
 *
 * @param pBuffer 指向接收 buffer 的指针
 * @param timeout 超时时间, 0=none blocking
 * @return int 接收到的数据长度, 若超时或失败返回 -1
 */
int ethReceiveRequest(void* pBuffer, uint32_t timeout) {
    ETH_AppBuff* appBuff = NULL;
    int len = -1;
    osMessageQueueGet(ethRxQueue, &appBuff, NULL, timeout);
    if (appBuff != NULL) {
        ETH_BufferTypeDef* pBuffDef = &appBuff->AppBuff;
        len = pBuffDef->len;
        if (len > 0) 
            memcpy(pBuffer, pBuffDef->buffer, pBuffDef->len);
        osMemoryPoolFree(rxBufferPool, appBuff);
    }

    return len;
}

int ethRxListPush(ETH_BufferTypeDef *pBuff)
{
    ETH_BufferTypeDef *temp;

    if (!pBuff)
    {
        return -1;
    }

    osMutexAcquire(ethRxListMutex, osWaitForever);
    if (rxList.len == 0)
    {
        rxList.head = pBuff;
        rxList.tail = pBuff;
        rxList.len++;
    }
    else
    {
        rxList.tail->next = pBuff;
        rxList.tail = pBuff;
        rxList.len++;
    }

    while ((temp = rxList.tail->next))
    {
        rxList.tail = temp;
        rxList.len++;
    }

    osMutexRelease(ethRxListMutex);

    return 0;
}

/**
 * @brief 弹出一个 RxBuffer
 *
 * @param pBuff 指向 ETH_BufferTypeDef 的指针
 * @return int 0=成功, -1=失败
 */
int ethRxListPop(ETH_BufferTypeDef **pBuff)
{
    osMutexAcquire(ethRxListMutex, osWaitForever);
    if (rxList.len == 0)
    {
        osMutexRelease(ethRxListMutex);
        return -1;
    }

    assert_param(rxList.head != NULL);

    *pBuff = rxList.head;

    if (rxList.head->next == NULL)
    {
        rxList.head = NULL;
        rxList.tail = NULL;
        rxList.len = 0;
    }
    else
    {
        rxList.head = rxList.head->next;
        rxList.len--;
    }
    osMutexRelease(ethRxListMutex);

    return 0;
}

/**
 * @brief 释放 buffer 存储空间
 *
 * @param pBuff packet 的内存地址（不是 ETH_BufferTypeDef 的内存地址）
 */
void ethRxBufferFree(void *pBuff)
{
    void *p = (uint8_t *)pBuff - offsetof(ETH_AppBuff, buffer);
    osMemoryPoolFree(rxBufferPool, p);
}

/* USER CODE END 1 */
