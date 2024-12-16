/*
 * PVD.c
 *
 *  Created on: 2022年9月22日
 *      Author: shengmidao
 */

#include "soem_motor.h"


#include <string.h>
#include <stdio.h>

#include "nicdrv.h"

#include <lwip/pbuf.h>
#include <lwip/inet.h>

#include "ethercat.h"
#include "FreeRTOSConfig.h"
#include "osal.h"
#include "oshw.h"
#include "netif.h"
#include "netdev_hook.h"

//csp代码
#define __is_print(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')
static void hex_dump(const uint8_t *ptr, size_t buflen)
{
    unsigned char *buf = (unsigned char *)ptr;
    int i, j;

    configASSERT(ptr != NULL);

    for (i = 0; i < buflen; i += 16)
    {
        printf("%08X: ", i);

        for (j = 0; j < 16; j++)
            if (i + j < buflen)
                printf("%02X ", buf[i + j]);
            else
                printf("   ");
        printf(" ");

        for (j = 0; j < 16; j++)
            if (i + j < buflen)
                printf("%c", __is_print(buf[i + j]) ? buf[i + j] : '.');
        printf("\n");
    }
}


static char IOmap[4096];
typedef struct  __attribute__((__packed__))
{
	unsigned char  mode_byte;
	unsigned short control_word;
	long  dest_pos;
	unsigned short error_word;
	unsigned short status_word;
	long  cur_pos;
}SERVO_DATA_T;

typedef struct
{
	SERVO_DATA_T servo_data[3];
}SERVOS_T;

SERVOS_T *servos = (SERVOS_T *)IOmap;

void view_slave_data()
{
	hex_dump(IOmap,32);
}

static void echo_time()
{
	struct timeval tp;
	osal_gettimeofday(&tp, 0);
//	printf("****cur time = %d,%03d,%03d(us)\n", tp.tv_sec,tp.tv_usec/1000,tp.tv_usec%1000);
}

int safe_SDOwrite(uint16 Slave, uint16 Index, uint8 SubIndex, int size, void *data)
{
	int wkc, cnt=0;
	do{
		wkc = ec_SDOwrite(Slave, Index, SubIndex, FALSE, size, data, EC_TIMEOUTRXM);
		cnt++;
	}while(wkc<=0 && cnt<10);
	return wkc;
}
int safe_SDCwrite_b(uint16 Slave, uint16 Index, uint8 SubIndex, uint8 b)
{
	return safe_SDOwrite(Slave, Index, SubIndex, 1, &b);
}
int safe_SDCwrite_w(uint16 Slave, uint16 Index, uint8 SubIndex, uint16 w)
{
	return safe_SDOwrite(Slave, Index, SubIndex, 2, &w);
}
int safe_SDCwrite_dw(uint16 Slave, uint16 Index, uint8 SubIndex, uint32 dw)
{
	return safe_SDOwrite(Slave, Index, SubIndex, 4, &dw);
}

int safe_SDOread(uint16 Slave, uint16 Index, uint8 SubIndex, int size, void *data)
{
	int wkc, cnt=0;
	do{
		wkc = ec_SDOread(Slave, Index, SubIndex, FALSE, &size, data, EC_TIMEOUTRXM);
	}while(wkc<=0 && cnt<10);
	return wkc;
}
int safe_SDOread_b(uint16 Slave, uint16 Index, uint8 SubIndex, uint8 b)
{
	return safe_SDOread(Slave, Index, SubIndex, 1, &b);
}

int safe_SDOread_w(uint16 Slave, uint16 Index, uint8 SubIndex, uint16 w)
{
	return safe_SDOread(Slave, Index, SubIndex, 2, &w);
}

int safe_SDOread_dw(uint16 Slave, uint16 Index, uint8 SubIndex, uint32 dw)
{
	return safe_SDOread(Slave, Index, SubIndex, 4, &dw);
}


void viewSDO(uint16_t slave, uint16_t index, uint16_t subindex, int bytes)
{
	uint32_t dw = 0;
	int wkc;
	safe_SDOread(slave, index, subindex, bytes, &dw);
	printf("SDO read=%s, SDO[0x%04x.%02x] = 0x%08x\n", wkc?"success":"fail",index, subindex, dw);
}

