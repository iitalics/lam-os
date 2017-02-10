#include "misc.h"

/// convert integer to string
#define ITOA_SIZE 32
u32 itoa (char* out, i32 n, u32 base)
{
    u32 tot = 0;
    if (n < 0) {
        *out++ = '-';
        n = -n;
        tot++;
    }

    u32 len = 0;
    {
        i32 k = 1;
        do { len++; k *= base; } while (k <= n);
    }
    out[len] = '\0';
    tot += len;

    do {
        i32 dig = n % base;
        if (dig >= 10)
            out[--len] = 'a' + dig - 10;
        else
            out[--len] = '0' + dig;
        n = n / base;
    } while (n != 0);
    return tot;
}

/// convert string to integer
i32 atoi (const char* in, char** new_pos, u32 base)
{
    while (*in == '0')
        in++;

    i32 sign = 1;
    if (*in == '-') {
        sign = -1;
        in++;
    }

    i32 acc = 0;
    for (;;) {
        u32 dig;
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
