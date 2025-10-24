// time
#include "globals.h"
#include "drivers/cmos.h"

nat32 standardizeFromBCD(nat32 value) {
    static bool initialized = false;
    static bool BCD_mode = false;

    if (!initialized) {
        out_byte(RTC_REGISTER, RTC_STATUS_REG_B);
        byte format = in_byte(RTC_DATA);
        BCD_mode = (format & 0x04) == 0;
        initialized = true;
    }

    if (BCD_mode)
        return ((value / 16) * 10) + (value & 0x0F);
    else
        return value;
}

nat32 standardizeFrom12h(nat32 hour)
{
    static bool initialized = false;
    static bool h12_mode = false;

    if (!initialized) {
        out_byte(RTC_REGISTER, RTC_STATUS_REG_B);
        byte format = in_byte(RTC_DATA);
        h12_mode = (format & 0x02) == 0;
        initialized = true;
    }

    if (h12_mode)
        return ((hour & 0x7F) + 12) % 24;
    else
        return hour;
}

nat32 getRTCSeconds() {
    out_byte(RTC_REGISTER, RTC_SECONDS);
    return standardizeFromBCD(in_byte(RTC_DATA));
}

nat32 getRTCMinutes() {
    out_byte(RTC_REGISTER, RTC_MINUTES);
    return standardizeFromBCD(in_byte(RTC_DATA));
}

nat32 getRTCHours() {
    nat32 hours;
    out_byte(RTC_REGISTER, RTC_HOURS);
    hours = in_byte(RTC_DATA);
    return standardizeFrom12h(standardizeFromBCD(hours & 0x7F) | (hours & 0x80));
}

nat32 getRTCWeekday() {
    out_byte(RTC_REGISTER, RTC_WEEKDAY);
    return standardizeFromBCD(in_byte(RTC_DATA));
}

nat32 getRTCDayOfMonth() {
    out_byte(RTC_REGISTER, RTC_DAY_OF_MONTH);
    return standardizeFromBCD(in_byte(RTC_DATA));
}

nat32 getRTCMonth() {
    out_byte(RTC_REGISTER, RTC_MONTH);
    return standardizeFromBCD(in_byte(RTC_DATA));
}

nat32 getRTCYear() {
    nat32 century;
    nat32 year;
    
    out_byte(RTC_REGISTER, RTC_CENTURY);

    century = standardizeFromBCD(in_byte(RTC_DATA));
    out_byte(RTC_REGISTER, RTC_YEAR);

    year = standardizeFromBCD(in_byte(RTC_DATA));
    return century * 100 + year;
}