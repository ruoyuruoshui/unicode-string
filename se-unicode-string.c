
/*
 * Copyright (C) 2000-2007 Beijing Komoxo Inc.
 * All rights reserved.
 */

#ifndef SE_UNICODE_H
    #include "se-unicode.h"
#endif

#ifndef SE_UTILS_H
    #include "se-utils.h"
#endif

/*

From Unicode Standard:


Well-Formed UTF-8 Byte Sequences
================================

    Code Points         1st Byte   2nd Byte   3rd Byte   4th Byte
    ------------------  --------   --------   --------   --------
    U+0000..U+007F      00..7F
    U+0080..U+07FF      C2..DF     80..BF
    U+0800..U+0FFF      E0         A0..BF     80..BF
    U+1000..U+CFFF      E1..EC     80..BF     80..BF
    U+D000..U+D7FF      ED         80..9F     80..BF
    U+E000..U+FFFF      EE..EF     80..BF     80..BF
    U+10000..U+3FFFF    F0         90..BF     80..BF     80..BF
    U+40000..U+FFFFF    F1..F3     80..BF     80..BF     80..BF
    U+100000..U+10FFFF  F4         80..8F     80..BF     80..BF
    ------------------  --------   --------   --------   --------


List of noncharacters
=====================

    U+FDD0..U+FDEF
    U+FFFE..U+FFFF
    U+1FFFE..U+1FFFF
    U+2FFFE..U+2FFFF
    U+3FFFE..U+3FFFF
    U+4FFFE..U+4FFFF
    U+5FFFE..U+5FFFF
    U+6FFFE..U+6FFFF
    U+7FFFE..U+7FFFF
    U+8FFFE..U+8FFFF
    U+9FFFE..U+9FFFF
    U+AFFFE..U+AFFFF
    U+BFFFE..U+BFFFF
    U+CFFFE..U+CFFFF
    U+DFFFE..U+DFFFF
    U+EFFFE..U+EFFFF
    U+FFFFE..U+FFFFF
    U+10FFFE..U+10FFFF

*/

/* Replacement character for ill-formed codes */
#define SE_REPLACEMENT_CHAR                 0xFFFD
#define SE_REPLACEMENT_CHAR_UTF8_1          '\xEF'
#define SE_REPLACEMENT_CHAR_UTF8_2          '\xBF'
#define SE_REPLACEMENT_CHAR_UTF8_3          '\xBD'
#define SE_REPLACEMENT_CHAR_UTF8_1_CODE     0xEF
#define SE_REPLACEMENT_CHAR_UTF8_2_CODE     0xBF
#define SE_REPLACEMENT_CHAR_UTF8_3_CODE     0xBD

#define SE_IS_VALID_SCALAR_VALUE(c) ( (c) < 0xD800 || ((c) >= 0xE000 && (c) < 0x110000) )
#define SE_IS_NONCHARACTER(c)       ( ((c) >= 0xFDD0 && (c) < 0xFDF0) || ((c) < 0x110000 && ((c) & 0xFFFE) != 0xFFFE) );

#define SE_IS_SURROGATE(c)          ( (c) >= 0xD800 && (c) < 0xE000 )
#define SE_IS_HI_SURROGATE(c)       ( (c) >= 0xD800 && (c) < 0xDC00 )
#define SE_IS_LO_SURROGATE(c)       ( (c) >= 0xDC00 && (c) < 0xE000 )
#define SE_SURROGATE_VALUE(hi, lo)  (seunichar32) ( ((((hi) & 0x3FF) << 10) | ((lo) & 0x3FF)) + 0x10000 )

#define VALIDATE(exp)               if (!(exp)) return FALSE
#define VALIDATE1(exp)              if (!(exp)) goto ill_formed_1
#define VALIDATE2(exp)              if (!(exp)) goto ill_formed_2

SE_API sebool se_unichar_is_valid(seunichar c)
{
    return SE_IS_VALID_SCALAR_VALUE(c);
}

SE_API sebool se_unichar_is_surrogate(seunichar c)
{
    return SE_IS_SURROGATE(c);
}

SE_API sebool se_unichar_is_hi_surrogate(seunichar c)
{
    return SE_IS_HI_SURROGATE(c);
}

SE_API sebool se_unichar_is_lo_surrogate(seunichar c)
{
    return SE_IS_LO_SURROGATE(c);
}

SE_API sebool se_unichar_is_noncharacter(seunichar c)
{
    return SE_IS_NONCHARACTER(c);
}

/***************************************************************************
 *                                                                         *
 * Unicode character convertion.                                           *
 *                                                                         *
 * For un-safe characters.                                                 *
 *                                                                         *
 ***************************************************************************/

SE_API int se_unsafe_unichar_to_utf8(seunichar c, seunichar8* buf)
{
    int len;

    if (!SE_IS_VALID_SCALAR_VALUE(c))
    {
        if (buf)
        {
            buf[0] = SE_REPLACEMENT_CHAR_UTF8_1;
            buf[1] = SE_REPLACEMENT_CHAR_UTF8_2;
            buf[2] = SE_REPLACEMENT_CHAR_UTF8_3;
        }
        len = 3;
    }
    else if (c < 0x80)
    {
        if (buf)
        {
            buf[0] = (seunichar8) c;
        }
        len = 1;
    }
    else if (c < 0x800)
    {
        if (buf)
        {
            buf[1] = (seunichar8) ((c & 0x3F) | 0x80);
            buf[0] = (seunichar8) ((c >> 6) | 0xC0);
        }
        len = 2;
    }
    else if (c < 0x10000)
    {
        if (buf)
        {
            buf[2] = (seunichar8) ((c & 0x3F) | 0x80);
            c >>= 6;
            buf[1] = (seunichar8) ((c & 0x3F) | 0x80);
            buf[0] = (seunichar8) ((c >> 6) | 0xE0);
        }
        len = 3;
    }
    else
    {
        if (buf)
        {
            buf[3] = (seunichar8) ((c & 0x3F) | 0x80);
            c >>= 6;
            buf[2] = (seunichar8) ((c & 0x3F) | 0x80);
            c >>= 6;
            buf[1] = (seunichar8) ((c & 0x3F) | 0x80);
            buf[0] = (seunichar8) ((c >> 6) | 0xF0);
        }
        len = 4;
    }

    #if SE_OPT_DEBUG
        if (buf)
        {
            SE_DEBUG_ASSERT(se_is_valid_utf8_str(buf, len));
        }
    #endif

    return len;
}

SE_API int se_unsafe_unichar_to_utf16(seunichar c, seunichar16* buf)
{
    int len;

    if (!SE_IS_VALID_SCALAR_VALUE(c))
    {
        if (buf)
        {
            buf[0] = SE_REPLACEMENT_CHAR;
        }
        len = 1;
    }
    else if (c < 0x10000)
    {
        if (buf)
        {
            buf[0] = (seunichar16) c;
        }
        len = 1;
    }
    else
    {
        if (buf)
        {
            c -= 0x10000;
            buf[1] = (seunichar16) ((c & 0x3FF) | 0xDC00);
            buf[0] = (seunichar16) ((c >> 10) | 0xD800);
        }
        len = 2;
    }

    #if SE_OPT_DEBUG
        if (buf)
        {
            SE_DEBUG_ASSERT(se_is_valid_utf16_str(buf, len));
        }
    #endif

    return len;
}

SE_API seunichar32 se_unsafe_utf8_get_char(const seunichar8* str)
{
    const unsigned char* iter;

    SE_DEBUG_ASSERT(str);

    iter = (const unsigned char*)str;
    if (iter[0] <= 0x7F)
    {
        return iter[0];
    }
    else if (iter[0] >= 0xC2 && iter[0] <= 0xDF)
    {
        if (iter[1] >= 0x80 && iter[1] <= 0xBF)
        {
            return (seunichar32) ( ((iter[0] & 0x1F) << 6) | (iter[1] & 0x3F) );
        }
    }
    else if (iter[0] == 0xE0)
    {
        if (iter[1] >= 0xA0 && iter[1] <= 0xBF &&
            iter[2] >= 0x80 && iter[2] <= 0xBF)
        {
            return (seunichar32) ( ((iter[1] & 0x3F) << 6) | (iter[2] & 0x3F) );
        }
    }
    else if (iter[0] >= 0xE1 && iter[0] <= 0xEC)
    {
        if (iter[1] >= 0x80 && iter[1] <= 0xBF &&
            iter[2] >= 0x80 && iter[2] <= 0xBF)
        {
            return (seunichar32) ( ((iter[0] & 0x0F) << 12) | ((iter[1] & 0x3F) << 6) | (iter[2] & 0x3F) );
        }
    }
    else if (iter[0] == 0xED)
    {
        if (iter[1] >= 0x80 && iter[1] <= 0x9F &&
            iter[2] >= 0x80 && iter[2] <= 0xBF)
        {
            return (seunichar32) ( 0xD000 | ((iter[1] & 0x3F) << 6) | (iter[2] & 0x3F) );
        }
    }
    else if (iter[0] >= 0xEE && iter[0] <= 0xEF)
    {
        if (iter[1] >= 0x80 && iter[1] <= 0xBF &&
            iter[2] >= 0x80 && iter[2] <= 0xBF)
        {
            return (seunichar32) ( ((iter[0] & 0x0F) << 12) | ((iter[1] & 0x3F) << 6) | (iter[2] & 0x3F) );
        }
    }
    else if (iter[0] == 0xF0)
    {
        if (iter[1] >= 0x90 && iter[1] <= 0xBF &&
            iter[2] >= 0x80 && iter[2] <= 0xBF &&
            iter[3] >= 0x80 && iter[3] <= 0xBF)
        {
            return (seunichar32) ( ((iter[1] & 0x3F) << 12) | ((iter[2] & 0x3F) << 6) | (iter[3] & 0x3F) );
        }
    }
    else if (iter[0] >= 0xF1 && iter[0] <= 0xF3)
    {
        if (iter[1] >= 0x80 && iter[1] <= 0xBF &&
            iter[2] >= 0x80 && iter[2] <= 0xBF &&
            iter[3] >= 0x80 && iter[3] <= 0xBF)
        {
            return (seunichar32) ( ((iter[0] & 7) << 18) | ((iter[1] & 0x3F) << 12) | ((iter[2] & 0x3F) << 6) | (iter[3] & 0x3F) );
        }
    }
    else if (iter[0] == 0xF4)
    {
        if (iter[1] >= 0x80 && iter[1] <= 0x8F &&
            iter[2] >= 0x80 && iter[2] <= 0xBF &&
            iter[3] >= 0x80 && iter[3] <= 0xBF)
        {
            return (seunichar32) ( 0x100000 | ((iter[1] & 0x3F) << 12) | ((iter[2] & 0x3F) << 6) | (iter[3] & 0x3F) );
        }
    }

    return SE_REPLACEMENT_CHAR;
}

SE_API seunichar32 se_unsafe_utf16_get_char(const seunichar16* str)
{
    #if SE_OPT_SURROGATE

        SE_DEBUG_ASSERT(str);

        if (SE_IS_HI_SURROGATE(str[0]))
        {
            if (SE_IS_LO_SURROGATE(str[1]))
                return SE_SURROGATE_VALUE(str[0], str[1]);
        }
        else if (SE_IS_LO_SURROGATE(str[0]))
        {
            return SE_REPLACEMENT_CHAR;
        }
        else
        {
            return str[0];
        }

        return SE_REPLACEMENT_CHAR;

    #else

        SE_DEBUG_ASSERT(str);

        return *str;

    #endif
}

/***************************************************************************
 *                                                                         *
 * Unicode character convertion.                                           *
 *                                                                         *
 * For safe characters.                                                    *
 *                                                                         *
 ***************************************************************************/

