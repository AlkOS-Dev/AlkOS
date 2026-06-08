#include "stdlib.h"

int atoi(const char *str)
{
    int result = 0;
    int sign   = 1;

    // Skip whitespace
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
        str++;
    }

    // Handle sign
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    // Convert digits
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }

    return sign * result;
}

double atof(const char *str)
{
    if (!str) {
        return 0.0;
    }

    double result      = 0.0;
    double sign        = 1.0;
    int decimal_places = 0;

    // Skip whitespace
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
        str++;
    }

    // Handle sign
    if (*str == '-') {
        sign = -1.0;
        str++;
    } else if (*str == '+') {
        str++;
    }

    // Convert digits before decimal point
    while (*str >= '0' && *str <= '9') {
        result = result * 10.0 + (*str - '0');
        str++;
    }

    // Handle decimal point
    if (*str == '.') {
        str++;
        double divisor = 10.0;
        while (*str >= '0' && *str <= '9') {
            result += (*str - '0') / divisor;
            divisor *= 10.0;
            str++;
        }
    }

    return sign * result;
}
