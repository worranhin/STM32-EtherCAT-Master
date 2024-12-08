/*
 * SV630N.c
 *
 *  Created on: Dec 8, 2024
 *      Author: Administrator
 */

#include <stdio.h>
#include "SV630N.h"

/**
 * @brief 配置 SV630N 的 SOEM 回调函数, BLOCKING
 */
int SV630N_Setup(uint16 slave)
{
	uint8_t val8;
	uint16_t val16;

	// 配置 PDO 映射组
	val8 = 0x00;
	ec_SDOwrite(slave, 0x1C12, 0x00, FALSE, sizeof(val8), &val8, EC_TIMEOUTSAFE); // 清除原有的映射组
	val16 = 0x1701;
	ec_SDOwrite(slave, 0x1C12, 0x01, FALSE, sizeof(val16), &val16, EC_TIMEOUTSAFE); // 写入映射组
	val8 = 0x01;
	ec_SDOwrite(slave, 0x1C12, 0x01, FALSE, sizeof(val8), &val8, EC_TIMEOUTSAFE); // 写入映射组总个数

	return 1;

//    int retval;
//    uint16 u16val;
//
//    retval = 0;
//
//    /* Map velocity PDO assignment via Complete Access*/
//    uint16 map_1c12[4] = {0x0003, 0x1601, 0x1602, 0x1604};
//    uint16 map_1c13[3] = {0x0002, 0x1a01, 0x1a03};
//    retval += ec_SDOwrite(slave, 0x1c12, 0x00, TRUE, sizeof(map_1c12), &map_1c12, EC_TIMEOUTSAFE);
//    retval += ec_SDOwrite(slave, 0x1c13, 0x00, TRUE, sizeof(map_1c13), &map_1c13, EC_TIMEOUTSAFE);
//    /* set some motor parameters, just as example */
//    u16val = 1200; // max motor current in mA
//    retval += ec_SDOwrite(slave, 0x8010, 0x01, FALSE, sizeof(u16val), &u16val, EC_TIMEOUTSAFE);
//    u16val = 150; // motor coil resistance in 0.01ohm
//    retval += ec_SDOwrite(slave, 0x8010, 0x04, FALSE, sizeof(u16val), &u16val, EC_TIMEOUTSAFE);
//    /* set other necessary parameters as needed */
//
//    printf("EL7031 slave %d set, retval = %d\n", slave, retval);
//    return 1;
}
