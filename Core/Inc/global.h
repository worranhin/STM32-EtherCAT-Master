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
extern osMutexId_t ecTxBuffMutex;
extern osMutexId_t oledMutex;
extern osSemaphoreId_t ethTxCpltSemaphore;
extern osSemaphoreId_t ethRxCpltSemaphore;
extern osMemoryPoolId_t rxBufferPool;

#endif /* INC_GLOBAL_H_ */
