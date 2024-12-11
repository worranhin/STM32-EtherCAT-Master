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

#define RX_BUFFER_POOL_SIZE 20

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

osMemoryPoolId_t rxBufferPool;

osMutexId_t ethTxBuffMutex;
osMutexAttr_t ecTxBuffMutex_attributes = {
		.name = "ethTxBuffMutex"
};

osMutexId_t oledMutex;
osMutexAttr_t oledMutex_attributes = {
		.name = "oledMutex"
};

osMutexId_t ethRxListMutex;
osMutexAttr_t ethRxListMutex_attributes = {
		.name = "ethRxListMutex"
};

osMutexId_t ethTxConfigMutex;
osMutexAttr_t ethTxConfigMutex_attributes = {
		.name = "ethTxConfigMutex"
};

// osSemaphoreId_t tim14ExpireSemaphore;
// const osSemaphoreAttr_t tim14ExpireSemaphore_attributes = {
// 		.name = "tim14ExpireSemaphore",
// 		.attr_bits = 0
// };

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
    .priority = (osPriority_t)osPriorityNormal,
};

osThreadId_t ledTaskHandle;
const osThreadAttr_t ledTask_attributes = {
    .name = "ledTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityRealtime,
};

osThreadId_t ethercatTaskHandle;
const osThreadAttr_t ethercatTask_attributes = {
    .name = "ethercatTask",
    .stack_size = 2048 * 4,
    .priority = (osPriority_t)osPriorityNormal,
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
static uint8_t addSum(const uint8_t *pData, int len);
static bool checkSum(const uint8_t *pData, int len, uint8_t target);

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

  oledLogClear();
  oledLog("Starting FreeRTOS");

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
    ethTxBuffMutex = osMutexNew(&ecTxBuffMutex_attributes);
    oledMutex = osMutexNew(&oledMutex_attributes);
    ethRxListMutex = osMutexNew(&ethRxListMutex_attributes);
    ethTxConfigMutex = osMutexNew(&ethTxConfigMutex_attributes);
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
    // tim14ExpireSemaphore = osSemaphoreNew(1, 0, &tim14ExpireSemaphore_attributes);
    ethTxCpltSemaphore = osSemaphoreNew(1, 1, &ethTxCpltSemaphore_attributes);
    ethRxCpltSemaphore = osSemaphoreNew(RX_BUFFER_POOL_SIZE, 0, &ethRxCpltSemaphore_attributes);
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
    UNUSED(argument);

    rxBufferPool = osMemoryPoolNew(RX_BUFFER_POOL_SIZE, sizeof(ETH_AppBuff), NULL);
    assert_param(rxBufferPool != NULL);

    MX_ETH_Init(); // 这个初始化过程必须在线程中进行，否则会出现一些地址错误，原因暂不明，猜测是跟
                   // RTOS 的内存管理相关
    MX_UART4_Init();
    printf("Uart setup done.\n");
    //	osDelay(5000);

    ledTaskHandle = osThreadNew(StartLedTask, NULL, &ledTask_attributes);
    //	usartTaskHandle = osThreadNew(StartUsartTask, NULL,
    //&usartTask_attributes);
    ethercatTaskHandle = osThreadNew(StartEthercatTask, NULL, &ethercatTask_attributes);

    oledLogClear();
    oledLog("Initialization done.");

    osThreadExit();

    /* Infinite loop */
    for (;;)
    {
    }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */



/**
 * @brief Function implementing the UsartTask thread.
 * @param argument: Not used
 * @retval None
 */
void StartUsartTask(void *argument)
{
    UNUSED(argument);
    const uint32_t rxTimeout = 100;
    const uint32_t txTimeout = 100;
    //	char txData[] = "Hello world!";
    uint8_t uartRxBuffer[20] = {0};
    uint8_t txBuff[20] = {0};
    HAL_StatusTypeDef status;
    uint16_t rxLength = 0;

    //	HAL_UART_Transmit(&huart4, (uint8_t *)txData, sizeof(txData), 1000);

    /* Infinite loop */
    for (;;)
    {
        //	  status = HAL_UART_Receive(&huart4, uartRxBuffer, 4, rxTimeout);
        status = HAL_UARTEx_ReceiveToIdle(&huart4, uartRxBuffer, sizeof(uartRxBuffer), &rxLength, rxTimeout);
        if (status != HAL_ERROR && uartRxBuffer[0] == 0xA4 && rxLength > 0)
        {
            uint8_t command = uartRxBuffer[1];
            uint8_t dataLength = uartRxBuffer[2];
            uint8_t headSum = uartRxBuffer[3];
            if (!checkSum(&uartRxBuffer[0], 3, headSum))
            {
                // 处理帧头接收错误
                continue;
            }

            switch (command)
            {
            case 0x01: // 获取当前状态
                memcpy(txBuff, uartRxBuffer, 4);
                txBuff[2] = 1;
                txBuff[3] = 0xA6;
                txBuff[4] = 0;
                txBuff[5] = 0;

                HAL_UART_Transmit(&huart4, txBuff, 4 + 2, txTimeout);
                break;

            case 0x11: // 写入目标位置
                if (rxLength >= 4 + dataLength + 1)
                {
                    uint8_t *pData = &uartRxBuffer[4];
                    uint8_t dataSum = pData[dataLength];
                    if (!checkSum(pData, dataLength, dataSum))
                    {
                        // 处理数据错误
                        continue;
                    }

                    int32_t *pPosition = (int32_t *)pData;
                    pdoOutputs->targetPostion = *pPosition;

                    memcpy(txBuff, uartRxBuffer, 4);
                    txBuff[2] = 1;
                    txBuff[3] = addSum(txBuff, 3);
                    txBuff[4] = 0;
                    txBuff[5] = 0;

                    HAL_UART_Transmit(&huart4, txBuff, 4 + 2, txTimeout);
                }
                break;

            default:
                break;
            }
        }
        else
        {
            osDelay(1);
        }
    }
}

void StartLedTask(void *argument)
{
    UNUSED(argument);
    for (;;)
    {
        HAL_GPIO_TogglePin(LED5_GPIO_Port, LED5_Pin);
        osDelay(1000);
    }
}

void StartEthercatTask(void *argument)
{
    UNUSED(argument);
    bool isRunning = FALSE;
    int SV630N_idx = 0;

    oledLogClear();
    oledLog("SOEM initializing.");

    if (ec_init("eth0") > 0)
    {
        oledLogClear();
        oledLog("SOEM configuring.");
        if (ec_config_init(FALSE) > 0)
        {
            // convert ec_slavecount to string
            char ec_slavecount_str[10];
            sprintf(ec_slavecount_str, "%d", ec_slavecount);
            oledLogClear();
            oledLog("Slaves found: ");
            oledLog(ec_slavecount_str);

            printf("%lu slaves found and configured.%lu \r\n",ec_slave[1].eep_man,ec_slave[1].eep_id);

            /* link slave specific setup to preop->safeop hook */
            for (int i = 1; i <= ec_slavecount; i++)
            {
                if (ec_slave[i].eep_man == SV630N_MANUFATURER_ID && ec_slave[i].eep_id == SV630N_PRODUCT_ID)
                {
                    ec_slave[i].PO2SOconfig = SV630N_Setup; // 设置回调函数
                    SV630N_idx = i;
                }
            }

            // config IOmap
            usedMemory = ec_config_map(&IOmap);
            if ((unsigned int)usedMemory > sizeof(IOmap))
            {
                oledLogClear();
                oledLog("Need more IOmap space.");
                Error_Handler(); // 需要增大 IOmap 的空间
            }

            if (SV630N_idx)
            {
                pdoOutputs = (SV630N_Outputs *)ec_slave[SV630N_idx].outputs;
                pdoInputs = (SV630N_Inputs *)ec_slave[SV630N_idx].inputs;
            }

            /* send one valid process data to make outputs in slaves happy*/
            ec_send_processdata();
            ec_receive_processdata(EC_TIMEOUTRET);

            ec_writestate(0);
            /* wait for all slaves to reach OP state */
            uint16 state = ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE);
            if (state == EC_STATE_OPERATIONAL)
            {
                // CiA402 状态切换过程
                while ((pdoInputs->statusWord & 0x03df) == 0x0250)
                {
                    pdoOutputs->controlword |= 0x0006;
                    ec_send_processdata();
                    ec_receive_processdata(EC_TIMEOUTRET);
                }

                while ((pdoInputs->statusWord & 0x03ff) == 0x0231)
                {
                    pdoOutputs->controlword |= 0x0007;
                    ec_send_processdata();
                    ec_receive_processdata(EC_TIMEOUTRET);
                }

                while ((pdoInputs->statusWord & 0x03ff) == 0x0233)
                {
                    pdoOutputs->controlword |= 0x000F;
                    ec_send_processdata();
                    ec_receive_processdata(EC_TIMEOUTRET);
                }

                if ((pdoInputs->statusWord & 0x03ff) == 0x0237)
                {
                    isRunning = TRUE;
                    oledLogClear();
                    oledLog("Slaves are running.");
                }
            }
        }
        else
        {
            oledLogClear();
            oledLog("No slave found");
        }
    }
    /* Infinite loop */
    for (;;)
    {
        if (isRunning)
        {
            ec_send_processdata();
            ec_receive_processdata(EC_TIMEOUTRET);
        }
        else
        {
            osThreadExit();
        }
    }
}