SE_API int se_safe_unichar_to_utf8(seunichar c, seunichar8* buf)
/*
 * c:
 *      Input Unicode character code.
 *
 * buf:
 *      Pointer to an UTF-8 string buffer for return output string.
 *      (Can be NULL to indicate that the result is not needed.)
 * 
 * Convert a single character into UTF-8. Output buffer must have at
 * least 4 bytes of space. Returns the number of bytes in the
 * result.
 */
{
    int len;

    SE_DEBUG_ASSERT(SE_IS_VALID_SCALAR_VALUE(c));

    if (c < 0x80)
    {
        if (buf)
        {
            buf[0] = (seunichar8) c;
        }
        len = 1;
    }
    else if (c < 0x800)
    {
        if (buf)
        {
            buf[1] = (seunichar8) ((c & 0x3F) | 0x80);
            buf[0] = (seunichar8) ((c >> 6) | 0xC0);
        }
        len = 2;
    }
    else if (c < 0x10000)
    {
        if (buf)
        {
            buf[2] = (seunichar8) ((c & 0x3F) | 0x80);
            c >>= 6;
            buf[1] = (seunichar8) ((c & 0x3F) | 0x80);
            buf[0] = (seunichar8) ((c >> 6) | 0xE0);
        }
        len = 3;
    }
    else
    {
        if (buf)
        {
            buf[3] = (seunichar8) ((c & 0x3F) | 0x80);
            c >>= 6;
            buf[2] = (seunichar8) ((c & 0x3F) | 0x80);
            c >>= 6;
            buf[1] = (seunichar8) ((c & 0x3F) | 0x80);
            buf[0] = (seunichar8) ((c >> 6) | 0xF0);
        }
        len = 4;
    }

    #if SE_OPT_DEBUG
        if (buf)
        {
            SE_DEBUG_ASSERT(se_is_valid_utf8_str(buf, len));
        }
    #endif

    return len;
}

SE_API int se_safe_unichar_to_utf16(seunichar c, seunichar16* buf)
{
    int len;

    SE_DEBUG_ASSERT(SE_IS_VALID_SCALAR_VALUE(c));

    if (c < 0x10000)
    {
        if (buf)
        {
            buf[0] = (seunichar16) c;
        }
        len = 1;
    }
    else
    {
        if (buf)
        {
            c -= 0x10000;
            buf[1] = (seunichar16) ((c & 0x3FF) | 0xDC00);
            buf[0] = (seunichar16) ((c >> 10) | 0xD800);
        }
        len = 2;
    }

    #if SE_OPT_DEBUG
        if (buf)
        {
            SE_DEBUG_ASSERT(se_is_valid_utf16_str(buf, len));
        }
    #endif

    return len;
}

SE_API seunichar32 se_safe_utf8_get_char(const seunichar8* str)
{
    const unsigned char* iter;

    SE_DEBUG_ASSERT(str);

    iter = (const unsigned char*)str;
    if (iter[0] <= 0x7F)
    {
        return iter[0];
    }
    else if (iter[0] >= 0xC2 && iter[0] <= 0xDF)
    {
        SE_DEBUG_ASSERT(iter[1] >= 0x80 && iter[1] <= 0xBF);
        return (seunichar32) ( ((iter[0] & 0x1F) << 6) | (iter[1] & 0x3F) );
    }
    else if (iter[0] == 0xE0)
    {
        SE_DEBUG_ASSERT(iter[1] >= 0xA0 && iter[1] <= 0xBF);
        SE_DEBUG_ASSERT(iter[2] >= 0x80 && iter[2] <= 0xBF);
        return (seunichar32) ( ((iter[1] & 0x3F) << 6) | (iter[2] & 0x3F) );
    }
    else if (iter[0] >= 0xE1 && iter[0] <= 0xEC)
    {
        SE_DEBUG_ASSERT(iter[1] >= 0x80 && iter[1] <= 0xBF);
        SE_DEBUG_ASSERT(iter[2] >= 0x80 && iter[2] <= 0xBF);
        return (seunichar32) ( ((iter[0] & 0x0F) << 12) | ((iter[1] & 0x3F) << 6) | (iter[2] & 0x3F) );
    }
    else if (iter[0] == 0xED)
    {
        SE_DEBUG_ASSERT(iter[1] >= 0x80 && iter[1] <= 0x9F);
        SE_DEBUG_ASSERT(iter[2] >= 0x80 && iter[2] <= 0xBF);
        return (seunichar32) ( 0xD000 | ((iter[1] & 0x3F) << 6) | (iter[2] & 0x3F) );
    }
    else if (iter[0] >= 0xEE && iter[0] <= 0xEF)
    {
        SE_DEBUG_ASSERT(iter[1] >= 0x80 && iter[1] <= 0xBF);
        SE_DEBUG_ASSERT(iter[2] >= 0x80 && iter[2] <= 0xBF);
        return (seunichar32) ( ((iter[0] & 0x0F) << 12) | ((iter[1] & 0x3F) << 6) | (iter[2] & 0x3F) );
    }
    else if (iter[0] == 0xF0)
    {
        SE_DEBUG_ASSERT(iter[1] >= 0x90 && iter[1] <= 0xBF);
        SE_DEBUG_ASSERT(iter[2] >= 0x80 && iter[2] <= 0xBF);
        SE_DEBUG_ASSERT(iter[3] >= 0x80 && iter[3] <= 0xBF);
        return (seunichar32) ( ((iter[1] & 0x3F) << 12) | ((iter[2] & 0x3F) << 6) | (iter[3] & 0x3F) );
    }
    else if (iter[0] >= 0xF1 && iter[0] <= 0xF3)
    {
        SE_DEBUG_ASSERT(iter[1] >= 0x80 && iter[1] <= 0xBF);
        SE_DEBUG_ASSERT(iter[2] >= 0x80 && iter[2] <= 0xBF);
        SE_DEBUG_ASSERT(iter[3] >= 0x80 && iter[3] <= 0xBF);
        return (seunichar32) ( ((iter[0] & 7) << 18) | ((iter[1] & 0x3F) << 12) | ((iter[2] & 0x3F) << 6) | (iter[3] & 0x3F) );
    }
    else
    {
        SE_DEBUG_ASSERT(iter[0] == 0xF4);
        SE_DEBUG_ASSERT(iter[1] >= 0x80 && iter[1] <= 0x8F);
        SE_DEBUG_ASSERT(iter[2] >= 0x80 && iter[2] <= 0xBF);
        SE_DEBUG_ASSERT(iter[3] >= 0x80 && iter[3] <= 0xBF);
        return (seunichar32) ( 0x100000 | ((iter[1] & 0x3F) << 12) | ((iter[2] & 0x3F) << 6) | (iter[3] & 0x3F) );
    }
}

#if SE_OPT_SURROGATE
SE_API seunichar32 se_safe_utf16_get_char(const seunichar16* str)
{
    SE_DEBUG_ASSERT(str);

    if (SE_IS_HI_SURROGATE(str[0]))
    {
        SE_DEBUG_ASSERT(SE_IS_LO_SURROGATE(str[1]));
        return SE_SURROGATE_VALUE(str[0], str[1]);
    }
    else
    {
        SE_DEBUG_ASSERT(!SE_IS_LO_SURROGATE(str[0]));
        SE_DEBUG_ASSERT(SE_IS_VALID_SCALAR_VALUE(str[0]));
        return str[0];
    }
}
#endif

/***************************************************************************
 *                                                                         *
 * NUL terminated Unicode string length in units.                          *
 *                                                                         *
 * For both safe and un-safe strings.                                      *
 *                                                                         *
 ***************************************************************************/

SE_API int se_utf8_str_len(const seunichar8* str)
{
    SE_DEBUG_ASSERT(str);

    return strlen(str);
}

SE_API int se_utf16_str_len(const seunichar16* str)
{
    const seunichar16* iter;

    SE_DEBUG_ASSERT(str);

    iter = str;
    while (*iter)
        iter++;

    return iter - str;
}

SE_API int se_utf32_str_len(const seunichar32* str)
{
    const seunichar32* iter;

    SE_DEBUG_ASSERT(str);

    iter = str;
    while (*iter)
        iter++;

    return iter - str;
}

/***************************************************************************
 *                                                                         *
 * Unicode string validation.                                              *
 *                                                                         *
 * For un-safe strings.                                                    *
 *                                                                         *
 ***************************************************************************/

SE_API sebool se_is_valid_utf8_str(const seunichar8* str, int len)
{
    const unsigned char* iter;
    const unsigned char* end;

    SE_DEBUG_ASSERT(str);

    if (len < 0)
        len = se_utf8_str_len(str);

    iter = (const unsigned char*)str;
    end = iter + len;
    while (iter < end)
    {
        if (iter[0] <= 0x7F)
        {
            /*
             * U+0000..U+007F
             *
             * 0xxxxxxx => 0xxxxxxx
             *             00....7F
             */
            iter++;
        }
        else if (iter[0] >= 0xC2 && iter[0] <= 0xDF)
        {
            /*
             * U+0080..U+07FF
             *
             * 00000yyy yyxxxxxx => 110yyyyy 10xxxxxx
             *                      C2....DF 80....BF
             */
            VALIDATE(iter + 2 <= end);
            VALIDATE(iter[1] >= 0x80 && iter[1] <= 0xBF);
            iter += 2;
        }
        else if (iter[0] == 0xE0)
        {
            /*
             * U+0800..U+0FFF
             *
             * zzzzyyyy yyxxxxxx => 1110zzzz 10yyyyyy 10xxxxxx
             *                      E0       A0....BF 80....BF
             */
            VALIDATE(iter + 3 <= end);
            VALIDATE(iter[1] >= 0xA0 && iter[1] <= 0xBF);
            VALIDATE(iter[2] >= 0x80 && iter[2] <= 0xBF);
            iter += 3;
        }
        else if (iter[0] >= 0xE1 && iter[0] <= 0xEC)
        {
            /*
             * U+1000..U+CFFF
             *
             * zzzzyyyy yyxxxxxx => 1110zzzz 10yyyyyy 10xxxxxx
             *                      E1....EC 80....BF 80....BF
             */
            VALIDATE(iter + 3 <= end);
            VALIDATE(iter[1] >= 0x80 && iter[1] <= 0xBF);
            VALIDATE(iter[2] >= 0x80 && iter[2] <= 0xBF);
            iter += 3;
        }
        else if (iter[0] == 0xED)
        {
            /*
             * U+D000..U+D7FF
             *
             * zzzzyyyy yyxxxxxx => 1110zzzz 10yyyyyy 10xxxxxx
             *                      ED       80....9F 80....BF
             */
            VALIDATE(iter + 3 <= end);
            VALIDATE(iter[1] >= 0x80 && iter[1] <= 0x9F);
            VALIDATE(iter[2] >= 0x80 && iter[2] <= 0xBF);
            iter += 3;
        }
        else if (iter[0] >= 0xEE && iter[0] <= 0xEF)
        {
            /*
             * U+E000..U+FFFF
             *
             * zzzzyyyy yyxxxxxx => 1110zzzz 10yyyyyy 10xxxxxx
             *                      EE....EF 80....BF 80....BF
             */
            VALIDATE(iter + 3 <= end);
            VALIDATE(iter[1] >= 0x80 && iter[1] <= 0xBF);
            VALIDATE(iter[2] >= 0x80 && iter[2] <= 0xBF);
            iter += 3;
        }
        else if (iter[0] == 0xF0)
        {
            /*
             * U+10000..U+3FFFF
             *
             * 000uuuuu zzzzyyyy yyxxxxxx ==> 11110uuu 10uuzzzz 10yyyyyy 10xxxxxx
             *                                F0       90....BF 80....BF 80....BF
             */
            VALIDATE(iter + 4 <= end);
            VALIDATE(iter[1] >= 0x90 && iter[1] <= 0xBF);
            VALIDATE(iter[2] >= 0x80 && iter[2] <= 0xBF);
            VALIDATE(iter[3] >= 0x80 && iter[3] <= 0xBF);
            iter += 4;
        }
        else if (iter[0] >= 0xF1 && iter[0] <= 0xF3)
        {
            /*
             * U+40000..U+FFFFF
             *
             * 000uuuuu zzzzyyyy yyxxxxxx ==> 11110uuu 10uuzzzz 10yyyyyy 10xxxxxx
             *                                F1....F3 80....BF 80....BF 80....BF
             */
            VALIDATE(iter + 4 <= end);
            VALIDATE(iter[1] >= 0x80 && iter[1] <= 0xBF);
            VALIDATE(iter[2] >= 0x80 && iter[2] <= 0xBF);
            VALIDATE(iter[3] >= 0x80 && iter[3] <= 0xBF);
            iter += 4;
        }
        else if (iter[0] == 0xF4)
        {
            /*
             * U+100000..U+10FFFF
             *
             * 000uuuuu zzzzyyyy yyxxxxxx ==> 11110uuu 10uuzzzz 10yyyyyy 10xxxxxx
             *                                F4       80....8F 80....BF 80....BF
             */
            VALIDATE(iter + 4 <= end);
            VALIDATE(iter[1] >= 0x80 && iter[1] <= 0x8F);
            VALIDATE(iter[2] >= 0x80 && iter[2] <= 0xBF);
            VALIDATE(iter[3] >= 0x80 && iter[3] <= 0xBF);
            iter += 4;
        }
        else
        {
            return FALSE;
        }
    }

    return TRUE;
}

