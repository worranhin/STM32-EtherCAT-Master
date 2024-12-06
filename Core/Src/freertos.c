/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "eth.h"
#include <stdio.h>
#include "oled.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

osMemoryPoolId_t rxBufferPool;

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for buttonTask */
osThreadId_t buttonTaskHandle;
const osThreadAttr_t buttonTask_attributes = {
  .name = "buttonTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartButtonTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of buttonTask */
  buttonTaskHandle = osThreadNew(StartButtonTask, NULL, &buttonTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */

	rxBufferPool = osMemoryPoolNew(ETH_RXBUFNB, ETH_RX_BUF_SIZE, NULL);
	assert_param(rxBufferPool != NULL);

	MX_ETH_Init(); // 这个初始化过程必须在线程中进行，否则会出现一些地址错误，原因暂不明，猜测是跟 RTOS 的内存管理相关

	ETH_BufferTypeDef TxBuffer;
	ethernet_frame_t frame;
	uint8_t dest_mac[] = {0x00, 0x80, 0xE1, 0x00, 0x00, 0x10}; // Destination MAC Address
	uint8_t src_mac[] = {0x00, 0x80, 0xE1, 0x00, 0x00, 0x00};  // Source MAC Address
	uint8_t type[] = {0x08,0x00 }; // EtherType set to IPV4 packet
	uint8_t payload[] = {0x54,0x65,0x73,0x74,0x69,0x6e,0x67,0x20,0x45,0x74,0x68,0x65,0x72,0x6e,0x65,0x74,0x20,0x6f,0x6e,0x20,0x53,0x54,0x4d,0x33,0x32};
	uint16_t payload_len = sizeof(payload);

	ETH_ConstructEthernetFrame(&frame, dest_mac, src_mac, type, payload, payload_len);
	TxBuffer.buffer = (uint8_t *)&frame;
	TxBuffer.len = sizeof(dest_mac) + sizeof(src_mac) + sizeof(type) + payload_len;
	TxBuffer.next = NULL;
	TxConfig.TxBuffer = &TxBuffer;

  /* Infinite loop */
  for(;;)
  {
	  HAL_StatusTypeDef status;
	  HAL_GPIO_TogglePin(LED5_GPIO_Port, LED5_Pin);
	  status = HAL_ETH_Transmit_IT( & heth, & TxConfig);
	  if (status != HAL_OK) {
		  uint32_t error = HAL_ETH_GetError(&heth);
		  printf("%lx", error);
		  if (error & HAL_ETH_ERROR_DMA) {
			  uint32_t dmaError = HAL_ETH_GetDMAError(&heth);
			  printf("DMA error: %lx", dmaError);
			  Error_Handler();
		  }
	  } else {
		  HAL_ETH_ReleaseTxPacket( & heth);
	  }
	  osDelay(1000);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartButtonTask */
/**
* @brief Function implementing the buttonTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartButtonTask */
void StartButtonTask(void *argument)
{
  /* USER CODE BEGIN StartButtonTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartButtonTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* 回调函数 */

void HAL_ETH_RxAllocateCallback(uint8_t ** buff) {
	ETH_AppBuff * pAppBuff = osMemoryPoolAlloc(rxBufferPool, 1000);
	if (pAppBuff) {
		ETH_BufferTypeDef * p = &(pAppBuff->AppBuff);
		*buff = pAppBuff->buffer;
		p->next = NULL;
		p->len = 0;
		p->buffer = *buff;
	} else {
		*buff = NULL;
	}
}

void HAL_ETH_RxLinkCallback(void ** pStart, void ** pEnd, uint8_t * buff, uint16_t Length)
{
	  ETH_BufferTypeDef ** ppStart = (ETH_BufferTypeDef ** ) pStart;
	  ETH_BufferTypeDef ** ppEnd = (ETH_BufferTypeDef ** ) pEnd;
	  ETH_BufferTypeDef * p = NULL;
	  p = (ETH_BufferTypeDef * )(buff - offsetof(ETH_AppBuff, buffer));
	  p -> next = NULL;
	  p -> len = Length;
	  p->buffer = buff;

	  if (! * ppStart)
	  {
	    * ppStart = p;
	  } else
	  {
	    ( * ppEnd) -> next = p;
	  }
	  * ppEnd = p;
}

void HAL_ETH_TxCpltCallback(ETH_HandleTypeDef * heth)
{
//  printf("Packet Transmitted successfully!\r\n");
//  fflush(0);
	oledLogClear();
	oledLog("Packet Transmitted successfully! ");

}
void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef * heth)
{
	ETH_BufferTypeDef * pBuff;
	HAL_StatusTypeDef status;
	status = HAL_ETH_ReadData(heth, (void**)&pBuff);
	if (status != HAL_OK) {
		Error_Handler();
	}
	if (pBuff) {
		oledLogClear();
		oledLog("Received: ");
		for (int i = 0; i < pBuff->len; i++) {
			oledLog((char*)(&(pBuff->buffer[i])));
		}
	}
	osMemoryPoolFree(rxBufferPool, pBuff);
}

/* USER CODE END Application */

