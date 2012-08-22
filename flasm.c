/*
flasm, command line assembler & disassembler of flash actionscript bytecode
Copyright (c) 2001 Opaque Industries, (c) 2002-2007 Igor Kogan, (c) 2005 Wang Zhen
All rights reserved. See LICENSE.TXT for terms of use.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>

#ifdef MEMWATCH
#include "memwatch.h"
#endif

#ifdef __MINGW32__
#include <conio.h>
#include <windows.h>
#endif
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#include "zlib.h"
#include "util.h"
#include "flasm.h"

extern FILE *yyin;
extern void skipProtected(FILE * f, unsigned long int length);
extern void disassembleSWF(FILE * in, char *fname);

int swfVersion;

char *inputName;				/* actual command-line parameter */
static char *updateName = NULL;	/* swf file name found in disassembly's first line (movie foo.swf...) */
static char *tempName = NULL;	/* temporary file during assembling */
static char *flmName = NULL;	/* temporary disassembly during update */
static char *backupName = NULL;	/* foo.$wf if foo.swf is updated */
static FILE *updateFile = NULL;
static FILE *tempFile = NULL;
static FILE *inputFile = NULL;
static FILE *logFile = NULL;

static char swfHeader[4];
static long int flength = 0;
static char backupCreated = 0;

char wasCompressed = 0;
char compressAfter = 0;
int mode;

/* ini entries */
int showoffset = 0, hexoffset = 0, boutput = 0, literalconstants = 0, literalregisters = 1, clearregisterargs = 1, logmode = 0;
static char *logto = NULL, *flaplayer = NULL, *flabrowser = NULL, *flatest = NULL;
static char flapath[MAX_PATH_LEN];

void yyerror(char *s);
void warning(char *s);
int yyparse(void);

static byte *output = NULL;

static void decompressSWF(FILE * f, char *fname);
static void compressSWF(FILE * f, char *fname);

static long int defineMCLengthPos = 0;
static long int buttonLengthPos = 0;
static long int buttonLastOffsetPos = 0;
static long int placeMCLengthPos = 0;
static long int MCAllEventsPos = 0;
static long int len;

static unsigned int nLabels = 0;
struct _label {
	char *name;
	/* offset >=0 if actual label definition, -1 if predefinition from branch */
	long int offset;
};
static struct _label *labels = NULL;

static void waitUserInput(void)
{
#ifdef __MINGW32__
	/* make sure we have kbhit(), otherwise do nothing */
	fprintf(stderr, "Hit any key to continue..");
	while (!kbhit()) {

	}
#endif
}

unsigned long int nStrings = 0;
char **aStrings = NULL;

static void mexit(int msg)
{
	unsigned long int i;
	for (i = 0; i < nStrings; ++i)
		free(aStrings[i]);

	if (output != NULL)
		free(output);

	if (aStrings != NULL)
		free(aStrings);

	if (nLabels > 0 && labels != NULL)
		free(labels);

	exit(msg);
}

void tellUser(int isError, char *s, ...)
{
	va_list ap;
	static int usestderr = 1;
	static int firsttime = 1;

	if (logto != NULL && firsttime == 1) {
		firsttime = 0;
		if (logmode == 0)
			logFile = fopen(logto, "ab");
		else
			logFile = fopen(logto, "wb");

		if (logFile == NULL)
			fprintf(stderr, "Couldn't write to log file %s\nUsing stderr instead\n", logto);
		else {
			usestderr = 0;
			if (logmode == 0) {
				time_t now;
				time(&now);
				fprintf(logFile, "________________________\n%.24s\n¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\n", ctime(&now));
			}
		}
	}

	if (s != NULL && strlen(s) > 0) {
		va_start(ap, s);
		if (usestderr) {
			vfprintf(stderr, s, ap);
			fputc('\n', stderr);
		}
		else {
			vfprintf(logFile, s, ap);
			fputc('\n', logFile);
		}
		va_end(ap);
	}

	if (isError) {
		if (mode >= MODE_IDE && usestderr)
			waitUserInput();

		if (tempFile != NULL) {
			fclose(tempFile);
			remove(tempName);
		}

		if (updateFile != NULL)
			fclose(updateFile);

		if (wasCompressed && backupCreated && updateName != NULL && access(backupName, R_OK) == 0) {
			remove(updateName);
			rename(backupName, updateName);
		}

		if (logFile != NULL)
			fclose(logFile);
	
		mexit(EXIT_FAILURE);
	}
}

char *mstrdup(const char *String)
{
	SUREALLOC(nStrings, aStrings, sizeof (char *));

	if (String == NULL) {
		if ((aStrings[nStrings] = malloc(256)) == NULL)
			tellUser(1, "Not enough memory to allocate a string");
	}
	else {
		size_t slen = strlen(String);

		if ((aStrings[nStrings] = malloc(slen + 1)) != NULL)
			memcpy(aStrings[nStrings], String, slen + 1);
		else
			tellUser(1, "Not enough memory to allocate a string");
	}

	return aStrings[nStrings++];
}

static unsigned int newLabel(char *name, long int offset)
{
	unsigned int i = nLabels;

	/* first search if label exists, and update offset if needed */
	if (name != NULL && nLabels>0) {
		do {
			i--;
			if (labels[i].name != NULL && strcmp(name, labels[i].name) == 0) {
				if (offset >= 0) {
					if (labels[i].offset == -1)
						labels[i].offset = offset;
					else
						yyerror("Label defined twice");
				}
				return i;
			}
		} while (i>0);
	}

	SUREALLOC(nLabels, labels, sizeof (struct _label));
	labels[nLabels].name = name;
	labels[nLabels].offset = offset;
	return (nLabels++);
}

void addLabel(char *label)
{
	newLabel(label, len);
}

int branchTarget(char *label)
{
	return writeShort(newLabel(label, -1));
}

int addNumLabel(int numLabel)
{
	/* numerical offset given in a branch */

	if (len + numLabel + 2 < 0)
		yyerror("Branch target out of range");

	return writeShort(newLabel(NULL, len + numLabel + 2));
}

int writeByte(byte num)
{
	if (len % OUTPUT_INC == 0)
		output = realloc(output, len + OUTPUT_INC);

	output[len++] = num;
	return 1;
}

int writeShort(unsigned int num)
{
	writeByte(num & 0xff);
	writeByte((num >> 8) & 0xff);
	return 2;
}

