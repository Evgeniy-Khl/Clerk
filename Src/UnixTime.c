#include "UnixTime.h"

//extern unixColodar unixTime;
extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef sDate;

//uint32_t colodarSetting(int year, char month, char day, char hour, char min, char sec)
//{
// unixTime.year=year;
// unixTime.mon=month;
// unixTime.mday=day;
// unixTime.hour=hour;
// unixTime.min=min;
// unixTime.sec=sec;
// return colodarToCounter(&unixTime);
//}
/*
void counterToColodar (unsigned long counter, unixColodar * unixTime)
{
	unsigned long a;
	char b;
	char c;
	char d;
	unsigned long time;

	time = counter%SECOND_A_DAY;
	a = ((counter+43200)/(SECOND_A_DAY>>1)) + (2440587<<1) + 1;
	a>>=1;
	unixTime->wday = a%7;
	a+=32044;
	b=(4*a+3)/146097;
	a=a-(146097*b)/4;
	c=(4*a+3)/1461;
	a=a-(1461*c)/4;
	d=(5*a+2)/153;
	unixTime->mday=a-(153*d+2)/5+1;
	unixTime->mon=d+3-12*(d/10);
	unixTime->year=100*b+c-4800+(d/10);
	unixTime->hour=time/3600;
	unixTime->min=(time%3600)/60;
	unixTime->sec=(time%3600)%60;
}
*/
unsigned long colodarToCounter (void)
{
	char a;
	int y;
	char m;
	unsigned long Uday;
	unsigned long time;

	a=((14-sDate.Month)/12);
	y=sDate.Year+4800-a;
	m=sDate.Month+(12*a)-3;
	Uday=(sDate.Date+((153*m+2)/5)+365*y+(y/4)-(y/100)+(y/400)-32045)-2440588;
	time=Uday*SECOND_A_DAY;
	time+=sTime.Seconds+sTime.Minutes*60+sTime.Hours*3600;
	return time;
}



