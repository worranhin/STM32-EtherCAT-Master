
/* 包含头文件 ----------------------------------------------------------------*/
	
#include "tim.h"

/* 私有类型定义 --------------------------------------------------------------*/

/* 私有宏定义 ----------------------------------------------------------------*/

/* 私有变量 ------------------------------------------------------------------*/

/* 扩展变量 ------------------------------------------------------------------*/

/* 私有函数原形 --------------------------------------------------------------*/

/* 函数体 --------------------------------------------------------------------*/


/**
  * 函数功能: GetSec
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
uint32_t GetSec(void)
{
    return TIM3->CNT;
}
/**
  * 函数功能: GetUSec
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */

uint32_t GetUSec(void)
{
    return TIM2->CNT;
}