int writeFloat(float num)
{
	byte *p = (byte *) &num;

	if (byteorder == FLASM_BIG_ENDIAN) {
		writeByte(p[3]);
		writeByte(p[2]);
		writeByte(p[1]);
		writeByte(p[0]);
	}
	else {
		writeByte(p[0]);
		writeByte(p[1]);
		writeByte(p[2]);
		writeByte(p[3]);
	}

	return 4;
}

int writeDouble(double num)
{
	byte *p = (byte *) &num;

	if (byteorder == FLASM_BIG_ENDIAN) {
		writeByte(p[3]);
		writeByte(p[2]);
		writeByte(p[1]);
		writeByte(p[0]);
		writeByte(p[7]);
		writeByte(p[6]);
		writeByte(p[5]);
		writeByte(p[4]);
	}
	else {
		writeByte(p[4]);
		writeByte(p[5]);
		writeByte(p[6]);
		writeByte(p[7]);
		writeByte(p[0]);
		writeByte(p[1]);
		writeByte(p[2]);
		writeByte(p[3]);
	}

	return 8;
}

int writeLongInt(long int num)
{
	byte *p = (byte *) &num;

	if (byteorder == FLASM_BIG_ENDIAN) {
		writeByte(p[3]);
		writeByte(p[2]);
		writeByte(p[1]);
		writeByte(p[0]);
	}
	else {
		writeByte(p[0]);
		writeByte(p[1]);
		writeByte(p[2]);
		writeByte(p[3]);
	}

	return 4;
}

int writeString(char *str)
{
	int i;

	for (i = 0; str[i] != '\0'; ++i)
		writeByte((byte) str[i]);

	writeByte(0);

	return i + 1;
}

unsigned int nConstants = 0;
static char *constants[MAX_CONSTANTS];

void addConstant(char *str)
{
	constants[nConstants] = str;
	++nConstants;

	if (nConstants > MAX_CONSTANTS)
		yyerror("Too many constants");
}

unsigned int writeConstants(void)
{
	unsigned int i;
	unsigned long int clen = 2;

	writeByte(SWFACTION_CONSTANTPOOL);
	/* length */
	writeShort(0);
	writeShort(nConstants);

	for (i = 0; i < nConstants; ++i)
		clen += writeString(constants[i]);

	if (clen < 65533)
		patchLength((unsigned int) clen, (unsigned int) clen);
	else
		tellUser(1, "Constant pool exceeds 64k");

	return (unsigned int) clen + 3;
}

unsigned int writePushString(char *str)
{
	unsigned int i;

	for (i = 0; i < nConstants; ++i) {
		if (strcmp(constants[i], str) == 0)
			break;
	}

	if (i < nConstants) {
		if (i < 256) {
			/* constant, 1-byte reference */
			writeByte(0x08);
			writeByte((byte) i);
			return 2;
		}
		else {
			/* constant, 2-byte reference */
			writeByte(0x09);
			writeShort(i);
			return 3;
		}
	}

	/* string */
	writeByte(0);
	return writeString(str) + 1;
}

static void patchTargets(void)
{
	long int i = 0;

	while (i < len) {
		Action op = (Action) output[i++];
		if (op & 0x80) {
			unsigned int blocklen = S16(output + i);
			i += 2;

			if (op == SWFACTION_BRANCHALWAYS || op == SWFACTION_BRANCHIFTRUE) {
				long int offset;
				unsigned int target = S16(output+i);

				if (labels[target].offset < 0) {
					char msg[256] = "Label not found: ";
					yyerror(strcat(msg, labels[target].name));
				}

				offset = labels[target].offset - (i + 2);
				if (offset > 32767 || offset < -32768) {
					char msg[256] = "Label too far away from branch: ";
					yyerror(strcat(msg, labels[target].name));
				}

				output[i] = (byte) (offset & 0xff);
				output[i+1] = (byte) ((offset >> 8) & 0xff);
			}
			i += blocklen;	
		}
	}
}

static int flput(char b)
{
	++flength;
	return fputc(b, tempFile);
}

static void flputShort(unsigned int w)
{
	fputc(0xff & w, tempFile);
	fputc(0xff & (w >> 8), tempFile);
	flength += 2;
}

static void flputLong(unsigned long int l)
{
	fputc(0xff & l, tempFile);
	fputc(0xff & (l >> 8), tempFile);
	fputc(0xff & (l >> 16), tempFile);
	fputc(0xff & (l >> 24), tempFile);
	flength += 4;
}

static void flputString(char *s)
{
	fwrite(s, strlen(s)+1, 1, tempFile);
	flength += strlen(s)+1;
}

static int dupByte(void)
{
	return flput(fgetc(updateFile));
}

static unsigned int dupWord(void)
{
	int b1, b2;
	b1 = fgetc(updateFile);
	flput(b1);
	b2 = fgetc(updateFile);
	flput(b2);
	return ((unsigned int) b1 + ((unsigned int) b2 << 8));
}

static unsigned long int dupLong(void)
{
	return ((unsigned long int) dupWord() + ((unsigned long int) dupWord() << 16));
}

static void dupBuffered(unsigned long int length)
{
	static byte dupBuffer[MAX_BUFFER];

	flength += length;

	while (length > MAX_BUFFER) {
		if (fread(dupBuffer, 1, MAX_BUFFER, updateFile) == MAX_BUFFER) {
			fwrite(dupBuffer, 1, MAX_BUFFER, tempFile);
			length -= MAX_BUFFER;
		}
		else
			yyerror("Couldn't update because of unexpected SWF structure");
	}

	if (length > 0) {
		if (fread(dupBuffer, 1, length, updateFile) == length)
			fwrite(dupBuffer, 1, length, tempFile);
		else
			yyerror("Couldn't update because of unexpected SWF structure");
	}
}

void patchLength(unsigned int back, unsigned int blen)
{
	output[len - back - 1] = (blen >> 8) & 0xff;
	output[len - back - 2] = blen & 0xff;
}

void patchFlag(unsigned int back, byte flag)
{
	output[len - back - 1] = flag;
}

void patchFrameLoaded(unsigned int back, int numActions)
{
	if (numActions > 0 || numActions < 256)
		output[len - back - 1] = (byte) numActions;
	else
		yyerror("IfFrameLoaded block larger then 255 bytes is not allowed");
}

static void updateTmpLength(long int lenpos, long int reallen)
{
	long int curpos;
	curpos = ftell(tempFile);

	fseek(tempFile, lenpos, SEEK_SET);

	/* update long length */
	/* dont use flput to not affect the file size calculation */
	fputc((reallen & 0xff), tempFile);
	fputc((reallen >> 8) & 0xff, tempFile);
	fputc((reallen >> 16) & 0xff, tempFile);
	fputc((reallen >> 24) & 0xff, tempFile);

	fseek(tempFile, curpos, SEEK_SET);
}