void process_data_config()
{
	u8_t     ind;

	for(int slave = 1; slave <= *ecx_context.slavecount; slave++)
	{
		//rpdo------------
		//1c12.0
		safe_SDCwrite_b(slave, 0x1c12, 0, 0);
		safe_SDCwrite_w(slave, 0x1c12, 1, htoes(0x1600));

		//1600
		ind = 0;
		safe_SDCwrite_b(slave, 0x1600, 0, 0);
		safe_SDCwrite_dw(slave, 0x1600, ++ind, htoel(0x60600008));//6060h(控制模式)
		safe_SDCwrite_dw(slave, 0x1600, ++ind, htoel(0x60400010));//6040h(控制字)
		safe_SDCwrite_dw(slave, 0x1600, ++ind, htoel(0x607a0020));//607Ah(目标位置)
		safe_SDCwrite_b(slave, 0x1600, 0, ind);

		//1c12.0
		safe_SDCwrite_b(slave, 0x1c12, 0, 1);

		//tpdo-------------
		//1c13.0
		safe_SDCwrite_b(slave, 0x1c13, 0x00, 0);
		safe_SDCwrite_w(slave, 0x1c13, 0x01, htoes(0x1a00));

		//1a00
		ind = 0;
		safe_SDCwrite_b(slave, 0x1a00, 0, 0);
		safe_SDCwrite_dw(slave, 0x1a00, ++ind, htoel(0x603F0010));//603Fh(错误码)
		safe_SDCwrite_dw(slave, 0x1a00, ++ind, htoel(0x60410010));//6041h(状态字)
		safe_SDCwrite_dw(slave, 0x1a00, ++ind, htoel(0x60640020));//6064h(位置反馈)
		safe_SDCwrite_b(slave, 0x1a00, 0, ind);

		//1c13.0
		safe_SDCwrite_b(slave, 0x1c13, 0, 1);

		safe_SDCwrite_b(slave, 0x6060, 0, 1);		//pp模式

	}
}


void servo_switch_op()
{
	int sta;
	for(int slave = 1; slave <= *ecx_context.slavecount; slave++)
	{
		int idx = slave - 1;
		sta = servos->servo_data[idx].status_word & 0x3ff;			//5687->0x1637:		5744->0x1670:0x270
		//printf("servo_switch_op: slave %d [6041]=%04x\n",slave,servos->servo_data[idx].status_word );
		if(servos->servo_data[idx].status_word & 0x8) //故障处理
		{  //0x21f  //0x218
			if(sta==0x0218)
			{
				servos->servo_data[idx].control_word = 0x80;
	//			printf("***slave %d control=%04x\n",slave,servos->servo_data[idx].control_word );
			}
			continue;
		}
		//printf("servo_switch_op: slave %d sta=%04x\n", slave, sta );
		//base on cia402       				control_word              	status_word bit0~9
		/* 上电初始化														0x0000
		 * 初*-伺服无故障													0x0250
		 * 伺*-伺服准备好						0x0006						0x0231
		 * 伺*-等待使能伺服						0x0007						0x0233
		 * 等*-伺服运行							0x000F						0x0237
		 *
		 * 伺*-等待打开伺服使能					0x0007						0x0233
		 * 等*-伺服准备好						0x0006						0x0231
		 * 伺*-伺服无故障						0x0000						0x0250
		 *
		 * 伺服运行-伺服准备好					0x0006						0x0231
		 * 伺服运行-伺服无故障					0x0000						0x0250
		 * 等待打开伺服使能-伺服无故障			0x0000						0x0250
		 * 伺服运行-快速停机						0x0002						0x0217
		 *
		 * 快速停机-伺服无故障												0x0250
		 * -故障停机															0x021F
		 * 故障停机-故障														0x0218
		 * 故障-伺服无故障						0x80						0x0250
		 * 快速停机-伺服运行						0x0F						0x0237
		 *
		 */

		switch(sta)
		{
			case 0x250:
			case 0x270:
				servos->servo_data[idx].control_word = 0x6;
				break;
			case 0x231:
				servos->servo_data[idx].control_word = 0x7;
				break;
			case 0x233:
				servos->servo_data[idx].control_word = 0xf;
				break;
			case 0x217:
				servos->servo_data[idx].control_word = 0xf;
				break;
			default:
				//servos->servo_data[idx].control_word = 0x6;
				break;
		}
		//printf("slave %d control=%04x\n",slave,servos->servo_data[idx].control_word );
	}

}
void servo_switch_idle()
{
	int sta;
	for(int slave = 1; slave <= *ecx_context.slavecount; slave++)
	{
		servos->servo_data[slave-1].control_word = 0x0;
	}
}

