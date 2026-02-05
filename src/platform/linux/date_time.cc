#include "date_time.h"
#include "syscall.h"
#include "system.h"

DateTime DateTime::Now()
{
    DateTime dt;
    timespec ts;

    // Get current time using clock_gettime syscall
    SSIZE result = System::Call(SYS_CLOCK_GETTIME, CLOCK_REALTIME, (USIZE)&ts);
    if (result != 0)
    {
        // If syscall fails, return epoch time (1970-01-01 00:00:00)
        dt.Years = 1970;
        dt.Monthes = 1;
        dt.Days = 1;
        return dt;
    }

    // Convert Unix timestamp (seconds since 1970-01-01) to date/time
    UINT64 totalSeconds = (UINT64)ts.tv_sec;
    UINT64 nanoseconds = (UINT64)ts.tv_nsec;

    // Constants
    const UINT64 SECONDS_PER_DAY = 86400;
    const UINT64 SECONDS_PER_HOUR = 3600;
    const UINT64 SECONDS_PER_MINUTE = 60;

    // Calculate days since Unix epoch (1970-01-01)
    UINT64 days = totalSeconds / SECONDS_PER_DAY;
    UINT64 timeOfDay = totalSeconds % SECONDS_PER_DAY;

    // Time of day
    dt.Hours = (UINT32)(timeOfDay / SECONDS_PER_HOUR);
    dt.Minutes = (UINT32)((timeOfDay % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE);
    dt.Seconds = (UINT32)(timeOfDay % SECONDS_PER_MINUTE);

    // Sub-second precision
    dt.Milliseconds = nanoseconds / 1000000;
    dt.Microseconds = (nanoseconds / 1000) % 1000;
    dt.Nanoseconds = nanoseconds % 1000;

    // Convert days since 1970-01-01 to (Year, Month, Day)
    UINT64 year = 1970;

    // Fast-forward through years
    while (TRUE)
    {
        UINT32 daysInYear = DateTime::IsLeapYear(year) ? 366 : 365;
        if (days >= daysInYear)
        {
            days -= daysInYear;
            year++;
        }
        else
        {
            break;
        }
    }

    // Use shared helper to convert day-of-year to month and day
    UINT32 month, day;
    DateTime::DaysToMonthDay(days, year, month, day);

    dt.Years = year;
    dt.Monthes = month;
    dt.Days = day;

    return dt;
}

UINT64 DateTime::GetMonotonicNanoseconds()
{
    timespec ts;

    // Get monotonic time (not affected by system clock changes)
    SSIZE result = System::Call(SYS_CLOCK_GETTIME, CLOCK_MONOTONIC, (USIZE)&ts);
    if (result != 0)
    {
        // Fallback: return 0 if syscall fails
        return 0;
    }

    // Convert to nanoseconds
    UINT64 nanoseconds = ((UINT64)ts.tv_sec * 1000000000ULL) + (UINT64)ts.tv_nsec;
    return nanoseconds;
}