SE_API sebool se_is_valid_utf16_str(const seunichar16* str, int len)
{
    #if SE_OPT_SURROGATE

        const seunichar16* iter;
        const seunichar16* end;

        if (!str)
            len = 0;

        if (len < 0)
            len = se_utf16_str_len(str);

        iter = str;
        end = iter + len;
        while (iter < end)
        {
            if (SE_IS_HI_SURROGATE(iter[0]))
            {
                VALIDATE(iter + 2 <= end);
                VALIDATE(SE_IS_LO_SURROGATE(iter[1]));
                iter += 2;
            }
            else if (SE_IS_LO_SURROGATE(iter[0]))
            {
                return FALSE;
            }
            else
            {
                iter++;
            }
        }

        return TRUE;

    #else

        return TRUE;

    #endif
}

SE_API sebool se_is_valid_utf32_str(const seunichar32* str, int len)
{
    int i;

    SE_DEBUG_ASSERT(str);

    if (len < 0)
        len = se_utf32_str_len(str);

    for (i = 0; i < len; i++)
    {
        VALIDATE(SE_IS_VALID_SCALAR_VALUE(str[i]));
    }

    return TRUE;
}

/***************************************************************************
 *                                                                         *
 * Copy Unicode string without validation.                                 *
 *                                                                         *
 * For both safe and un-safe strings.                                      *
 *                                                                         *
 ***************************************************************************/

SE_API seunichar8* se_utf8_str_copy(const seunichar8* str, int len)
{
    seunichar8* new_str;

    SE_DEBUG_ASSERT(str);
    SE_DEBUG_ASSERT(len >= 0);

    new_str = SE_MALLOC((len + 1) * sizeof(seunichar8));
    memcpy(new_str, str, len * sizeof(seunichar8));
    new_str[len] = 0;

    return new_str;
}

SE_API seunichar16* se_utf16_str_copy(const seunichar16* str, int len)
{
    seunichar16* new_str;

    SE_DEBUG_ASSERT(str);
    SE_DEBUG_ASSERT(len >= 0);

    new_str = SE_MALLOC((len + 1) * sizeof(seunichar16));
    memcpy(new_str, str, len * sizeof(seunichar16));
    new_str[len] = 0;

    return new_str;
}

SE_API seunichar32* se_utf32_str_copy(const seunichar32* str, int len)
{
    seunichar32* new_str;

    SE_DEBUG_ASSERT(str);
    SE_DEBUG_ASSERT(len >= 0);

    new_str = SE_MALLOC((len + 1) * sizeof(seunichar32));
    memcpy(new_str, str, len * sizeof(seunichar32));
    new_str[len] = 0;

    return new_str;
}

/***************************************************************************
 *                                                                         *
 * Validate and copy un-safe Unicode string to safe string.                *
 * Invalid codes are replaced.                                             *
 *                                                                         *
 * Accept un-safe strings as input,                                        *
 * output NULL terminated safe strings.                                    *
 *                                                                         *
 ***************************************************************************/

SE_API seunichar8* se_unsafe_utf8_str_safe_copy(const seunichar8* str, int len, int* out_len)
{
    int new_str_len;
    seunichar8* new_str;
    unsigned char* new_str_iter;
    const unsigned char* iter;
    const unsigned char* end;

    SE_DEBUG_ASSERT(str);

    if (len < 0)
        len = se_utf8_str_len(str);

    new_str_len = 0;

    iter = (const unsigned char*)str;
    end = iter + len;
    while (iter < end)
    {
        if (iter[0] <= 0x7F)
        {
            new_str_len++;
            iter++;
        }
        else if (iter[0] >= 0xC2 && iter[0] <= 0xDF)
        {
            VALIDATE1(iter + 2 <= end);
            VALIDATE1(iter[1] >= 0x80 && iter[1] <= 0xBF);
            new_str_len += 2;
            iter += 2;
        }
        else if (iter[0] == 0xE0)
        {
            VALIDATE1(iter + 3 <= end);
            VALIDATE1(iter[1] >= 0xA0 && iter[1] <= 0xBF);
            VALIDATE1(iter[2] >= 0x80 && iter[2] <= 0xBF);
            new_str_len += 3;
            iter += 3;
        }
        else if (iter[0] >= 0xE1 && iter[0] <= 0xEC)
        {
            VALIDATE1(iter + 3 <= end);
            VALIDATE1(iter[1] >= 0x80 && iter[1] <= 0xBF);
            VALIDATE1(iter[2] >= 0x80 && iter[2] <= 0xBF);
            new_str_len += 3;
            iter += 3;
        }
        else if (iter[0] == 0xED)
        {
            VALIDATE1(iter + 3 <= end);
            VALIDATE1(iter[1] >= 0x80 && iter[1] <= 0x9F);
            VALIDATE1(iter[2] >= 0x80 && iter[2] <= 0xBF);
            new_str_len += 3;
            iter += 3;
        }
        else if (iter[0] >= 0xEE && iter[0] <= 0xEF)
        {
            VALIDATE1(iter + 3 <= end);
            VALIDATE1(iter[1] >= 0x80 && iter[1] <= 0xBF);
            VALIDATE1(iter[2] >= 0x80 && iter[2] <= 0xBF);
            new_str_len += 3;
            iter += 3;
        }
        else if (iter[0] == 0xF0)
        {
            VALIDATE1(iter + 4 <= end);
            VALIDATE1(iter[1] >= 0x90 && iter[1] <= 0xBF);
            VALIDATE1(iter[2] >= 0x80 && iter[2] <= 0xBF);
            VALIDATE1(iter[3] >= 0x80 && iter[3] <= 0xBF);
            new_str_len += 4;
            iter += 4;
        }
        else if (iter[0] >= 0xF1 && iter[0] <= 0xF3)
        {
            VALIDATE1(iter + 4 <= end);
            VALIDATE1(iter[1] >= 0x80 && iter[1] <= 0xBF);
            VALIDATE1(iter[2] >= 0x80 && iter[2] <= 0xBF);
            VALIDATE1(iter[3] >= 0x80 && iter[3] <= 0xBF);
            new_str_len += 4;
            iter += 4;
        }
        else if (iter[0] == 0xF4)
        {
            VALIDATE1(iter + 4 <= end);
            VALIDATE1(iter[1] >= 0x80 && iter[1] <= 0x8F);
            VALIDATE1(iter[2] >= 0x80 && iter[2] <= 0xBF);
            VALIDATE1(iter[3] >= 0x80 && iter[3] <= 0xBF);
            new_str_len += 4;
            iter += 4;
        }
        else
        {
ill_formed_1:
            new_str_len += 3;
            iter++;
        }
    }

    new_str = SE_MALLOC((new_str_len + 1) * sizeof(seunichar8));
    new_str_iter = (unsigned char*)new_str;

    iter = (const unsigned char*)str;
    while (iter < end)
    {
        if (iter[0] <= 0x7F)
        {
            *new_str_iter++ = *iter++;
        }
        else if (iter[0] >= 0xC2 && iter[0] <= 0xDF)
        {
            VALIDATE2(iter + 2 <= end);
            VALIDATE2(iter[1] >= 0x80 && iter[1] <= 0xBF);
            *new_str_iter++ = *iter++;
            *new_str_iter++ = *iter++;
        }
        else if (iter[0] == 0xE0)
        {
            VALIDATE2(iter + 3 <= end);
            VALIDATE2(iter[1] >= 0xA0 && iter[1] <= 0xBF);
            VALIDATE2(iter[2] >= 0x80 && iter[2] <= 0xBF);
            *new_str_iter++ = *iter++;
            *new_str_iter++ = *iter++;
            *new_str_iter++ = *iter++;
        }
        else if (iter[0] >= 0xE1 && iter[0] <= 0xEC)
        {
            VALIDATE2(iter + 3 <= end);
            VALIDATE2(iter[1] >= 0x80 && iter[1] <= 0xBF);
            VALIDATE2(iter[2] >= 0x80 && iter[2] <= 0xBF);
            *new_str_iter++ = *iter++;
            *new_str_iter++ = *iter++;
            *new_str_iter++ = *iter++;
        }
        else if (iter[0] == 0xED)
        {
            VALIDATE2(iter + 3 <= end);
            VALIDATE2(iter[1] >= 0x80 && iter[1] <= 0x9F);
            VALIDATE2(iter[2] >= 0x80 && iter[2] <= 0xBF);
            *new_str_iter++ = *iter++;
            *new_str_iter++ = *iter++;
            *new_str_iter++ = *iter++;
        }
        else if (iter[0] >= 0xEE && iter[0] <= 0xEF)
        {
            VALIDATE2(iter + 3 <= end);
            VALIDATE2(iter[1] >= 0x80 && iter[1] <= 0xBF);
            VALIDATE2(iter[2] >= 0x80 && iter[2] <= 0xBF);
            *new_str_iter++ = *iter++;
            *new_str_iter++ = *iter++;
            *new_str_iter++ = *iter++;
        }
        else if (iter[0] == 0xF0)
        {
            VALIDATE2(iter + 4 <= end);
            VALIDATE2(iter[1] >= 0x90 && iter[1] <= 0xBF);
            VALIDATE2(iter[2] >= 0x80 && iter[2] <= 0xBF);
            VALIDATE2(iter[3] >= 0x80 && iter[3] <= 0xBF);
            *new_str_iter++ = *iter++;
            *new_str_iter++ = *iter++;
            *new_str_iter++ = *iter++;
            *new_str_iter++ = *iter++;
        }
        else if (iter[0] >= 0xF1 && iter[0] <= 0xF3)
        {
            VALIDATE2(iter + 4 <= end);
            VALIDATE2(iter[1] >= 0x80 && iter[1] <= 0xBF);
            VALIDATE2(iter[2] >= 0x80 && iter[2] <= 0xBF);
            VALIDATE2(iter[3] >= 0x80 && iter[3] <= 0xBF);
            *new_str_iter++ = *iter++;
            *new_str_iter++ = *iter++;
            *new_str_iter++ = *iter++;
            *new_str_iter++ = *iter++;
        }
        else if (iter[0] == 0xF4)
        {
            VALIDATE2(iter + 4 <= end);
            VALIDATE2(iter[1] >= 0x80 && iter[1] <= 0x8F);
            VALIDATE2(iter[2] >= 0x80 && iter[2] <= 0xBF);
            VALIDATE2(iter[3] >= 0x80 && iter[3] <= 0xBF);
            *new_str_iter++ = *iter++;
            *new_str_iter++ = *iter++;
            *new_str_iter++ = *iter++;
            *new_str_iter++ = *iter++;
        }
        else
        {
ill_formed_2:
            *new_str_iter++ = SE_REPLACEMENT_CHAR_UTF8_1_CODE;
            *new_str_iter++ = SE_REPLACEMENT_CHAR_UTF8_2_CODE;
            *new_str_iter++ = SE_REPLACEMENT_CHAR_UTF8_3_CODE;
            iter++;
        }
    }

    SE_DEBUG_ASSERT(new_str_iter - (unsigned char*)new_str == new_str_len);
    *new_str_iter = 0;

    SE_DEBUG_ASSERT(se_is_valid_utf8_str(new_str, new_str_len));

    if (out_len)
        *out_len = new_str_len;

    return new_str;
}