static void updateShort(long int lenpos, unsigned int reallen)
{
	long int curpos = ftell(tempFile);

	fseek(tempFile, lenpos, SEEK_SET);

	/* update short length */
	/* dont use flput to not affect the file size calculation */
	fputc(reallen & 0xff, tempFile);
	fputc((reallen >> 8) & 0xff, tempFile);

	fseek(tempFile, curpos, SEEK_SET);
}

static void flushOutput(void)
{
	patchTargets();

	if (fwrite(output, 1, (size_t) len, tempFile) != (size_t) len)
		tellUser(1, "Error writing action block");

	flength += len;

	len = 0;

	if (nLabels > 0) {
		nLabels = 0;
		free(labels);
		labels = NULL;
	}

	nConstants = 0;
}

static void writeTagHeader(unsigned int type, unsigned long int length)
{
	if (length >= 63
		|| type == TAG_DEFINEBITSLOSSLESS
		|| type == TAG_DEFINEBITSLOSSLESS2
		|| type == TAG_SOUNDSTREAMBLOCK
		|| type == TAG_DEFINEBITS
		|| type == TAG_DEFINEBITSJPEG2
		|| type == TAG_DEFINEBITSJPEG3) {
		/*
		   long length, and also workaround for a really strange bug in flash player:
		   the above tags must always have long length to work
		 */
		flputShort((type << 6) + 63);
		flputLong(length);
	}
	else {
		/* short length */
		flputShort((type << 6) + length);
	}
}

static int handledTag(unsigned int tag)
{
	return (tag == TAG_PROTECT
			|| tag == TAG_ENABLEDEBUGGER
			|| tag == TAG_ENABLEDEBUGGER2
			|| tag == TAG_SCRIPTLIMITS
			|| tag == TAG_FILEATTRIBUTES
			|| tag == TAG_METADATA);
}

static unsigned long int findNextTag(unsigned int typeneeded)
{
	/*
	   dumps all tags of the updateFile to the tempFile
	   until required tag is found, returns its length 
	 */
	unsigned int type;
	unsigned long int length;

	while (!feof(updateFile)) {
		parseTagHeader(updateFile, &type, &length);
		if (type == typeneeded)
			return length;
		if (handledTag(type))
			skipProtected(updateFile, length);
		else {
			writeTagHeader(type, length);
			dupBuffered(length);
		}
	}
	tellUser(1, "Couldn't update because of unexpected SWF structure:\nunexpected end of SWF looking for Tag %04x", typeneeded);
	return (0);
}

static unsigned long int findNextTags(unsigned int *typefound, int numtypes, ...)
{
	/*
	   dumps all blocks of the updateFile to the tempFile
	   until one of the required tags is found, returns its length 
	 */
	va_list typelist;
	unsigned int type;
	unsigned long int length;
	int i;

	while (!feof(updateFile)) {
		parseTagHeader(updateFile, &type, &length);
		va_start(typelist, numtypes);
		for (i = 1; i <= numtypes; i++) {
			if (type == va_arg(typelist, unsigned int)) {
				*typefound = type;
				va_end(typelist);
				return length;
			}
		}
		va_end(typelist);

		if (handledTag(type))
			skipProtected(updateFile, length);
		else {
			writeTagHeader(type, length);
			dupBuffered(length);
		}
	}
	tellUser(1, "Couldn't update because of unexpected SWF structure:\nunexpected end of SWF.");
	return (0);
}

void writeDoAction()
{
	unsigned long int oldLength = findNextTag(TAG_DOACTION);

	/* skip old action  */
	skipProtected(updateFile, oldLength);

	writeTagHeader(TAG_DOACTION, len);

	/* write bison output from buffer to file and empty buffer */
	flushOutput();
}

void writeInitMC(unsigned int clipID)
{
	unsigned int oldID;
	unsigned long int oldLength = findNextTag(TAG_INITMOVIECLIP);

	writeTagHeader(TAG_INITMOVIECLIP, len + 2);

	oldID = dupWord();
	if (oldID != clipID)
		tellUser(1, "Couldn't update because of unexpected SWF structure:\nold initMovieClip ID %u doesn't match new ID %u", oldID, clipID);

	skipProtected(updateFile, oldLength - 2);

	flushOutput();
}

static unsigned int bitPos;
static unsigned int bitBuf;

static unsigned int dupBits(unsigned int n)
{
	unsigned long int v = 0;

	while (n > bitPos) {
		n -= bitPos;
		v |= bitBuf << n;
		bitBuf = (unsigned int) dupByte();
		bitPos = 8;
	}

	bitPos -= n;
	v |= bitBuf >> bitPos;
	bitBuf &= 0xff >> (8 - bitPos);
	/* never need more than 16 bits */
	return (unsigned int) v;
}

static void dupMatrix(void)
{
	bitPos = 8;
	bitBuf = (unsigned int) dupByte();

	if (dupBits(1))
		dupBits(dupBits(5) * 2);
	if (dupBits(1))
		dupBits(dupBits(5) * 2);
	dupBits(dupBits(5) * 2);
}

static void dupColorTransform(void)
{
	unsigned int needAdd, needMul, nBits;

	bitPos = 8;
	bitBuf = (unsigned int) dupByte();

	needAdd = dupBits(1);
	needMul = dupBits(1);
	nBits = dupBits(4);
	if (needMul)
		dupBits(nBits * 4);
	if (needAdd)
		dupBits(nBits * 4);
}

static void dupFilters(void)
{
	byte numfilters = dupByte();
	while(numfilters--)
		switch(dupByte()){
			case FILTER_DROPSHADOW:
				dupBuffered(23);
				break;
			case FILTER_BLUR:
				dupBuffered(9);
				break;
			case FILTER_GLOW:
				dupBuffered(15);
				break;
			case FILTER_BEVEL:
				dupBuffered(27);
				break;
			case FILTER_GRADIENTGLOW:
				dupBuffered(dupByte()*5 + 19);
				break;
			case FILTER_ADJUSTCOLOR:
				dupBuffered(80);
				break;
			case FILTER_GRADIENTBEVEL:
				dupBuffered(dupByte()*5 + 19);
				break;
			default:
				tellUser(1, "Unknown filter");
		}
}

