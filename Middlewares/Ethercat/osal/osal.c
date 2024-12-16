/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

#include <osal.h>
#include <time.h>

/* User Include */
//#include "tim.h"
#include "cmsis_os.h"
#include "main.h"


#define  timercmp(a, b, CMP)                                \
  (((a)->tv_sec == (b)->tv_sec) ?                           \
   ((a)->tv_usec CMP (b)->tv_usec) :                        \
   ((a)->tv_sec CMP (b)->tv_sec))
#define  timeradd(a, b, result)                             \
  do {                                                      \
    (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;           \
    (result)->tv_usec = (a)->tv_usec + (b)->tv_usec;        \
    if ((result)->tv_usec >= 1000000)                       \
    {                                                       \
       ++(result)->tv_sec;                                  \
       (result)->tv_usec -= 1000000;                        \
    }                                                       \
  } while (0)
#define  timersub(a, b, result)                             \
  do {                                                      \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;           \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;        \
    if ((result)->tv_usec < 0) {                            \
      --(result)->tv_sec;                                   \
      (result)->tv_usec += 1000000;                         \
    }                                                       \
  } while (0)

#define USECS_PER_SEC   1000000
//#define CFG_TICKS_PER_SECOND 1000
//#define USECS_PER_TICK  (USECS_PER_SEC / CFG_TICKS_PER_SECOND)


void osal_timer_start (osal_timert * self, uint32 timeout_usec)
{
   ec_timet ec_t_now;

   timeval start_time;
   timeval timeout;
   timeval stop_time;

   ec_t_now = osal_current_time();

   start_time.tv_sec = ec_t_now.sec;
   start_time.tv_usec = ec_t_now.usec;

   timeout.tv_sec = timeout_usec / USECS_PER_SEC;
   timeout.tv_usec = timeout_usec % USECS_PER_SEC;
   timeradd(&start_time, &timeout, &stop_time);

   self->stop_time.sec = stop_time.tv_sec;
   self->stop_time.usec = stop_time.tv_usec;

}

boolean osal_timer_is_expired (osal_timert * self)
{
    ec_timet ec_t_now;

   timeval current_time;
   timeval stop_time;
   int is_not_yet_expired;

   ec_t_now = osal_current_time();
   current_time.tv_sec = ec_t_now.sec;
   current_time.tv_usec = ec_t_now.usec;

   stop_time.tv_sec = self->stop_time.sec;
   stop_time.tv_usec = self->stop_time.usec;
   is_not_yet_expired = timercmp(&current_time, &stop_time, <);

   return is_not_yet_expired == FALSE;
}

int osal_usleep(uint32 usec)
{
   osal_timert qtime;
   osal_timer_start(&qtime, usec);

   while(!osal_timer_is_expired(&qtime));
   return 1;
}


/**
 * @brief 获取当前时间
 * @retval ec_timet
 */
ec_timet osal_current_time (void)
{
	osKernelLock();
	uint32_t sec = getCurrentSecond();
	uint32_t us = getCurrentUs();
	ec_timet time = {sec, us};
	osKernelUnlock();
	return time;

//	osKernelLock(); // 保证一致性
//	uint32_t tick = osKernelGetTickCount();
//	uint32_t freq = osKernelGetTickFreq();
//	osKernelUnlock();
//	uint32_t second = tick / freq;
//	uint32_t us = (tick - second * freq) * USECS_PER_SEC / freq;
}

void osal_time_diff(ec_timet *start, ec_timet *end, ec_timet *diff)
{
   if (end->usec < start->usec) {
      diff->sec = end->sec - start->sec - 1;
      diff->usec = end->usec + USECS_PER_SEC - start->usec;
   }
   else {
      diff->sec = end->sec - start->sec;
      diff->usec = end->usec - start->usec;
   }
}

/**
 * 创建一个线程
 * @retval 成功返回 1，否则返回 0
 */
int osal_thread_create(void *thandle, int stacksize, osThreadFunc_t func, void *param)
{
	osThreadAttr_t attr = {
			.name = "worker",
			.stack_size = stacksize,
			.priority = osPriorityNormal
	};

	thandle = osThreadNew(func, param, &attr);
	if (thandle == NULL) {
		return 0;
	}

	return 1;
}

int osal_thread_create_rt(void *thandle, int stacksize, osThreadFunc_t func, void *param)
{
	osThreadAttr_t attr = {
			.name = "worker_rt",
			.stack_size = stacksize,
			.priority = osPriorityRealtime
	};

	thandle = osThreadNew(func, param, &attr);
	if (thandle == NULL) {
		return 0;
	}

	return 1;
}

