#include <string.h>


size_t strlen(const char *str)
{	
	size_t sz = 0;
	while( (str != NULL) && (*str != '\0') )
	{
		sz++;
		str++;
	}
	return sz;
}

int strcmp(const char *str1, const char *str2)
{
	int cmp = 0;
	
	while( (*str1 != '\0') && (*str1 == *str2))
	{
		str1++;str2++;
	}
	cmp = (*str1 - *str2);
	return cmp;

}

int memcmp(const void *p1, const void *p2, size_t n)
{
	char *str1 = (char*) p1;
	char *str2 = (char*) p2;
	
	size_t index = 0;
	
	int cmp = 0;
	
	while( (index < n) && (*str1 == *str2))
	{
		str1++;str2++;
		index++;
	}
	cmp = (*str1 - *str2);
	return cmp;
}

char *strstr(const char *haystack, const char *needle)
{
	/* clarification: walk over the haystack a char at a time
	 * check the substring from the current char to the end of needle for a match with needle
	 * if we find it, return the current position
	 * if we go out the end, return NULL
	 */
	 
	size_t sz_needle = strlen(needle);
	while(*haystack != '\0')
	{
		if(!memcmp(haystack, needle, sz_needle))
		{
			return (char*) haystack;
		}
		haystack++;
	
	}
	
	return NULL;

}

void *memset(void *str, int c, size_t n)
{
	size_t index = 0;
	char *dst = (char *) str;
	while(index < n)
	{
		dst[index++] = (char) c;
	}
	
	return str;
}

char *strncpy(char *dest, const char *src, size_t n)
{
	size_t index = 0;

	while(index < n)
	{
		dest[index] = src[index];
		index++;
	}
	
 
	return dest;
}
void *memcpy(void *dest, const void * src, size_t n)
{
	return (void*)strncpy(dest, (const char *)src, n);
}
