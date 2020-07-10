#ifndef __BSP_WWDG_H
#define	__BSP_WWDG_H

#include "stm32h7xx.h"


void WWDG_Config(uint8_t tr, uint8_t wr, uint32_t prv);
void WWDG_Feed(void);

extern WWDG_HandleTypeDef WWDG_Handle;

#endif /* __WWDG_H */

