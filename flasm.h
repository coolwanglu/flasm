/*
flasm, command line assembler & disassembler of flash actionscript bytecode
Copyright (c) 2001 Opaque Industries, (c) 2002-2007 Igor Kogan
All rights reserved. See LICENSE.TXT for terms of use.
*/

#ifndef	FLASM_H_INCLUDED
#define	FLASM_H_INCLUDED
#include "action.h"
#include "util.h"

extern int swfVersion;

char *mstrdup(const char *String);

int writeByte(unsigned char num);
int writeShort(unsigned int num);
int writeLongInt(long int num);
int writeFloat(float num);
int writeDouble(double num);
int writeString(char *str);
unsigned int writePushString(char *str);

void addConstant(char *string);
unsigned int writeConstants(void);
void patchFlag(unsigned int back, byte flag);
void patchLength(unsigned int back, unsigned int blen);
void patchFrameLoaded(unsigned int len, int numActions);
int branchTarget(char *label);
int addNumLabel(int numLabel);
void addLabel(char *label);

void writeDoAction(void);
void writeInitMC(unsigned int clipID);
void writeOnClipEvent(unsigned long int flag);
void writePlaceMCStart(unsigned int clipID);
void writePlaceMCEnd(unsigned long int flags);
void writeButtonStart(unsigned int buttonID);
void writeButtonEnd(void);
void writeButtonEvent(unsigned int flags);
void writeDefineMCStart(unsigned int clipID);
void writeDefineMCEnd(void);
void writeProtect(char *str);
void writeEnableDebugger(char *str);
void writeEnableDebugger2(char *str);
void writeMetadata(char *str);
void writeFileAttrs(unsigned long int attrs);
void writeScriptLimits(unsigned int recursion, unsigned int timeout);
void writeExportAssets(unsigned int numAssets);
void writeImportAssets(char *str, unsigned int numAssets);
void startUpdate(char *str);
void finishUpdate(void);
void writeASBytecode(void);
void tellUser(int isError, char *s, ...);

#endif /* FLASM_H_INCLUDED	*/
