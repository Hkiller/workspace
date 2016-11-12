#include "cpe/pal/pal_string.h"

typedef unsigned chartype;

/* 此函数实现在phaystack指定长度phaystack_len内搜索串pneedle，成功返回起始位置，否则返回NULL */
char * cpe_strnstr (const char *phaystack, const char *pneedle, const int phaystack_len)
{
    const unsigned char *haystack, *needle;
    chartype b;
    const unsigned char *rneedle;

    haystack = (const unsigned char *) phaystack;

    if ((b = *(needle = (const unsigned char *) pneedle)))
    {
        int _count = 0;
        chartype c;
        haystack--; /* possible ANSI violation */

        {
            chartype a;
            do {
                _count++;
                if (!(a = *++haystack) || _count > phaystack_len)
                    goto ret0;
            } while (a != b);
            /* 两个串的字符有相等的时候跳出，否则直到转到返回NULL */
        }

        if (!(c = *++needle))
            goto foundneedle;
        ++needle;
        goto jin;

        for (;_count <= phaystack_len;)
        {
            {
                chartype a;
                if (0)
                    jin:{ /* JIN: 在needle是一个字符串时，匹配它的第二个字符 成功GOTO：crest*/
                        _count++;
                        if ((a = *++haystack) == c)
                            goto crest;
                    }
                else {
                    a = *++haystack;
                    _count++;
                }
                do
                {
                    for (; a != b && _count <= phaystack_len; a = *++haystack, _count++)
                    {
                        if (!a || _count > phaystack_len)
                            goto ret0;
                        if ((a = *++haystack) == b)
                            break;
                        if (!a || _count > phaystack_len)
                            goto ret0;
                    }
                    _count++;
                }
                while ((a = *++haystack) != c && _count <= phaystack_len);
            }
crest: /* 保留上一次找到的源串字符起始位置 rhaystack = haystack-- + 1 */
            {
                chartype a;
                {
                    const unsigned char *rhaystack;
            /* rneedle 保存 needle的起始位置 */
                    if (*(rhaystack = haystack-- + 1) == (a = *(rneedle = needle))) {
                        do
                        {
                            if (!a) /* needle 的串匹配完成，直接返回匹配成功的起始位置 */
                                goto foundneedle;
                            if (*++rhaystack != (a = *++needle)) {
                                _count++;
                                break;
                            }
                            if (!a)
                                goto foundneedle;
                            _count++;
                        }
                        while (*++rhaystack == (a = *++needle) && _count <= phaystack_len);
                        _count++;
                    }
                    needle = rneedle; /* took the register-poor aproach */
                }
                if (_count > phaystack_len) {
                    goto ret0;
                }
                if (!a)
                    break;
            }
        }
    }
foundneedle:
    return (char *) haystack;
ret0:
    return 0;
}
