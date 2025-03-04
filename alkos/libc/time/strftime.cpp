#include <assert.h>
#include <time.h>

#include <sys/calls.h>

#include <extensions/internal/formats.hpp>
#include <extensions/time.hpp>

class StrfTimeWriter final
{
    public:
    StrfTimeWriter(char* buf, const size_t size, const char* format, const tm* time_ptr)
        : buf_{buf}, format_{format}, size_{size}, written_{0}, time_ptr_{time_ptr}
    {
    }

    NODISCARD FORCE_INLINE_F bool Write()
    {
        if (size_ == 0) {
            return false;
        }

        /* we will copy null terminator inside the loop */
        do {
            const char c = *(format_++);

            if (c == '%') {
                ProcessFormat_();
            } else {
                buf_[written_++] = c;
            }

        } while (written_ < size_ && *format_ != '\0');

        /* If we have written all the characters, return true */
        return *format_ == '\0';
    }

    NODISCARD FORCE_INLINE_F size_t Written() const { return written_; }

    private:
    FORCE_INLINE_F void ProcessFormat_()
    {
        /* check for special modifiers E and O */
        char c = *(format_++);

        if (c == 'O' || c == 'E') {
            TODO_POSIX_COMPLIANCE

            /* skip the next character */
            c = *(format_++);
        }

        switch (c) {
            case 'Y':
                Y_Format_();
                break;
            case 'y':
                y_Format_();
                break;
            case 'm':
                m_Format_();
                break;
            case 'd':
                d_Format_();
                break;
            case 'H':
                H_Format_();
                break;
            case 'M':
                M_Format_();
                break;
            case 'S':
                S_Format_();
                break;
            case 'A':
                A_Format_();
                break;
            case 'a':
                a_Format_();
                break;
            case 'B':
                B_Format_();
                break;
            case 'b':
                b_Format_();
                break;
            case 'C':
                C_Format_();
                break;
            case 'D':
                D_Format_();
                break;
            case 'F':
                F_Format_();
                break;
            case 'I':
                I_Format_();
                break;
            case 'j':
                j_Format_();
                break;
            case 'p':
                p_Format_();
                break;
            case 'R':
                R_Format_();
                break;
            case 'T':
                T_Format_();
                break;
            case 'U':
                U_Format_();
                break;
            case 'W':
                W_Format_();
                break;
            case 'w':
                w_Format_();
                break;
            case 'x':
                x_Format_();
                break;
            case 'X':
                X_Format_();
                break;
            case 'Z':
                Z_Format_();
                break;
            case 'z':
                z_Format_();
                break;
            case '%':
                Percent_Format_();
                break;
            case 'e':
                e_Format_();
                break;
            case 'h':
                h_Format_();
                break;
            case 'g':
                g_Format_();
                break;
            case 'G':
                G_Format_();
                break;
            case 'u':
                u_Format_();
                break;
            case 'V':
                V_Format_();
                break;
            case 'r':
                r_Format();
                break;
            default:
                InvalidFormat_();
                break;
        }
    }

    // ------------------------------
    // Standard C formats
    // ------------------------------

    FORCE_INLINE_F void Y_Format_() { WriteUint_(kTmBaseYear + time_ptr_->tm_year); }

    FORCE_INLINE_F void y_Format_()
    {
        const uintmax_t nums = time_ptr_->tm_year % 100;
        Write2Digits_(nums);
    }

    FORCE_INLINE_F void m_Format_() { Write2Digits_(time_ptr_->tm_mon + 1); }

    FORCE_INLINE_F void d_Format_() { Write2Digits_(time_ptr_->tm_mday); }

    FORCE_INLINE_F void H_Format_() { Write2Digits_(time_ptr_->tm_hour); }

    FORCE_INLINE_F void M_Format_() { Write2Digits_(time_ptr_->tm_min); }

    FORCE_INLINE_F void S_Format_() { Write2Digits_(time_ptr_->tm_sec); }

    FORCE_INLINE_F void e_Format_()
    {
        if (time_ptr_->tm_mday < 10) {
            WriteChar_(' ');
        }
        WriteUint_(time_ptr_->tm_mday);
    }

    FORCE_INLINE_F void h_Format_() { b_Format_(); }

    FORCE_INLINE_F void A_Format_()
    {
        TODO_LOCALE_SUPPORT
        static constexpr const char* kWeekdays[] = {"Sunday",   "Monday", "Tuesday", "Wednesday",
                                                    "Thursday", "Friday", "Saturday"};

        WriteString_(kWeekdays[time_ptr_->tm_wday]);
    }

    FORCE_INLINE_F void a_Format_()
    {
        TODO_LOCALE_SUPPORT
        static constexpr const char* kWeekdays[] = {"Sun", "Mon", "Tue", "Wed",
                                                    "Thu", "Fri", "Sat"};

        WriteString_(kWeekdays[time_ptr_->tm_wday]);
    }

    FORCE_INLINE_F void B_Format_()
    {
        TODO_LOCALE_SUPPORT
        static constexpr const char* kMonths[] = {"January",   "February", "March",    "April",
                                                  "May",       "June",     "July",     "August",
                                                  "September", "October",  "November", "December"};

        WriteString_(kMonths[time_ptr_->tm_mon]);
    }

