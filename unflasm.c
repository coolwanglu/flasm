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
#include <time.h>

#ifdef MEMWATCH
#include "memwatch.h"
#endif

#include "util.h"
#include "action.h"

void disassembleSWF(FILE *f, char *fname);
void skipProtected(FILE *f, unsigned long int length);

extern void tellUser(int isError, char *s, ...);

static int indent = 1;
static int targetIndent = 0;
static long int swfabspos = -1;
static long int swfrelpos = -1;

int swfVersion;

static int showLiterals;
static int nDict = 0;
struct _dict {
	char *value;
	int count;
};
static struct _dict dictionary[MAX_CONSTANTS];
static char *flasmDict = NULL;

static long int numLabels = 0;
static long int curLabel = 0;
static unsigned long int *labels = NULL;

static unsigned long int conststart = 0, constend = 0;

/* from flasm.c */
extern int mode;
extern char wasCompressed;

/* from flasm.c <- flasm.ini */
extern int showoffset, hexoffset, literalregisters, literalconstants;

enum {
	eventLoad			= 0x01,
	eventEnterFrame		= 0x02,
	eventUnload			= 0x04,
	eventMouseMove		= 0x08,
	eventMouseDown		= 0x10,
	eventMouseUp		= 0x20,
	eventKeyDown		= 0x40,
	eventKeyUp			= 0x80,
	eventData			= 0x100,
	eventInitialize		= 0x200, /* flash 6/7 only; flash 5 stores smartClip parameters in eventLoad */
	eventPress			= 0x400,
	eventRelease		= 0x800,
	eventReleaseOutside	= 0x1000,
	eventRollOver		= 0x2000,
	eventRollOut		= 0x4000,
	eventDragOver		= 0x8000,
	eventDragOut		= 0x10000,
	eventKeyPress		= 0x20000,
	eventConstruct		= 0x40000 /* flash 7 only */
};

enum {
	IdleToOverUp		= 0x01,
	OverUpToIdle		= 0x02,
	OverUpToOverDown	= 0x04,
	OverDownToOverUp	= 0x08,
	OverDownToOutDown	= 0x10,
	OutDownToOverDown	= 0x20,
	OutDownToIdle		= 0x40,
	IdleToOverDown		= 0x80,
	OverDownToIdle		= 0x100
};

enum {
	keyLeft				= 1,
	keyRight			= 2,
	keyHome				= 3,
	keyEnd				= 4,
	keyInsert			= 5,
	keyDelete			= 6,
	keyBackspace		= 8,
	keyEnter			= 13,
	keyUp				= 14,
	keyDown				= 15,
	keyPageUp			= 16,
	keyPageDown			= 17,
	keyTab				= 18,
	keyEscape			= 19,
	keySpace			= 32
};

/* names of automatic values for function2 */
char *arNames[] = {"this", "arguments", "super", "_root", "_parent", "_global"};
/* preload flags set for particular automatic value */
unsigned int arPreFlags[] = {0x0001, 0x0004, 0x0010, 0x0040, 0x0080, 0x0100};
/* suppress flags set for particular automatic value */
unsigned int arSupFlags[] = {0x0002, 0x0008, 0x0020, 0x0000, 0x0000, 0x0000};

							   
void skipProtected(FILE *f, unsigned long int length)
{
	if (fseek(f, length, SEEK_CUR) != 0)
		tellUser(1, "Unexpected end of file");
}

static void printIndent(int i)
{
	size_t buflen = INDENT_LEVEL*i;
	static long int lastpos = 0;
	long int swfpos;

	if (showoffset > 0) {
		if (showoffset == 1)
			swfpos = swfrelpos;
		else
			swfpos = swfabspos+swfrelpos;

		/* don't print adress if unchanged */
		if (swfpos >= 0 && (swfpos > lastpos || swfpos == 0)) {
			lastpos = swfpos;
			if (hexoffset == 1)
				/* big endian issues? */
				printf("%04X%04X", (unsigned int) (0xFFFF & (swfpos>>16)), (unsigned int) (0xFFFF & swfpos));
			else
				printf("%08li", swfpos);
		}
		else
			buflen += 8;
	}

	if (i>0) {
		char buf[buflen];
		memset(buf, ' ', buflen);
		fwrite(buf, sizeof(char), buflen, stdout);
	}
}

static void print(char *s, ...)
{
	va_list ap;

	printIndent(indent);

	va_start(ap, s);
	vprintf(s, ap);
	va_end(ap);
}

static void printstr(char *str)
{
	char buf[strlen(str)*2+3];
	char *bufp = buf, *bufstr = str;

	*bufp++ = '\'';
	while (*bufstr++ != '\0') {
		switch (*(bufstr-1)) {
			case '\b':
				*bufp++ = '\\';	*bufp++ = 'b';
				break;
			case '\t':
				*bufp++ = '\\';	*bufp++ = 't';
				break;
			case '\n':
				*bufp++ = '\\';	*bufp++ = 'n';
				break;
			case '\f':
				*bufp++ = '\\';	*bufp++ = 'f';
				break;
			case '\r':
				*bufp++ = '\\';	*bufp++ = 'r';
				break;
			case '\'':
				*bufp++ = '\\';	*bufp++ = '\'';
				break;
			case  '\\':
				*bufp++ = '\\';
				*bufp++ = '\\';
				break;
			default:
				*bufp++ = *(bufstr-1);
		}
	}
	*bufp++ = '\'';
    fwrite(buf, sizeof(char), bufp-buf, stdout);
}

static void printFloat(float f, int intCast)
{
	char s[60];
	char *sp, *xpn;

	if (f != f)
		printf("_NANF");
	else if (f == (1.0f / 0.0f))
		printf("POSITIVE_INFINITYF");
	else if (f == (-1.0f / 0.0f))
		printf("NEGATIVE_INFINITYF");
	else {
		sprintf(s, "%#g", (double) f);

		if ((xpn = strchr(s, 'e')) == NULL)
			sp = s + strlen(s) - 1;
		else
			sp = xpn - 2;

		while (*sp == '0')
			--sp;

		if (intCast == 1) {
			if (*sp == '.')
				--sp;
			*(sp + 1) = '\0';
			printf("%s", s);
		}
		else {
			if (*sp == '.')
				*++sp = '0';
			*++sp = '\0';
			printf("%s", s);
			if (xpn != NULL)
				printf("%s", xpn);
			putchar('f');
		}
	}
}

static unsigned int bitPos;
static unsigned int bitBuf;

static void InitBits(FILE *f)
{
	bitPos = 8;
	bitBuf = ((unsigned int) fgetc(f)) & 0xff;
}

static unsigned int getBits(FILE *f, unsigned int n)
{
	unsigned long int v = 0;

	while (n > bitPos) {
		n -= bitPos;
		v |= bitBuf << n;
		bitBuf = ((unsigned int) fgetc(f)) & 0xff;
		bitPos = 8;
	}

	bitPos -= n;
	v |= bitBuf >> bitPos;
	bitBuf &= 0xff >> (8 - bitPos);
	/* never need more than 16 bits */
	return (unsigned int) v & 0xffff;
}

static void printFrameNum(unsigned int frameNum)
{
	printf("\n");
	printIndent(indent);
	printf("frame %u\n", frameNum);
}

static void addLabel(unsigned long int offset)
{
	long int i;

	for (i = 0; i < numLabels; ++i) {
		if (labels[i] == offset)
			return;
	}

	SUREALLOC(numLabels, labels, sizeof (unsigned long int));
	
	if (labels == NULL)
		tellUser(1, "Not enough memory to store all labels");

	labels[numLabels++] = offset;
}

static void processASMLine(char *line);

