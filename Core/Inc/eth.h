
#ifndef __eth_H
#define __eth_H
#ifdef __cplusplus
 extern "C" {
#endif

/* 包含头文件 ----------------------------------------------------------------*/
#include "stm32F4xx_hal.h"
#include "main.h"

/* 私有类型定义 --------------------------------------------------------------*/

/* 私有宏定义 ----------------------------------------------------------------*/
//#define ETH_RCC_CLK_ENABLE()                __HAL_RCC_ETH_CLK_ENABLE()
//
//#define ETH_GPIO_ClK_ENABLE()              {__HAL_RCC_GPIOA_CLK_ENABLE();__HAL_RCC_GPIOC_CLK_ENABLE();\
//                                            __HAL_RCC_GPIOB_CLK_ENABLE();__HAL_RCC_GPIOG_CLK_ENABLE();}
//
//#define GPIO_AFx_ETH                        GPIO_AF11_ETH
//
//#ifndef ETH_TX_DESC_CNT
// #define ETH_TX_DESC_CNT         4
//#endif
//
//#ifndef ETH_RX_DESC_CNT
// #define ETH_RX_DESC_CNT         4
//#endif

/* 私有变量 ------------------------------------------------------------------*/

/* 扩展变量 ------------------------------------------------------------------*/

extern ETH_HandleTypeDef heth;

/* 私有函数原形 --------------------------------------------------------------*/

/* 函数体 --------------------------------------------------------------------*/

//void MX_ETH_Init(void);
//void PHY_Init(void);
//void low_level_output(uint8_t *p,uint32_t length);
int bfin_EMAC_send (void *packet, int length);
int bfin_EMAC_recv (uint8_t * packet, size_t size);
//void printfBuffer(uint8_t *dat, uint32_t len );


#ifdef __cplusplus
}
#endif
#endif /*__ eth_H */
