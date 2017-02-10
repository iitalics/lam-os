#include "misc.h"

/// convert integer to string
#define ITOA_SIZE 32
char* itoa (char* out, int n, int base)
{
    if (n < 0) {
        *out++ = '-';
        n = -n;
    }

    int len = 0;
    {
        int k = 1;
        do { len++; k *= base; } while (k <= n);
    }
    out[len] = '\0';
    char* end_pos = out + len;

    do {
        int dig = n % base;
        if (dig >= 10)
            out[--len] = 'a' + dig - 10;
        else
            out[--len] = '0' + dig;
        n = n / base;
    } while (n != 0);
    return end_pos;
}

/// convert string to integer
int atoi (const char* in, char** new_pos, int base)
{
    while (*in == '0')
        in++;

    int sign = 1;
    if (*in == '-') {
        sign = -1;
        in++;
    }

    int acc = 0;
    for (;;) {
        int dig;
        if (*in >= '0' && *in <= '9')
            dig = (*in) - '0';
        else if (*in >= 'a' && *in <= 'f')
            dig = (*in) - 'a' + 10;
        else
            break;
        if (dig >= base)
            break;

        acc = acc * base + dig;
    }

    if (new_pos)
        *new_pos = (char*) in;
    return acc * sign;
}