void writePlaceMCStart(unsigned int clipID)
{
	unsigned int oldID = 0;
	unsigned int typefound;
	unsigned int flags;
	unsigned long int length, eventLength;

	/* skip all mcs without onClipEvent actions */
	do {
		length = findNextTags(&typefound, 2, TAG_PLACEOBJECT2, TAG_PLACEOBJECT3);
		if (typefound == TAG_PLACEOBJECT2) {
			flags = fgetc(updateFile);
			if (flags & PF_ONCLIPEVENTS)
				break;
			writeTagHeader(TAG_PLACEOBJECT2, length);
			flput(flags);
			/* copy the rest of the placeobject2 */
			dupBuffered(length - 1);
		}
		else {
			flags = getWord(updateFile);
			if (flags & PF_ONCLIPEVENTS)
				break;
			writeTagHeader(TAG_PLACEOBJECT3, length);
			flputShort(flags);
			/* copy the rest of the placeobject3 */
			dupBuffered(length - 2);
		}
	} while (1);

	/* placemc header, temporary long length, save length pos */
	writeTagHeader(typefound, 0xFF);
	placeMCLengthPos = ftell(tempFile) - 4;

	if (typefound == TAG_PLACEOBJECT3)
		flputShort(flags);
	else
		flput(flags);

	/* character depth */
	dupWord();

	if (flags & PF_CHARACTER)
		oldID = dupWord();
	else
		tellUser(1, "Couldn't update because of unexpected SWF structure:\nold placeMovieClip ID missing, new ID %u", clipID);
	if (oldID != clipID)
		tellUser(1, "Couldn't update because of unexpected SWF structure:\nold placeMovieClip ID %u doesn't match new ID %u", oldID, clipID);

	if (flags & PF_MATRIX)
		dupMatrix();
	if (flags & PF_COLORTRANSFORM)
		dupColorTransform();
	if (flags & PF_RATIO)
		dupWord();
	if (flags & PF_NAME)
		while (dupByte() != 0);
	if (flags & PF_DEFINECLIP)
		dupWord();
	if (flags & PF_FILTERS)
		dupFilters();
	if (flags & PF_BLENDMODE)
		dupByte();
	if (flags & PF_BITMAPCACHING)
		dupByte();

	/* reserved: always 0 */
	dupWord();

	/* save the pos for logical or of all events */
	MCAllEventsPos = ftell(tempFile);

	/* skip old events */
	if (swfVersion <= 5) {
		/* dup logical or of all events - temporary */
		dupWord();
		while (getWord(updateFile) > 0) {
			eventLength = getDoubleWord(updateFile);
			skipProtected(updateFile, eventLength);
		}
	}
	else if (swfVersion >= 6) {
		/* dup logical or of all events - temporary */
		dupLong();
		while (getDoubleWord(updateFile) > 0) {
			eventLength = getDoubleWord(updateFile);
			skipProtected(updateFile, eventLength);
		}
	}
}

void writePlaceMCEnd(unsigned long int flags)
{
	/* write empty event */
	flputShort(0);
	if (swfVersion >= 6)
		flputShort(0);

	updateTmpLength(placeMCLengthPos, ftell(tempFile) - placeMCLengthPos - 4);

	if (swfVersion <= 5) {
		updateShort(MCAllEventsPos, (unsigned int) flags);
	}
	else if (swfVersion >= 6) {
		updateShort(MCAllEventsPos, (unsigned int) (flags & 0xFFFF));
		updateShort(MCAllEventsPos + 2, (unsigned int) ((flags >> 16) & 0xFFFF));
	}

	placeMCLengthPos = 0;
	MCAllEventsPos = 0;
}

void writeOnClipEvent(unsigned long int flag)
{
	/* write event flag */
	flputShort(flag);
	if (swfVersion >= 6)
		flputShort((unsigned int) (flag >> 16));

	/* write action length */
	flputLong(len);

	flushOutput();
}

void writeButtonStart(unsigned int buttonID)
{
	int trackAsMenu;
	unsigned int oldID, actionOffset;
	unsigned long int length;

	/* skip all buttons without actions */
	do {
		length = findNextTag(TAG_DEFINEBUTTON2);
		oldID = getWord(updateFile);
		trackAsMenu = fgetc(updateFile);
		actionOffset = getWord(updateFile);

		if (actionOffset > 0)
			break;

		writeTagHeader(TAG_DEFINEBUTTON2, length);
		flputShort(oldID);
		flput(trackAsMenu);
		/* put action offset = 0 */
		flputShort(0);
		/* copy the rest of the button */
		dupBuffered(length - 5);
	} while(1);

	if (oldID != buttonID)
		tellUser(1, "Couldn't update because of unexpected SWF structure:\nold defineButton ID %u doesn't match new ID %u", oldID, buttonID);

	/* defineButton2 header, temporary long length, save length pos */
	writeTagHeader(TAG_DEFINEBUTTON2, 0xFF);
	buttonLengthPos = ftell(tempFile) - 4;

	flputShort(oldID);
	flput(trackAsMenu);
	flputShort(actionOffset);

	/* copy button visual part */
	dupBuffered(actionOffset - 2);

	/* skip old button actions part */
	skipProtected(updateFile, length - 3 - actionOffset);
}

void writeButtonEnd(void)
{
	updateTmpLength(buttonLengthPos, ftell(tempFile) - buttonLengthPos - 4);

	/* last offset is always 0 */
	updateShort(buttonLastOffsetPos, 0);

	buttonLengthPos = 0;
	buttonLastOffsetPos = 0;
}

void writeButtonEvent(unsigned int flags)
{
	/* write offset to the next condition */
	buttonLastOffsetPos = ftell(tempFile);
	flputShort(len+4);

	/* write button event flags */
	flputShort(flags);
	flushOutput();
}

void writeDefineMCStart(unsigned int clipID)
{
	unsigned int oldID;

	findNextTag(TAG_DEFINEMOVIECLIP);

	/* defineMovieClip header, temporary long length, save length pos */
	writeTagHeader(TAG_DEFINEMOVIECLIP, 0xFF);
	defineMCLengthPos = ftell(tempFile) - 4;

	oldID = dupWord();
	if (oldID != clipID)
		tellUser(1, "Couldn't update because of unexpected SWF structure:\nold defineMovieClip ID %u doesn't match new ID %u", oldID, clipID);
	
	/* frames total */
	dupWord();
}

void writeDefineMCEnd(void)
{
	/* dump the rest of movie clip */
	findNextTag(TAG_END);

	/* write movie clip end tag */
	flputShort(0);

	updateTmpLength(defineMCLengthPos, ftell(tempFile) - defineMCLengthPos - 4);

	defineMCLengthPos = 0;
}

