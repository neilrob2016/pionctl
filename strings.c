#include "globals.h"

#ifdef __linux__
/*** Its a BSD function, not found in glibc ***/
char *strnstr(char *haystack, char *needle, size_t len)
{
	char *s1;
	char *s2;
	char *s3;
	size_t cnt1;
	size_t cnt2;

	for(s1=haystack,cnt1=0;*s1 && cnt1 < len;++s1,++cnt1)
	{
		for(s2=needle,s3=s1,cnt2=cnt1;
		    *s2 && *s3 == *s2 && cnt2 < len;
		    ++s2,++s3,++cnt2);
		if (!*s2) return s1;
	}
	return NULL;
}
#endif



/*** Returns 1 if the string matches the pattern, else 0. Supports wildcard 
     patterns containing '*' and '?'. Case insensitive. ***/
int wildMatch(char *str, char *pat)
{
	char *s,*p,*s2;

	for(s=str,p=pat;*s && *p;++s,++p)
	{
		switch(*p)
		{
		case '?':
			continue;

		case '*':
			if (!*(p+1)) return 1;
			for(s2=s;*s2;++s2) if (wildMatch(s2,p+1)) return 1;
			return 0;
		}
		if (toupper(*s) != toupper(*p)) return 0;
	}

	/* Could have '*' leftover in the pattern which can match nothing.
	   eg: "abc*" should match "abc" and "*" should match "" */
	if (!*s)
	{
		/* Could have *'s on the end which should all match "" */
		for(;*p && *p == '*';++p);
		if (!*p) return 1;
	}

	return 0;
}




int isPattern(char *str)
{
	return (strchr(str,'*') || strchr(str,'?'));
}




int isNumber(char *str)
{
	char *s;
	if (!str) return 0;
	for(s=str;*s;++s) if (!isdigit(*s)) return 0;
	return 1;
}




int isNumberWithLen(char *str, int len)
{
	int i;
	if (!str) return 0;
	for(i=0;i < len && isdigit(str[i]);++i);
	return (i == len);
}