SE_API seunichar16* se_unsafe_utf16_str_safe_copy(const seunichar16* str, int len, int* out_len)
{
    int new_str_len;
    seunichar16* new_str;
    seunichar16* new_str_iter;
    const seunichar16* iter;
    const seunichar16* end;

    if (len < 0)
        len = se_utf16_str_len(str);

    new_str_len = 0;

    iter = str;
    end = iter + len;
    while (iter < end)
    {
        if (SE_IS_HI_SURROGATE(iter[0]))
        {
            VALIDATE1(iter + 2 <= end);
            VALIDATE1(SE_IS_LO_SURROGATE(iter[1]));
            new_str_len += 2;
            iter += 2;
        }
        else
        {
ill_formed_1:
            new_str_len++;
            iter++;
        }
    }

    new_str = SE_MALLOC((new_str_len + 1) * sizeof(seunichar16));
    new_str_iter = new_str;

    iter = str;
    while (iter < end)
    {
        if (SE_IS_HI_SURROGATE(iter[0]))
        {
            VALIDATE2(iter + 2 <= end);
            VALIDATE2(SE_IS_LO_SURROGATE(iter[1]));
            *new_str_iter++ = *iter++;
            *new_str_iter++ = *iter++;
        }
        else if (SE_IS_LO_SURROGATE(iter[0]))
        {
ill_formed_2:
            *new_str_iter++ = SE_REPLACEMENT_CHAR;
            iter++;
        }
        else
        {
            *new_str_iter++ = *iter++;
        }
    }

    SE_DEBUG_ASSERT(new_str_iter - new_str == new_str_len);
    *new_str_iter = 0;

    SE_DEBUG_ASSERT(se_is_valid_utf16_str(new_str, new_str_len));

    if (out_len)
        *out_len = new_str_len;

    return new_str;
}

SE_API seunichar32* se_unsafe_utf32_str_safe_copy(const seunichar32* str, int len, int* out_len)
{
    int i;
    seunichar32* new_str;

    SE_DEBUG_ASSERT(str);

    if (len < 0)
        len = se_utf32_str_len(str);

    new_str = SE_MALLOC((len + 1) * sizeof(seunichar32));
    for (i = 0; i < len; i++)
    {
        if (SE_IS_VALID_SCALAR_VALUE(str[i]))
            new_str[i] = str[i];
        else
            new_str[i] = SE_REPLACEMENT_CHAR;
    }
    new_str[len] = 0;

    SE_DEBUG_ASSERT(se_is_valid_utf32_str(new_str, len));

    if (out_len)
        *out_len = len;

    return new_str;
}

/***************************************************************************
 *                                                                         *
 * Unicode string length in characters.                                    *
 *                                                                         *
 * For safe strings only.                                                  *
 *                                                                         *
 ***************************************************************************/

SE_API int se_safe_utf8_str_char_count(const seunichar8* str, int len)
/*
 * str:
 *      Pointer to the start of a UTF-8 encoded string.
 *
 * len:
 *      The maximum string length to examine.
 *      If len is less than 0, then the string is assumed to be
 *      nul-terminated.
 *      If len is 0, str will not be examined and may be NULL.
 *
 * Returns:
 *      The length of the string in characters.
 */
{
    int char_count;
    const unsigned char* iter;
    const unsigned char* end;

    SE_DEBUG_ASSERT(se_is_valid_utf8_str(str, len));

    if (str && len < 0)
        len = se_utf8_str_len(str);

    char_count = 0;

    iter = (const unsigned char*)str;
    end = iter + len;
    while (iter < end)
    {
        if (*iter <= 0x7F)
            iter++;
        else if (*iter >= 0xC2 && *iter <= 0xDF)
            iter += 2;
        else if (*iter >= 0xE0 && *iter <= 0xEF)
            iter += 3;
        else /* *iter >= 0xF0 && *iter <= 0xF4 */
            iter += 4;
        char_count++;
    }

    return char_count;
}

SE_API int se_safe_utf16_str_char_count(const seunichar16* str, int len)
{
    #if SE_OPT_SURROGATE

        int char_count;
        const seunichar16* iter;
        const seunichar16* end;

        SE_DEBUG_ASSERT(se_is_valid_utf16_str(str, len));

        if (str && len < 0)
            len = se_utf16_str_len(str);

        char_count = 0;

        iter = str;
        end = iter + len;
        while (iter < end)
        {
            if (SE_IS_HI_SURROGATE(*iter))
                iter += 2;
            else
                iter++;
            char_count++;
        }

        return char_count;

    #else

        SE_DEBUG_ASSERT(se_is_valid_utf16_str(str, len));

        if (str && len < 0)
            return se_utf16_str_len(str);
        else
            return len;

    #endif
}

SE_API int se_safe_utf32_str_char_count(const seunichar32* str, int len)
{
    SE_DEBUG_ASSERT(se_is_valid_utf32_str(str, len));

    if (len < 0)
        return se_utf32_str_len(str);
    else
        return len;
}

/***************************************************************************
 *                                                                         *
 * Validate and convert un-safe Unicode string to safe string.             *
 * Invalid codes are replaced.                                             *
 *                                                                         *
 * Accept un-safe strings as input,                                        *
 * output NULL terminated safe strings.                                    *
 *                                                                         *
 ***************************************************************************/

SE_API seunichar16* se_unsafe_utf8_to_safe_utf16(const seunichar8* str, int len, int* out_len)
{
    int new_str_len;
    seunichar16* new_str;
    seunichar16* new_str_iter;
    const unsigned char* iter;
    const unsigned char* end;
    seunichar32 c;

    SE_DEBUG_ASSERT(str);

    if (len < 0)
        len = se_utf8_str_len(str);

    new_str_len = 0;

    iter = (const unsigned char*)str;
    end = iter + len;
    while (iter < end)
    {
        if (iter[0] <= 0x7F)
        {
            new_str_len++;
            iter++;
        }
        else if (iter[0] >= 0xC2 && iter[0] <= 0xDF)
        {
            VALIDATE1(iter + 2 <= end);
            VALIDATE1(iter[1] >= 0x80 && iter[1] <= 0xBF);
            new_str_len++;
            iter += 2;
        }
        else if (iter[0] == 0xE0)
        {
            VALIDATE1(iter + 3 <= end);
            VALIDATE1(iter[1] >= 0xA0 && iter[1] <= 0xBF);
            VALIDATE1(iter[2] >= 0x80 && iter[2] <= 0xBF);
            new_str_len++;
            iter += 3;
        }
        else if (iter[0] >= 0xE1 && iter[0] <= 0xEC)
        {
            VALIDATE1(iter + 3 <= end);
            VALIDATE1(iter[1] >= 0x80 && iter[1] <= 0xBF);
            VALIDATE1(iter[2] >= 0x80 && iter[2] <= 0xBF);
            new_str_len++;
            iter += 3;
        }
        else if (iter[0] == 0xED)
        {
            VALIDATE1(iter + 3 <= end);
            VALIDATE1(iter[1] >= 0x80 && iter[1] <= 0x9F);
            VALIDATE1(iter[2] >= 0x80 && iter[2] <= 0xBF);
            new_str_len++;
            iter += 3;
        }
        else if (iter[0] >= 0xEE && iter[0] <= 0xEF)
        {
            VALIDATE1(iter + 3 <= end);
            VALIDATE1(iter[1] >= 0x80 && iter[1] <= 0xBF);
            VALIDATE1(iter[2] >= 0x80 && iter[2] <= 0xBF);
            new_str_len++;
            iter += 3;
        }
        else if (iter[0] == 0xF0)
        {
            VALIDATE1(iter + 4 <= end);
            VALIDATE1(iter[1] >= 0x90 && iter[1] <= 0xBF);
            VALIDATE1(iter[2] >= 0x80 && iter[2] <= 0xBF);
            VALIDATE1(iter[3] >= 0x80 && iter[3] <= 0xBF);
            new_str_len += 2;
            iter += 4;
        }
        else if (iter[0] >= 0xF1 && iter[0] <= 0xF3)
        {
            VALIDATE1(iter + 4 <= end);
            VALIDATE1(iter[1] >= 0x80 && iter[1] <= 0xBF);
            VALIDATE1(iter[2] >= 0x80 && iter[2] <= 0xBF);
            VALIDATE1(iter[3] >= 0x80 && iter[3] <= 0xBF);
            new_str_len += 2;
            iter += 4;
        }
        else if (iter[0] == 0xF4)
        {
            VALIDATE1(iter + 4 <= end);
            VALIDATE1(iter[1] >= 0x80 && iter[1] <= 0x8F);
            VALIDATE1(iter[2] >= 0x80 && iter[2] <= 0xBF);
            VALIDATE1(iter[3] >= 0x80 && iter[3] <= 0xBF);
            new_str_len += 2;
            iter += 4;
        }
        else
        {
ill_formed_1:
            new_str_len++;
            iter++;
        }
    }

    new_str = SE_MALLOC((new_str_len + 1) * sizeof(seunichar16));
    new_str_iter = new_str;

    iter = (const unsigned char*)str;
    while (iter < end)
    {
        if (iter[0] <= 0x7F)
        {
            *new_str_iter++ = *iter++;
        }
        else if (iter[0] >= 0xC2 && iter[0] <= 0xDF)
        {
            VALIDATE2(iter + 2 <= end);
            VALIDATE2(iter[1] >= 0x80 && iter[1] <= 0xBF);
            *new_str_iter++ = (seunichar16) ( ((iter[0] & 0x1F) << 6) | (iter[1] & 0x3F) );
            iter += 2;
        }
        else if (iter[0] == 0xE0)
        {
            VALIDATE2(iter + 3 <= end);
            VALIDATE2(iter[1] >= 0xA0 && iter[1] <= 0xBF);
            VALIDATE2(iter[2] >= 0x80 && iter[2] <= 0xBF);
            *new_str_iter++ = (seunichar16) ( ((iter[1] & 0x3F) << 6) | (iter[2] & 0x3F) );
            iter += 3;
        }
        else if (iter[0] >= 0xE1 && iter[0] <= 0xEC)
        {
            VALIDATE2(iter + 3 <= end);
            VALIDATE2(iter[1] >= 0x80 && iter[1] <= 0xBF);
            VALIDATE2(iter[2] >= 0x80 && iter[2] <= 0xBF);
            *new_str_iter++ = (seunichar16) ( ((iter[0] & 0x0F) << 12) | ((iter[1] & 0x3F) << 6) | (iter[2] & 0x3F) );
            iter += 3;
        }
        else if (iter[0] == 0xED)
        {
            VALIDATE2(iter + 3 <= end);
            VALIDATE2(iter[1] >= 0x80 && iter[1] <= 0x9F);
            VALIDATE2(iter[2] >= 0x80 && iter[2] <= 0xBF);
            *new_str_iter++ = (seunichar16) ( 0xD000 | ((iter[1] & 0x3F) << 6) | (iter[2] & 0x3F) );
            iter += 3;
        }
        else if (iter[0] >= 0xEE && iter[0] <= 0xEF)
        {
            VALIDATE2(iter + 3 <= end);
            VALIDATE2(iter[1] >= 0x80 && iter[1] <= 0xBF);
            VALIDATE2(iter[2] >= 0x80 && iter[2] <= 0xBF);
            *new_str_iter++ = (seunichar16) ( ((iter[0] & 0x0F) << 12) | ((iter[1] & 0x3F) << 6) | (iter[2] & 0x3F) );
            iter += 3;
        }
        else if (iter[0] == 0xF0)
        {
            VALIDATE2(iter + 4 <= end);
            VALIDATE2(iter[1] >= 0x90 && iter[1] <= 0xBF);
            VALIDATE2(iter[2] >= 0x80 && iter[2] <= 0xBF);
            VALIDATE2(iter[3] >= 0x80 && iter[3] <= 0xBF);
            c = (seunichar32) ( ((iter[1] & 0x3F) << 12) | ((iter[2] & 0x3F) << 6) | (iter[3] & 0x3F) );
            new_str_iter += se_safe_unichar_to_utf16(c, new_str_iter);
            iter += 4;
        }
        else if (iter[0] >= 0xF1 && iter[0] <= 0xF3)
        {
            VALIDATE2(iter + 4 <= end);
            VALIDATE2(iter[1] >= 0x80 && iter[1] <= 0xBF);
            VALIDATE2(iter[2] >= 0x80 && iter[2] <= 0xBF);
            VALIDATE2(iter[3] >= 0x80 && iter[3] <= 0xBF);
            c = (seunichar32) ( ((iter[0] & 7) << 18) | ((iter[1] & 0x3F) << 12) | ((iter[2] & 0x3F) << 6) | (iter[3] & 0x3F) );
            new_str_iter += se_safe_unichar_to_utf16(c, new_str_iter);
            iter += 4;
        }
        else if (iter[0] == 0xF4)
        {
            VALIDATE2(iter + 4 <= end);
            VALIDATE2(iter[1] >= 0x80 && iter[1] <= 0x8F);
            VALIDATE2(iter[2] >= 0x80 && iter[2] <= 0xBF);
            VALIDATE2(iter[3] >= 0x80 && iter[3] <= 0xBF);
            c = (seunichar32) ( 0x100000 | ((iter[1] & 0x3F) << 12) | ((iter[2] & 0x3F) << 6) | (iter[3] & 0x3F) );
            new_str_iter += se_safe_unichar_to_utf16(c, new_str_iter);
            iter += 4;
        }
        else
        {
ill_formed_2:
            *new_str_iter++ = SE_REPLACEMENT_CHAR;
            iter++;
        }
    }

    SE_DEBUG_ASSERT(new_str_iter - new_str == new_str_len);
    *new_str_iter = 0;

    SE_DEBUG_ASSERT(se_is_valid_utf16_str(new_str, new_str_len));

    if (out_len)
        *out_len = new_str_len;

    return new_str;
}