#define EC_TIMEOUTMON 500
int expectedWKC;
boolean needlf;
volatile int wkc;
boolean inOP;
uint8 currentgroup = 0;
long test_poit;
uint16 test_error;
void sv660n_config(char *ifname)
{
	needlf = FALSE;
    inOP = FALSE;

	ecx_context.manualstatechange = 1;

//	printf("========================\n");
//	printf("sv660 config\n");
	echo_time();

	if (ec_init(ifname))
	{
//		printf("ec_init on %s succeeded.\n",ifname);

		//init status
//		printf("\nRequest init state for all slaves\n");
		ec_slave[0].state = EC_STATE_INIT;
		//request INIT state for all slaves
		ec_writestate(0);
		ec_readstate();

		//显示1状态
		/* wait for all slaves to reach SAFE_OP state */
		ec_statecheck(0, EC_STATE_INIT,  EC_TIMEOUTSTATE * 3);
		if (ec_slave[0].state != EC_STATE_INIT ){
//			printf("Not all slaves reached init state.\n");
			ec_readstate();
			for(int i = 1; i<=ec_slavecount ; i++){
				if(ec_slave[i].state != EC_STATE_INIT){
//					printf("Slave %d State=0x%2x StatusCode=0x%04x : %s\n", i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
				}
			}
		}
		echo_time();
		//if ( ec_config(FALSE, &IOmap) > 0 )

		wkc = ec_config_init(0/*usetable*/);

		if (wkc > 0)
		{

			ec_configdc();
//			ec_dcsync0(1, TRUE, 2000000, 50); // SYNC0 on slave 1
			while(EcatError) printf("%s", ec_elist2string());
//			printf("%d slaves found and configured.\n",ec_slavecount);


			/* request pre_op for slave */
//			printf("\nRequest pre_op state for all slaves\n");
			ec_slave[0].state = EC_STATE_PRE_OP | EC_STATE_ACK;
			ec_writestate(0);
			ec_readstate();

//			//故障复位
//			safe_SDOread(1,0x603f,0,2,&test_error);		//30081
//			if(test_error==30081)
//			{
//				safe_SDCwrite_w(1,0x6040, 0, 0x80);
//			}
//			ec_readstate();

			//现在应该在pre_op状态
			//显示2状态
			process_data_config(); //config tpdo/rpdo

			//config fmmu
			ec_config_map(IOmap);


			/* request safe_op for slave */
			ec_slave[0].state = EC_STATE_SAFE_OP;
			ec_writestate(0);
			ec_readstate();

			//safe-op
			expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
//			printf("Calculated workcounter %d\n", expectedWKC);

			/* wait for all slaves to reach SAFE_OP state */
			ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE * 3);
			if (ec_slave[0].state != EC_STATE_SAFE_OP ){
//				printf("Not all slaves reached safe operational state.\n");
				ec_readstate();
				for(int i = 1; i<=ec_slavecount ; i++){
					if(ec_slave[i].state != EC_STATE_SAFE_OP){
//						printf("Slave %d State=0x%2x StatusCode=0x%04x : %s\n", i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
					}
				}
			}else{
				//显示4状态
				//启动伺服
				servos->servo_data[0].mode_byte = 8; //csp mode

				//op status
//				printf("Request operational state for all slaves\n");
				expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
//				printf("Calculated workcounter %d\n", expectedWKC);

				// send one valid process data to make outputs in slaves happy
				ec_slave[0].state = EC_STATE_OPERATIONAL;
				ec_send_processdata();
				ec_receive_processdata(EC_TIMEOUTRET*3);
				ec_slave[0].state = EC_STATE_OPERATIONAL;
				ec_writestate(0);
				ec_readstate();

				int chk = 200;
				// wait for all slaves to reach OP state
				do
				{
					ec_slave[0].state = EC_STATE_OPERATIONAL;
					ec_send_processdata();
					ec_receive_processdata(EC_TIMEOUTRET);
//					printf("--->workcounter %d\n", wkc);
					ec_statecheck(0, EC_STATE_OPERATIONAL, 50000);
				}
				while (chk-- && (ec_slave[0].state != EC_STATE_OPERATIONAL));

				if (ec_slave[0].state == EC_STATE_OPERATIONAL )
				{
//					printf("<<<Operational>>> state reached for all slaves.\n");
					inOP = TRUE;

					osal_timert t;
					osal_timer_start(&t, 1000);

						// cyclic loop
						for(int i = 1; i <= 20000; i++)
						{
							servo_switch_op();
							if(servos->servo_data[0].control_word==7){
								servos->servo_data[0].dest_pos = servos->servo_data[0].cur_pos;
								test_poit=servos->servo_data[0].dest_pos;
								//printf("cur pos = %ld\n", servos->servo_data[0].cur_pos);
							}
							if(servos->servo_data[0].control_word==0xf){
								if(i<=300)
								{
									servos->servo_data[0].dest_pos += (i*10);
								}else
								{
									servos->servo_data[0].dest_pos += 3000;
								}
							}
							while(osal_timer_is_expired(&t)==FALSE);
							osal_timer_start(&t, 1000);

							ec_send_processdata();
							wkc = ec_receive_processdata(EC_TIMEOUTRET);

							if(wkc >= expectedWKC){
								//printf("~~~~WKC %d \n", wkc);
							}
							if(wkc <=0 ){
	//							printf("Error.\n");
								break;
							}
						}
						osal_timer_start(&t, 1000);
						while(osal_timer_is_expired(&t)==FALSE);
						servos->servo_data[0].control_word = 0x0002;
						ec_send_processdata();
						ec_receive_processdata(EC_TIMEOUTRET);

//						servos->servo_data[0].control_word = 0x0;
//						ec_send_processdata();
//						ec_receive_processdata(EC_TIMEOUTRET);
//
//						servos->servo_data[0].control_word = 0x0;
//						ec_send_processdata();
//						ec_receive_processdata(EC_TIMEOUTRET);
						inOP = FALSE;

				}
				else
				{
//					printf("Not all slaves reached operational state.\n");
					ec_readstate();
					for(int i = 1; i<=ec_slavecount ; i++)
					{
						if(ec_slave[i].state != EC_STATE_OPERATIONAL)
						{
//							printf("Slave %d State=0x%2.2x StatusCode=0x%4.4x : %s\n",i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
						}
					}
				}

				//init status
//				printf("\nRequest init state for all slaves\n");
				ec_slave[0].state = EC_STATE_INIT;
				//request INIT state for all slaves
				ec_writestate(0);
			}
		} else {
//			printf("No slaves found!\n");
		}

		echo_time();
//		printf("End soem, close socket\n");

		// stop SOEM, close socket
		ec_close();
	}else{
//		printf("ec_init on %s failed.\n",ifname);
	}
//	printf("IOMAP addr = 0x%08x\n", (uint32_t)IOmap);

//	printf("========================\n");
	view_slave_data();
}



//伺服控制线程
#include "lwip.h"
extern struct netif gnetif;

void soem_motor_Task(void* parameter)
{

	while(1)
	{
//测试代码程序
//		printf("SOEM (Simple Open EtherCAT Master)\n");
//
//		//
//		printf("test_oshw_htons...");
//		test_oshw_htons ();
//		printf("Ok\n");
//
//		//
//		printf("test_oshw_htons...");
//		test_oshw_ntohs ();
//		printf("Ok\n");
//
//		printf("test_osal_current_time...\n");
//		test_osal_current_time ();
//
//		printf("Test finished\n");
//
		sv660n_config("st0");
		vTaskDelay(10000);
	}
}
