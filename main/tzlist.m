// Timezone list
// Does not have to list all, as can be set via web config

#ifndef	tz
#define	tz(name,...)
#endif

tz(UTC,UTC0)
tz(UK,GMT+0BST,M3.5.0,M10.5.0)
tz(CET,CET-1CEST,M3.5.0,M10.5.0/3)
tz(EET,EET-2EEST,M3.5.0/3,M10.5.0/4)
tz(UTC+1,X1)
tz(UTC+2,X2)
tz(UTC+3,X3)
tz(UTC+4,X4)
tz(UTC+5,X5)
tz(UTC+6,X6)
tz(UTC+7,X7)
tz(UTC+8,X8)
tz(UTC+9,X9)
tz(UTC+10,X10)
tz(UTC+11,X11)
tz(UTC+12,X12)
tz(UTC-12,X-12)
tz(UTC-11,X-11)
tz(UTC-10,X-10)
tz(UTC-9,X-9)
tz(UTC-8,X-8)
tz(UTC-7,X-7)
tz(UTC-6,X-6)
tz(UTC-5,X-5)
tz(UTC-4,X-4)
tz(UTC-3,X-3)
tz(UTC-2,X-2)
tz(UTC-1,X-1)

#undef	tz