SE_API seunichar8* se_unsafe_utf16_to_safe_utf8(const seunichar16* str, int len, int* out_len)
{
    int new_str_len;
    seunichar8* new_str;
    seunichar8* new_str_iter;
    const seunichar16* iter;
    const seunichar16* end;
    seunichar c;

    SE_DEBUG_ASSERT(str);

    if (len < 0)
        len = se_utf16_str_len(str);

    new_str_len = 0;

    iter = str;
    end = iter + len;
    while (iter < end)
    {
        if (SE_IS_HI_SURROGATE(iter[0]))
        {
            VALIDATE1(iter + 2 <= end);
            VALIDATE1(SE_IS_LO_SURROGATE(iter[1]));
            new_str_len += 4;
            iter += 2;
        }
        else if (SE_IS_LO_SURROGATE(iter[0]))
        {
ill_formed_1:
            new_str_len += 3;
            iter++;
        }
        else
        {
            new_str_len += se_safe_unichar_to_utf8(iter[0], 0);
            iter++;
        }
    }

    new_str = SE_MALLOC((new_str_len + 1) * sizeof(seunichar8));
    new_str_iter = new_str;

    iter = str;
    while (iter < end)
    {
        if (SE_IS_HI_SURROGATE(iter[0]))
        {
            VALIDATE2(iter + 2 <= end);
            VALIDATE2(SE_IS_LO_SURROGATE(iter[1]));
            c = SE_SURROGATE_VALUE(iter[0], iter[1]);
            new_str_iter += se_safe_unichar_to_utf8(c, new_str_iter);
            iter += 2;
        }
        else if (SE_IS_LO_SURROGATE(iter[0]))
        {
ill_formed_2:
            *new_str_iter++ = SE_REPLACEMENT_CHAR_UTF8_1;
            *new_str_iter++ = SE_REPLACEMENT_CHAR_UTF8_2;
            *new_str_iter++ = SE_REPLACEMENT_CHAR_UTF8_3;
            iter++;
        }
        else
        {
            new_str_iter += se_safe_unichar_to_utf8(iter[0], new_str_iter);
            iter++;
        }
    }

    SE_DEBUG_ASSERT(new_str_iter - new_str == new_str_len);
    *new_str_iter = 0;

    SE_DEBUG_ASSERT(se_is_valid_utf8_str(new_str, new_str_len));

    if (out_len)
        *out_len = new_str_len;

    return new_str;
}

SE_API seunichar32* se_unsafe_utf8_to_safe_utf32(const seunichar8* str, int len, int* out_len)
{
    int new_str_len;
    seunichar32* new_str;
    seunichar32* new_str_iter;
    const unsigned char* iter;
    const unsigned char* end;

    SE_DEBUG_ASSERT(str);

    if (len < 0)
        len = se_utf8_str_len(str);

    new_str_len = 0;

    iter = (const unsigned char*)str;
    end = iter + len;
    while (iter < end)
    {
        if (iter[0] <= 0x7F)
        {
            iter++;
        }
        else if (iter[0] >= 0xC2 && iter[0] <= 0xDF)
        {
            VALIDATE1(iter + 2 <= end);
            VALIDATE1(iter[1] >= 0x80 && iter[1] <= 0xBF);
            iter += 2;
        }
        else if (iter[0] == 0xE0)
        {
            VALIDATE1(iter + 3 <= end);
            VALIDATE1(iter[1] >= 0xA0 && iter[1] <= 0xBF);
            VALIDATE1(iter[2] >= 0x80 && iter[2] <= 0xBF);
            iter += 3;
        }
        else if (iter[0] >= 0xE1 && iter[0] <= 0xEC)
        {
            VALIDATE1(iter + 3 <= end);
            VALIDATE1(iter[1] >= 0x80 && iter[1] <= 0xBF);
            VALIDATE1(iter[2] >= 0x80 && iter[2] <= 0xBF);
            iter += 3;
        }
        else if (iter[0] == 0xED)
        {
            VALIDATE1(iter + 3 <= end);
            VALIDATE1(iter[1] >= 0x80 && iter[1] <= 0x9F);
            VALIDATE1(iter[2] >= 0x80 && iter[2] <= 0xBF);
            iter += 3;
        }
        else if (iter[0] >= 0xEE && iter[0] <= 0xEF)
        {
            VALIDATE1(iter + 3 <= end);
            VALIDATE1(iter[1] >= 0x80 && iter[1] <= 0xBF);
            VALIDATE1(iter[2] >= 0x80 && iter[2] <= 0xBF);
            iter += 3;
        }
        else if (iter[0] == 0xF0)
        {
            VALIDATE1(iter + 4 <= end);
            VALIDATE1(iter[1] >= 0x90 && iter[1] <= 0xBF);
            VALIDATE1(iter[2] >= 0x80 && iter[2] <= 0xBF);
            VALIDATE1(iter[3] >= 0x80 && iter[3] <= 0xBF);
            iter += 4;
        }
        else if (iter[0] >= 0xF1 && iter[0] <= 0xF3)
        {
            VALIDATE1(iter + 4 <= end);
            VALIDATE1(iter[1] >= 0x80 && iter[1] <= 0xBF);
            VALIDATE1(iter[2] >= 0x80 && iter[2] <= 0xBF);
            VALIDATE1(iter[3] >= 0x80 && iter[3] <= 0xBF);
            iter += 4;
        }
        else if (iter[0] == 0xF4)
        {
            VALIDATE1(iter + 4 <= end);
            VALIDATE1(iter[1] >= 0x80 && iter[1] <= 0x8F);
            VALIDATE1(iter[2] >= 0x80 && iter[2] <= 0xBF);
            VALIDATE1(iter[3] >= 0x80 && iter[3] <= 0xBF);
            iter += 4;
        }
        else
        {
ill_formed_1:
            iter++;
        }
        new_str_len++;
    }

    new_str = SE_MALLOC((new_str_len + 1) * sizeof(seunichar32));
    new_str_iter = new_str;

    iter = (const unsigned char*)str;
    while (iter < end)
    {
        if (iter[0] <= 0x7F)
        {
            *new_str_iter++ = *iter++;
        }
        else if (iter[0] >= 0xC2 && iter[0] <= 0xDF)
        {
            VALIDATE2(iter + 2 <= end);
            VALIDATE2(iter[1] >= 0x80 && iter[1] <= 0xBF);
            *new_str_iter++ = (seunichar32) ( ((iter[0] & 0x1F) << 6) | (iter[1] & 0x3F) );
            iter += 2;
        }
        else if (iter[0] == 0xE0)
        {
            VALIDATE2(iter + 3 <= end);
            VALIDATE2(iter[1] >= 0xA0 && iter[1] <= 0xBF);
            VALIDATE2(iter[2] >= 0x80 && iter[2] <= 0xBF);
            *new_str_iter++ = (seunichar32) ( ((iter[1] & 0x3F) << 6) | (iter[2] & 0x3F) );
            iter += 3;
        }
        else if (iter[0] >= 0xE1 && iter[0] <= 0xEC)
        {
            VALIDATE2(iter + 3 <= end);
            VALIDATE2(iter[1] >= 0x80 && iter[1] <= 0xBF);
            VALIDATE2(iter[2] >= 0x80 && iter[2] <= 0xBF);
            *new_str_iter++ = (seunichar32) ( ((iter[0] & 0x0F) << 12) | ((iter[1] & 0x3F) << 6) | (iter[2] & 0x3F) );
            iter += 3;
        }
        else if (iter[0] == 0xED)
        {
            VALIDATE2(iter + 3 <= end);
            VALIDATE2(iter[1] >= 0x80 && iter[1] <= 0x9F);
            VALIDATE2(iter[2] >= 0x80 && iter[2] <= 0xBF);
            *new_str_iter++ = (seunichar32) ( 0xD000 | ((iter[1] & 0x3F) << 6) | (iter[2] & 0x3F) );
            iter += 3;
        }
        else if (iter[0] >= 0xEE && iter[0] <= 0xEF)
        {
            VALIDATE2(iter + 3 <= end);
            VALIDATE2(iter[1] >= 0x80 && iter[1] <= 0xBF);
            VALIDATE2(iter[2] >= 0x80 && iter[2] <= 0xBF);
            *new_str_iter++ = (seunichar32) ( ((iter[0] & 0x0F) << 12) | ((iter[1] & 0x3F) << 6) | (iter[2] & 0x3F) );
            iter += 3;
        }
        else if (iter[0] == 0xF0)
        {
            VALIDATE2(iter + 4 <= end);
            VALIDATE2(iter[1] >= 0x90 && iter[1] <= 0xBF);
            VALIDATE2(iter[2] >= 0x80 && iter[2] <= 0xBF);
            VALIDATE2(iter[3] >= 0x80 && iter[3] <= 0xBF);
            *new_str_iter++ = (seunichar32) ( ((iter[1] & 0x3F) << 12) | ((iter[2] & 0x3F) << 6) | (iter[3] & 0x3F) );
            iter += 4;
        }
        else if (iter[0] >= 0xF1 && iter[0] <= 0xF3)
        {
            VALIDATE2(iter + 4 <= end);
            VALIDATE2(iter[1] >= 0x80 && iter[1] <= 0xBF);
            VALIDATE2(iter[2] >= 0x80 && iter[2] <= 0xBF);
            VALIDATE2(iter[3] >= 0x80 && iter[3] <= 0xBF);
            *new_str_iter++ = (seunichar32) ( ((iter[0] & 7) << 18) | ((iter[1] & 0x3F) << 12) | ((iter[2] & 0x3F) << 6) | (iter[3] & 0x3F) );
            iter += 4;
        }
        else if (iter[0] == 0xF4)
        {
            VALIDATE2(iter + 4 <= end);
            VALIDATE2(iter[1] >= 0x80 && iter[1] <= 0x8F);
            VALIDATE2(iter[2] >= 0x80 && iter[2] <= 0xBF);
            VALIDATE2(iter[3] >= 0x80 && iter[3] <= 0xBF);
            *new_str_iter++ = (seunichar32) ( 0x100000 | ((iter[1] & 0x3F) << 12) | ((iter[2] & 0x3F) << 6) | (iter[3] & 0x3F) );
            iter += 4;
        }
        else
        {
ill_formed_2:
            *new_str_iter++ = SE_REPLACEMENT_CHAR;
            iter++;
        }
    }

    SE_DEBUG_ASSERT(new_str_iter - new_str == new_str_len);
    *new_str_iter = 0;

    SE_DEBUG_ASSERT(se_is_valid_utf32_str(new_str, new_str_len));

    if (out_len)
        *out_len = new_str_len;

    return new_str;
}