static void includeFile(const char *ifilename)
{
	FILE *ifile;
	char buf[256];

	while (*ifilename == ' ' || *ifilename == '\t')
		ifilename++;

	if ((ifile = fopen(ifilename, "rt")) == NULL)
		tellUser(1, "Couldn't include file: %s", ifilename);

	printf("\n// start of %s\n", ifilename);

	while (fgets(buf, 256, ifile))
		processASMLine(buf);

	printf("\n// end of %s\n", ifilename);

	fclose(ifile);
}

static void processASMLine(char *line)
{
	char *ci = strIstr(line, "constants ");
	char *fi = strIstr(line, "#include ");

	if ((ci == NULL) || (constend == 0) || !showLiterals) {
		/* line contains no 'constants' or prev constant declaration absent */
		/* or multiple/broken constant pools found in action block */
		if (fi != NULL)
			includeFile(fi + 9);
		else
			printf("%s", line);
	}
	else if (flasmDict == NULL) {
		/* start dict */
		flasmDict = strdup(ci + 10);
	}
	else {
		/* add user constants */
		flasmDict = realloc(flasmDict, strlen(flasmDict) + strlen(ci + 10) + 3);
		strcat(flasmDict, ", ");
		strcat(flasmDict, ci + 10);
	}
}

static void checkLabel(unsigned long int addr)
{
	if (curLabel < numLabels) {
		while (addr > labels[curLabel]) {
			printIndent(indent-1);
			printf(" %s%li: // Wild label in the middle of an action, now placed before next action\n", mode < MODE_UPDATE ? "label" : "lbl", ++curLabel);
			tellUser(0, "Branch into the middle of an action, %s%li (off by %i bytes) is placed before next action", mode < MODE_UPDATE ? "label" : "lbl", curLabel, (int)(addr - labels[curLabel-1]));
		}
		if (addr == labels[curLabel]) {
			printIndent(indent-1);
			/* make sure generated labels don't match user labels */
			printf(" %s%li:\n", mode < MODE_UPDATE ? "label" : "lbl", ++curLabel);
		}
	}
}

static byte *buffer;

static void printActionRecord(byte *p, Action type, unsigned int *lenptr, char **regtable);

static unsigned long int printActions(byte *p, unsigned long int length, unsigned long int maxActions, char **regtable)
{
	/*
	   processes also nested blocks like 'function', 'with', and 'ifFrameLoaded'
	   stops at given length (in bytes) or after given numActions
	   give ANY_VALUE as parameter, if one of them is unused
	 */
	unsigned long int i = 0;
	unsigned long int curAction = 0;
	unsigned int blocklen;

	while ((i < length) && (curAction < maxActions)) {
		Action type = (Action) p[i];

		checkLabel(p + i - buffer);
		swfrelpos = p + i - buffer;

		++i;

		if ((type & 0x80) != 0) {
			blocklen = S16(p + i);
			i += 2;
		}
		else
			blocklen = 0;

		if (i+blocklen <= length)
			printActionRecord(p + i, type, &blocklen, regtable);
		else {
			tellUser(0, "Disassembly may be incomplete: wrong action length encountered");
			return length;
		}

		i += blocklen;
		curAction++;
	}

	checkLabel(p + i - buffer);

	return i;
}

