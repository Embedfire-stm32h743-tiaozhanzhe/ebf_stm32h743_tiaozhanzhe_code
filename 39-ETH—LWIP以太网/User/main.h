#ifndef __MAIN_H
#define __MAIN_H

#include "stm32h7xx_hal.h"

#define DEST_IP_ADDR0   ((uint8_t)192U)
#define DEST_IP_ADDR1   ((uint8_t)168U)
#define DEST_IP_ADDR2   ((uint8_t)1U)
#define DEST_IP_ADDR3   ((uint8_t)198U)

#define DEST_PORT       ((uint16_t)7U)
   
//¾²Ì¬IPµØÖ·: IP_ADDR0.IP_ADDR1.IP_ADDR2.IP_ADDR3
#define IP_ADDR0   ((uint8_t) 192U)
#define IP_ADDR1   ((uint8_t) 168U)
#define IP_ADDR2   ((uint8_t) 0U)
#define IP_ADDR3   ((uint8_t) 132U)
   
//×ÓÍøÑÚÂë
#define NETMASK_ADDR0   ((uint8_t) 255U)
#define NETMASK_ADDR1   ((uint8_t) 255U)
#define NETMASK_ADDR2   ((uint8_t) 255U)
#define NETMASK_ADDR3   ((uint8_t) 0U)

//Íø¹Ø
#define GW_ADDR0   ((uint8_t) 192U)
#define GW_ADDR1   ((uint8_t) 168U)
#define GW_ADDR2   ((uint8_t) 0U)
#define GW_ADDR3   ((uint8_t) 1U) 



static void SystemClock_Config(void);
static void MPU_Config(void);
#endif /* __MAIN_H */

