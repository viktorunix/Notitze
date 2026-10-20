#ifndef TABLET_SUPPORT_H
#define TABLET_SUPPORT_H

#include <stdbool.h>

void InitTabletSupport(void);
bool IsUsingTablet(void);
float GetTabletPressure(void);
#endif