#include "date_time.h"

VOID DateTime::DaysToMonthDay(UINT64 dayOfYear, UINT64 year, UINT32& outMonth, UINT32& outDay) noexcept
{
	BOOL isLeap = IsLeapYear(year);
	UINT32 month = 1;
	UINT64 remainingDays = dayOfYear;

	while (month <= 12)
	{
		UINT32 daysInMonth = GetDaysInMonth(month, isLeap);
		if (remainingDays < daysInMonth)
			break;
		remainingDays -= daysInMonth;
		month++;
	}

	outMonth = month;
	outDay = (UINT32)remainingDays + 1;  // Days are 1-indexed
}

VOID DateTime::FromDaysAndTime(DateTime& dt, UINT64 days, UINT64 baseYear,
                               UINT64 timeOfDaySeconds, UINT64 subSecondNanoseconds) noexcept
{
	// Fast-forward through years
	UINT64 year = baseYear;
	while (TRUE)
	{
		UINT32 daysInYear = IsLeapYear(year) ? 366u : 365u;
		if (days >= daysInYear)
		{
			days -= daysInYear;
			year++;
		}
		else
			break;
	}

	// Month and day
	UINT32 month, day;
	DaysToMonthDay(days, year, month, day);

	dt.Years = year;
	dt.Monthes = month;
	dt.Days = day;

	// Time of day
	dt.Hours = (UINT32)(timeOfDaySeconds / UINT64(3600u));
	dt.Minutes = (UINT32)((timeOfDaySeconds / UINT64(60u)) % UINT64(60u));
	dt.Seconds = (UINT32)(timeOfDaySeconds % UINT64(60u));

	// Sub-second precision
	dt.Milliseconds = subSecondNanoseconds / UINT64(1000000u);
	dt.Microseconds = (subSecondNanoseconds / UINT64(1000u)) % UINT64(1000u);
	dt.Nanoseconds = subSecondNanoseconds % UINT64(1000u);
}