void writeScriptLimits(unsigned int recursion, unsigned int timeout)
{
	writeTagHeader(TAG_SCRIPTLIMITS, 4);
	flputShort(recursion);
	flputShort(timeout);
}

void writeProtect(char *str)
{
	if (strlen(str) == 0) {
		/* no password */
		writeTagHeader(TAG_PROTECT, 0);
	}
	else {
		/* password found */
		writeTagHeader(TAG_PROTECT, strlen(str) + 3);
		flputShort(0);
		flputString(str);
	}
}

void writeEnableDebugger(char *str)
{
	if (strlen(str) == 0) {
		/* no password, probably impossible? */
		writeTagHeader(TAG_ENABLEDEBUGGER, 0);
	}
	else {
		/* password found */
		writeTagHeader(TAG_ENABLEDEBUGGER, strlen(str) + 3);
		flputShort(0);
		flputString(str);
	}
}

void writeEnableDebugger2(char *str)
{
	if (strlen(str) == 0) {
		/* no password, probably impossible? */
		writeTagHeader(TAG_ENABLEDEBUGGER2, 0);
	}
	else {
		/* password found */
		writeTagHeader(TAG_ENABLEDEBUGGER2, strlen(str) + 3);
		flputShort(0);
		flputString(str);
	}
}

void writeMetadata(char *str)
{
	writeTagHeader(TAG_METADATA, strlen(str) + 1);
	flputString(str);
}

void writeFileAttrs(unsigned long int attrs)
{
	writeTagHeader(TAG_FILEATTRIBUTES, 4);
	flputLong(attrs);
}

void writeImportAssets(char *from, unsigned int numAssets)
{
	if (numAssets>0) {
		unsigned int typefound;
		skipProtected(updateFile, findNextTags(&typefound, 2, TAG_IMPORTASSETS, TAG_IMPORTASSETS2));
		if (typefound == TAG_IMPORTASSETS) {
			writeTagHeader(TAG_IMPORTASSETS, len + strlen(from) + 3);
			flputString(from);
			flputShort(numAssets);
		}
		else {
			writeTagHeader(TAG_IMPORTASSETS2, len + strlen(from) + 5);
			flputString(from);
			/* Reserved: always 1 */
			flputShort(1);
			flputShort(numAssets);
		}
		if (fwrite(output, 1, (size_t) len, tempFile) != (size_t) len)
			tellUser(1, "Error writing importAssets");
		flength += len;
	}
	len = 0;
}

void writeExportAssets(unsigned int numAssets)
{
	if (numAssets>0) {
		skipProtected(updateFile, findNextTag(TAG_EXPORTASSETS));
		writeTagHeader(TAG_EXPORTASSETS, len+2);
		flputShort(numAssets);

		if (fwrite(output, 1, (size_t) len, tempFile) != (size_t) len)
			tellUser(1, "Error writing exportAssets");

		flength += len;
	}
	len = 0;
}

static void finalizeTemporaryFile(char *name)
{
	fclose(tempFile);
	if (!backupCreated) {
		backupName = mstrdup(name);
		backupCreated = 1;
		strcpy(backupName + strlen(backupName) - 4, ".$wf");
		/* remove evtl. existing previous backup foo.$wf */
		remove(backupName);
		/* foo.swf -> foo.$wf */
		if (rename(name, backupName) != 0)
			tellUser(1, "couldn't update: file %s is in use", name);
		/* foo.tmp (assemble) or flasm.tmp (compress/decompress) -> foo.swf */
		rename(tempName, name);
	}
	else {
		/* backup already here, just make temp file our final file */
		if (remove(name) != 0)
			tellUser(1, "couldn't update: file %s is in use", name);
		rename(tempName, name);
	}
}

static void getSWFHeader(FILE * f)
{
	fread(swfHeader, 1, 3, f);
	swfHeader[3] = '\0';
}

void startUpdate(char *outputName)
{
	int b, i, bitstotal;

	if ((updateFile = fopen(outputName, "rb")) == NULL)
		tellUser(1, "Couldn't open file %s for update", outputName);

	getSWFHeader(updateFile);

	if (strcmp(swfHeader, "FWS") != 0) {
		if (strcmp(swfHeader, "CWS") == 0) {
			/* decompresses foo.swf, foo.$wf backup is created */
			decompressSWF(updateFile, outputName);
			updateFile = fopen(outputName, "rb");
			/* now we are just after the header in decompressed file */
			getSWFHeader(updateFile);
		}
		else
			tellUser(1, "Input file %s doesn't appear to be an SWF file", outputName);
	}

	flength = 0;

	updateName = mstrdup(outputName);
	tempName = mstrdup(outputName);
	strcpy(tempName + strlen(tempName) - 4, ".tmp");

	if ((tempFile = fopen(tempName, "wb")) == NULL)
		tellUser(1, "Couldn't create temporary file");

	/* SWF header */
	flput('F');
	flput('W');
	flput('S');

	/* version */
	flput(swfVersion = fgetc(updateFile));

	/* file length - temporary! */
	flput(fgetc(updateFile));
	flput(fgetc(updateFile));
	flput(fgetc(updateFile));
	flput(fgetc(updateFile));

	/* movie bounds */
	b = fgetc(updateFile);
	bitstotal = 5 + 4 * ((b & 0xf8) >> 3);
	flput(b);
	for (i = 0; i < (bitstotal + 7) / 8 - 1; ++i)
		flput(fgetc(updateFile));

	/* frame rate and # of frames */
	flput(fgetc(updateFile));
	flput(fgetc(updateFile));
	flput(fgetc(updateFile));
	flput(fgetc(updateFile));
}

void finishUpdate(void)
{
	/* dump the rest of SWF */
	findNextTag(0);
	/* write movie end */
	flputShort(0);

	updateTmpLength(4, flength);

	fclose(updateFile);

	finalizeTemporaryFile(updateName);

	if (compressAfter) {
		updateFile = fopen(updateName, "rb");
		getSWFHeader(updateFile);
		compressSWF(updateFile, updateName);
	}

	if (mode == MODE_UPDATE)
		tellUser(0, "%s successfully updated, %li bytes", updateName, flength);
	else if (mode == MODE_ASSEMBLE)
		tellUser(0, "%s successfully assembled to %s, %li bytes", inputName, updateName, flength);
}

void writeASBytecode(void)
{
	long int n = 0;
	patchTargets();

	if (nLabels > 0) {
		nLabels = 0;
		free(labels);
		labels = NULL;
	}
	
	if (boutput == 0) {
		printf("%s", "__bytecode__(\"");
		while(n++ < len)
			printf("%02x", *(output+n-1));
		printf("%s", "\");\n");
	}
	else
		fwrite(output, 1, len, stdout);
}

