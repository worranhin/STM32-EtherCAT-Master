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
#include "ecatuser.h"
#include "SV630N.h"
#include <stdbool.h>
#include "usart.h"
#include <string.h>

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

osMutexId_t ecTxBuffMutex;
osMutexAttr_t ecTxBuffMutex_attributes = {
		.name = "ecTxBuff"
};

osSemaphoreId_t tim14ExpireSemaphore;
const osSemaphoreAttr_t tim14ExpireSemaphore_attributes = {
		.name = "tim14ExpireSemaphore",
		.attr_bits = 0
};

osSemaphoreId_t ethTxCpltSemaphore;
const osSemaphoreAttr_t ethTxCpltSemaphore_attributes = {
		.name = "ethTxCpltSemaphore",
		.attr_bits = 0
};

osSemaphoreId_t ethRxCpltSemaphore;
const osSemaphoreAttr_t ethRxCpltSemaphore_attributes = {
		.name = "ethRxCpltSemaphore",
		.attr_bits = 0
};

osThreadId_t usartTaskHandle;
const osThreadAttr_t usartTask_attributes = {
  .name = "usartTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t ledTaskHandle;
const osThreadAttr_t ledTask_attributes = {
  .name = "ledTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityRealtime,
};

osThreadId_t ethercatTaskHandle;
const osThreadAttr_t ethercatTask_attributes = {
  .name = "ethercatTask",
  .stack_size = 2048 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

static char IOmap[128];
int usedMemory;
SV630N_Outputs *pdoOutputs;
SV630N_Inputs *pdoInputs;


/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

void StartUsartTask(void *argument);
void StartLedTask(void *argument);
void StartEthercatTask(void *argument);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

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
	ecTxBuffMutex = osMutexNew(&ecTxBuffMutex_attributes);
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
	tim14ExpireSemaphore = osSemaphoreNew(1, 0, &tim14ExpireSemaphore_attributes);
	ethTxCpltSemaphore = osSemaphoreNew(1, 1, &ethTxCpltSemaphore_attributes);
	ethRxCpltSemaphore = osSemaphoreNew(1, 0, &ethRxCpltSemaphore_attributes);
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
	MX_UART4_Init();
//	osDelay(5000);

	ledTaskHandle = osThreadNew(StartLedTask, NULL, &ledTask_attributes);
	usartTaskHandle = osThreadNew(StartUsartTask, NULL, &usartTask_attributes);
	ethercatTaskHandle = osThreadNew(StartEthercatTask, NULL, &ethercatTask_attributes);

	osThreadExit();

//	ecat_init();

//	ETH_BufferTypeDef TxBuffer;
//	ethernet_frame_t frame;
//	uint8_t dest_mac[] = {0x00, 0x80, 0xE1, 0x00, 0x00, 0x10}; // Destination MAC Address
//	uint8_t src_mac[] = {0x00, 0x80, 0xE1, 0x00, 0x00, 0x00};  // Source MAC Address
//	uint8_t type[] = {0x08,0x00 }; // EtherType set to IPV4 packet
//	uint8_t payload[] = {0x54,0x65,0x73,0x74,0x69,0x6e,0x67,0x20,0x45,0x74,0x68,0x65,0x72,0x6e,0x65,0x74,0x20,0x6f,0x6e,0x20,0x53,0x54,0x4d,0x33,0x32};
//	uint16_t payload_len = sizeof(payload);
//
//	ETH_ConstructEthernetFrame(&frame, dest_mac, src_mac, type, payload, payload_len);
//	TxBuffer.buffer = (uint8_t *)&frame;
//	TxBuffer.len = sizeof(dest_mac) + sizeof(src_mac) + sizeof(type) + payload_len;
//	TxBuffer.next = NULL;
//	TxConfig.TxBuffer = &TxBuffer;

  /* Infinite loop */
  for(;;)
  {
//	  ecat_loop();
//	  HAL_StatusTypeDef status;
//	  HAL_GPIO_TogglePin(LED5_GPIO_Port, LED5_Pin);
//	  status = HAL_ETH_Transmit_IT( & heth, & TxConfig);
//	  if (status != HAL_OK) {
//		  uint32_t error = HAL_ETH_GetError(&heth);
//		  printf("%lx", error);
//		  if (error & HAL_ETH_ERROR_DMA) {
//			  uint32_t dmaError = HAL_ETH_GetDMAError(&heth);
//			  printf("DMA error: %lx", dmaError);
//			  Error_Handler();
//		  }
//	  } else {
//		  HAL_ETH_ReleaseTxPacket( & heth);
//	  }
//	  osDelay(1000);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/**
 * @brief 计算校验和
 * @param pData: 数据指针，指向待加和的数组
 * @param len: 数组的大小（元素的个数，而不是内存大小）
 * @retval 加和的值
 */
uint8_t addSum(const uint8_t *pData, int len) {
	uint8_t sum = 0;
	for (int i = 0; i < len; i++) {
		sum += pData[i];
	}

	return sum;
}

/**
 * @brief 校验和
 * @param pData: 数据指针，指向待加和的数组
 * @param len: 数组的大小（元素的个数，而不是内存大小）
 * @param target: 目标的值
 * @retval true=数组加和等于 target, false=其它
 */
bool checkSum(const uint8_t *pData, int len, uint8_t target) {
	uint8_t sum = addSum(pData, len);
	return (sum == target);
}

/**
* @brief Function implementing the UsartTask thread.
* @param argument: Not used
* @retval None
*/
void StartUsartTask(void *argument)
{
	const uint32_t rxTimeout = 100;
	const uint32_t txTimeout = 100;
//	char txData[] = "Hello world!";
	uint8_t uartRxBuffer[20] = {0};
	uint8_t txBuff[20] = {0};
	HAL_StatusTypeDef status;
	uint16_t rxLength = 0;

//	HAL_UART_Transmit(&huart4, (uint8_t *)txData, sizeof(txData), 1000);
	
  /* Infinite loop */
  for(;;)
  {
//	  status = HAL_UART_Receive(&huart4, uartRxBuffer, 4, rxTimeout);
	  status = HAL_UARTEx_ReceiveToIdle(&huart4, uartRxBuffer, sizeof(uartRxBuffer), &rxLength, rxTimeout);
	  if (status != HAL_ERROR && uartRxBuffer[0] == 0xA4 && rxLength > 0) {
		  uint8_t command = uartRxBuffer[1];
		  uint8_t dataLength = uartRxBuffer[2];
		  uint8_t headSum = uartRxBuffer[3];
		  if(!checkSum(&uartRxBuffer[0], 3, headSum)) {
			  // 处理帧头接收错误
			  continue;
		  }

		  switch(command) {
		  case 0x01: // 获取当前状态
			  memcpy(txBuff, uartRxBuffer, 4);
			  txBuff[2] = 1;
			  txBuff[3] = 0xA6;
			  txBuff[4] = 0;
			  txBuff[5] = 0;

			  HAL_UART_Transmit(&huart4, txBuff, 4+2, txTimeout);
			  break;

		  case 0x11: // 写入目标位置
			  if (rxLength >= 4+dataLength+1) {
				  uint8_t* pData = &uartRxBuffer[4];
				  uint8_t dataSum = pData[dataLength];
				  if (!checkSum(pData, dataLength, dataSum)) {
					  // 处理数据错误
					  continue;
				  }

				  int32_t* pPosition = (int32_t*)pData;
				  pdoOutputs->targetPostion = *pPosition;

				  memcpy(txBuff, uartRxBuffer, 4);
				  txBuff[2] = 1;
				  txBuff[3] = addSum(txBuff, 3);
				  txBuff[4] = 0;
				  txBuff[5] = 0;

				  HAL_UART_Transmit(&huart4, txBuff, 4+2, txTimeout);
			  }
			  break;

		  default:
			  break;
		  }
	  } else {
		  osDelay(1);
	  }
  }
}

void StartLedTask(void *argument)
{
	for(;;)
	{
		HAL_GPIO_TogglePin(LED5_GPIO_Port, LED5_Pin);
		osDelay(1000);
	}
}

void StartEthercatTask(void *argument)
{
	bool isRunning = FALSE;
	int SV630N_idx = 0;

	if (ec_init("eth0") > 0) {
		if (ec_config_init(FALSE) > 0) {
			oledLogClear();
			oledLog("Slaves found.");
			printf("%d slaves found and configured.\n",ec_slavecount);

			/* link slave specific setup to preop->safeop hook */
			for(int i = 1; i <= ec_slavecount; i++) {
				if (ec_slave[i].eep_man == SV630N_MANUFATURER_ID && ec_slave[i].eep_id == SV630N_PRODUCT_ID) {
					ec_slave[i].PO2SOconfig = SV630N_Setup;  // 设置回调函数
					SV630N_idx = i;
				}
			}

			// config IOmap
			usedMemory = ec_config_map(&IOmap);
			if (usedMemory > sizeof(IOmap)) {
				oledLogClear();
				oledLog("Need more IOmap space.");
				Error_Handler(); // 需要增大 IOmap 的空间
			}

			if (SV630N_idx) {
				pdoOutputs = (SV630N_Outputs*)ec_slave[SV630N_idx].outputs;
				pdoInputs = (SV630N_Inputs*)ec_slave[SV630N_idx].inputs;
			}

			/* send one valid process data to make outputs in slaves happy*/
			ec_send_processdata();
			ec_receive_processdata(EC_TIMEOUTRET);

			ec_writestate(0);
			/* wait for all slaves to reach OP state */
			uint16 state = ec_statecheck(0, EC_STATE_OPERATIONAL,  EC_TIMEOUTSTATE);
			if (state == EC_STATE_OPERATIONAL) {
				// CiA402 状态切换过程
				while ((pdoInputs->statusWord & 0x03df) == 0x0250) {
					pdoOutputs->controlword |= 0x0006;
					ec_send_processdata();
					ec_receive_processdata(EC_TIMEOUTRET);
				}

				while ((pdoInputs->statusWord & 0x03ff) == 0x0231) {
					pdoOutputs->controlword |= 0x0007;
					ec_send_processdata();
					ec_receive_processdata(EC_TIMEOUTRET);
				}

				while ((pdoInputs->statusWord & 0x03ff) == 0x0233) {
					pdoOutputs->controlword |= 0x000F;
					ec_send_processdata();
					ec_receive_processdata(EC_TIMEOUTRET);
				}

				if ((pdoInputs->statusWord & 0x03ff) == 0x0237) {
					isRunning = TRUE;
					oledLogClear();
					oledLog("Slaves are running.");
				}
			}
		} else {
			oledLogClear();
			oledLog("No slave found");
		}
	}
  /* Infinite loop */
  for(;;)
  {
	  if (isRunning) {
		  ec_send_processdata();
		  ec_receive_processdata(EC_TIMEOUTRET);
	  } else {
		  osThreadExit();
	  }
  }
}


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
	osSemaphoreRelease(ethTxCpltSemaphore);
//	oledLog("Eth tx cplt ");
	// const char* str = "Eth tx cplt.";
	// HAL_UART_Transmit(&huart4, (uint8_t*)str, strlen(str), 100);
//	osMutexRelease(ecTxBuffMutex);
//  printf("Packet Transmitted successfully!\r\n");
//  fflush(0);
//	oledLogClear();
//	oledLog("Packet Transmitted successfully! ");

}
void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef * heth)
{
	osSemaphoreRelease(ethRxCpltSemaphore);
//	ETH_BufferTypeDef * pBuff;
//	HAL_StatusTypeDef status;
//	status = HAL_ETH_ReadData(heth, (void**)&pBuff);
//	if (status != HAL_OK) {
//		uint32_t err = HAL_ETH_GetError(heth);
//		if (err & HAL_ETH_ERROR_DMA) {
//			err = HAL_ETH_GetDMAError(heth);
//			Error_Handler();
//		} else if (err & HAL_ETH_ERROR_MAC) {
//			err = HAL_ETH_GetMACError(heth);
//			Error_Handler();
//		} else if (err & HAL_ETH_ERROR_PARAM) {
//			Error_Handler();
//		}
//	}
//
//	if (pBuff && pBuff->len > 0) {
//		oledLogClear();
//		oledLog("Received: ");
//		for (int i = 0; i < pBuff->len; i++) {
//			oledLog((char*)(&(pBuff->buffer[i])));
//		}
//	}
//	osMemoryPoolFree(rxBufferPool, pBuff);
}

void HAL_ETH_ErrorCallback(ETH_HandleTypeDef *heth)
{
  if((HAL_ETH_GetDMAError(heth) & ETH_DMASR_RBUS) == ETH_DMASR_RBUS)
  {
     osSemaphoreRelease(ethRxCpltSemaphore);
  }
}

/* USER CODE END Application */

