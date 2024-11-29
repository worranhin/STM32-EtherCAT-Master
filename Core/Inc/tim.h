
#ifndef __tim_H
#define __tim_H
#ifdef __cplusplus
 extern "C" {
#endif

/* 包含头文件 ----------------------------------------------------------------*/
#include "stm32F4xx_hal.h"
#include "main.h"


uint32_t GetSec(void);
uint32_t GetUSec(void);


#ifdef __cplusplus
}
#endif
#endif /*__ tim_H */
