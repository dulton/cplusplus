
#include "rvstr.h"

#include "rvstdio.h"

/* Copy source to destination, appending a null character in any case */
char *rvStrNcpyz(char *dst, const char *src, RvSize_t size) {
    strncpy(dst, src, size);
    dst[size] = '\0';
    return dst;
}

/* Perform a case-insensitive comparison on the first n characters */
/* Return 0 if strings are equal, negative if str1 < str2, positive if str1 > str2 */
int rvStrNicmp(const char *str1, const char *str2, RvSize_t n) {
    RvSize_t i;
    for (i = 0; (i < n) && (*str1) && (*str2); ++i, ++str1, ++str2) {
        int diff = tolower(*str1) - tolower(*str2);
        if (diff)
            return diff;
    }

    if (i == n)
        return 0;

    return *str1 ? 1 : *str2 ? -1 : 0;
}

/* Perform a case-insensitive comparison */
/* Return 0 if strings are equal, negative if str1 < str2, positive if str1 > str2 */
int rvStrIcmp(const char *str1, const char *str2) {

    while (*str1 && *str2) {
        int diff = tolower(*str1) - tolower(*str2);
        if(diff)
            return diff;
        str1++;
        str2++;
    }

    return *str1 ? 1 : *str2 ? -1 : 0;
}

/* Convert a string to lowercase in-place */
void rvStrToLower(char* str) {
    while (*str) {
        *str = (char)tolower(*str);
        str++;
    }
}

/* Specialized variant of strcpy which returns pointer to the end */
/* of the destination string.  This helps speed up copying of */
/* subsequent strings */
char* rvStrCopy(char* d, const char* s) {
    while (*s != '\0') *d++ = *s++;
    *d = '\0';
    return d;
}

/* SPIRENT_BEGIN */
/* Specialized variant of strncpy which returns pointer to the end */
/* of the destination string.  This helps speed up copying of */
/* subsequent strings */
char* rvStrNCopy(char* d, const char* s, RvSize_t n) {
	for (; ((n > 0) && (*s != '\0')); --n) {
		*d++ = *s++;
	}
	*d = '\0';
	return d;
}
/* SPIRENT_END */

/* Modified version of strrev which avoids call to strlen when */
/* the string length is already known */
char* rvStrRevN(char* s, int n) {
    int i, j;
    char c;

    for (i = 0, j = n - 1; i < j; ++i, --j) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }

    return s;
}

char* rvStrFindFirstNotOf(char* str, char* delimiters) {
    for (; *str != '\0' && strchr(delimiters, *str) != NULL; ++str)
        ;
    return str;
}

char* rvStrFindFirstOf(char* str, char* delimiters) {
    for (; *str != '\0' && strchr(delimiters, *str) == NULL; ++str)
        ;
    return str;
}

RvStrTok* rvStrTokConstruct(RvStrTok* t, char* delimiters, char* str) {
    t->delimiters = delimiters;
    t->p = (char*)rvStrFindFirstNotOf(str, delimiters);
    return t;
}

char* rvStrTokGetToken(RvStrTok* t) {
    char* str = t->p;

    if (*str == '\0')
        return NULL;

    t->p = (char*)rvStrFindFirstOf(t->p, t->delimiters);
    if (*(t->p) != '\0') {
        *(t->p) = '\0';
        t->p = (char*)rvStrFindFirstNotOf((t->p) + 1, t->delimiters);
    }
    return str;
}
