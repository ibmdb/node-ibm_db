#ifndef _STRPTIME_H
#define _STRPTIME_H

#define ALT_E          0x01
#define ALT_O          0x02
//#define LEGAL_ALT(x)       { if (alt_format & ~(x)) return (0); }
#define LEGAL_ALT(x)       { ; }
#define TM_YEAR_BASE   (1900)

#include <ctype.h>
#include <string.h>
#include <time.h>
#include <conio.h>

#ifdef __cplusplus
extern "C" {  
#endif  

char * strptime(const char *buf, const char *fmt, struct tm *tm);

#ifdef __cplusplus
}
#endif

static const char *day[7] = {
     "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
     "Friday", "Saturday"
};

static const char *abday[7] = {
     "Sun","Mon","Tue","Wed","Thu","Fri","Sat"
};

static const char *mon[12] = {
     "January", "February", "March", "April", "May", "June", "July",
     "August", "September", "October", "November", "December"
};

static const char *abmon[12] = {
     "Jan", "Feb", "Mar", "Apr", "May", "Jun",
     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
static const char *am_pm[2] = {
     "AM", "PM"
};

#endif