static void printActionRecord(byte *p, Action type, unsigned int *lenptr, char **regtable)
{
	unsigned int len = *lenptr;

    // Added by WangLu
    if (type != SWFACTION_CONSTANTPOOL) {
        printf ("//");
        printf (" %02x", (byte)type);
        if (len > 0) {
            printf (" %02x %02x", (byte)(len&0xff), (byte)((len&0xff00))>>8);
            unsigned int i;
            for (i = 0; i<len; ++i)
                printf(" %02x", p[i]);
        }
        printf("\n");
    }

	switch (type) {
		case SWFACTION_ADD:
			print("oldAdd\n");
			break;
		case SWFACTION_SUBTRACT:
			print("subtract\n");
			break;
		case SWFACTION_MULTIPLY:
			print("multiply\n");
			break;
		case SWFACTION_DIVIDE:
			print("divide\n");
			break;
		case SWFACTION_EQUALS:
			print("oldEquals\n");
			break;
		case SWFACTION_LESSTHAN:
			print("oldLessThan\n");
			break;
		case SWFACTION_LOGICALAND:
			print("and\n");
			break;
		case SWFACTION_LOGICALOR:
			print("or\n");
			break;
		case SWFACTION_LOGICALNOT:
			print("not\n");
			break;
		case SWFACTION_STRINGEQ:
			print("stringEq\n");
			break;
		case SWFACTION_STRINGLENGTH:
			print("stringLength\n");
			break;
		case SWFACTION_SUBSTRING:
			print("substring\n");
			break;
		case SWFACTION_INT:
			print("int\n");
			break;
		case SWFACTION_POP:
			print("pop\n");
			break;
		case SWFACTION_SWAP:
			print("swap\n");
			break;
		case SWFACTION_INITOBJECT:
			print("initObject\n");
			break;
		case SWFACTION_INITARRAY:
			print("initArray\n");
			break;
		case SWFACTION_GETVARIABLE:
			print("getVariable\n");
			break;
		case SWFACTION_SETVARIABLE:
			print("setVariable\n");
			break;
		case SWFACTION_STRINGCONCAT:
			print("concat\n");
			break;
		case SWFACTION_GETPROPERTY:
			print("getProperty\n");
			break;
		case SWFACTION_SETPROPERTY:
			print("setProperty\n");
			break;
		case SWFACTION_DUPLICATECLIP:
			print("duplicateClip\n");
			break;
		case SWFACTION_REMOVECLIP:
			print("removeClip\n");
			break;
		case SWFACTION_TRACE:
			print("trace\n");
			break;
		case SWFACTION_STARTDRAGMOVIE:
			print("startDrag\n");
			break;
		case SWFACTION_STOPDRAGMOVIE:
			print("stopDrag\n");
			break;
		case SWFACTION_STRINGLESSTHAN:
			print("stringLessThan\n");
			break;
		case SWFACTION_STRINGGREATERTHAN:
			print("stringGreaterThan\n");
			break;
		case SWFACTION_RANDOM:
			print("random\n");
			break;
		case SWFACTION_MBLENGTH:
			print("mbLength\n");
			break;
		case SWFACTION_ORD:
			print("ord\n");
			break;
		case SWFACTION_CHR:
			print("chr\n");
			break;
		case SWFACTION_GETTIMER:
			print("getTimer\n");
			break;
		case SWFACTION_MBSUBSTRING:
			print("mbSubstring\n");
			break;
		case SWFACTION_MBORD:
			print("mbOrd\n");
			break;
		case SWFACTION_MBCHR:
			print("mbChr\n");
			break;
		case SWFACTION_NEXTFRAME:
			print("nextFrame\n");
			break;
		case SWFACTION_PREVFRAME:
			print("prevFrame\n");
			break;
		case SWFACTION_PLAY:
			print("play\n");
			break;
		case SWFACTION_STOP:
			print("stop\n");
			break;
		case SWFACTION_TOGGLEQUALITY:
			print("toggleQuality\n");
			break;
		case SWFACTION_STOPSOUNDS:
			print("stopSounds\n");
			break;

			/* ops with args */
		case SWFACTION_PUSHDATA:
			{
				byte pushtype;
				byte *start = p;
				long int pushstart;
				int n = 0;

				/* may need to go back and erase push while processing flasm macros */
				pushstart = ftell(stdout);

				print("push ");

				while (p < start + len) {
					switch (pushtype = *p++) {
						case 0:														 /* string */
							{
								char *d = (char *) p;

								if (mode >= MODE_UPDATE && (Action) start[len] == SWFACTION_POP) {
									fseek(stdout, pushstart, SEEK_SET);				 /* go back to overwrite 'push'            */
									processASMLine(d);
									p = start + len;								 /* skip to the end of push statement      */
									*lenptr += 1;
								}
								else {
									printf("%s", n++ > 0 ? ", " : "");
									printstr(d);
									p += strlen(d) + 1;
								}
								break;
							}

						case 1:														 /* float, used by flash for properties only */
							{
								float f;
								double prop;

								if (byteorder == FLASM_BIG_ENDIAN) {
									byte *fp = (byte *) (&f);

									fp[0] = p[3];
									fp[1] = p[2];
									fp[2] = p[1];
									fp[3] = p[0];
								}
								else
									f = *(float *) p;

								printf("%s", (n++ > 0) ? ", " : "");

								if (modf((double) f, &prop) == 0) {						 /* integer, most likely property */
									switch ((int) prop) {
										case 0:
											printf("X_PROPERTY");
											break;
										case 1:
											printf("Y_PROPERTY");
											break;
										case 2:
											printf("XSCALE_PROPERTY");
											break;
										case 3:
											printf("YSCALE_PROPERTY");
											break;
										case 4:
											printf("CURRENTFRAME_PROPERTY");
											break;
										case 5:
											printf("TOTALFRAMES_PROPERTY");
											break;
										case 6:
											printf("ALPHA_PROPERTY");
											break;
										case 7:
											printf("VISIBLE_PROPERTY");
											break;
										case 8:
											printf("WIDTH_PROPERTY");
											break;
										case 9:
											printf("HEIGHT_PROPERTY");
											break;
										case 10:
											printf("ROTATION_PROPERTY");
											break;
										case 11:
											printf("TARGET_PROPERTY");
											break;
										case 12:
											printf("FRAMESLOADED_PROPERTY");
											break;
										case 13:
											printf("NAME_PROPERTY");
											break;
										case 14:
											printf("DROPTARGET_PROPERTY");
											break;
										case 15:
											printf("URL_PROPERTY");
											break;
										case 16:
											printf("HIGHQUALITY_PROPERTY");
											break;
										case 17:
											printf("FOCUSRECT_PROPERTY");
											break;
										case 18:
											printf("SOUNDBUFTIME_PROPERTY");
											break;
										case 19:
											printf("QUALITY_PROPERTY");
											break;
										case 20:
											printf("XMOUSE_PROPERTY");
											break;
										case 21:
											printf("YMOUSE_PROPERTY");
											break;
										default:
											printFloat(f, 0);
									}
								}
								else
									printFloat(f, 0);

								p += 4;
								break;
							}

						case 2:
							/* null */
							printf("%sNULL", (n++ > 0) ? ", " : "");
							break;

						case 3:
							/* undefined */
							printf("%sUNDEF", (n++ > 0) ? ", " : "");
							break;

						case 4:
							/* register */
							if (n++ > 0)
								printf(", ");
							if (literalregisters && regtable != NULL && regtable[*p] != NULL && *regtable[*p] != '\0') {
								if (goodID(regtable[*p]))
									printf("r:%s", regtable[*p]);
								else
									printf("r:'%s'", regtable[*p]);
							}
							else {
								printf("r:%i", *p);
							}
							p++;
							break;

						case 5:
							/* boolean */
							if (*p++)
								printf("%sTRUE", (n++ > 0) ? ", " : "");
							else
								printf("%sFALSE", (n++ > 0) ? ", " : "");
							break;

						case 6:
							/* double */
							{
								double d;
								byte *dp = (byte *) (&d);
								char s[100];
								char *sp, *xpn;

								if (byteorder == FLASM_BIG_ENDIAN) {
									dp[0] = p[3];
									dp[1] = p[2];
									dp[2] = p[1];
									dp[3] = p[0];
									dp[4] = p[7];
									dp[5] = p[6];
									dp[6] = p[5];
									dp[7] = p[4];
								}
								else {
									dp[0] = p[4];
									dp[1] = p[5];
									dp[2] = p[6];
									dp[3] = p[7];
									dp[4] = p[0];
									dp[5] = p[1];
									dp[6] = p[2];
									dp[7] = p[3];
								}

								/* the old way without '1.5e-24'-like notation
								   fprec = 15-floor(log10(fabs(d)));
								   sprintf(s,"%.*f",fprec,d); */

								printf("%s", (n++ > 0) ? ", " : "");

								if (d == 0) {
									if (mode < MODE_UPDATE)
										printf("0.0");
									else
										printf("0");									 /* save some bytes in update mode - integer instead of double */
								}
								else if (d != d)
									printf("_NAN");
								else if (d == (1.0 / 0.0))
									printf("POSITIVE_INFINITY");
								else if (d == (-1.0 / 0.0))
									printf("NEGATIVE_INFINITY");
								else {
									sprintf(s, "%#.*g", 16, d);

									if ((xpn = strchr(s, 'e')) == NULL)
										sp = s + strlen(s) - 1;
									else
										sp = xpn - 2;									 /* one digit less precision for exp form values
																						    preventing MAX_NUMBER rounding to INFINITY */

									while (*sp == '0')									 /* delete 0's at the end of the number or mantissa */
										--sp;

									if (*sp == '.')										 /* expand values like "1." to "1.0" */
										*++sp = '0';

									*++sp = '\0';										 /* terminate buffer (exponent is cutted off) */

									printf("%s", s);

									if (xpn != NULL)									 /* if exponent here, print it */
										printf("%s", xpn);
								}

								p += 8;
								break;
							}

						case 7:
							/* integer */
							{
								int i;

								if (byteorder == FLASM_BIG_ENDIAN) {
									byte *ip = (byte *) (&i);

									ip[0] = p[3];
									ip[1] = p[2];
									ip[2] = p[1];
									ip[3] = p[0];
								}
								else
									i = *(int *) p;

								printf("%s%i", (n++ > 0) ? ", " : "", i);
								p += 4;
								break;
							}

						case 8:
							/* dictionary, 1-byte reference */
							{
								char *d;
								if (showLiterals && (*p < nDict)) {
									d = dictionary[*p].value;
									if (mode >= MODE_UPDATE && (Action) start[len] == SWFACTION_POP) {
										fseek(stdout, pushstart, SEEK_SET);			 /* go back to overwrite 'push'            */
										processASMLine(d);
										p = start + len;							 /* skip to the end of push statement      */
										*lenptr += 1;
									}
									else {
										printf("%s", n++ > 0 ? ", " : "");
										printstr(d);
										dictionary[*p].count++;						  /* constant used one more time */
										p++;
									}
								}
								else {
									printf("%sc:%u", n++ > 0 ? ", " : "", *p);
									p++;
								}
								break;
							}

						case 9:
							/* dictionary, 2-byte reference */
							{
								char *d;
								if (showLiterals && (S16(p) < nDict)) {
									d = dictionary[S16(p)].value;
									if (mode >= MODE_UPDATE && (Action) start[len] == SWFACTION_POP) {
										fseek(stdout, pushstart, SEEK_SET);			 /* go back to overwrite 'push'            */
										processASMLine(d);
										p = start + len;							 /* skip to the end of push statement      */
										*lenptr += 1;
									}
									else {
										printf("%s", n++ > 0 ? ", " : "");
										printstr(d);
										dictionary[S16(p)].count++;					  /* constant used one more time */
										p += 2;
									}
								}
								else {
									printf("%sc:%u", n++ > 0 ? ", " : "", S16(p));
									p += 2;
								}
								break;
							}

						default:
							printf("%s%s // unknown push type %i: rest of push skipped", (n++ > 0) ? ", " : "", "???", pushtype);
							tellUser(0, "Unknown push type %i: rest of push skipped", pushtype);
							p = start + len;
					}
				}

				putchar('\n');

				break;
			}

		case SWFACTION_GOTOFRAME:
			print("gotoFrame %u\n", S16(p));
			p += 2;
			break;

		case SWFACTION_GETURL:
		{
			char *url = (char *) p;
			p += strlen(url) + 1;
			print("getURL ");
			printstr(url);
			putchar(' ');
			printstr((char *) p);
			putchar('\n');
			break;
		}

		case SWFACTION_BRANCHALWAYS:
		{
			long int l = longintBinaryFind((unsigned long int)(p + 2 + S16signed(p) - buffer), labels, numLabels);
			if (l >= 0) {
				print("branch %s%i", mode < MODE_UPDATE ? "label" : "lbl", l + 1, labels);
				if (showoffset == 0)
					putchar('\n');
				else
					printf(" // offset %i\n", S16signed(p));
			}
			else {
				print("branch %i // branch target not found\n", S16signed(p));
				tellUser(0, "branch target not found: %li", S16signed(p));
			}
			break;
		}

		case SWFACTION_BRANCHIFTRUE:
		{
			long int l = longintBinaryFind((unsigned long int)(p + 2 + S16signed(p) - buffer), labels, numLabels);
			if (l >= 0) {
				print("branchIfTrue %s%i", mode < MODE_UPDATE ? "label" : "lbl", l + 1, labels);
				if (showoffset == 0)
					putchar('\n');
				else
					printf(" // offset %i\n", S16signed(p));
			}
			else {
				print("branchIfTrue %i // branch target not found\n", S16signed(p));
				tellUser(0, "branchIfTrue target not found: %li", S16signed(p));
			}
			break;
		}

		case SWFACTION_GETURL2:
			{
				byte flags = *p;
				switch (flags) {
					case 0:
						print("getURL2\n");
						break;
					case 1:
						print("getURL2 GET\n");
						break;
					case 2:
						print("getURL2 POST\n");
						break;
					case 0x40:
						print("loadMovie\n");
						break;
					case 0x41:
						print("loadMovie GET\n");
						break;
					case 0x42:
						print("loadMovie POST\n");
						break;
					case 0x80:
						print("loadVariablesNum\n");
						break;
					case 0x81:
						print("loadVariablesNum GET\n");
						break;
					case 0x82:
						print("loadVariablesNum POST\n");
						break;
					case 0xC0:
						print("loadVariables\n");
						break;
					case 0xC1:
						print("loadVariables GET\n");
						break;
					case 0xC2:
						print("loadVariables POST\n");
						break;
					default:
						print("getURL2 0x%x // unknown flag\n", flags);
						tellUser(0, "Unknown getURL2 flag: 0x%x");
				}

				break;
			}

		case SWFACTION_CALLFRAME:
			print("callFrame\n");
			break;

		case SWFACTION_GOTOEXPRESSION:
			print("goto");
			if (*p == 0)
				puts("AndStop");
			else if (*p == 1)
				puts("AndPlay");
			else if ((*p == 2) && (len == 3)) {
				/* undocumented additional argument - the number of frames in all previous scenes */
				p++;
				printf("AndStop skip %u\n", S16(p));
			}
			else if ((*p == 3) && (len == 3)) {
				/* undocumented additional argument - the number of frames in all previous scenes */
				p++;
				printf("AndPlay skip %u\n", S16(p));
			}
			else {
				/* what the hell is it? assume andPlay, since flag>1 */
				printf("AndPlay // unknown goto flag %i\n", *p);
				tellUser(0, "Unknown goto flag %i", *p);
			}
			break;

		case SWFACTION_IFFRAMELOADED:
			{
				unsigned int frame = S16(p);
				byte frameLoadedActions;
				p += 2;
				print("ifFrameLoaded %u\n", frame);
				++indent;
				frameLoadedActions = *p++;
				*lenptr += printActions(p, ANY_VALUE, (unsigned long int) frameLoadedActions, regtable);
				--indent;
				print("end // of ifFrameLoaded %u\n\n", frame);
				break;
			}

		case SWFACTION_IFFRAMELOADEDEXPRESSION:
			{
				byte frameLoadedActions;
				print("ifFrameLoadedExpr\n");
				++indent;
				frameLoadedActions = *p++;
				*lenptr += printActions(p, ANY_VALUE, (unsigned long int) frameLoadedActions, regtable);
				--indent;
				print("end // of ifFrameLoadedExpr\n\n");
				break;
			}

		case SWFACTION_SETTARGET:
			{
				if (targetIndent == 1) {
					--indent;
					print("end\n");
					targetIndent = 0;
				}
				if (strlen((char *) p) > 0) {
					print("setTarget '%s'\n", p);
					++indent;
					targetIndent = 1;
				}
				break;
			}

		case SWFACTION_SETTARGETEXPRESSION:
			if (targetIndent == 1) {
				--indent;
				print("end\n");
				targetIndent = 0;
			}
			print("setTargetExpr\n");
			++indent;
			targetIndent = 1;
			break;

		case SWFACTION_GOTOLABEL:
			print("gotoLabel '%s'\n", p);
			break;

		case SWFACTION_END:
			break;

			/* f5 ops */
		case SWFACTION_DELETE:
			print("delete\n");
			break;
		case SWFACTION_DELETE2:
			print("delete2\n");
			break;
		case SWFACTION_VAR:
			print("var\n");
			break;
		case SWFACTION_VAREQUALS:
			print("varEquals\n");
			break;
		case SWFACTION_CALLFUNCTION:
			print("callFunction\n");
			break;
		case SWFACTION_RETURN:
			print("return\n");
			break;
		case SWFACTION_MODULO:
			print("modulo\n");
			break;
		case SWFACTION_NEW:
			print("new\n");
			break;
		case SWFACTION_TYPEOF:
			print("typeof\n");
			break;
		case SWFACTION_TARGETPATH:
			print("targetPath\n");
			break;
		case SWFACTION_NEWADD:
			print("add\n");
			break;
		case SWFACTION_NEWLESSTHAN:
			print("lessThan\n");
			break;
		case SWFACTION_NEWEQUALS:
			print("equals\n");
			break;
		case SWFACTION_TONUMBER:
			print("toNumber\n");
			break;
		case SWFACTION_TOSTRING:
			print("toString\n");
			break;
		case SWFACTION_DUP:
			print("dup\n");
			break;
		case SWFACTION_GETMEMBER:
			print("getMember\n");
			break;
		case SWFACTION_SETMEMBER:
			print("setMember\n");
			break;
		case SWFACTION_INCREMENT:
			print("increment\n");
			break;
		case SWFACTION_DECREMENT:
			print("decrement\n");
			break;
		case SWFACTION_NEWMETHOD:
			print("newMethod\n");
			break;
		case SWFACTION_CALLMETHOD:
			print("callMethod\n");
			break;
		case SWFACTION_BITWISEAND:
			print("bitwiseAnd\n");
			break;
		case SWFACTION_BITWISEOR:
			print("bitwiseOr\n");
			break;
		case SWFACTION_BITWISEXOR:
			print("bitwiseXor\n");
			break;
		case SWFACTION_SHIFTLEFT:
			print("shiftLeft\n");
			break;
		case SWFACTION_SHIFTRIGHT:
			print("shiftRight\n");
			break;
		case SWFACTION_SHIFTRIGHT2:
			print("shiftRight2\n");
			break;

		case SWFACTION_CONSTANTPOOL:
			{
				unsigned int i, n = S16(p);
				int willInclude = 0;
				unsigned int constlen = 2;

				if (n > MAX_CONSTANTS)
					tellUser(0, "Too many constants");

				p += 2;
				conststart = ftell(stdout);
				print("constants ");

				nDict = 0;

				for (i = 0; i < n; ++i) {
					if (strnIcmp((char *) p, "#include", 8) == 0)
						willInclude = 1;

					dictionary[i].value = (char *) p;
					dictionary[i].count = 0;
					nDict++;

					printstr((char *) p);
					if (i < n - 1)
						printf("%s", ", ");
					else
						printf("%s", "  ");

					constlen += strlen((char *) p) + 1;
					p += strlen((char *) p) + 1;
				}

				if (constlen != len) {
					tellUser(0, "Declared constant pool length %u differs from calculated length %u", len, constlen);
					/* try to restore the real constant pool length */
					*lenptr = 0xff & constlen;
					*(lenptr+1) = 0xff & (constlen >> 8);
				}

				printf("%s", "\n");

				/* put some free space after constant pool to add user constants from included file later */
				if (mode >= MODE_UPDATE && willInclude == 1)
					for (i = 0; i < MAX_INCLUDE_POOL; ++i)
						putchar(' ');

				constend = ftell(stdout);

				break;
			}

		case SWFACTION_WITH:
			{
				unsigned int withlen = S16(p);

				print("with\n");

				++indent;
				printActions(p + 2, (unsigned long int) withlen, ANY_VALUE, regtable);
				--indent;

				*lenptr += withlen;

				print("end\n");

				break;
			}

		case SWFACTION_DEFINEFUNCTION:
			{
				int nargs;
				unsigned int funclen;
				char *name = (char *) p;
				char *argname;

				p += strlen(name) + 1;
				nargs = S16(p);
				p += 2;

				if (*name != '\0') {
					if (mode < MODE_UPDATE && goodID(name))
						print("function %s (", name);
					else
						print("function '%s' (", name);
				}
				else
					print("function (");

				if (nargs > 0) {
					argname = (char *) p;
					printf("'%s'", argname);
					p += strlen(argname) + 1;
					--nargs;
				}

				for (; nargs > 0; --nargs) {
					argname = (char *) p;
					printf(", '%s'", argname);
					p += strlen(argname) + 1;
				}

				putchar(')');
				putchar('\n');

				funclen = S16(p);
				p += 2;

				++indent;
				printActions(p, (unsigned long int) funclen, ANY_VALUE, NULL);
				--indent;

				print("end // of function %s\n\n", name);

				*lenptr += funclen;

				break;
			}

		case SWFACTION_ENUMERATE:
			print("enumerate\n");
			break;

		case SWFACTION_SETREGISTER:
			if (literalregisters && regtable != NULL && regtable[*p] != NULL && *regtable[*p] != '\0') {
				if (goodID(regtable[*p]))
					print("setRegister r:%s\n", regtable[*p]);
				else
					print("setRegister r:'%s'\n", regtable[*p]);
			}
			else
				print("setRegister r:%i\n", *p);
			break;

		case SWFACTION_STRICTEQUALS:
			print("strictEquals\n");
			break;

		case SWFACTION_GREATERTHAN:
			print("greaterThan\n");
			break;

		case SWFACTION_ENUMERATEVALUE:
			print("enumerateValue\n");
			break;

		case SWFACTION_INSTANCEOF:
			print("instanceOf\n");
			break;

		case SWFACTION_STRICTMODE:
			print("strictMode");
			if (*p > 0)
				puts(" ON");
			else
				puts(" OFF");
			break;

		case SWFACTION_DEFINEFUNCTION2:
			{
				unsigned int i, funclen, nargs, autoregFlags;
				int firstprint = 1;
				byte nregisters, curautoreg;
				char *argnames[MAX_REGISTERS];
				char *name = (char *) p;
				memset(argnames, 0, MAX_REGISTERS * sizeof (char *));

				p += strlen(name) + 1;
				nargs = S16(p);
				p += 2;
				nregisters = *p++;
				autoregFlags = S16(p);
				p += 2;
				
				if (*name != '\0') {
					if (mode < MODE_UPDATE && goodID(name))
						print("function2 %s (", name);
					else
						print("function2 '%s' (", name);
				}
				else
					print("function2 (");

				/* print function arguments, store links to their names in register allocation table */
				for (; nargs > 0; nargs--) {
					byte reg = *p;
					char *arg = (char *)(p+1);

					if (reg != 0) {
						if (argnames[reg] != NULL) {
							tellUser (0, "Duplicate register allocation in function2 %s: %s and %s both go to r:%u", name, argnames[reg], arg, reg);
						}
						argnames[reg] = arg;
						printf("r:%u='%s'%s", reg, arg, nargs == 1 ? "" : ", ");
					}
					else
						printf("'%s'%s", arg, nargs == 1 ? "" : ", ");

					p += strlen(arg) + 2;
				}

				printf("%s", ") (");

				/* allocate registers for "automatic" names based on flags, skip suppressed parameters */
				curautoreg = 1;

				for (i = 0; i < MAX_AUTO_REGS; i++) {
					if ((autoregFlags & arPreFlags[i]) == arPreFlags[i]) {
						/* preload flag */

						if (argnames[curautoreg] != NULL && strcmp(argnames[curautoreg], arNames[i]))
							tellUser (0, "Duplicate register allocation in function2 %s: %s and %s both go to r:%u", name, argnames[curautoreg], arNames[i], curautoreg);
						if (autoregFlags & arSupFlags[i])
							tellUser (0, "Preload and suppress flags are both set for %s parameter of function2 %s", arNames[i], name);

						argnames[curautoreg] = arNames[i];
						autoregFlags -= arPreFlags[i];
						printf("%sr:%u='%s'", firstprint == 0 ? ", " : "", curautoreg, arNames[i]);
						firstprint = 0;
						curautoreg++;
					}
					else if ((autoregFlags & arSupFlags[i]) == arSupFlags[i]) {
						/* suppress flag */
						autoregFlags -= arSupFlags[i];
					}
					else {
						/* the parameter is neither preloaded nor suppressed */
						printf("%s'%s'", firstprint == 0 ? ", " : "", arNames[i]);
						firstprint = 0;
					}
				}

				printf("%s", ")\n");

				if (autoregFlags != 0)
					tellUser(0,"Unknown register flag for function2 %s: %u", name, autoregFlags);
				
				funclen = S16(p);
				p += 2;

				++indent;
				printActions(p, (unsigned long int) funclen, ANY_VALUE, argnames);
				--indent;

				print("end // of function %s\n\n", name);

				*lenptr += funclen;

				break;
			}

		case SWFACTION_THROW:
			print("throw\n");
			break;

		case SWFACTION_EXTENDS:
			print("extends\n");
			break;

		case SWFACTION_IMPLEMENTS:
			print("implements\n");
			break;

		case SWFACTION_CAST:
			print("cast\n");
			break;

		case SWFACTION_FSCOMMAND2:
			print("FSCommand2\n");
			break;

		case SWFACTION_TRY:
			{
				unsigned int trylen, catchlen, finallylen;
				/* try type */
				byte catchtype = *p++;
				trylen = S16(p);
				p += 2;
				catchlen = S16(p);
				p += 2;
				finallylen = S16(p);
				p += 2;

				if (catchtype & 4) {
					/* error in register */
					if (literalregisters && regtable != NULL && regtable[*p] != NULL && *regtable[*p] != '\0')
						print("try r:%s\n", regtable[*p++]);
					else
						print("try r:%u\n", *p++);
				}
				else {
					/* error in variable */
					print("try");
					if (*p != '\0')
						printf(" '%s'", (char *) p);
					putchar('\n');
					p += len-7;
				}

				++indent;
				printActions(p, (unsigned long int) trylen, ANY_VALUE, regtable);
				--indent;

				if (catchlen > 0) {
					print("catch\n");
					++indent;
					printActions(p+trylen, (unsigned long int) catchlen, ANY_VALUE, regtable);
					--indent;
				}
				if (finallylen > 0) {
					print("finally\n");
					++indent;
					printActions(p+trylen+catchlen, (unsigned long int) finallylen, ANY_VALUE, regtable);
					--indent;
				}

				*lenptr += trylen+catchlen+finallylen;
				print("end // of try\n");
				break;
			}

		default:
			print("swfAction 0x%02x", type);
			if (len > 0) {
				unsigned int i;
				printf("%s", " hexdata ");
				for (i = 0; i < len; ++i) {
					printf("0x%02X", *p);
					if (i < len - 1)
						printf("%s", ",");
					p++;
				}
			}
			printf("%s", " // unknown action\n");
			tellUser(0, "Unknown action 0x%02x", type);
	}
	return;
}