    FORCE_INLINE_F void b_Format_()
    {
        TODO_LOCALE_SUPPORT
        static constexpr const char* kMonths[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                                  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

        WriteString_(kMonths[time_ptr_->tm_mon]);
    }

    FORCE_INLINE_F void C_Format_() { Write2Digits_(19 + time_ptr_->tm_year / 100); }

    FORCE_INLINE_F void D_Format_()
    {
        m_Format_();
        WriteChar_('/');
        d_Format_();
        WriteChar_('/');
        y_Format_();
    }

    FORCE_INLINE_F void F_Format_()
    {
        Y_Format_();
        WriteChar_('-');
        m_Format_();
        WriteChar_('-');
        d_Format_();
    }

    FORCE_INLINE_F void I_Format_() { Write2Digits_(time_ptr_->tm_hour % 12); }

    FORCE_INLINE_F void j_Format_()
    {
        if (time_ptr_->tm_yday < 100) {
            WriteZeroBoundaryCheck_();
        }

        if (time_ptr_->tm_yday < 10) {
            WriteZeroBoundaryCheck_();
        }

        WriteUint_(time_ptr_->tm_yday);
    }

    FORCE_INLINE_F void p_Format_()
    {
        const char* am_pm = time_ptr_->tm_hour < 12 ? "AM" : "PM";
        WriteString_(am_pm);
    }

    FORCE_INLINE_F void R_Format_()
    {
        H_Format_();
        WriteChar_(':');
        M_Format_();
    }

    FORCE_INLINE_F void T_Format_()
    {
        H_Format_();
        WriteChar_(':');
        M_Format_();
        WriteChar_(':');
        S_Format_();
    }

    FORCE_INLINE_F void U_Format_()
    {
        const int64_t sunday_based_week = CalculateSundayBasedWeek(*time_ptr_);
        Write2Digits_(sunday_based_week);
    }

    FORCE_INLINE_F void W_Format_()
    {
        const int64_t monday_based_week = CalculateMondayBasedWeek(*time_ptr_);
        Write2Digits_(monday_based_week);
    }

    FORCE_INLINE_F void w_Format_() { WriteUint_(time_ptr_->tm_wday); }

    FORCE_INLINE_F void x_Format_()
    {
        TODO_LOCALE_SUPPORT
        D_Format_();
    }

    FORCE_INLINE_F void X_Format_()
    {
        TODO_LOCALE_SUPPORT
        I_Format_();
        p_Format_();
    }

    FORCE_INLINE_F void Z_Format_()
    {
        const auto time_zone = GetTimezoneSysCall();

        int64_t offset = time_zone.west_offset_minutes;
        if (time_ptr_->tm_isdst && time_zone.dst_time_offset_minutes) {
            offset += time_zone.dst_time_offset_minutes;
        }

        const bool is_negative = offset < 0;
        WriteChar_(is_negative ? '-' : '+');

        if (offset < 1000) {
            WriteChar_('0');
        }

        if (offset < 100) {
            WriteChar_('0');
        }

        if (offset < 10) {
            WriteChar_('0');
        }

        WriteUint_(is_negative ? -offset : offset);
    }

    FORCE_INLINE_F void z_Format_()
    {
        TODO_LOCALE_SUPPORT
        Z_Format_();
    }

    FORCE_INLINE_F void Percent_Format_() { WriteChar_('%'); }

    FORCE_INLINE_F void InvalidFormat_()
    {
        WriteChar_('%');
        WriteChar_(*(format_ - 1));
    }

    FORCE_INLINE_F void r_Format()
    {
        I_Format_();
        WriteChar_(':');
        M_Format_();
        WriteChar_(':');
        S_Format_();
        WriteChar_(' ');
        p_Format_();
    }

    // ------------------------------
    // ISO 8601
    // ------------------------------

    FORCE_INLINE_F void G_Format_()
    {
        const int64_t iso_year = CalculateIsoBasedYear(*time_ptr_);
        WriteUint_(iso_year);
    }

    FORCE_INLINE_F void g_Format_()
    {
        const int64_t iso_year = CalculateIsoBasedYear(*time_ptr_);
        Write2Digits_(iso_year % 100);
    }

    FORCE_INLINE_F void V_Format_()
    {
        const int64_t iso_week = CalculateIsoBasedWeek(*time_ptr_);
        Write2Digits_(iso_week);
    }

    FORCE_INLINE_F void u_Format_()
    {
        const int64_t iso_weekday = time_ptr_->tm_wday == 0 ? 6 : time_ptr_->tm_wday - 1;
        WriteUint_(iso_weekday + 1);
    }

    // ------------------------------
    // Helpers
    // ------------------------------

    FORCE_INLINE_F void WriteUint_(const uintmax_t num)
    {
        written_ += FormatUIntWoutNullTerm(num, buf_ + written_, size_ - written_);
    }

    FORCE_INLINE_F void WriteChar_(const char c)
    {
        if (written_ < size_) {
            buf_[written_++] = c;
        }
    }

    FORCE_INLINE_F void WriteZeroBoundaryCheck_() { WriteChar_('0'); }

    FORCE_INLINE_F void WriteString_(const char* str)
    {
        while (*str != '\0' && written_ < size_) {
            buf_[written_++] = *(str++);
        }
    }

    FORCE_INLINE_F void Write2Digits_(const uintmax_t num)
    {
        if (num < 10) {
            WriteZeroBoundaryCheck_();
        }

        WriteUint_(num);
    }

    char* const buf_;
    const char* format_;
    const size_t size_;
    size_t written_;
    const tm* const time_ptr_;
};

size_t strftime(char* s, const size_t max_size, const char* format, const tm* time_ptr)
{
    StrfTimeWriter writer(s, max_size, format, time_ptr);
    const bool is_success = writer.Write();
    return is_success ? writer.Written() : 0;
}
