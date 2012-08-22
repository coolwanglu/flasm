/*
flasm, command line assembler & disassembler of flash actionscript bytecode
Copyright (c) 2001 Opaque Industries, (c) 2002-2007 Igor Kogan, (c) 2005 Wang Zhen
All rights reserved. See LICENSE.TXT for terms of use.
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "util.h"
#include "action.h"
#ifdef MEMWATCH
#include "memwatch.h"
#endif

int byteorder;

void checkByteOrder(void)
{
  unsigned int x;
  unsigned char *p;

  x = 0x01020304;
  p = (unsigned char *)&x;

  if(*p == 1)
    byteorder = FLASM_BIG_ENDIAN;
  else
    byteorder = FLASM_LITTLE_ENDIAN;
}

int longintCompare(const void *ap, const void *bp)
{
	unsigned long int a = *(const unsigned long int *) ap;
	unsigned long int b = *(const unsigned long int *) bp;

	if (a < b)
		return -1;
	else if (a > b)
		return 1;
	else
		return 0;
}

long int longintBinaryFind(const unsigned long int item, const unsigned long int *table, const long int tablesize)
{
	long int left, right, middle;

	left = 0;
	right = tablesize - 1;

	while (left < right) {
		middle = (left + right) / 2;
		if (table[middle] < item)
			left = middle + 1;
		else
			right = middle;
	}

	if (table[right] == item)
		return(right);
	else
		return(-1);
}

char *strIstr(char *String, char *Pattern)
{
	char *pptr, *sptr, *start;
	size_t slen, plen;

	if (String == NULL)
		return (NULL);

	for (start = String, pptr = Pattern, slen = strlen(String), plen = strlen(Pattern); slen >= plen; start++, slen--) {
		while (toupper(*start) != toupper(*Pattern)) {
			start++;
			slen--;

			if (slen < plen)
				return (NULL);
		}

		sptr = start;
		pptr = Pattern;

		while (toupper(*sptr) == toupper(*pptr)) {
			sptr++;
			pptr++;

			if (*pptr == '\0')
				return (start);
		}
	}
	return (NULL);
}

int strIcmp(const char *s, const char *t)
{
	int d = 0;
	do {
		d = toupper(*s) - toupper(*t);
	} while (*s++ && *t++ && !d);
	return (d);
}

int strnIcmp(const char *s1, const char *s2, size_t len)
{
	int d1, d2;
	if (len == 0)
		return 0;
	do {
		d1 = tolower(*s1++);
		d2 = tolower(*s2++);
	} while (--len && d1 && d2 && d1 == d2);
	return d1 - d2;
}

void lowercase(const char *str, char *buf)
{
	while (*str)
		*buf++ = tolower(*str++);

	*buf = '\0';
}

unsigned long int xtoi(const char *p)
{
	unsigned long int num = 0;
	char c;

	while ((c = tolower(*p)) != '\0') {
		num <<= 4;

		if (isdigit(c))
			num += c - '0';
		else if (c >= 'a' && c <= 'f')
			num += c - 'a' + 10;
		else
			return 0;

		++p;
	}

	return num;
}

uint16_t getWord(FILE *f)
{
	unsigned int byte1 = (unsigned int) fgetc(f);
	unsigned int byte2 = (unsigned int) fgetc(f);
	return (uint16_t)(byte1 & 0xff) | ((byte2 & 0xff) << 8);
}

uint32_t getDoubleWord(FILE *f)
{
  uint32_t low = getWord(f);
  uint32_t hi = getWord(f);

  return low | (hi << 16);
}

int goodID(const char *str)
{
	char buf[MAX_KEYWORD_LEN];
	char *bufp = buf;
	size_t len = 0;

	if (*str == '\0')
		return 1;

	if (!isalpha(*str) && *str != '_')
		return 0;
	
	do {
		++len;
		*bufp++ = tolower(*str++);
	} while (*str && (isalnum(*str) || *str == '_'));

	if (*str != '\0')
		return 0;

	*bufp = '\0';

	if (in_word_set(buf, len) != NULL)
		return 0;
	else
		return 1;
}

void parseTagHeader(FILE *f, unsigned int *typeptr, unsigned long int *lenptr)
{
	unsigned int block = getWord(f);;
	*typeptr = block >> 6;
	*lenptr = block & 63;
	if (*lenptr == 63)
		*lenptr = getDoubleWord(f);
}

char *getTagString(unsigned int tag)
{
	switch(tag) {
		case TAG_END:					return"TAG_END";
		case TAG_SHOWFRAME:				return"TAG_SHOWFRAME";
		case TAG_DEFINESHAPE:			return"TAG_DEFINESHAPE";
		case TAG_FREECHARACTER:			return"TAG_FREECHARACTER";
		case TAG_PLACEOBJECT:			return"TAG_PLACEOBJECT";
		case TAG_REMOVEOBJECT:			return"TAG_REMOVEOBJECT";
		case TAG_DEFINEBITS:			return"TAG_DEFINEBITS";
		case TAG_DEFINEBUTTON:			return"TAG_DEFINEBUTTON";
		case TAG_JPEGTABLES:			return"TAG_JPEGTABLES";
		case TAG_SETBACKGROUNDCOLOR:	return"TAG_SETBACKGROUNDCOLOR";
		case TAG_DEFINEFONT:			return"TAG_DEFINEFONT";
		case TAG_DEFINETEXT:			return"TAG_DEFINETEXT";
		case TAG_DOACTION:				return"TAG_DOACTION";
		case TAG_DEFINEFONTINFO:		return"TAG_DEFINEFONTINFO";
		case TAG_DEFINESOUND:			return"TAG_DEFINESOUND";
		case TAG_STARTSOUND:			return"TAG_STARTSOUND";
		case TAG_STOPSOUND:				return"TAG_STOPSOUND";
		case TAG_DEFINEBUTTONSOUND:		return"TAG_DEFINEBUTTONSOUND";
		case TAG_SOUNDSTREAMHEAD:		return"TAG_SOUNDSTREAMHEAD";
		case TAG_SOUNDSTREAMBLOCK:		return"TAG_SOUNDSTREAMBLOCK";
		case TAG_DEFINEBITSLOSSLESS:	return"TAG_DEFINEBITSLOSSLESS";
		case TAG_DEFINEBITSJPEG2:		return"TAG_DEFINEBITSJPEG2";
		case TAG_DEFINESHAPE2:			return"TAG_DEFINESHAPE2";
		case TAG_DEFINEBUTTONCXFORM:	return"TAG_DEFINEBUTTONCXFORM";
		case TAG_PROTECT:				return"TAG_PROTECT";
		case TAG_PATHSAREPOSTSCRIPT:	return"TAG_PATHSAREPOSTSCRIPT";
		case TAG_PLACEOBJECT2:			return"TAG_PLACEOBJECT2";
		case TAG_REMOVEOBJECT2:			return"TAG_REMOVEOBJECT2";
		case TAG_SYNCFRAME:				return"TAG_SYNCFRAME";
		case TAG_FREEALL:				return"TAG_FREEALL";
		case TAG_DEFINESHAPE3:			return"TAG_DEFINESHAPE3";
		case TAG_DEFINETEXT2:			return"TAG_DEFINETEXT2";
		case TAG_DEFINEBUTTON2:			return"TAG_DEFINEBUTTON2";
		case TAG_DEFINEBITSJPEG3:		return"TAG_DEFINEBITSJPEG3";
		case TAG_DEFINEBITSLOSSLESS2:	return"TAG_DEFINEBITSLOSSLESS2";
		case TAG_DEFINEEDITTEXT:		return"TAG_DEFINEEDITTEXT";
		case TAG_DEFINEVIDEO:			return"TAG_DEFINEVIDEO";
		case TAG_DEFINEMOVIECLIP:		return"TAG_DEFINEMOVIECLIP";
		case TAG_NAMECHARACTER:			return"TAG_NAMECHARACTER";
		case TAG_SERIALNUMBER:			return"TAG_SERIALNUMBER";
		case TAG_DEFINETEXTFORMAT:		return"TAG_DEFINETEXTFORMAT";
		case TAG_FRAMELABEL:			return"TAG_FRAMELABEL";
		case TAG_SOUNDSTREAMHEAD2:		return"TAG_SOUNDSTREAMHEAD2";
		case TAG_DEFINEMORPHSHAPE:		return"TAG_DEFINEMORPHSHAPE";
		case TAG_GENFRAME:				return"TAG_GENFRAME";
		case TAG_DEFINEFONT2:			return"TAG_DEFINEFONT2";
		case TAG_GENCOMMAND:			return"TAG_GENCOMMAND";
		case TAG_DEFINECOMMANDOBJ:		return"TAG_DEFINECOMMANDOBJ";
		case TAG_CHARACTERSET:			return"TAG_CHARACTERSET";
		case TAG_FONTREF:				return"TAG_FONTREF";
		case TAG_EXPORTASSETS:			return"TAG_EXPORTASSETS";
		case TAG_IMPORTASSETS:			return"TAG_IMPORTASSETS";
		case TAG_ENABLEDEBUGGER:		return"TAG_ENABLEDEBUGGER";
		case TAG_INITMOVIECLIP:			return"TAG_INITMOVIECLIP";
		case TAG_DEFINEVIDEOSTREAM:		return"TAG_DEFINEVIDEOSTREAM";
		case TAG_VIDEOFRAME:			return"TAG_VIDEOFRAME";
		case TAG_DEFINEFONTINFO2:		return"TAG_DEFINEFONTINFO2";
		case TAG_ENABLEDEBUGGER2:		return"TAG_ENABLEDEBUGGER2";
		case TAG_SCRIPTLIMITS:			return"TAG_SCRIPTLIMITS";
		case TAG_SETTABINDEX:			return"TAG_SETTABINDEX";
		case TAG_DEFINESHAPE4:			return"TAG_DEFINESHAPE4";
		case TAG_FILEATTRIBUTES:		return"TAG_FILEATTRIBUTES";
		case TAG_PLACEOBJECT3:			return"TAG_PLACEOBJECT3";
		case TAG_IMPORTASSETS2:			return"TAG_IMPORTASSETS2";
		case TAG_DEFINEFONTINFO3:		return"TAG_DEFINEFONTINFO3";
		case TAG_DEFINETEXTINFO:		return"TAG_DEFINETEXTINFO";
		case TAG_DEFINEFONT3:			return"TAG_DEFINEFONT3";
		case TAG_METADATA:				return"TAG_METADATA";
		case TAG_SLICE9:				return"TAG_SLICE9";
		case TAG_DEFINESHAPE5:			return"TAG_DEFINESHAPE5";
		case TAG_DEFINEMORPHSHAPE2:		return"TAG_DEFINEMORPHSHAPE2";
		case TAG_DEFINEBITSPTR:			return"TAG_DEFINEBITSPTR";
		default:						return NULL;
	}
}