static void rebuildConstantPool(void)
{
	long int curpos = ftell(stdout);
	unsigned long int k;
	int i;
	int constOK = 0;

	fseek(stdout, conststart, SEEK_SET);												 /* go back to the last constants declaration */

	for (i = 0; i < nDict; ++i) {
		/* remove constants used less than 2 times, and empty strings */
		if ((dictionary[i].count > 1) && (strlen(dictionary[i].value) > 0)) {
			if (constOK == 0) {
				print("constants ");
				constOK = 1;
			}

			printstr(dictionary[i].value);
			printf("%s", ", ");
		}
	}

	if (flasmDict != NULL) {
		if (constOK == 0) {
			print("constants ");
			constOK = 1;
		}
		if (ftell(stdout) + strlen(flasmDict) >= constend)
			tellUser(0, "Too many user constants: %s", flasmDict);
		else
			printf("%s", flasmDict);													 /* add user constants */
	}
	else if (constOK == 1)
		fseek(stdout, -2, SEEK_CUR);													 /* remove last ", " */

	putchar('\n');

	for (k = ftell(stdout); k < constend - 1; ++k)										 /* fill the rest of former constants with spaces */
		putchar(' ');

	putchar('\n');

	if (flasmDict != NULL) {
		free(flasmDict);
		flasmDict = NULL;
	}
	fseek(stdout, curpos, SEEK_SET);
}