static void createTemporaryFile(void)
{
	tempName = mstrdup("flasm.tmp");
	if ((tempFile = fopen(tempName, "wb+")) == NULL)
		tellUser(1, "Couldn't create file: %s", tempName);
}

static void decompressSWF(FILE *f, char *fname)
{
	z_stream stream;
	static byte inputBuffer[MAX_BUFFER], outputBuffer[MAX_BUFFER];
	int status, count;

	flength = 0;
	createTemporaryFile();

	/* SWF header */
	flput('F');
	flput('W');
	flput('S');

	/* version */
	flput(fgetc(f));

	flput(fgetc(f));
	flput(fgetc(f));
	flput(fgetc(f));
	flput(fgetc(f));

	stream.avail_in = 0;
	stream.next_in = inputBuffer;
	stream.next_out = outputBuffer;
	stream.zalloc = (alloc_func) NULL;
	stream.zfree = (free_func) NULL;
	stream.opaque = (voidpf) 0;
	stream.avail_out = MAX_BUFFER;

	status = inflateInit(&stream);
	if (status != Z_OK)
		tellUser(0, "Error %i decompressing SWF: %s\n", status, stream.msg);

	do {
		if (stream.avail_in == 0) {
			stream.next_in = inputBuffer;
			stream.avail_in = fread(inputBuffer, 1, MAX_BUFFER, f);
		}

		if (stream.avail_in == 0)
			break;

		status = inflate(&stream, Z_SYNC_FLUSH);

		count = MAX_BUFFER - stream.avail_out;
		if (count) {
			fwrite(outputBuffer, 1, count, tempFile);
			flength += count;
		}

		stream.next_out = outputBuffer;
		stream.avail_out = MAX_BUFFER;
	}
	while (status == Z_OK);

	if (status != Z_STREAM_END && status != Z_OK)
		tellUser(0, "Error %i decompressing SWF: %s\n", status, stream.msg);

	status = inflateEnd(&stream);
	if (status != Z_OK)
		tellUser(0, "Error %i decompressing SWF: %s\n", status, stream.msg);

	fclose(f);

	if (mode != MODE_DISASSEMBLE)
		finalizeTemporaryFile(fname);

	if (mode == MODE_DECOMPRESS)
		tellUser(0, "%s successfully decompressed, %li bytes", fname, flength);

	wasCompressed = 1;
}

static void compressSWF(FILE *f, char *fname)
{
	z_stream stream;
	static byte inputBuffer[MAX_BUFFER], outputBuffer[MAX_BUFFER];
	int status, count;

	flength = 0;
	createTemporaryFile();

	/* SWF header  */
	flput('C');
	flput('W');
	flput('S');

	/* version */
	flput(fgetc(f));

	flput(fgetc(f));
	flput(fgetc(f));
	flput(fgetc(f));
	flput(fgetc(f));

	stream.avail_in = 0;
	stream.next_out = outputBuffer;
	stream.avail_out = MAX_BUFFER;
	stream.zalloc = (alloc_func) NULL;
	stream.zfree = (free_func) NULL;
	stream.opaque = (voidpf) 0;
	stream.next_in = inputBuffer;

	status = deflateInit(&stream, Z_BEST_COMPRESSION);

	if (status != Z_OK)
		tellUser(1, "Error %i compressing SWF: %s\n", status, stream.msg);

	while (1) {
		if (stream.avail_in == 0) {
			stream.next_in = inputBuffer;
			stream.avail_in = fread(inputBuffer, 1, MAX_BUFFER, f);
		}

		if (stream.avail_in == 0)
			break;

		status = deflate(&stream, Z_NO_FLUSH);
		if (status != Z_OK)
			tellUser(1, "Error %i compressing SWF: %s\n", status, stream.msg);

		count = MAX_BUFFER - stream.avail_out;
		if (count) {
			fwrite(outputBuffer, 1, count, tempFile);
			flength += count;
		}

		stream.next_out = outputBuffer;
		stream.avail_out = MAX_BUFFER;
	}

	stream.next_out = outputBuffer;
	stream.avail_out = MAX_BUFFER;

	do {
		status = deflate(&stream, Z_FINISH);

		count = MAX_BUFFER - stream.avail_out;
		if (count) {
			fwrite(outputBuffer, 1, count, tempFile);
			flength += count;
		}

		stream.next_out = outputBuffer;
		stream.avail_out = MAX_BUFFER;
	}
	while (status == Z_OK);

	if (status != Z_STREAM_END)
		tellUser(1, "Error %i compressing SWF: %s\n", status, stream.msg);

	status = deflateEnd(&stream);
	if (status != Z_OK)
		tellUser(1, "Error %i compressing SWF: %s\n", status, stream.msg);

	fclose(f);

	finalizeTemporaryFile(fname);

	if (mode == MODE_COMPRESS)
		tellUser(0, "%s successfully compressed, %li bytes", fname, flength);

}

extern int yydebug;

static void usage(void)
{
	printf("\nFlasm %s build %s", FLASM_VERSION, __DATE__);
	// printf(" (zlib %s)", ZLIB_VERSION);
	puts("");
	puts("");
	puts("(c) 2001 Opaque Industries, (c) 2002-2007 Igor Kogan, (c) 2005 Wang Zhen");
	puts("All rights reserved. See LICENSE.TXT for terms of use.");
	puts("");
	puts("Usage: flasm [command] filename");
	puts("");
	puts("Commands:");
	puts("   -d     Disassemble SWF file to the console");
	puts("   -a     Assemble Flasm project (FLM)");
	puts("   -u     Update SWF file, replace Flasm macros");
	puts("   -b     Assemble actions to __bytecode__ instruction or byte sequence");
	puts("   -z     Compress SWF with zLib");
	puts("   -x     Decompress SWF");
	puts("");
	puts("Backups with $wf extension are created for altered SWF files.");
	puts("");
	puts("To save disassembly or __bytecode__ to file, redirect it:");
	puts("flasm -d foo.swf > foo.flm");
	puts("flasm -b foo.txt > foo.as");
	puts("");
	puts("Read flasm.html for more information.");

	mexit(EXIT_FAILURE);
}

