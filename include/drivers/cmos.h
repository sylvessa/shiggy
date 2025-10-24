#ifndef CMOS_H
#define CMOS_H
#include "types.h"

#define RTC_REGISTER 0X70
#define RTC_DATA 0X71
#define RTC_STATUS_REG_B 0X0B
#define RTC_SECONDS 0X00
#define RTC_MINUTES 0X02
#define RTC_HOURS 0X04
#define RTC_WEEKDAY 0X06
#define RTC_DAY_OF_MONTH 0X07
#define RTC_MONTH 0X08
#define RTC_YEAR 0X09
#define RTC_CENTURY 0X32

nat32 getRTCSeconds();
nat32 getRTCMinutes();
nat32 getRTCHours();
nat32 getRTCWeekday();
nat32 getRTCDayOfMonth();
nat32 getRTCMonth();
nat32 getRTCYear();

#endif