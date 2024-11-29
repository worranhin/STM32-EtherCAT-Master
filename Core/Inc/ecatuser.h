
#ifndef _ECATUSER_H_
#define _ECATUSER_H_

/* 包含头文件 ----------------------------------------------------------------*/
#include "ethercat.h"
#include "osal.h"

/* 私有类型定义 --------------------------------------------------------------*/
PACKED_BEGIN
typedef struct PACKED
{
	uint16 controlword;
	//uint16 setword;

}PDO_Outputs; //1701
PACKED_END

PACKED_BEGIN
typedef struct PACKED
{
	uint16 errorCode;
	//uint16 Code;

}PDO_Input;
PACKED_END

/* 私有宏定义 ----------------------------------------------------------------*/

/* 扩展变量 ------------------------------------------------------------------*/
extern int32_t ecOffset;

/* 私有函数原形 --------------------------------------------------------------*/

/* 函数体 --------------------------------------------------------------------*/

void ecat_loop_init(void);
void ecat_init(void);
void ecat_loop(void);
void simpletest(void);


#endif // _ECATUSER_H_