static void unescapePath(char *argument)
{
	int n = 0;
	char esc[3];
	char *s = argument;

	inputName = mstrdup(NULL);

	while ((inputName[n] = *s++) != 0) {
		if ((inputName[n] == '.') && (*s == 'h') && (*(s + 1) == 't') && (*(s + 2) == 'm'))
			/* extension found */
			break;

		if (inputName[n] == '%') {
			/* unescape URL encoded char */
			esc[0] = *s++;
			esc[1] = *s++;
			esc[2] = 0;

			inputName[n] = (char) (xtoi(esc));
		}
		n++;
	}

	inputName[n] = '\0';
	strcat(inputName, ".swf");
}

static int readINI(char *exepath, char *names, char *types, ...)
{
	/* slightly changed INITVARS from SNIPPETS collection, public domain by Raymond Gardner, Sept. 1991 */
	FILE *iniFile;
	char ln[256], *p, *namep, *typep, name[40], *e;
	va_list arglist;
	void *argp;
	int k;

	if ((iniFile = fopen("flasm.ini", "r")) == NULL) {
		/* flasm.ini not found in current directory, look at executable's path */
		int pathlen = strlen(exepath);
		char inipath[pathlen + 10], *inipathptr;
		if (exepath == NULL || *exepath == '\0')
			return -1;
		strcpy(inipath, exepath);
		inipathptr = inipath + pathlen;
		while (*inipathptr != '\\' && *inipathptr != '/' && *inipathptr != ':' && inipathptr>=inipath)
			inipathptr--;

		strcpy(inipathptr + 1, "flasm.ini");
		if ((iniFile = fopen(inipath, "r")) == NULL)
			return -1;
	}

	while (fgets(ln, 256, iniFile)) {													 /* read ini file */
		while (isspace(ln[0]))															 /* drop leading whitespace */
			memmove(ln, ln + 1, strlen(ln));
		if (ln[0] == '\0' || ln[0] == '#')												 /* skip if blank line or comment */
			continue;

		p = strchr(ln, '=');															 /* find equal sign */
		if (p == NULL)																	 /* error if none */
			tellUser(1, "Error parsing flasm.ini: %s", ln);

		while (p > ln && isspace(p[-1])) {												 /* remove whitespace before eq sign */
			memmove(p - 1, p, strlen(p - 1));
			--p;
		}
		*p++ = '\0';																	 /* plug EOS over eq sign */

		while (isspace(p[0]))															 /* remove leading space on init string */
			memmove(p, p + 1, strlen(p));

		k = strlen(p) - 1;																 /* init string length */
		if (k < 0)
			tellUser(1, "Error parsing flasm.ini: %s", ln);

		if (p[k] == '\n')
			p[k] = '\0';																 /* plug EOS over newline */
		else if (feof(iniFile))															 /* '\n' is missing - last line or buffer exceeded? */
			p[k + 1] = '\0';
		else
			tellUser(1, "Line too long in flasm.ini: %s", ln);

		va_start(arglist, types);														 /* setup for arglist search */

		namep = names;																	 /* init ptr to var names */
		typep = types;																	 /* init ptr to var types */

		while (*namep == ' ')															 /* skip blanks before namelist */
			++namep;

		while (*typep) {																 /* while any typelist items left... */
			argp = (void *) va_arg(arglist, void *);									 /* get var arg */

			k = strcspn(namep, " ");													 /* length of namelist entry */
			memmove(name, namep, k);													 /* put into name hold area */
			name[k] = '\0';																 /* terminate it */
			if (strIcmp(name, ln) != 0) {												 /* if it doesn't match... */
				namep += k;																 /* get next name */
				while (*namep == ' ')
					++namep;
				++typep;																 /* get next type */
			}
			else {																		 /* else name is found... */
				if (*typep == 'i') {													 /* if it's an int, init it */
					*(int *) argp = atoi(p);
				}
				else if (*typep == 's' || *typep == 'p') {
					if (*p == '"') {													 /* is string in quotes? */
						++p;															 /* skip leading quote, and */
						e = strchr(p, '"');												 /* look for trailing quote */
						if (e)															 /* terminate string if found */
							*e = '\0';
					}
					if (*typep == 'p')													 /* if it's a char *ptr */
						*(char **) argp = mstrdup(p);
					else																 /* must be char array */
						strcpy(argp, p);												 /* copy in string */
				}
				else
					tellUser(1, "Contact developer: bad argument type in readINI() call");

				break;																	 /* break search; get next line */
			}
		}
		va_end(arglist);
	}

	fclose(iniFile);
	return 0;
}

static char *executablePath(void) {
	char path[MAX_PATH_LEN];
#ifdef __MINGW32__
	if(GetModuleFileName(NULL, path, MAX_PATH_LEN - 1))
		return mstrdup(path);
#elif defined(__APPLE__)
	uint32_t pathlen = MAX_PATH_LEN - 1;
	if (_NSGetExecutablePath(path, &pathlen) == 0)
		return mstrdup(path);
#else
	int length = readlink("/proc/self/exe", path, sizeof(path));
	if (length > 0 && length < sizeof(path)) {
		 path[length] = '\0';
		 return mstrdup(path);
	}
#endif
	return NULL;
}

