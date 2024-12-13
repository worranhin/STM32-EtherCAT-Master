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

#define RX_BUFFER_POOL_SIZE 10
#define TX_BUFFER_POOL_SIZE 4

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

// CMSIS-RTOS Related

osMutexId_t ethTxBuffMutex;
const osMutexAttr_t ecTxBuffMutex_attributes = {
		.name = "ethTxBuffMutex"
};

osMutexId_t oledMutex;
const osMutexAttr_t oledMutex_attributes = {
		.name = "oledMutex"
};

osMutexId_t ethRxListMutex;
const osMutexAttr_t ethRxListMutex_attributes = {
		.name = "ethRxListMutex"
};

osMutexId_t ethTxConfigMutex;
const osMutexAttr_t ethTxConfigMutex_attributes = {
		.name = "ethTxConfigMutex"
};

// osMutexId_t uartMutex;
// const osMutexAttr_t uartMutex_attributes = {
// 		.name = "uartMutex"
// };

// osSemaphoreId_t tim14ExpireSemaphore;
// const osSemaphoreAttr_t tim14ExpireSemaphore_attributes = {
// 		.name = "tim14ExpireSemaphore",
// 		.attr_bits = 0
// };

// osSemaphoreId_t ethTxReqSemaphore;
// const osSemaphoreAttr_t ethTxReqSemaphore_attributes = {
// 		.name = "ethTxReqSemaphore",
// 		.attr_bits = 0
// };

// osSemaphoreId_t ethTxCpltSemaphore;
// const osSemaphoreAttr_t ethTxCpltSemaphore_attributes = {
// 		.name = "ethTxCpltSemaphore",
// 		.attr_bits = 0
// };

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

