#ifndef __LUKOS_C_LIB_STRING__
#define __LUKOS_C_LIB_STRING__

#include <stddef.h>

#ifndef NULL
#define NULL 	(void*(0))
#endif


/**
 * get the length of str
 * @param str		the string
 * @return the length of the string 
 */
size_t strlen(const char *str);

/**
 * compare strings
 * @param str1		the first string
 * @param str2		the second string
 * @return  < 0 then it indicates str1 is less than str2,  > 0 then it indicates str1 is greater than str2,  = 0 then it indicates str1 is equal to str2
 */
int strcmp(const char *str1, const char *str2);

/**
 * find the first instance of needle in haystack
 * @param haystack		the target of the search
 * @param needle		the thing we're looking for
 * @return a pointer to the first occurrence in haystack of any of the entire sequence of characters specified in needle or a NULL pointer if the sequence is not present in haystack
 */
char *strstr(const char *haystack, const char *needle);

/**
 * compare memory
 * @see strcmp
 */
int memcmp(const void *str1, const void *str2, size_t n);

/**
 * copy n bytes of src into dest
 * @param dest 		the destination buffer
 * @param src		the source buffer
 * @param n			the size
 */
char *strncpy(char *dest, const char *src, size_t n);

/**
 * set n bytes of str to c
 * @param str		the destination string
 * @param c			the byte to copy in
 * @param n			the number of bytes
 * @return a pointer to str
 */
void *memset(void *str, int c, size_t n);

/**
 * copy n bytes from src to dest
 * @param dest 		the destination buffer
 * @param src		the source buffer
 * @param n			the size
 */
void *memcpy(void *dest, const void * src, size_t n);
#endif