static void readActionBuffer(FILE *f, unsigned long int length)
{
	if (length == 0)
		return;

	buffer = malloc(length);
	if (buffer == NULL)
		tellUser(1, "Not enough memory to process action block");

	if (fread(buffer, 1, length, f) != length)
		tellUser(1, "Attempt to read beyond EOF");
}

static void createLabels(unsigned long int length)
{
	unsigned long int i;
	unsigned int blocklen;
	unsigned int numpools = 0;

	numLabels = 0;
	curLabel = 0;
	for (i = 0; i < length; ++i) {
		if (buffer[i] & 0x80) {
			blocklen = S16(buffer + i + 1);

			if ((Action) buffer[i] == SWFACTION_BRANCHALWAYS || (Action) buffer[i] == SWFACTION_BRANCHIFTRUE) {
				if ((signed long int) (i + 3 + blocklen + S16signed(buffer + i + 3)) >= 0) {
					addLabel(i + 3 + blocklen + S16signed(buffer + i + 3));
				}
			}
			else if ((Action) buffer[i] == SWFACTION_CONSTANTPOOL) {
				byte *p = buffer + i + 3;
				unsigned int n, nStrings = S16(p);
				numpools++;
				p += 2;
				for (n = 0; n < nStrings; ++n)
					p += strlen((char *) p) + 1;

				if (blocklen != p - buffer - i - 3) {
					/* fix broken length */
					blocklen = p - buffer - i - 3;
				}
			}

			i += blocklen + 2;
		}
	}
	if ((numpools > 1) && (literalconstants < 2))
		showLiterals = 0;
}