osThreadId_t ethInitTaskHandle;
const osThreadAttr_t ethInitTask_attributes = {
    .name = "ethInitTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

osThreadId_t ethReceiveTaskHandle;
const osThreadAttr_t ethReceiveTask_attributes = {
    .name = "ethReceiveTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityRealtime,
};

osThreadId_t ethSendTaskHandle;
const osThreadAttr_t ethSendTask_attributes = {
    .name = "ethSendTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityRealtime,
};

osThreadId_t ethercatTaskHandle;
const osThreadAttr_t ethercatTask_attributes = {
    .name = "ethercatTask",
    .stack_size = 2048 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

osThreadId_t ethercatRtTaskHandle;
const osThreadAttr_t ethercatRtTask_attributes = {
    .name = "ethercatRtTask",
    .stack_size = 512 * 4,
    .priority = (osPriority_t)osPriorityRealtime,
};

osMemoryPoolId_t rxBufferPool;
osMemoryPoolId_t txBufferPool;

osMessageQueueId_t ethRxQueue;
const osMessageQueueAttr_t ethRxQueue_attributes = {
    .name = "ethRxQueue",
    .attr_bits = 0
};

osMessageQueueId_t ethTxQueue;
const osMessageQueueAttr_t ethTxQueue_attributes = {
    .name = "ethTxQueue",
    .attr_bits = 0
};


static char IOmap[128];
int usedMemory;
SV630N_Outputs *pdoOutputs;
SV630N_Inputs *pdoInputs;
bool ecIsRunning = FALSE;

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

void StartUsartTask(void *argument);
void StartLedTask(void *argument);
void StartEthInitTask(void *argument);
void StartEthReceiveTask(void *argument);
void StartEthSendTask(void *argument);
void StartEthercatTask(void *argument);
void StartEthercatRtTask(void *argument);
static uint8_t addSum(const uint8_t *pData, int len);
static bool checkSum(const uint8_t *pData, int len, uint8_t target);
void switchState();

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{
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
    // ethTxReqSemaphore = osSemaphoreNew(1, 0, &ethTxReqSemaphore_attributes);
    // ethTxCpltSemaphore = osSemaphoreNew(1, 1, &ethTxCpltSemaphore_attributes);
    ethRxCpltSemaphore = osSemaphoreNew(RX_BUFFER_POOL_SIZE, 0, &ethRxCpltSemaphore_attributes);
    /* USER CODE END RTOS_SEMAPHORES */

    /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
    /* USER CODE END RTOS_TIMERS */

    /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
    ethRxQueue = osMessageQueueNew(RX_BUFFER_POOL_SIZE, sizeof(ETH_AppBuff*), &ethRxQueue_attributes);
    ethTxQueue = osMessageQueueNew(TX_BUFFER_POOL_SIZE, sizeof(ETH_AppBuff*), &ethTxQueue_attributes);
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
    txBufferPool = osMemoryPoolNew(TX_BUFFER_POOL_SIZE, sizeof(ETH_AppBuff), NULL);
    assert_param(rxBufferPool != NULL);
    assert_param(txBufferPool != NULL);

    MX_UART4_Init();
    printf("Uart setup done.\n");
    //	osDelay(5000);

    ledTaskHandle = osThreadNew(StartLedTask, NULL, &ledTask_attributes);
    ethInitTaskHandle = osThreadNew(StartEthInitTask, NULL, &ethInitTask_attributes);
    //	usartTaskHandle = osThreadNew(StartUsartTask, NULL,
    //&usartTask_attributes);
    

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

void StartEthInitTask(void *argument) 
{
    UNUSED(argument);
    MX_ETH_Init(); // 这个初始化过程必须在线程中进行，否则会出现一些地址错误，原因暂不明，猜测是跟 RTOS 的内存管理相关
    ethSendTaskHandle = osThreadNew(StartEthSendTask, NULL, &ethSendTask_attributes);
    ethReceiveTaskHandle = osThreadNew(StartEthReceiveTask, NULL, &ethReceiveTask_attributes);
    ethercatRtTaskHandle = osThreadNew(StartEthercatRtTask, NULL, &ethercatRtTask_attributes);
    ethercatTaskHandle = osThreadNew(StartEthercatTask, NULL, &ethercatTask_attributes);
    
    osThreadExit();
    for (;;);
}

void StartEthSendTask(void *argument) {
    UNUSED(argument);

    ETH_AppBuff *appBuff = NULL;

    for (;;)
    {
        // osSemaphoreAcquire(ethTxReqSemaphore, osWaitForever);
        // osSemaphoreRelease(ethTxCpltSemaphore);
        osMessageQueueGet(ethTxQueue, &appBuff, NULL, osWaitForever);
        ethSendProcess(appBuff, EC_TIMEOUTRET);
    }
}

void StartEthReceiveTask(void *argument)
{
    const unsigned int EthTimeout = 100;
    UNUSED(argument);
    ETH_AppBuff *appBuff = NULL;
    ETH_BufferTypeDef *pBuffDef = NULL;
    HAL_StatusTypeDef status = HAL_OK;

    for (;;)
    {
        osSemaphoreAcquire(ethRxCpltSemaphore, osWaitForever); // 等待 ETH 硬件接收完成通知
        status = HAL_ETH_ReadData(&heth, (void **)&pBuffDef);
        assert_param(status == HAL_OK);
        if (pBuffDef)
        {
            appBuff = (ETH_AppBuff *)pBuffDef - offsetof(ETH_AppBuff, AppBuff);

#ifdef DEBUG_MESSAGE
            // uint32_t len = appBuff->AppBuff.len;
            // uint8_t *pdata = appBuff->AppBuff.buffer;
            // printf("Received:\n");
            // for (uint32_t i = 0; i < len; i++)
            // {
            //     printf("%02x ", *pdata++);
            // }
            // printf("\n");
#endif // DEBUG_MESSAGE

            if (osMessageQueuePut(ethRxQueue, &appBuff, 0, EthTimeout) == osErrorTimeout)
            {
                ETH_AppBuff *temp = NULL;
                osMessageQueueGet(ethRxQueue, &temp, NULL, EthTimeout);
                osMemoryPoolFree(rxBufferPool, temp);
                osMessageQueuePut(ethRxQueue, &appBuff, 0, EthTimeout);
            }
        }
    }
}

void StartEthercatTask(void *argument)
{
#define SYNC0TIME 1000000
    UNUSED(argument);
    int SV630N_idx = 0;
    uint16_t ecState = 0;

    oledLogClear();
    oledLog("SOEM initializing.");

    if (ec_init("eth0") > 0)
    {
        oledLogClear();
        oledLog("SOEM configuring.");
        
        if (ec_config_init(FALSE) > 0) // 进入 PRE-OP 状态
        {
            ecState = ec_statecheck(0, EC_STATE_PRE_OP, EC_TIMEOUTSTATE); // 现在应该处于 PRE-OP 状态
            DEBUG_PRINT("EC state: %u\n", ecState);

            char ec_slavecount_str[20];
            sprintf(ec_slavecount_str, "Slaves found: %d", ec_slavecount);  // convert ec_slavecount to string
            oledLogClear();
            oledLog(ec_slavecount_str);

            /* link slave specific setup to preop->safeop hook */
            for (int i = 1; i <= ec_slavecount; i++)
            {
                if (ec_slave[i].eep_man == SV630N_MANUFATURER_ID && ec_slave[i].eep_id == SV630N_PRODUCT_ID)
                {
                    ec_slave[i].PO2SOconfig = SV630N_Setup; // 设置回调函数
                    SV630N_idx = i;
                    DEBUG_PRINT("SV630N found and configured.\r\n");
                }
            }

            // 配置 DC 时钟
            if (ec_configdc()) {
                DEBUG_PRINT("DC configured.\r\n");
            }

            ecIsRunning = TRUE;
            uint16 slaveh = ecx_context.slavelist[SV630N_idx].configadr;
            uint32_t dcDiff = 0xffffffff;
            osDelay(10000); // 让子弹飞一会

            while(dcDiff > 1000000) {
                ec_FPRD(slaveh, ECT_REG_DCSYSDIFF, sizeof(dcDiff), &dcDiff, EC_TIMEOUTRET);
                bool localTimeIsSmaller = ((dcDiff & 0x80000000) >> 31);
                dcDiff = dcDiff & 0x7fffffff;
                if (localTimeIsSmaller) {
                    DEBUG_PRINT("local time is smaller than received one\n");
                } else {
                    DEBUG_PRINT("local time is greater or equal than received one\n");
                }
                DEBUG_PRINT("DC diff: %lu\r\n", dcDiff);
                osDelay(1000);
            }

            // config IOmap
            usedMemory = ec_config_map(&IOmap);
            DEBUG_PRINT("IOmap size: %d\r\n", usedMemory);
            if ((unsigned int)usedMemory > sizeof(IOmap))
            {
                oledLogClear();
                oledLog("Need more IOmap space.");
                Error_Handler(); // 需要增大 IOmap 的空间
            }

            // 同步时钟 sync0
            ec_dcsync0(SV630N_idx, TRUE, SYNC0TIME, 3000);
            DEBUG_PRINT("DC sync complete.\r\n");

            // 确认进入 SAFE_OP 状态
			ecState = ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE);
            DEBUG_PRINT("slave state: %u\r\n", ecState); 
            // ecIsRunning = TRUE;
            // osDelay(10000); // 让子弹飞一会

            // // 请求进入 OPERATIONAL 状态
            uint16_t expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
            DEBUG_PRINT("Calculated workcounter %d\n", expectedWKC);

            ec_slave[0].state = EC_STATE_OPERATIONAL;
            osDelay(5);
            // ec_send_processdata();
            // ec_receive_processdata(EC_TIMEOUTRET);
            ec_writestate(0);
            // ec_readstate();

            int check = 2000;
            do
            {
                // ec_slave[0].state = EC_STATE_OPERATIONAL;
                // ec_send_processdata();
                // ec_receive_processdata(EC_TIMEOUTRET);
                ec_statecheck(0, EC_STATE_OPERATIONAL, 50000);
                // DEBUG_PRINT("slave state: %u\r\n", ecState);
            } while (check-- && (ec_slave[0].state != EC_STATE_OPERATIONAL));

            ecState = ec_statecheck(0, EC_STATE_OPERATIONAL, 50000); // 确认已进入 OP 状态
            DEBUG_PRINT("slave state: %u\r\n", ecState);

            for(int i=0; i<10; i++ )
			{
				ec_slave[0].state = EC_STATE_OPERATIONAL;
				ec_writestate(0);
			}

            // 配置 PDO 映射
            if (ec_slave[SV630N_idx].state == EC_STATE_OPERATIONAL)
            {
                pdoOutputs = (SV630N_Outputs *)ec_slave[SV630N_idx].outputs;
                pdoInputs = (SV630N_Inputs *)ec_slave[SV630N_idx].inputs;
                ecIsRunning = 1;
                DEBUG_PRINT("PDO set up.\r\n");
            } else {
                DEBUG_PRINT("E/BOX not found in slave configuration.\r\n");
            }

            // /* send one valid process data to make outputs in slaves happy*/
            // ec_send_processdata();
            // ec_receive_processdata(EC_TIMEOUTRET);

            // ec_writestate(0);
            // /* wait for all slaves to reach OP state */
            // uint16 state = ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE);
            // DEBUG_PRINT("slave state: %u\r\n", state);
            // if (state == EC_STATE_OPERATIONAL)
            // {
            //     DEBUG_PRINT("Get in operational state.\r\n");
            //     ecIsRunning = 1;
            //     // CiA402 状态切换过程
            //     // while ((pdoInputs->statusWord & 0x03df) == 0x0250)
            //     // {
            //     //     pdoOutputs->controlword |= 0x0006;
            //     //     ec_send_processdata();
            //     //     ec_receive_processdata(EC_TIMEOUTRET);
            //     // }

            //     // while ((pdoInputs->statusWord & 0x03ff) == 0x0231)
            //     // {
            //     //     pdoOutputs->controlword |= 0x0007;
            //     //     ec_send_processdata();
            //     //     ec_receive_processdata(EC_TIMEOUTRET);
            //     // }

            //     // while ((pdoInputs->statusWord & 0x03ff) == 0x0233)
            //     // {
            //     //     pdoOutputs->controlword |= 0x000F;
            //     //     ec_send_processdata();
            //     //     ec_receive_processdata(EC_TIMEOUTRET);
            //     // }

            //     // if ((pdoInputs->statusWord & 0x03ff) == 0x0237)
            //     // {
            //     //     isRunning = TRUE;
            //     //     oledLogClear();
            //     //     oledLog("Slaves are running.");
            //     // }
            // }
        }
        else
        {
            oledLogClear();
            oledLog("No slave found");
            DEBUG_PRINT("No slave found.\r\n");
        }
    } else {
        DEBUG_PRINT("No socket connection Excecute as root\r\n");
    }

    
    /* Infinite loop */
    for (;;)
    {
        if (ecIsRunning)
        {
            switchState();
            ec_send_processdata();
            ec_receive_processdata(EC_TIMEOUTRET);
        }
        else
        {
            osThreadExit();
        }
    }
}

void StartEthercatRtTask(void *argument) {
    UNUSED(argument);

    for (;;)
    {
        if (ecIsRunning)
        {
            ec_send_processdata();
            ec_receive_processdata(EC_TIMEOUTRET);
        }
        osDelay(1);               
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

void switchState() {
    // CiA402 状态切换过程
    switch (pdoInputs->statusWord & 0x03ff)
    {
    case 0x0250:
    case 0x0270:
        pdoOutputs->controlword = 0x0006;
        break;

    case 0x0231:
        pdoOutputs->controlword = 0x0007;
        break;

    case 0x0233:
        pdoOutputs->controlword = 0x000F;
        break;

    case 0x0237:
        pdoOutputs->controlword = 0x000F;
        break;
    
    default:
        break;
    }
    // while ((pdoInputs->statusWord & 0x03df) == 0x0250)
    // {
    //     pdoOutputs->controlword |= 0x0006;
    //     ec_send_processdata();
    //     ec_receive_processdata(EC_TIMEOUTRET);
    // }

    // while ((pdoInputs->statusWord & 0x03ff) == 0x0231)
    // {
    //     pdoOutputs->controlword |= 0x0007;
    //     ec_send_processdata();
    //     ec_receive_processdata(EC_TIMEOUTRET);
    // }

    // while ((pdoInputs->statusWord & 0x03ff) == 0x0233)
    // {
    //     pdoOutputs->controlword |= 0x000F;
    //     ec_send_processdata();
    //     ec_receive_processdata(EC_TIMEOUTRET);
    // }

    // if ((pdoInputs->statusWord & 0x03ff) == 0x0237)
    // {
    //     isRunning = TRUE;
    //     oledLogClear();
    //     oledLog("Slaves are running.");
    // }
}

// void simpletest(char *ifname)
// {
//     int i, j, oloop, iloop, chk, wkc;
//     bool needlf = FALSE;
//     bool inOP = FALSE;
//     bool forceByteAlignment = FALSE;
//     uint16_t expectedWKC = 0;

//     printf("Starting simple test\n");

//     /* initialise SOEM, bind socket to ifname */
//     if (ec_init(ifname))
//     {
//         printf("ec_init on %s succeeded.\n", ifname);
//         /* find and auto-config slaves */

//         if (ec_config_init(FALSE) > 0)
//         {
//             printf("%d slaves found and configured.\n", ec_slavecount);

//             if (forceByteAlignment)
//             {
//                 ec_config_map_aligned(&IOmap);
//             }
//             else
//             {
//                 ec_config_map(&IOmap);
//             }

//             ec_configdc();

//             printf("Slaves mapped, state to SAFE_OP.\n");
//             /* wait for all slaves to reach SAFE_OP state */
//             ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 4);

//             oloop = ec_slave[0].Obytes;
//             if ((oloop == 0) && (ec_slave[0].Obits > 0))
//                 oloop = 1;
//             if (oloop > 8)
//                 oloop = 8;
//             iloop = ec_slave[0].Ibytes;
//             if ((iloop == 0) && (ec_slave[0].Ibits > 0))
//                 iloop = 1;
//             if (iloop > 8)
//                 iloop = 8;

//             printf("segments : %d : %d %d %d %d\n", ec_group[0].nsegments, ec_group[0].IOsegment[0],
//                    ec_group[0].IOsegment[1], ec_group[0].IOsegment[2], ec_group[0].IOsegment[3]);

//             printf("Request operational state for all slaves\n");
//             expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
//             printf("Calculated workcounter %d\n", expectedWKC);
//             ec_slave[0].state = EC_STATE_OPERATIONAL;
//             /* send one valid process data to make outputs in slaves happy*/
//             ec_send_processdata();
//             ec_receive_processdata(EC_TIMEOUTRET);
//             /* request OP state for all slaves */
//             ec_writestate(0);
//             chk = 200;
//             /* wait for all slaves to reach OP state */
//             do
//             {
//                 ec_send_processdata();
//                 ec_receive_processdata(EC_TIMEOUTRET);
//                 ec_statecheck(0, EC_STATE_OPERATIONAL, 50000);
//             } while (chk-- && (ec_slave[0].state != EC_STATE_OPERATIONAL));
//             if (ec_slave[0].state == EC_STATE_OPERATIONAL)
//             {
//                 printf("Operational state reached for all slaves.\n");
//                 inOP = TRUE;
//                 /* cyclic loop */
//                 for (i = 1; i <= 10000; i++)
//                 {
//                     ec_send_processdata();
//                     wkc = ec_receive_processdata(EC_TIMEOUTRET);

//                     if (wkc >= expectedWKC)
//                     {
//                         printf("Processdata cycle %4d, WKC %d , O:", i, wkc);

//                         for (j = 0; j < oloop; j++)
//                         {
//                             printf(" %2.2x", *(ec_slave[0].outputs + j));
//                         }

//                         printf(" I:");
//                         for (j = 0; j < iloop; j++)
//                         {
//                             printf(" %2.2x", *(ec_slave[0].inputs + j));
//                         }
//                         printf(" T:%I64d\r", ec_DCtime);
//                         needlf = TRUE;
//                     }
//                     osal_usleep(5000);
//                 }
//                 inOP = FALSE;
//             }
//             else
//             {
//                 printf("Not all slaves reached operational state.\n");
//                 ec_readstate();
//                 for (i = 1; i <= ec_slavecount; i++)
//                 {
//                     if (ec_slave[i].state != EC_STATE_OPERATIONAL)
//                     {
//                         printf("Slave %d State=0x%2.2x StatusCode=0x%4.4x : %s\n", i, ec_slave[i].state,
//                                ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
//                     }
//                 }
//             }
//             printf("\nRequest init state for all slaves\n");
//             ec_slave[0].state = EC_STATE_INIT;
//             /* request INIT state for all slaves */
//             ec_writestate(0);
//         }
//         else
//         {
//             printf("No slaves found!\n");
//         }
//         printf("End simple test, close socket\n");
//         /* stop SOEM, close socket */
//         ec_close();
//     }
//     else
//     {
//         printf("No socket connection on %s\nExecute as root\n", ifname);
//     }
// }

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
        osMemoryPoolGetSpace(rxBufferPool);
        // printf("Eth rx alloc. left=%lu\r\n", left);
    }
    else
    {
        *buff = NULL;
        uint32_t left = osMemoryPoolGetSpace(rxBufferPool);
        DEBUG_PRINT("Eth rx alloc failed. left=%lu\r\n", left);
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

    // printf("Eth rx link, len=%d\r\n", Length);
}

/**
 * @brief ETH 发送完成 ISR，注意 RTOS 的使用
 * 
 * @param heth 
 */
void HAL_ETH_TxCpltCallback(ETH_HandleTypeDef *heth)
{
    UNUSED(heth);
    // printf("Eth tx cplt.\r\n");
    // osMutexRelease(ethTxBuffMutex);
    // osSemaphoreRelease(ethTxCpltSemaphore);
    //	oledLog("Eth tx cplt ");
    // const char* str = "Eth tx cplt.";
    // HAL_UART_Transmit(&huart4, (uint8_t*)str, strlen(str), 100);
    //	osMutexRelease(ecTxBuffMutex);
    //  printf("Packet Transmitted successfully!\r\n");
    //  fflush(0);
    //	oledLogClear();
    //	oledLog("Packet Transmitted successfully! ");
}

/**
 * @brief ETH 接收完成中断 ISR
 * 
 * @param heth 
 */
void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth)
{
    UNUSED(heth);
    // ETH_BufferTypeDef *pBuff;
    // HAL_StatusTypeDef status = HAL_ETH_ReadData(heth, (void**)&pBuff);  // TODO: 需要将这个过程放到线程中
    // if (status != HAL_OK) {
    // 	// while(1);
    // 	Error_Handler();
    // }
    // if (pBuff) {
    //     ethRxListPush(pBuff);
    //     printf("Rx %lu bytes\n", pBuff->len);
    //     for (uint32_t i = 0; i < pBuff->len; i++) {
    //         printf("%02x ", pBuff->buffer[i]);
    //     }
    //     printf("\r\n");
    // }
    // printf("RxCplt\n");
    osSemaphoreRelease(ethRxCpltSemaphore);
}

/**
 * @brief ETH 错误 ISR
 * 
 * @param heth 
 */
void HAL_ETH_ErrorCallback(ETH_HandleTypeDef *heth)
{
    if ((HAL_ETH_GetDMAError(heth) & ETH_DMASR_RBUS) == ETH_DMASR_RBUS)
    {
        osSemaphoreRelease(ethRxCpltSemaphore);
    }
}

/* USER CODE END Application */

