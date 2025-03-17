#include <drivers/cmos/cmos.hpp>
#include <drivers/cmos/rtc.hpp>
#include <todo.hpp>

#include <extensions/time.hpp>

// ------------------------------
// RTC registers
// ------------------------------

static constexpr byte kRtcSecondsRegister = 0x00;
static constexpr byte kRtcMinutesRegister = 0x02;
static constexpr byte kRtcHoursRegister   = 0x04;
static constexpr byte kRtcWeekdayRegister = 0x06;
static constexpr byte kRtcDayRegister     = 0x07;
static constexpr byte kRtcMonthRegister   = 0x08;
static constexpr byte kRtcYearRegister    = 0x09;
static constexpr byte kRtcStatusRegisterA = 0x0A;
static constexpr byte kRtcStatusRegisterB = 0x0B;

// ------------------------------
// RTC bit flags
// ------------------------------

static constexpr byte kRtcStatusAInProgressFlag = 0x80;
static constexpr byte kRtcStatusBBinaryFlag     = 0x04;
static constexpr byte kRtcStatusB24HourFlag     = 0x02;

static constexpr byte kRtcPmBit = 0x80;

// ------------------------------
// static elements
// ------------------------------

/* There might be a century register available on the board */
/* But this must be detected by ACPI */
TODO_WHEN_ACPI_WORKS
static u64 g_CenturyRegister = 0x0;

FAST_CALL bool IsUpdateInProgress()
{
    return ReadCmosRegister(kRtcStatusRegisterA) & kRtcStatusAInProgressFlag;
}

FAST_CALL bool IsBCD() { return !(ReadCmosRegister(kRtcStatusRegisterB) & kRtcStatusBBinaryFlag); }

FAST_CALL bool Is12HourClock()
{
    return !(ReadCmosRegister(kRtcStatusRegisterB) & kRtcStatusB24HourFlag);
}

struct RtcTime {
    tm time;
    u32 century;
};

FAST_CALL bool CompareTimes(const RtcTime &time, const RtcTime &last_time)
{
    return time.time.tm_sec == last_time.time.tm_sec && time.time.tm_min == last_time.time.tm_min &&
           time.time.tm_hour == last_time.time.tm_hour &&
           time.time.tm_mday == last_time.time.tm_mday &&
           time.time.tm_mon == last_time.time.tm_mon &&
           time.time.tm_year == last_time.time.tm_year && time.century == last_time.century;
}

FAST_CALL RtcTime FillRtcStruct()
{
    RtcTime time;

    while (IsUpdateInProgress()) {
        /* Wait for the update to finish */
    }

    time.century      = g_CenturyRegister != 0 ? ReadCmosRegister(g_CenturyRegister) : 0;
    time.time.tm_sec  = ReadCmosRegister(kRtcSecondsRegister);
    time.time.tm_min  = ReadCmosRegister(kRtcMinutesRegister);
    time.time.tm_hour = ReadCmosRegister(kRtcHoursRegister);
    time.time.tm_mday = ReadCmosRegister(kRtcDayRegister);
    time.time.tm_mon  = ReadCmosRegister(kRtcMonthRegister);
    time.time.tm_year = ReadCmosRegister(kRtcYearRegister);

    return time;
}

FAST_CALL int ConvertBcdToBinary(const int in)
{
    return ((in & 0xF0) >> 1) + ((in & 0xF0) >> 3) + (in & 0xf);
}

FAST_CALL void ConvertBcdToBinary(RtcTime &time)
{
    time.time.tm_sec  = ConvertBcdToBinary(time.time.tm_sec);
    time.time.tm_min  = ConvertBcdToBinary(time.time.tm_min);
    time.time.tm_hour = ConvertBcdToBinary(time.time.tm_hour);
    time.time.tm_mday = ConvertBcdToBinary(time.time.tm_mday);
    time.time.tm_mon  = ConvertBcdToBinary(time.time.tm_mon);
    time.time.tm_year = ConvertBcdToBinary(time.time.tm_year);
}

// ------------------------------
// ACPI function
// ------------------------------

void SetCenturyRegisterAddress(const u64 address) { g_CenturyRegister = address; }

// ---------------------------------
// RTC querying implementation
// ---------------------------------

tm ReadRtcTime()
{
    RtcTime time = FillRtcStruct();

    RtcTime last_time;
    do {
        last_time = time;
        time      = FillRtcStruct();
    } while (!CompareTimes(time, last_time));

    /* Convert BCD to binary if necessary */
    if (IsBCD()) {
        ConvertBcdToBinary(time);
    }

    /* Convert 12-hour clock to 24-hour clock */
    if (Is12HourClock() && (time.time.tm_hour & kRtcPmBit)) {
        time.time.tm_hour = ((time.time.tm_hour & ~kRtcPmBit) + 12) % 24;
    }

    /* Adjust year by the century year */
    if (g_CenturyRegister != 0) {
        time.time.tm_year += time.century * 100;
    } else {
        time.time.tm_year += kOsCentury * 100;

        /* will work also for next century */
        if (time.time.tm_year < kOsYear) {
            time.time.tm_year += 100;
        }
    }

    /* Adjust year to match tm struct */
    time.time.tm_year -= kTmBaseYear;
    /* Adjust month to match tm struct */
    time.time.tm_mon -= 1;
    /* Disable dst guess */
    time.time.tm_isdst = 0;

    return time.time;
}