static void printActionBlock(FILE *f, unsigned long int length, abtype abtype, unsigned int flags, byte key)
{
	swfabspos = ftell(f);
	swfrelpos = 0;
	conststart = constend = 0;

	nDict = 0;
	showLiterals = literalconstants;

	++indent;

	readActionBuffer(f, length);

	createLabels(length);

	qsort(labels, (size_t) numLabels, sizeof (unsigned long int), longintCompare);

	printActions(buffer, length, ANY_VALUE, NULL);

	if ((mode >= MODE_UPDATE) && (constend > 0) && showLiterals)
		rebuildConstantPool();

	if (targetIndent == 1) {
		--indent;
		print("end\n");
		targetIndent = 0;
	}

	--indent;

	if (buffer != NULL) {
		free(buffer);
		buffer = NULL;
	}

	if (labels != NULL) {
		free(labels);
		labels = NULL;
	}

	swfabspos = -1;
	swfrelpos = -1;
}

static void skipMatrix(FILE *f)
{
	InitBits(f);
	if (getBits(f, 1))
		getBits(f, getBits(f, 5) * 2);
	if (getBits(f, 1))
		getBits(f, getBits(f, 5) * 2);
	getBits(f, getBits(f, 5) * 2);
}

static void skipColorTransform(FILE *f)
{
	unsigned int needAdd, needMul, nBits;
	InitBits(f);
	needAdd = getBits(f, 1);
	needMul = getBits(f, 1);
	nBits = getBits(f, 4);
	if (needMul)
		getBits(f, nBits * 4);
	if (needAdd)
		getBits(f, nBits * 4);
}

static void skipFilters(FILE *f, byte numfilters)
{
	while(numfilters--) {
		int filter = fgetc(f);
		switch(filter){
			case FILTER_DROPSHADOW:
				skipProtected(f, 23);
				break;
			case FILTER_BLUR:
				skipProtected(f, 9);				
				break; 
			case FILTER_GLOW:
				skipProtected(f, 15);				
				break; 
			case FILTER_BEVEL:
				skipProtected(f, 27);				
				break; 
			case FILTER_GRADIENTGLOW:
				skipProtected(f, fgetc(f)*5 + 19);	
				break; 
			case FILTER_ADJUSTCOLOR:
				skipProtected(f, 80);				
				break;
			case FILTER_GRADIENTBEVEL:
				skipProtected(f, fgetc(f)*5 + 19);	
				break;
			default:
				tellUser(1, "Unknown filter %i", filter);
		}
	}
}

static void parseKeyPressEvent(byte onKey)
{
	printf("keyPress ");

	if (onKey == keySpace)
		printf("_SPACE");
	else if (onKey == '\'')
		printf("'%s'", "\\'");
	else if (onKey == '\\')
		printf("'%s'", "\\\\");
	else if (onKey > 32)
		printf("'%c'", onKey);
	else
		switch (onKey) {
			case keyLeft:
				printf("_LEFT");
				break;
			case keyRight:
				printf("_RIGHT");
				break;
			case keyHome:
				printf("_HOME");
				break;
			case keyEnd:
				printf("_END");
				break;
			case keyInsert:
				printf("_INSERT");
				break;
			case keyDelete:
				printf("_DELETE");
				break;
			case keyBackspace:
				printf("_BACKSPACE");
				break;
			case keyEnter:
				printf("_ENTER");
				break;
			case keyUp:
				printf("_UP");
				break;
			case keyDown:
				printf("_DOWN");
				break;
			case keyPageUp:
				printf("_PAGEUP");
				break;
			case keyPageDown:
				printf("_PAGEDOWN");
				break;
			case keyTab:
				printf("_TAB");
				break;
			case keyEscape:
				printf("_ESCAPE");
				break;
			default:
				printf("0x%02x // unknown key", onKey);
				tellUser(0, "Unknown key 0x%02x in keyPress event", onKey);
		}
}