SE_API seunichar8* se_unsafe_utf32_to_safe_utf8(const seunichar32* str, int len, int* out_len)
{
    int i;
    int new_str_len;
    seunichar8* new_str;
    seunichar8* new_str_iter;

    SE_DEBUG_ASSERT(str);

    if (len < 0)
        len = se_utf32_str_len(str);

    new_str_len = 0;

    for (i = 0; i < len; i++)
    {
        if (SE_IS_VALID_SCALAR_VALUE(str[i]))
            new_str_len += se_safe_unichar_to_utf8(str[i], 0);
        else
            new_str_len += 3;
    }

    new_str = SE_MALLOC((new_str_len + 1) * sizeof(seunichar8));
    new_str_iter = new_str;

    for (i = 0; i < len; i++)
    {
        if (SE_IS_VALID_SCALAR_VALUE(str[i]))
        {
            new_str_iter += se_safe_unichar_to_utf8(str[i], new_str_iter);
        }
        else
        {
            *new_str_iter++ = SE_REPLACEMENT_CHAR_UTF8_1;
            *new_str_iter++ = SE_REPLACEMENT_CHAR_UTF8_2;
            *new_str_iter++ = SE_REPLACEMENT_CHAR_UTF8_3;
        }
    }

    SE_DEBUG_ASSERT(new_str_iter - new_str == new_str_len);
    *new_str_iter = 0;

    SE_DEBUG_ASSERT(se_is_valid_utf8_str(new_str, new_str_len));

    if (out_len)
        *out_len = new_str_len;

    return new_str;
}

SE_API seunichar32* se_unsafe_utf16_to_safe_utf32(const seunichar16* str, int len, int* out_len)
{
    int new_str_len;
    seunichar32* new_str;
    seunichar32* new_str_iter;
    const seunichar16* iter;
    const seunichar16* end;

    SE_DEBUG_ASSERT(str);

    if (len < 0)
        len = se_utf16_str_len(str);

    new_str_len = 0;

    iter = str;
    end = iter + len;
    while (iter < end)
    {
        if (SE_IS_HI_SURROGATE(iter[0]))
        {
            VALIDATE1(iter + 2 <= end);
            VALIDATE1(SE_IS_LO_SURROGATE(iter[1]));
            iter += 2;
        }
        else
        {
ill_formed_1:
            iter++;
        }
        new_str_len++;
    }

    new_str = SE_MALLOC((new_str_len + 1) * sizeof(seunichar32));
    new_str_iter = new_str;

    iter = str;
    while (iter < end)
    {
        if (SE_IS_HI_SURROGATE(iter[0]))
        {
            VALIDATE2(iter + 2 <= end);
            VALIDATE2(SE_IS_LO_SURROGATE(iter[1]));
            *new_str_iter++ = SE_SURROGATE_VALUE(iter[0], iter[1]);
            iter += 2;
        }
        else if (SE_IS_LO_SURROGATE(iter[0]))
        {
ill_formed_2:
            *new_str_iter++ = SE_REPLACEMENT_CHAR;
            iter++;
        }
        else
        {
            *new_str_iter++ = *iter++;
        }
    }

    SE_DEBUG_ASSERT(new_str_iter - new_str == new_str_len);
    *new_str_iter = 0;

    SE_DEBUG_ASSERT(se_is_valid_utf32_str(new_str, new_str_len));

    if (out_len)
        *out_len = new_str_len;

    return new_str;
}

SE_API seunichar16* se_unsafe_utf32_to_safe_utf16(const seunichar32* str, int len, int* out_len)
{
    int i;
    int new_str_len;
    seunichar16* new_str;
    seunichar16* new_str_iter;

    SE_DEBUG_ASSERT(str);

    if (len < 0)
        len = se_utf32_str_len(str);

    new_str_len = 0;

    for (i = 0; i < len; i++)
    {
        if (SE_IS_VALID_SCALAR_VALUE(str[i]))
            new_str_len += se_safe_unichar_to_utf16(str[i], 0);
        else
            new_str_len++;
    }

    new_str = SE_MALLOC((new_str_len + 1) * sizeof(seunichar16));
    new_str_iter = new_str;

    for (i = 0; i < len; i++)
    {
        if (SE_IS_VALID_SCALAR_VALUE(str[i]))
            new_str_iter += se_safe_unichar_to_utf16(str[i], new_str_iter);
        else
            *new_str_iter++ = SE_REPLACEMENT_CHAR;
    }

    SE_DEBUG_ASSERT(new_str_iter - new_str == new_str_len);
    *new_str_iter = 0;

    SE_DEBUG_ASSERT(se_is_valid_utf16_str(new_str, new_str_len));

    if (out_len)
        *out_len = new_str_len;

    return new_str;
}

/***************************************************************************
 *                                                                         *
 * Safe Unicode string converting.                                         *
 *                                                                         *
 * Accept safe strings as input,                                           *
 * output NULL terminated safe strings.                                    *
 *                                                                         *
 ***************************************************************************/

SE_API seunichar16* se_safe_utf8_to_utf16(const seunichar8* str, int len, int* out_len)
{
    int new_str_len;
    seunichar16* new_str;
    seunichar16* new_str_iter;
    const unsigned char* iter;
    const unsigned char* end;
    seunichar32 c;

    SE_DEBUG_ASSERT(se_is_valid_utf8_str(str, len));

    if (len < 0)
        len = se_utf8_str_len(str);

    new_str_len = 0;

    iter = (const unsigned char*)str;
    end = iter + len;
    while (iter < end)
    {
        if (iter[0] <= 0x7F)
        {
            new_str_len++;
            iter++;
        }
        else if (iter[0] >= 0xC2 && iter[0] <= 0xDF)
        {
            new_str_len++;
            iter += 2;
        }
        else if (iter[0] >= 0xE0 && iter[0] <= 0xEF)
        {
            new_str_len++;
            iter += 3;
        }
        else
        {
            SE_DEBUG_ASSERT(iter[0] >= 0xF0 && iter[0] <= 0xF4);
            new_str_len += 2;
            iter += 4;
        }
    }

    new_str = SE_MALLOC((new_str_len + 1) * sizeof(seunichar16));
    new_str_iter = new_str;

    iter = (const unsigned char*)str;
    while (iter < end)
    {
        if (iter[0] <= 0x7F)
        {
            *new_str_iter++ = *iter++;
        }
        else if (iter[0] >= 0xC2 && iter[0] <= 0xDF)
        {
            *new_str_iter++ = (seunichar16) ( ((iter[0] & 0x1F) << 6) | (iter[1] & 0x3F) );
            iter += 2;
        }
        else if (iter[0] >= 0xE0 && iter[0] <= 0xEF)
        {
            *new_str_iter++ = (seunichar16) ( ((iter[0] & 0x0F) << 12) | ((iter[1] & 0x3F) << 6) | (iter[2] & 0x3F) );
            iter += 3;
        }
        else
        {
            SE_DEBUG_ASSERT(iter[0] >= 0xF0 && iter[0] <= 0xF4);
            c = (seunichar32) ( ((iter[0] & 7) << 18) | ((iter[1] & 0x3F) << 12) | ((iter[2] & 0x3F) << 6) | (iter[3] & 0x3F) );
            new_str_iter += se_safe_unichar_to_utf16(c, new_str_iter);
            iter += 4;
        }
    }

    SE_DEBUG_ASSERT(new_str_iter - new_str == new_str_len);
    *new_str_iter = 0;

    SE_DEBUG_ASSERT(se_is_valid_utf16_str(new_str, new_str_len));

    if (out_len)
        *out_len = new_str_len;

    return new_str;
}

SE_API seunichar8* se_safe_utf16_to_utf8(const seunichar16* str, int len, int* out_len)
{
    int new_str_len;
    seunichar8* new_str;
    seunichar8* new_str_iter;
    const seunichar16* iter;
    const seunichar16* end;
    seunichar c;

    SE_DEBUG_ASSERT(se_is_valid_utf16_str(str, len));

    if (len < 0)
        len = se_utf16_str_len(str);

    new_str_len = 0;

    iter = str;
    end = iter + len;
    while (iter < end)
    {
        if (SE_IS_HI_SURROGATE(iter[0]))
        {
            c = SE_SURROGATE_VALUE(iter[0], iter[1]);
            new_str_len += se_safe_unichar_to_utf8(c, 0);
            iter += 2;
        }
        else
        {
            new_str_len += se_safe_unichar_to_utf8(iter[0], 0);
            iter++;
        }
    }

    new_str = SE_MALLOC((new_str_len + 1) * sizeof(seunichar8));
    new_str_iter = new_str;

    iter = str;
    while (iter < end)
    {
        if (SE_IS_HI_SURROGATE(iter[0]))
        {
            c = SE_SURROGATE_VALUE(iter[0], iter[1]);
            new_str_iter += se_safe_unichar_to_utf8(c, new_str_iter);
            iter += 2;
        }
        else
        {
            new_str_iter += se_safe_unichar_to_utf8(iter[0], new_str_iter);
            iter++;
        }
    }

    SE_DEBUG_ASSERT(new_str_iter - new_str == new_str_len);
    *new_str_iter = 0;

    SE_DEBUG_ASSERT(se_is_valid_utf8_str(new_str, new_str_len));

    if (out_len)
        *out_len = new_str_len;

    return new_str;
}