/**
 * @brief 计算校验和
 * @param pData: 数据指针，指向待加和的数组
 * @param len: 数组的大小（元素的个数，而不是内存大小）
 * @retval 加和的值
 */
uint8_t addSum(const uint8_t *pData, int len)
{
    uint8_t sum = 0;
    for (int i = 0; i < len; i++)
    {
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
bool checkSum(const uint8_t *pData, int len, uint8_t target)
{
    uint8_t sum = addSum(pData, len);
    return (sum == target);
}

/* 回调函数 */

void HAL_ETH_RxAllocateCallback(uint8_t **buff)
{
    ETH_AppBuff *pAppBuff = osMemoryPoolAlloc(rxBufferPool, 1000); // TODO:　这里从 IQR 调用会出问题，需要立即修复
    if (pAppBuff)
    {
        ETH_BufferTypeDef *p = &(pAppBuff->AppBuff);
        *buff = pAppBuff->buffer;
        p->next = NULL;
        p->len = 0;
        p->buffer = *buff;
        uint32_t left = osMemoryPoolGetSpace(rxBufferPool);
        printf("Eth rx alloc. left=%lu\r\n", left);
    }
    else
    {
        *buff = NULL;
        uint32_t left = osMemoryPoolGetSpace(rxBufferPool);
        printf("Eth rx alloc failed. left=%lu\r\n", left);
    }
}

void HAL_ETH_RxLinkCallback(void **pStart, void **pEnd, uint8_t *buff, uint16_t Length)
{
    ETH_BufferTypeDef **ppStart = (ETH_BufferTypeDef **)pStart;
    ETH_BufferTypeDef **ppEnd = (ETH_BufferTypeDef **)pEnd;
    ETH_BufferTypeDef *p = NULL;
    p = (ETH_BufferTypeDef *)(buff - offsetof(ETH_AppBuff, buffer));
    p->next = NULL;
    p->len = Length;
    p->buffer = buff;

    if (!*ppStart)
    {
        *ppStart = p;
    }
    else
    {
        (*ppEnd)->next = p;
    }
    *ppEnd = p;

    printf("Eth rx link, len=%d\r\n", Length);
}

void HAL_ETH_TxCpltCallback(ETH_HandleTypeDef *heth)
{
    UNUSED(heth);
    // printf("Eth tx cplt.\r\n");
    // osMutexRelease(ethTxBuffMutex);
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
void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth)
{
    ETH_BufferTypeDef *pBuff;
    HAL_StatusTypeDef status = HAL_ETH_ReadData(heth, (void**)&pBuff);
    if (status != HAL_OK) {
    	// while(1);
    	Error_Handler();
    }
    if (pBuff) {
        ethRxListPush(pBuff);
        printf("Rx %lu bytes\n", pBuff->len);
        for (uint32_t i = 0; i < pBuff->len; i++) {
            printf("%02x ", pBuff->buffer[i]);
        }
        printf("\r\n");
    }
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
    if ((HAL_ETH_GetDMAError(heth) & ETH_DMASR_RBUS) == ETH_DMASR_RBUS)
    {
        osSemaphoreRelease(ethRxCpltSemaphore);
    }
}

/* USER CODE END Application */