static void parseButtonEvent(FILE *f, unsigned int event, unsigned long int length)
{
	char delim = ' ';
	byte key = 0;

	putchar('\n');
	++indent;
	print("on");

	if (event & IdleToOverUp) {
		putchar(delim);
		printf("idleToOverUp");
		delim = ',';
	}

	if (event & OverUpToIdle) {
		putchar(delim);
		printf("overUpToIdle");
		delim = ',';
	}

	if (event & OverUpToOverDown) {
		putchar(delim);
		printf("overUpToOverDown");
		delim = ',';
	}

	if (event & OverDownToOverUp) {
		putchar(delim);
		printf("overDownToOverUp");
		delim = ',';
	}

	if (event & OverDownToOutDown) {
		putchar(delim);
		printf("overDownToOutDown");
		delim = ',';
	}

	if (event & OutDownToOverDown) {
		putchar(delim);
		printf("outDownToOverDown");
		delim = ',';
	}

	if (event & OutDownToIdle) {
		putchar(delim);
		printf("outDownToIdle");
		delim = ',';
	}

	if (event & IdleToOverDown) {
		putchar(delim);
		printf("idleToOverDown");
		delim = ',';
	}

	if (event & OverDownToIdle) {
		putchar(delim);
		printf("overDownToIdle");
		delim = ',';
	}

	/* keyPress */
	if (event > 0x1FF) {
		key = (byte) (event >> 9);
		putchar(delim);
		parseKeyPressEvent(key);
	}

	printf("\n");
	printActionBlock(f, length, AB_BUTTONEVENT, event, key);
	print("end\n");
	--indent;
}

static void parseButton2(FILE *f, unsigned long int length)
{
	unsigned int condition, buttonID, actionOffset;
	unsigned long int lastABLength;

	buttonID = getWord(f);
	/* trackAsMenu */
	fgetc(f);

	actionOffset = getWord(f);
	lastABLength = length - 8 - actionOffset;

	if (actionOffset > 0) {
		printf("\n");
		print("defineButton %u\n", buttonID);

		/* skip button data */
		while (actionOffset > 2) {
			fgetc(f);
			--actionOffset;
		}

		while ((actionOffset = getWord(f)) > 0) {
			lastABLength -= actionOffset;
			condition = getWord(f);
			parseButtonEvent(f, condition, actionOffset - 4);
		}

		/* last event */
		condition = getWord(f);
		parseButtonEvent(f, condition, lastABLength);

		print("end // of defineButton %u\n", buttonID);
	}
	else {
		/* no button events */
		while (lastABLength + 2 > 0) {
			fgetc(f);
			--lastABLength;
		}
	}
	
	/* button end */
	fgetc(f);
}

static int printBitMasked(unsigned long int *eventptr, unsigned long int e, char *delimptr, char *str)
{
	if (*eventptr & e) {
		*eventptr -= e;
		printf("%c%s", *delimptr, str);
		*delimptr = ',';
		return 1;
	}
	else
		return 0;
}

static void parseEvent(FILE *f, unsigned long int event, unsigned long int length)
{
	byte key = 0;
	char delim = ' ';
	unsigned long int event2 = event;
	
	putchar('\n');
	++indent;
	print("onClipEvent");

	printBitMasked(&event2, eventLoad,				&delim, "load");
	printBitMasked(&event2, eventEnterFrame,		&delim, "enterFrame");
	printBitMasked(&event2, eventUnload,			&delim, "unload");
	printBitMasked(&event2, eventMouseMove,			&delim, "mouseMove");
	printBitMasked(&event2, eventMouseDown,			&delim, "mouseDown");
	printBitMasked(&event2, eventMouseUp,			&delim, "mouseUp");
	printBitMasked(&event2, eventKeyDown,			&delim, "keyDown");
	printBitMasked(&event2, eventKeyUp,				&delim, "keyUp");
	printBitMasked(&event2, eventData,				&delim, "data");
	printBitMasked(&event2, eventInitialize,		&delim, "initialize");
	printBitMasked(&event2, eventConstruct,			&delim, "construct");
	printBitMasked(&event2, eventPress,				&delim, "press");
	printBitMasked(&event2, eventRelease,			&delim, "release");
	printBitMasked(&event2, eventReleaseOutside,	&delim, "releaseOutside");
	printBitMasked(&event2, eventRollOver,			&delim, "rollOver");
	printBitMasked(&event2, eventRollOut,			&delim, "rollOut");
	printBitMasked(&event2, eventDragOver,			&delim, "dragOver");
	printBitMasked(&event2, eventDragOut,			&delim, "dragOut");

	if (printBitMasked(&event2, eventKeyPress, &delim, "")) {
		key = (byte) fgetc(f);
		parseKeyPressEvent(key);
		length--;
		delim = ',';
	}

	if (event2 != 0) {
		printf("%c%lu // unknown event", delim, event2);
		tellUser(0, "Unknown event: %lu", event2);
	}
	printf("\n");
	printActionBlock(f, length, AB_MCEVENT, event, key);
	print("end\n");
	--indent;
}

static void parsePlaceObject(FILE *f, unsigned long int length, unsigned int type)
{
	int i;
	unsigned int flags;
	unsigned int clipID = 0;
	unsigned int depth;
	unsigned long int curEvent;
	
	if (type == TAG_PLACEOBJECT2)
		flags = fgetc(f);
	else
		flags = getWord(f);

	if (flags & PF_ONCLIPEVENTS) {
		printf("\n");

		/* character depth */
		depth = getWord(f);

		/* clipID should always be present */
		if (flags & PF_CHARACTER) {
			print("placeMovieClip %u ", clipID = getWord(f));
		}
		else {
			print("placeMovieClip ??? ");
			tellUser(0, "placeMovieClip: clip ID not found");
		}

		if (flags & PF_MATRIX)
			skipMatrix(f);
		if (flags & PF_COLORTRANSFORM)
			skipColorTransform(f);
		if (flags & PF_RATIO)
			getWord(f);

		if (flags & PF_NAME) {
			printf("as ");
			putchar('\'');
			while ((i = fgetc(f)) != 0)
				putchar((char) i);
			putchar('\'');
		}

		if (flags & PF_DEFINECLIP)
			getWord(f);

		if (type == TAG_PLACEOBJECT3) {
			if (flags & PF_FILTERS)
				skipFilters(f, (byte) fgetc(f));
			if (flags & PF_BLENDMODE)
				fgetc(f);
			if (flags & PF_BITMAPCACHING)
				fgetc(f);
		}

		printf("\n");


		/* reserved: always 0 */
		getWord(f);

		if (swfVersion >= 6 || type == TAG_PLACEOBJECT3) {
			/* flash 6 supports button events for mcs, therefore going long here */
			getDoubleWord(f);

			while ((curEvent = getDoubleWord(f)) != 0)
				parseEvent(f, curEvent, getDoubleWord(f));
		}
		else {
			/* all events */
			getWord(f);
			while ((curEvent = (unsigned long int) getWord(f)) != 0)
				parseEvent(f, curEvent, getDoubleWord(f));
		}

		if (flags & PF_CHARACTER)
			print("end // of placeMovieClip %u\n", clipID);
		else
			print("end // of placeMovieClip ???\n");
		
	}
	else {
		/* no events found, skip the rest of placeObject2/3 */
		if (type == TAG_PLACEOBJECT3)
			skipProtected(f, length - 2);
		else
			skipProtected(f, length - 1);
	}
}

