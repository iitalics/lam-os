#include "misc.h"
#include <limits.h>

u32 str_copy_cap (char* dst, u32 cap, const char* src)
{
    u32 i;
    for (i = 0; src[i] && i < (cap - 1); i++)
        dst[i] = src[i];
    dst[i] = '\0';
    return i;
}

#if (__WORDSIZE == 32 || WORD_BIT == 32) && CHAR_BIT == 8
#define DUFF32
#endif

void mem_copy (void* dst, void* src, u32 size)
{
#ifdef DUFF32
    {
        u32* src_l = (u32*) src;
        u32* dst_l = (u32*) dst;
        i32 n = size >> 2;
        switch (n % 8) {
            while (n > 0) {
            case 0: *dst_l++ = *src_l++; n--;
            case 1: *dst_l++ = *src_l++; n--;
            case 2: *dst_l++ = *src_l++; n--;
            case 3: *dst_l++ = *src_l++; n--;
            case 4: *dst_l++ = *src_l++; n--;
            case 5: *dst_l++ = *src_l++; n--;
            case 6: *dst_l++ = *src_l++; n--;
            case 7: *dst_l++ = *src_l++; n--;
            }
            break;
        }
        size = size & 3;
        dst = dst_l;
        src = src_l;
    }
#endif
    for (u32 i = 0; i < size; i++)
        ((char*) dst)[i] = ((char*) src)[i];
}

void mem_fill (void* dst, u32 size, u8 with)
{
#ifdef DUFF32
    {
        u32* dst_l = (u32*) dst;
        u32 with_l = with
            | (((u32) with) << 8)
            | (((u32) with) << 16)
            | (((u32) with) << 24);
        i32 n = size >> 2;
        switch (n % 8) {
            while (n > 0) {
            case 0: *dst_l++ = with_l; n--;
            case 1: *dst_l++ = with_l; n--;
            case 2: *dst_l++ = with_l; n--;
            case 3: *dst_l++ = with_l; n--;
            case 4: *dst_l++ = with_l; n--;
            case 5: *dst_l++ = with_l; n--;
            case 6: *dst_l++ = with_l; n--;
            case 7: *dst_l++ = with_l; n--;
            }
            break;
        }
        size = size & 3;
        dst = dst_l;
    }
#endif
    for (u32 i = 0; i < size; i++)
        ((char*) dst)[i] = with;
}

void mem_clear (void* dst, u32 size)
{
    mem_fill(dst, size, 0);
}

/// convert integer to string
#define ITOA_SIZE 32
u32 itoa (char* out, i32 n, u32 base)
{
    if (n < 0) {
        *out++ = '-';
        return 1 + itoau(out, -n, base);
    }
    else {
        return itoau(out, n, base);
    }
}
u32 itoau (char* out, u32 n, u32 base)
{
    char* start = out;

    char tmp[16];
    u32 idx = sizeof tmp;
    do {
        int dig = n % base;
        if (dig >= 10)
            tmp[--idx] = dig - 10 + 'a';
        else
            tmp[--idx] = dig + '0';

        n = n / base;
    } while (n != 0);

    while (idx < sizeof tmp)
        *out++ = tmp[idx++];

    *out = '\0';
    return (u32) (out - start);
}

/// convert string to integer
i32 atoi (const char* in, char** new_pos, u32 base)
{
    while (*in == ' ')
        in++;

    if (*in == '-')
        return -atoiu(in + 1, new_pos, base);
    else
        return atoiu(in + 1, new_pos, base);
}
u32 atoiu (const char* in, char** new_pos, u32 base)
{
    u32 acc = 0;
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
    return acc;
}