SE_API seunichar32* se_safe_utf8_to_utf32(const seunichar8* str, int len, int* out_len)
/*
 * str:
 *      Input UTF-8 encoded string.
 *
 * len:
 *      The byte length of input string.
 *      If len < 0, then the string is NUL terminated.
 *
 * out_len:
 *      Location to return the length of the output string.
 *      (Can be NULL to indicate that the result is not needed.)
 *
 * Convert a string from UTF-8 to UTF-32.
 * Assuming the input string is well-formed.
 * 
 * Return:
 *      A pointer to a newly allocated UTF-32 string.
 *      This string must be freed by caller.
 */
{
    int new_str_len;
    seunichar32* new_str;
    seunichar32* new_str_iter;
    const unsigned char* iter;
    const unsigned char* end;

    SE_DEBUG_ASSERT(se_is_valid_utf8_str(str, len));

    if (len < 0)
        len = se_utf8_str_len(str);

    new_str_len = 0;

    iter = (const unsigned char*)str;
    end = iter + len;
    while (iter < end)
    {
        if (iter[0] <= 0x7F)
            iter++;
        else if (iter[0] >= 0xC2 && iter[0] <= 0xDF)
            iter += 2;
        else if (iter[0] >= 0xE0 && iter[0] <= 0xEF)
            iter += 3;
        else /* iter[0] >= 0xF0 && iter[0] <= 0xF4 */
            iter += 4;
        new_str_len++;
    }

    new_str = SE_MALLOC((new_str_len + 1) * sizeof(seunichar32));
    new_str_iter = new_str;

    iter = (const unsigned char*)str;
    while (iter < end)
    {
        if (iter[0] <= 0x7F)
        {
            *new_str_iter++ = *iter++;
        }
        else if (iter[0] >= 0xC2 && iter[0] <= 0xDF)
        {
            *new_str_iter++ = (seunichar32) ( ((iter[0] & 0x1F) << 6) | (iter[1] & 0x3F) );
            iter += 2;
        }
        else if (iter[0] >= 0xE0 && iter[0] <= 0xEF)
        {
            *new_str_iter++ = (seunichar32) ( ((iter[0] & 0x0F) << 12) | ((iter[1] & 0x3F) << 6) | (iter[2] & 0x3F) );
            iter += 3;
        }
        else /* iter[0] >= 0xF0 && iter[0] <= 0xF4 */
        {
            *new_str_iter++ = (seunichar32) ( ((iter[0] & 7) << 18) | ((iter[1] & 0x3F) << 12) | ((iter[2] & 0x3F) << 6) | (iter[3] & 0x3F) );
            iter += 4;
        }
    }

    SE_DEBUG_ASSERT(new_str_iter - new_str == new_str_len);
    *new_str_iter = 0;

    SE_DEBUG_ASSERT(se_is_valid_utf32_str(new_str, new_str_len));

    if (out_len)
        *out_len = new_str_len;

    return new_str;
}

SE_API seunichar8* se_safe_utf32_to_utf8(const seunichar32* str, int len, int* out_len)
{
    int i;
    int new_str_len;
    seunichar8* new_str;
    seunichar8* new_str_iter;

    SE_DEBUG_ASSERT(se_is_valid_utf32_str(str, len));

    if (len < 0)
        len = se_utf32_str_len(str);

    new_str_len = 0;
    for (i = 0; i < len; i++)
        new_str_len += se_safe_unichar_to_utf8(str[i], 0);

    new_str = SE_MALLOC((new_str_len + 1) * sizeof(seunichar8));

    new_str_iter = new_str;
    for (i = 0; i < len; i++)
        new_str_iter += se_safe_unichar_to_utf8(str[i], new_str_iter);

    SE_DEBUG_ASSERT(new_str_iter - new_str == new_str_len);
    *new_str_iter = 0;

    SE_DEBUG_ASSERT(se_is_valid_utf8_str(new_str, new_str_len));

    if (out_len)
        *out_len = new_str_len;

    return new_str;
}

SE_API seunichar32* se_safe_utf16_to_utf32(const seunichar16* str, int len, int* out_len)
{
    int new_str_len;
    seunichar32* new_str;
    seunichar32* new_str_iter;
    const seunichar16* iter;
    const seunichar16* end;

    SE_DEBUG_ASSERT(se_is_valid_utf16_str(str, len));

    if (len < 0)
        len = se_utf16_str_len(str);

    new_str_len = 0;

    iter = str;
    end = iter + len;
    while (iter < end)
    {
        if (SE_IS_HI_SURROGATE(iter[0]))
            iter += 2;
        else
            iter++;
        new_str_len++;
    }

    new_str = SE_MALLOC((new_str_len + 1) * sizeof(seunichar32));
    new_str_iter = new_str;

    iter = str;
    while (iter < end)
    {
        if (SE_IS_HI_SURROGATE(iter[0]))
        {
            *new_str_iter++ = SE_SURROGATE_VALUE(iter[0], iter[1]);
            iter += 2;
        }
        else
        {
            *new_str_iter++ = *iter++;
        }
    }

    SE_DEBUG_ASSERT(new_str_iter - new_str == new_str_len);
    *new_str_iter = 0;

    SE_DEBUG_ASSERT(se_is_valid_utf32_str(new_str, new_str_len));

    if (out_len)
        *out_len = new_str_len;

    return new_str;
}

SE_API seunichar16* se_safe_utf32_to_utf16(const seunichar32* str, int len, int* out_len)
{
    int i;
    int new_str_len;
    seunichar16* new_str;
    seunichar16* new_str_iter;

    SE_DEBUG_ASSERT(se_is_valid_utf32_str(str, len));

    if (len < 0)
        len = se_utf32_str_len(str);

    new_str_len = 0;
    for (i = 0; i < len; i++)
        new_str_len += se_safe_unichar_to_utf16(str[i], 0);

    new_str = SE_MALLOC((new_str_len + 1) * sizeof(seunichar16));

    new_str_iter = new_str;
    for (i = 0; i < len; i++)
        new_str_iter += se_safe_unichar_to_utf16(str[i], new_str_iter);

    SE_DEBUG_ASSERT(new_str_iter - new_str == new_str_len);
    *new_str_iter = 0;

    SE_DEBUG_ASSERT(se_is_valid_utf16_str(new_str, new_str_len));

    if (out_len)
        *out_len = new_str_len;

    return new_str;
}

#undef VALIDATE2
#undef VALIDATE1
#undef VALIDATE

/***************************************************************************
 *                                                                         *
 * UTF-8 string manipulating.                                              *
 *                                                                         *
 ***************************************************************************/

SE_API int se_utf8_strcmp_ignore_ascii_case(const char* s1, const char* s2)
{
    const unsigned char* p1;
    const unsigned char* p2;
    int c1;
    int c2;

    SE_DEBUG_ASSERT(s1);
    SE_DEBUG_ASSERT(s2);

    if (s1 == s2)
        return 0;

    p1 = (const unsigned char*)s1;
    p2 = (const unsigned char*)s2;

    while ( (*p1) && (*p2) )
    {
        if ( (*p1 >= 'A') && (*p1 <= 'Z') )
            c1 = *p1 - 'A' + 'a';
        else
            c1 = *p1;

        if ( (*p2 >= 'A') && (*p2 <= 'Z') )
            c2 = *p2 - 'A' + 'a';
        else
            c2 = *p2;

        if (c1 != c2)
            return c1 - c2;

        p1++;
        p2++;
    }

    c1 = *p1;
    c2 = *p2;
    return c1 - c2;
}

SE_API int se_utf8_strcmp_n_ignore_ascii_case(const char* s1, const char* s2, int n)
{
    const unsigned char* p1;
    const unsigned char* p2;
    int c1;
    int c2;

    if (s1 == s2)
        return 0;

    SE_DEBUG_ASSERT(s1);
    SE_DEBUG_ASSERT(s2);

    p1 = (const unsigned char*)s1;
    p2 = (const unsigned char*)s2;

    while ( (n > 0) && (*p1) && (*p2) )
    {
        if ( (*p1 >= 'A') && (*p1 <= 'Z') )
            c1 = *p1 - 'A' + 'a';
        else
            c1 = *p1;

        if ( (*p2 >= 'A') && (*p2 <= 'Z') )
            c2 = *p2 - 'A' + 'a';
        else
            c2 = *p2;

        if (c1 != c2)
            return c1 - c2;

        p1++;
        p2++;
        n--;
    }

    if (n > 0)
    {
        c1 = *p1;
        c2 = *p2;
        return c1 - c2;
    }
    else
    {
        return 0;
    }
}

SE_API int se_utf8_strcmp_ignore_space_and_ascii_case(const char* s1, const char* s2)
{
    const unsigned char* p1;
    const unsigned char* p2;
    int c1;
    int c2;

    SE_DEBUG_ASSERT(s1);
    SE_DEBUG_ASSERT(s2);

    if (s1 == s2)
        return 0;

    p1 = (const unsigned char*)s1;
    p2 = (const unsigned char*)s2;

    while ( (*p1) && (*p2) )
    {
        while (*p1 == ' ')
            p1++;
        if ( (*p1 >= 'A') && (*p1 <= 'Z') )
            c1 = *p1 - 'A' + 'a';
        else
            c1 = *p1;

        while (*p2 == ' ')
            p2++;
        if ( (*p2 >= 'A') && (*p2 <= 'Z') )
            c2 = *p2 - 'A' + 'a';
        else
            c2 = *p2;

        if (c1 != c2)
            return c1 - c2;

        p1++;
        p2++;
    }

    c1 = *p1;
    c2 = *p2;
    return c1 - c2;
}

SE_API const char* se_utf8_strstr_ignore_space_and_ascii_case(const char* s1, const char* s2)
{
    SE_DEBUG_ASSERT(s1);
    SE_DEBUG_ASSERT(s2);

    if (s1 == s2)
        return s1;

    while (*s1)
    {
        int c1;
        int c2;
        const unsigned char* p1;
        const unsigned char* p2;

        p1 = (const unsigned char*)s1;
        p2 = (const unsigned char*)s2;

        for (;;)
        {
            while (*p1 == ' ')
                p1++;
            if ( (*p1 >= 'A') && (*p1 <= 'Z') )
                c1 = *p1 - 'A' + 'a';
            else
                c1 = *p1;

            while (*p2 == ' ')
                p2++;
            if ( (*p2 >= 'A') && (*p2 <= 'Z') )
                c2 = *p2 - 'A' + 'a';
            else
                c2 = *p2;

            if (c1 == 0 || c2 == 0)
                break;
            if (c1 != c2)
                break;

            p1++;
            p2++;
        }
        if (c1 == c2)
            return s1;

        s1++;
    }

    return 0;
}

SE_API unsigned int se_utf8_str_hash(const seunichar8* str)
{
    const unsigned char* p;
    unsigned int h;

    SE_DEBUG_ASSERT(str);

    p = (const unsigned char*)str;

    h = *p;
    p++;

    if (h)
    {
        while (*p)
        {
            h = (h << 5) - h + *p;
            p++;
        }
    }

    return h;
}

SE_API char* se_utf8_strdup(const seunichar8* str)
{
    seunichar8* new_str;

    if (str)
    {
        int bytes = se_utf8_str_len(str) + 1;
        new_str = SE_MALLOC(bytes);
        memcpy(new_str, str, bytes);
    }
    else
    {
        new_str = 0;
    }

    return new_str;
}

SE_API char* se_utf8_strdup_n(const seunichar8* str, int len)
{
    seunichar8* new_str;

    if (str)
    {
        SE_DEBUG_ASSERT(len >= 0);
        new_str = SE_MALLOC(len + 1);
        memcpy(new_str, str, len);
        new_str[len] = 0;
    }
    else
    {
        new_str = 0;
    }

    return new_str;
}

