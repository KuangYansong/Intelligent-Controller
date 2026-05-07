#ifndef BOARD_H
#define BOARD_H

#include "sscb_types.h"

SscbStatus Board_Init(void);
void Board_Idle(void);
void Board_SoftwareReset(void);

#endif

