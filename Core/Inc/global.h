/*
 * global.h
 *
 *  Created on: Dec 8, 2024
 *      Author: Administrator
 */

#ifndef INC_GLOBAL_H_
#define INC_GLOBAL_H_

#include "cmsis_os2.h"

// in freertos.c
// extern osSemaphoreId_t tim14ExpireSemaphore;
extern osMutexId_t ethTxBuffMutex;
extern osMutexId_t oledMutex;
extern osMutexId_t ethRxListMutex;
extern osMutexId_t ethTxConfigMutex;
// extern osSemaphoreId_t ethTxReqSemaphore;
// extern osSemaphoreId_t ethTxCpltSemaphore;
extern osSemaphoreId_t ethRxCpltSemaphore;
extern osMemoryPoolId_t rxBufferPool;
extern osMemoryPoolId_t txBufferPool;
extern osMessageQueueId_t ethRxQueue;
extern osMessageQueueId_t ethTxQueue;

#endif /* INC_GLOBAL_H_ */