SE_API const seunichar8* se_safe_utf8_next_char(const seunichar8* str)
{
    static const unsigned char SE_UTF8_SKIP[256] =
    {
        /* 0x00 - 0x0F */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
        /* 0x10 - 0x1F */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
        /* 0x20 - 0x2F */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
        /* 0x30 - 0x3F */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
        /* 0x40 - 0x4F */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
        /* 0x50 - 0x5F */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
        /* 0x60 - 0x6F */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
        /* 0x70 - 0x7F */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
        /* 0x80 - 0x8F */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
        /* 0x90 - 0x9F */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
        /* 0xA0 - 0xAF */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
        /* 0xB0 - 0xBF */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
        /* 0xC0 - 0xCF */ 1,1,2,2, 2,2,2,2, 2,2,2,2, 2,2,2,2,
        /* 0xD0 - 0xDF */ 2,2,2,2, 2,2,2,2, 2,2,2,2, 2,2,2,2,
        /* 0xE0 - 0xEF */ 3,3,3,3, 3,3,3,3, 3,3,3,3, 3,3,3,3,
        /* 0xF0 - 0xFF */ 4,4,4,4, 4,1,1,1, 1,1,1,1, 1,1,1,1
    };

    #if SE_OPT_DEBUG

        const unsigned char* iter;
        const seunichar8* p1;
        const seunichar8* p2;

        SE_DEBUG_ASSERT(str);

        iter = (const unsigned char*)str;
        if (iter[0] <= 0x7F)
        {
            p1 = str + 1;
        }
        else if (iter[0] >= 0xC2 && iter[0] <= 0xDF)
        {
            SE_DEBUG_ASSERT(iter[1] >= 0x80 && iter[1] <= 0xBF);
            p1 = str + 2;
        }
        else if (iter[0] == 0xE0)
        {
            SE_DEBUG_ASSERT(iter[1] >= 0xA0 && iter[1] <= 0xBF);
            SE_DEBUG_ASSERT(iter[2] >= 0x80 && iter[2] <= 0xBF);
            p1 = str + 3;
        }
        else if (iter[0] >= 0xE1 && iter[0] <= 0xEC)
        {
            SE_DEBUG_ASSERT(iter[1] >= 0x80 && iter[1] <= 0xBF);
            SE_DEBUG_ASSERT(iter[2] >= 0x80 && iter[2] <= 0xBF);
            p1 = str + 3;
        }
        else if (iter[0] == 0xED)
        {
            SE_DEBUG_ASSERT(iter[1] >= 0x80 && iter[1] <= 0x9F);
            SE_DEBUG_ASSERT(iter[2] >= 0x80 && iter[2] <= 0xBF);
            p1 = str + 3;
        }
        else if (iter[0] >= 0xEE && iter[0] <= 0xEF)
        {
            SE_DEBUG_ASSERT(iter[1] >= 0x80 && iter[1] <= 0xBF);
            SE_DEBUG_ASSERT(iter[2] >= 0x80 && iter[2] <= 0xBF);
            p1 = str + 3;
        }
        else if (iter[0] == 0xF0)
        {
            SE_DEBUG_ASSERT(iter[1] >= 0x90 && iter[1] <= 0xBF);
            SE_DEBUG_ASSERT(iter[2] >= 0x80 && iter[2] <= 0xBF);
            SE_DEBUG_ASSERT(iter[3] >= 0x80 && iter[3] <= 0xBF);
            p1 = str + 4;
        }
        else if (iter[0] >= 0xF1 && iter[0] <= 0xF3)
        {
            SE_DEBUG_ASSERT(iter[1] >= 0x80 && iter[1] <= 0xBF);
            SE_DEBUG_ASSERT(iter[2] >= 0x80 && iter[2] <= 0xBF);
            SE_DEBUG_ASSERT(iter[3] >= 0x80 && iter[3] <= 0xBF);
            p1 = str + 4;
        }
        else if (iter[0] == 0xF4)
        {
            SE_DEBUG_ASSERT(iter[1] >= 0x80 && iter[1] <= 0x8F);
            SE_DEBUG_ASSERT(iter[2] >= 0x80 && iter[2] <= 0xBF);
            SE_DEBUG_ASSERT(iter[3] >= 0x80 && iter[3] <= 0xBF);
            p1 = str + 4;
        }
        else
        {
            SE_DEBUG_ASSERT(FALSE);
            p1 = str + 1;
        }

        p2 = str + SE_UTF8_SKIP[*(unsigned char*)str];

        SE_DEBUG_ASSERT(p1 == p2);

        return p1;

    #else

        return str + SE_UTF8_SKIP[*(unsigned char*)str];

    #endif
}

SE_API const seunichar8* se_safe_utf8_prev_char(const seunichar8* str)
{
#if SE_OPT_DEBUG

    const seunichar8* p = str;

    for (;;)
    {
        str--;
        if ((*(const unsigned char*)str & 0xc0) != 0x80)
            break;
    }

    SE_DEBUG_ASSERT(se_safe_utf8_next_char(str) == p);

    return str;

#else

    for (;;)
    {
        str--;
        if ((*(const unsigned char*)str & 0xc0) != 0x80)
            break;
    }

    return str;

#endif
}

SE_API const seunichar8* se_safe_utf8_offset_to_pointer(const seunichar8* str, int offset)
/*
 * str: a UTF-8 encoded string
 * offset: a character offset within str
 * 
 * Converts from an integer character offset to a pointer to a position
 * within the string.
 * 
 * This function allows to pass a negative offset to
 * step backwards. It is usually worth stepping backwards from the end
 * instead of forwards if offset is in the last fourth of the string, 
 * since moving forward is about 3 times faster than moving backward.
 * 
 * Return: the resulting pointer
 */
{
    if (offset > 0) 
    {
        while (offset)
        {
            str = se_safe_utf8_next_char(str);
            offset--;
        }
    }
    else
    {
        while (offset)
        {
            str = se_safe_utf8_prev_char(str);
            offset++;
        }
    }

    return str;
}

SE_API int se_safe_utf8_pointer_to_offset(const seunichar8* str, const seunichar8* pos)
/*
 * str: a UTF-8 encoded string
 * pos: a pointer to a position within str
 * 
 * Converts from a pointer to position within a string to a integer
 * character offset.
 *
 * This function allows pos to be before str, and returns
 * a negative offset in this case.
 * 
 * Return: the resulting character offset
 */
{
    int offset = 0;    

    if (pos < str) 
    {
        while (pos < str)
        {
            pos = se_safe_utf8_next_char(pos);
            offset--;
        }
    }
    else
    {
        while (str < pos)
        {
            str = se_safe_utf8_next_char(str);
            offset++;
        }
    }

    return offset;
}

/***************************************************************************
 *                                                                         *
 * UTF-16 string manipulating.                                             *
 *                                                                         *
 ***************************************************************************/

SE_API int se_utf16_strcmp(const seunichar16* str1, const seunichar16* str2)
{
    SE_DEBUG_ASSERT(str1);
    SE_DEBUG_ASSERT(str2);

    while (*str1 && *str2 && *str1 == *str2)
    {
        str1++;
        str2++;
    }

    return *str1 - *str2;
}

SE_API int se_utf16_strncmp(const seunichar16* str1, const seunichar16* str2, int len)
{
    if (len <= 0)
        return 0;

    SE_DEBUG_ASSERT(str1);
    SE_DEBUG_ASSERT(str2);

    while (len && *str1 && *str2 && *str1 == *str2)
    {
        len--;
        str1++;
        str2++;
    }

    if (len)
        return *str1 - *str2;
    else
        return 0;
}

SE_API seunichar16* se_utf16_strdup(const seunichar16* str)
{
    seunichar16* new_str;

    if (str)
    {
        int bytes = (se_utf16_str_len(str) + 1) * sizeof(seunichar16);
        new_str = SE_MALLOC(bytes);
        memcpy(new_str, str, bytes);
    }
    else
    {
        new_str = 0;
    }

    return new_str;
}

SE_API seunichar16* se_utf16_strdup_n(const seunichar16* str, int len)
{
    seunichar16* new_str;

    if (str)
    {
        SE_DEBUG_ASSERT(len >= 0);
        new_str = SE_MALLOC((len + 1) * sizeof(seunichar16));
        memcpy(new_str, str, len * sizeof(seunichar16));
        new_str[len] = 0;
    }
    else
    {
        new_str = 0;
    }

    return new_str;
}

#if SE_OPT_SURROGATE
SE_API const seunichar16* se_safe_utf16_next_char(const seunichar16* str)
{
    SE_DEBUG_ASSERT(str);

    if (SE_IS_HI_SURROGATE(str[0]))
    {
        SE_DEBUG_ASSERT(SE_IS_LO_SURROGATE(str[1]));
        return str + 2;
    }
    else
    {
        SE_DEBUG_ASSERT(!SE_IS_LO_SURROGATE(str[0]));
        SE_DEBUG_ASSERT(SE_IS_VALID_SCALAR_VALUE(str[0]));
        return str + 1;
    }
}
#endif

#if SE_OPT_SURROGATE
SE_API const seunichar16* se_safe_utf16_prev_char(const seunichar16* str)
{
    SE_DEBUG_ASSERT(str);

    if (SE_IS_LO_SURROGATE(str[-1]))
    {
        SE_DEBUG_ASSERT(SE_IS_HI_SURROGATE(str[-2]));
        return str - 2;
    }
    else
    {
        SE_DEBUG_ASSERT(!SE_IS_HI_SURROGATE(str[-1]));
        SE_DEBUG_ASSERT(SE_IS_VALID_SCALAR_VALUE(str[-1]));
        return str - 1;
    }
}
#endif

#if SE_OPT_SURROGATE
SE_API const seunichar16* se_safe_utf16_offset_to_pointer(const seunichar16* str, int offset)
{
    if (offset > 0) 
    {
        while (offset)
        {
            str = se_safe_utf16_next_char(str);
            offset--;
        }
    }
    else
    {
        while (offset)
        {
            str = se_safe_utf16_prev_char(str);
            offset++;
        }
    }

    return str;
}
#endif

#if SE_OPT_SURROGATE
SE_API int se_safe_utf16_pointer_to_offset(const seunichar16* str, const seunichar16* pos)
{
    int offset = 0;    

    if (pos < str) 
    {
        while (pos < str)
        {
            pos = se_safe_utf16_next_char(pos);
            offset--;
        }
    }
    else
    {
        while (str < pos)
        {
            str = se_safe_utf16_next_char(str);
            offset++;
        }
    }

    return offset;
}
#endif

/***************************************************************************
 *                                                                         *
 * UTF-32 string manipulating.                                             *
 *                                                                         *
 ***************************************************************************/

SE_API seunichar32* se_utf32_strdup(const seunichar32* str)
{
    seunichar32* new_str;

    if (str)
    {
        int bytes = (se_utf32_str_len(str) + 1) * sizeof(seunichar16);
        new_str = SE_MALLOC(bytes);
        memcpy(new_str, str, bytes);
    }
    else
    {
        new_str = 0;
    }

    return new_str;
}

SE_API seunichar32* se_utf32_strdup_n(const seunichar32* str, int len)
{
    seunichar32* new_str;

    if (str)
    {
        SE_DEBUG_ASSERT(len >= 0);
        new_str = SE_MALLOC((len + 1) * sizeof(seunichar32));
        memcpy(new_str, str, len * sizeof(seunichar32));
        new_str[len] = 0;
    }
    else
    {
        new_str = 0;
    }

    return new_str;
}