static void parseMovieClip(FILE *f)
{
	unsigned int clipID, frameNum = 0, framesTotal = 0;

	clipID = getWord(f);
	framesTotal = getWord(f);

	printf("\n");
	print("defineMovieClip %u // total frames: %u\n", clipID, framesTotal);
	indent++;

	while (!feof(f)) {
		unsigned int type;
		unsigned long int length;
		
		parseTagHeader(f, &type, &length);

		if (type == 0)
			break;

		switch (type) {
			case TAG_DOACTION:
				printFrameNum(frameNum);
				printActionBlock(f, length, AB_FRAME, frameNum, 0);
				print("end // of frame %u\n", frameNum);
				break;

			case TAG_PLACEOBJECT2:
				/* possibly onClipEvents inside */
				parsePlaceObject(f, length, TAG_PLACEOBJECT2);
				break;

			case TAG_PLACEOBJECT3:
				/* possibly onClipEvents inside */
				parsePlaceObject(f, length, TAG_PLACEOBJECT3);
				break;

			case TAG_SHOWFRAME:
				++frameNum;
				break;

			default:
				if (getTagString(type) == NULL) {
					print("\n");
					print("// unknown tag %lu length %li\n\n", type, length);
				}
				skipProtected(f, length);
		}
	}

	--indent;
	print("end // of defineMovieClip %u\n", clipID);
}

void disassembleSWF(FILE *f, char *fname)
{
	unsigned int componentID, frameNum = 0, framesTotal = 0, bits;
	unsigned long int i, size;
	float frameRate, movieWidth, movieHeight;

	swfVersion = fgetc(f);
	size = getDoubleWord(f);

	/* movie bounds */
	InitBits(f);
	bits = getBits(f, 5);
	/* xMin - always 0 */
	getBits(f, bits);
	/* xMax */
	movieWidth = ((float) getBits(f, bits)) / 20;
	/* yMin - always 0 */
	getBits(f, bits);
	/* yMax */
	movieHeight = ((float) getBits(f, bits)) / 20;

	frameRate = ((float) fgetc(f)) / 256;
	frameRate += (float) fgetc(f);

	framesTotal = getWord(f);

	printf("movie '%s'", fname);

	if (wasCompressed)
		printf(" compressed");
	printf(" // flash %i, total frames: %u, frame rate: ", swfVersion, framesTotal);
	printFloat(frameRate, 1);
	printf(" fps, ");
	printFloat(movieWidth, 1);
	putchar('x');
	printFloat(movieHeight, 1);
	printf(" px\n");

	while (!feof(f)) {
		unsigned int type;
		unsigned long int length;

		parseTagHeader(f, &type, &length);

		if (type == 0)
			break;

		switch (type) {
			case TAG_DOACTION:
				printFrameNum(frameNum);
				printActionBlock(f, length, AB_FRAME, frameNum, 0);
				print("end // of frame %u\n", frameNum);
				break;

			case TAG_INITMOVIECLIP:
				print("\n");
				componentID = getWord(f);
				print("initMovieClip %i\n", componentID);
				printActionBlock(f, length - 2, AB_INITMC, componentID, 0);
				print("end // of initMovieClip %i\n", componentID);
				break;

			case TAG_PLACEOBJECT2:
				/* possibly onClipEvents inside */
				parsePlaceObject(f, length, TAG_PLACEOBJECT2);
				break;

			case TAG_PLACEOBJECT3:
				/* possibly onClipEvents inside */
				parsePlaceObject(f, length, TAG_PLACEOBJECT3);
				break;

			case TAG_DEFINEBUTTON2:
				/* possibly button events inside */
				parseButton2(f, length);
				break;

			case TAG_SHOWFRAME:
				++frameNum;
				break;

			case TAG_SCRIPTLIMITS: {
				unsigned int recursion = getWord(f);
				unsigned int timeout = getWord(f);
				print("\n");
				print("scriptLimits recursion %u timeout %u\n", recursion, timeout);
				break;
			}

			case TAG_PROTECT:
				print("\n");
				print("protect");
				if (length > 0) {
					/* password found */
					printf(" '");
					/* always 0 */
					getWord(f);
					for (i = 2; i < length - 1; ++i) {
						putchar((char) fgetc(f));
					}
					/* always 0 - string end */
					fgetc(f);
					putchar('\'');
				}
				putchar('\n');
				break;

			case TAG_ENABLEDEBUGGER:
				print("\n");
				print("enableDebugger");
				if (length > 0) {
					/* password found */
					/* debugger always uses password, even for empty one */
					printf(" '");
					/* always 0 */
					getWord(f);
					for (i = 2; i < length - 1; ++i) {
						putchar((char) fgetc(f));
					}
					/* always 0 - string end */
					fgetc(f);
					putchar('\'');
				}
				putchar('\n');
				break;

			case TAG_ENABLEDEBUGGER2:
				/* flash MX debugger */
				print("\n");
				print("enableDebugger2");
				if (length > 0) {
					/* password found */
					/* debugger always uses password, even for empty one */
					printf(" '");
					/* reserved, always 0 */
					getWord(f);
					for (i = 2; i < length - 1; ++i) {
						putchar((char) fgetc(f));
					}
					/* always 0 - string end */
					fgetc(f);
					putchar('\'');
				}
				putchar('\n');
				break;

			case TAG_DEFINEMOVIECLIP:
				parseMovieClip(f);
				break;

			case TAG_EXPORTASSETS: {
				unsigned int assetID, numAssets = getWord(f);
				int n;
				print("\n");
				print("exportAssets\n");
				++indent;
				while (numAssets--) {
					print("%u as '", assetID = getWord(f));
					while ((n = fgetc(f)) != 0) {
						putchar((char) n);
					}
					printf("'\n");
				}
				--indent;
				print("end // of exportAssets\n");
				break;
			}

			case TAG_IMPORTASSETS:
			case TAG_IMPORTASSETS2: {
				unsigned int assetID, numAssets, attr;
				int n;
				print("\n");
				print("importAssets from '");
				while ((n = fgetc(f)) != 0) {
					putchar((char) n);
				}
				printf("'\n");
				if (type == TAG_IMPORTASSETS2) {
					/* Reserved: always 1 */
					attr = getWord(f);
					if (attr != 1)
					   tellUser(0, "Unknown importAssets2 attribute: %u (should be 1)", attr);
				}
				numAssets = getWord(f);
				++indent;
				while (numAssets--) {
					assetID = getWord(f);
					print("'");
					while ((n = fgetc(f)) != 0) {
						putchar((char) n);
					}
					printf("' as %u\n", assetID);
				}
				--indent;
				print("end // of importAssets\n");
				break;
			}

			case TAG_METADATA: {
				static char buf[MAX_BUFFER];
				print("\n");
				print("metadata ");
				fread(buf, 1, length, f);
				printstr(buf);
				putchar('\n');
				break;
			}

			case TAG_FILEATTRIBUTES: {
				char delim = ' ';
				unsigned long int attrs = getDoubleWord(f);
				if (attrs) {
					print("\n");
					print("fileAttributes");
				}
				printBitMasked(&attrs, ATTR_USENETWORK,					&delim, "attrUseNetwork");
				printBitMasked(&attrs, ATTR_RELATIVEURLS,				&delim, "attrRelativeURLs");
				printBitMasked(&attrs, ATTR_SUPPRESSCROSSDOMAINCACHE,	&delim, "attrSuppressCrossDomainCache");
				printBitMasked(&attrs, ATTR_ACTIONSCRIPT3,				&delim, "attrActionScript3");
				printBitMasked(&attrs, ATTR_HASMETADATA,				&delim, "attrHasMetadata");
				if (attrs != 0) {
					printf("%c%lu // unknown attribute", delim, attrs);
					tellUser(0, "Unknown file attribute: %lu", attrs);
				}
				printf("\n");
				break;
			}

			default:
				if (getTagString(type) == NULL) {
					print("\n");
					print("// unknown tag %lu length %li\n", type, length);
				}
				skipProtected(f, length);
		}
	}

	fclose(f);
	printf("end\n");
}