static void parseArgs(int argc, char *argv[])
{
	int ini;
	char *exepath;

	if (argv[1] == NULL)
		usage();

	exepath = executablePath();
	if (exepath == NULL)
		exepath = argv[0];

	ini = readINI(exepath,
			"logto flabrowser flaplayer flatest showoffset hexoffset boutput literalconstants literalregisters clearregisterargs logmode",
			"ppppiiiiiii",
			&logto, &flabrowser, &flaplayer, &flatest, &showoffset, &hexoffset, &boutput, &literalconstants, &literalregisters, &clearregisterargs, &logmode);

	if (ini != 0)
		tellUser(0, "Flasm configuration file flasm.ini not found, using default values");

	if ((strstr(argv[1], ".htm") != NULL) || (strstr(argv[1], "http://") != NULL)) {
		mode = MODE_IDE;

		if ((strIstr(argv[1], "ContextHelp") != NULL) || (strIstr(argv[1], "http://") != NULL)) {
			/* an attempt to access flash help */
			mode = MODE_FLASH_HELP;

			if (strlen(flabrowser) == 0)
				tellUser(1, "You must define 'flabrowser' value in flasm.ini to access flash help");

			if (access(flabrowser, X_OK) != 0)
				tellUser(1, "Couldn't start browser: %s", flabrowser);

			strcpy(flapath, flabrowser);
			strcat(flapath, " ");
			strcat(flapath, argv[1]);
			return;
		}

		if (strIstr(flatest, "FLAPLAYER") != NULL)
			strcpy(flapath, flaplayer);
		else if (strIstr(flatest, "FLABROWSER") != NULL)
			strcpy(flapath, flabrowser);
		else
			tellUser(1, "Invalid or missing 'flatest' value in flasm.ini");

		if (strlen(flapath) == 0)
			tellUser(1, "You must define '%s' value in flasm.ini for debugging", flatest);

		if (access(flapath, X_OK) != 0)
			tellUser(1, "Couldn't start: %s", flapath);

		if (strstr(argv[1], "file:///") != NULL)
			/* skip "file:///" (8 chars) */
			unescapePath(argv[1] + 8);
		else
			unescapePath(argv[1]);

		if ((inputFile = fopen(inputName, "rb")) == NULL)
			tellUser(1, "Couldn't open input file %s for reading", inputName);

		strcat(flapath, " ");
		strcat(flapath, argv[1]);

		if (strIstr(flatest, "FLAPLAYER") != NULL)
			strcpy(strstr(flapath, ".htm"), ".swf\0");

		return;
	}

	if (argv[1][0] == '-') {
		int i;
		if (argc < 3)
			usage();
		inputName = mstrdup(NULL);
		strcpy(inputName, argv[2]);

		/* join all arguments into one string - to support spaces in file names */
		for (i = 3; i < argc; i++) {
			strcat(inputName, " ");
			strcat(inputName, argv[i]);
		}

		switch (argv[1][1]) {
			case 'd':
				mode = MODE_DISASSEMBLE;
				break;

			case 'a':
				mode = MODE_ASSEMBLE;
				break;

			case 'u':
				mode = MODE_UPDATE;
				break;

			case 'b':
				mode = MODE_ASBYTECODE;
				break;

			case 'x':
				mode = MODE_DECOMPRESS;
				break;

			case 'z':
				mode = MODE_COMPRESS;
				break;

			default:
				usage();
		}
	}
	else {
		int i;
		inputName = mstrdup(NULL);
		strcpy(inputName, argv[1]);

		/* join all arguments into one string - to support spaces in file names */
		for (i = 2; i < argc; i++) {
			strcat(inputName, " ");
			strcat(inputName, argv[i]);
		}

		if (strIstr(inputName, ".swf") != NULL) {
			flmName = mstrdup(inputName);
			strcpy(strIstr(flmName, ".swf"), ".flm");
			/* redirect stdout to inputName.flm; should close it later? */
			if (freopen(flmName, "wb", stdout) == NULL)
				tellUser(1, "Couldn't open output file %s for writing", flmName);

			mode = MODE_DISASSEMBLE;
		}
		else if (strIstr(inputName, ".flm") != NULL) {
			mode = MODE_ASSEMBLE;
		}
		else
			usage();
	}

	if (inputName == NULL)
		usage();

	if ((inputFile = fopen(inputName, "rb")) == NULL)
		tellUser(1, "Couldn't open input file %s for reading", inputName);
}

int main(int argc, char *argv[])
{
	yydebug = 0;

#ifdef MEMWATCH
	mwStatistics(2);
#endif

	checkByteOrder();

	parseArgs(argc, argv);

	if (mode == MODE_DISASSEMBLE) {
		getSWFHeader(inputFile);
		if (strcmp(swfHeader, "FWS") != 0) {
			if (strcmp(swfHeader, "CWS") == 0) {
				decompressSWF(inputFile, inputName);
				/* skip SWF header, we know it's 'FWS' */
				fseek(tempFile, 3, SEEK_SET);
				disassembleSWF(tempFile, inputName);
				remove(tempName);
			}
			else
				tellUser(1, "Input file doesn't appear to be an SWF file..");
		}
		else
			disassembleSWF(inputFile, inputName);

		mexit(EXIT_SUCCESS);
	}

	if (mode == MODE_DECOMPRESS) {
		getSWFHeader(inputFile);
		if (strcmp(swfHeader, "CWS") != 0) {
			if (strcmp(swfHeader, "FWS") == 0)
				tellUser(1, "Know what, the SWF isn't compressed");
			else
				tellUser(1, "Input file doesn't appear to be an SWF file..");
		}
		decompressSWF(inputFile, inputName);
		mexit(EXIT_SUCCESS);
	}

	if (mode == MODE_COMPRESS) {
		getSWFHeader(inputFile);
		if (strcmp(swfHeader, "FWS") != 0) {
			if (strcmp(swfHeader, "CWS") == 0)
				tellUser(1, "Know what, the SWF is already compressed");
			else
				tellUser(1, "Input file doesn't appear to be an SWF file..");
		}
		compressSWF(inputFile, inputName);
		mexit(EXIT_SUCCESS);
	}

	if (mode == MODE_ASSEMBLE || mode == MODE_ASBYTECODE) {
		yyin = inputFile;
		yyparse();
		fclose(inputFile);
		mexit(EXIT_SUCCESS);
	}

	if ((mode >= MODE_UPDATE) && (mode != MODE_FLASH_HELP)) {
		FILE *stdoutTempFile;
		getSWFHeader(inputFile);
		
		/* overwrite user settings */
		showoffset = 0;
		literalconstants = 1;
		literalregisters = 0;

		if (strcmp(swfHeader, "FWS") != 0) {
			if (strcmp(swfHeader, "CWS") == 0) {
				decompressSWF(inputFile, inputName);
				inputFile = fopen(inputName, "rb");
				getSWFHeader(inputFile);
			}
			else
				tellUser(1, "Input file doesn't appear to be an SWF file..");
		}

		flmName = mstrdup(inputName);
		strcpy(strIstr(flmName, ".swf"), ".$lm");

		/* redirect stdout to inputName.flm */
		stdoutTempFile = freopen(flmName, "wb", stdout);

		if (stdoutTempFile == NULL)
			tellUser(1, "Couldn't create temporary file");

		disassembleSWF(inputFile, inputName);
		fclose(stdoutTempFile);

		inputFile = fopen(flmName, "rb");
		inputName = mstrdup(flmName);
		yyin = inputFile;
		yyparse();

		fclose(inputFile);
		remove(flmName);
	}

	if (mode >= MODE_IDE) {
		if (system(flapath) == -1)
			tellUser(1, "Couldn't execute: %s", flapath);
	}

	mexit(EXIT_SUCCESS);
	/* to make compiler happy */
	exit(0);
}

/* indent -l200 --ignore-newlines -bap -nbc -fc1 -npsl -sob -ncdb -brs -br -nce -cdw -npcs -i4 -ts4 -c60 -cd30 -cli4 -cbi4 -bs -nss flasm.c */
/* splint -retvalint +posixlib -boolops -retvalother -realcompare -predboolint unflasm.c flasm.c util.c assembler.tab.c > flasm.lint */
