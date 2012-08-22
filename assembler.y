/*
flasm, command line assembler & disassembler of flash actionscript bytecode
Copyright (c) 2001 Opaque Industries, (c) 2002-2007 Igor Kogan, (c) 2005 Wang Zhen
All rights reserved. See LICENSE.TXT for terms of use.
*/

%{

#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef MEMWATCH
#include "memwatch.h"
#endif
#include "util.h"
#include "flasm.h"

void yyerror(char *s);
void warning(char *msg);
int yylex(void);

extern char *yytext;
extern long int len;
extern unsigned int nConstants;
extern int mode;
extern char compressAfter;
extern int numActions;
extern int clearregisterargs;
extern char *arNames[];
extern unsigned int arPreFlags[];
extern unsigned int arSupFlags[];

static char *func_args[MAX_FUNCARGS];
static char *regfunc_args[MAX_FUNCDEPTH][MAX_REGISTERS];
static unsigned int numRegisters[MAX_FUNCDEPTH];
static int curFunc = -1;
static unsigned int numAssets;
static unsigned long int curEvent, allEvents;
static int frameloadedStart = -1;
static unsigned int numArgs;
static unsigned int autoregFlags, nosuppressFlags;
%}


%union
{
  long int num;
  char *str;
}

%expect 1  /* double nots shift/reduce warning */

%token <str> MOVIENAME
%token <str> STRING
%token <str> HEX
%token <str> FLOAT
%token <str> DOUBLE
%token <str> TRUEVAL
%token <str> FALSEVAL
%token <str> NULLVAL
%token <str> UNDEFVAL
%token <str> LABEL
%token <num> INTEGER
%token <num> CONSTANT

%token MOVIE
%token COMPRESSED
%token PROTECT
%token SCRIPTLIMITS
%token RECURSION
%token TIMEOUT
%token ENABLEDEBUGGER
%token ENABLEDEBUGGER2
%token FRAME
%token PLACEMOVIECLIP
%token DEFINEMOVIECLIP
%token INITMOVIECLIP
%token DEFINEBUTTON
%token ON
%token ONCLIPEVENT
%token AS
%token IMPORTASSETS
%token EXPORTASSETS
%token METADATA
%token FILEATTRIBUTES
%token FROM

// special double values
%token _NAN
%token POSITIVE_INFINITY
%token NEGATIVE_INFINITY

// special float values - push type 1
%token _NANF
%token POSITIVE_INFINITYF
%token NEGATIVE_INFINITYF

// button events
%token BIDLETOOVERUP
%token BOVERUPTOIDLE
%token BOVERUPTOOVERDOWN
%token BOVERDOWNTOOVERUP
%token BOVERDOWNTOOUTDOWN
%token BOUTDOWNTOOVERDOWN
%token BOUTDOWNTOIDLE
%token BIDLETOOVERDOWN
%token BOVERDOWNTOIDLE

%token KEYPRESS

// keyPress pre-defined
%token _LEFT
%token _RIGHT
%token _HOME
%token _END
%token _INSERT
%token _DELETE
%token _BACKSPACE
%token _ENTER
%token _UP
%token _DOWN
%token _PAGEUP
%token _PAGEDOWN
%token _TAB
%token _ESCAPE
%token _SPACE

// onClipEvents
%token MCLOAD
%token MCENTERFRAME
%token MCUNLOAD
%token MCMOUSEMOVE
%token MCMOUSEDOWN
%token MCMOUSEUP
%token MCKEYDOWN
%token MCKEYUP
%token MCINITIALIZE
%token MCCONSTRUCT
%token MCDATA
%token MCPRESS
%token MCRELEASE
%token MCRELEASEOUTSIDE
%token MCROLLOVER
%token MCROLLOUT
%token MCDRAGOVER
%token MCDRAGOUT

// old-style properties
%token X_PROPERTY
%token Y_PROPERTY
%token XSCALE_PROPERTY
%token YSCALE_PROPERTY
%token WIDTH_PROPERTY
%token HEIGHT_PROPERTY
%token ALPHA_PROPERTY
%token VISIBLE_PROPERTY
%token ROTATION_PROPERTY
%token CURRENTFRAME_PROPERTY
%token TOTALFRAMES_PROPERTY
%token TARGET_PROPERTY
%token FRAMESLOADED_PROPERTY
%token NAME_PROPERTY
%token DROPTARGET_PROPERTY
%token URL_PROPERTY
%token QUALITY_PROPERTY
%token XMOUSE_PROPERTY
%token YMOUSE_PROPERTY
%token HIGHQUALITY_PROPERTY
%token FOCUSRECT_PROPERTY
%token SOUNDBUFTIME_PROPERTY

%token NEXTFRAME
%token PREVFRAME
%token GOTOFRAME
%token GOTOLABEL
%token PLAY
%token STOP
%token TOGGLEQUALITY
%token STOPSOUNDS
%token FUNCTION
%token FUNCTION2
%token CONSTANTPOOL
%token END
%token DUP
%token SWAP
%token POP
%token WITH
%token PUSH
%token SETREGISTER
%token CALLFUNCTION
%token RETURN
%token NEWMETHOD
%token CALLMETHOD
%token BITWISEAND
%token BITWISEOR
%token BITWISEXOR
%token MODULO
%token NEWADD
%token NEWLESSTHAN
%token NEWEQUALS
%token TONUMBER
%token TOSTRING
%token INCREMENT
%token DECREMENT
%token TYPEOF
%token TARGETPATH
%token ENUMERATE
%token ENUMERATEVALUE
%token INSTANCEOF
%token DELETE
%token DELETE2
%token NEW
%token INITARRAY
%token INITOBJECT
%token GETMEMBER
%token SETMEMBER
%token SHIFTLEFT
%token SHIFTRIGHT
%token SHIFTRIGHT2
%token VAR
%token VAREQUALS
%token OLDADD
%token SUBTRACT
%token MULTIPLY
%token DIVIDE
%token OLDEQUALS
%token OLDLESSTHAN
%token STRICTEQUALS
%token GREATERTHAN
%token LOGICALAND
%token LOGICALOR
%token LOGICALNOT
%token STRINGEQ
%token STRINGLENGTH
%token SUBSTRING
%token INT
%token GETVARIABLE
%token SETVARIABLE
%token SETTARGET
%token SETTARGETEXPR
%token STRINGCONCAT
%token GETPROPERTY
%token SETPROPERTY
%token DUPLICATECLIP
%token REMOVECLIP
%token TRACE
%token STARTDRAGMOVIE
%token STOPDRAGMOVIE
%token STRINGLESSTHAN
%token STRINGGREATERTHAN
%token RANDOM
%token MBLENGTH
%token ORD
%token CHR
%token GETTIMER
%token MBSUBSTRING
%token MBORD
%token MBCHR
%token BRANCHALWAYS
%token GETURL
%token GETURL2
%token LOADMOVIE
%token LOADMOVIENUM
%token LOADVARIABLES
%token LOADVARIABLESNUM
%token POST
%token GET
%token BRANCHIFTRUE
%token CALLFRAME
%token GOTOANDPLAY
%token GOTOANDSTOP
%token SKIP
%token IFFRAMELOADEDEXPR
%token IFFRAMELOADED
%token ELSE
%token STRICTMODE
%token OFF
%token IMPLEMENTS
%token EXTENDS
%token THROW
%token CAST
%token TRY
%token CATCH
%token FINALLY
%token FSCOMMAND2
%token REGISTER
/* file attributes */
%token ATTRUSENETWORK
%token ATTRRELATIVEURLS
%token ATTRSUPPRESSCROSSDOMAINCACHE
%token ATTRACTIONSCRIPT3
%token ATTRHASMETADATA
/* unknown instruction handling */
%token SWFACTION
%token HEXDATA

%token '(' ')' ',' ':' '"' '.' '='

%type <str> name_opt mcname_opt
%type <num> statements statement statements_opt
%type <num> function function_args function2 regarglist regarg autoregarglist autoregarg
%type <num> trycatchfinally catch_opt finally_opt
%type <num> impassets impassetsblocks_opt impassetsblocks impassetsblock
%type <num> expassets expassetsblocks_opt expassetsblocks expassetsblock
%type <num> push_list push_item
%type <num> register
%type <num> with
%type <num> settarget settargetexpression ifframeloaded ifframeloadedexpression
%type <num> actionblocks actionblock actionblocks_opt
%type <num> buttoneventblocks buttoneventblock buttoneventblocks_opt
%type <num> mceventblocks mceventblock mceventblocks_opt
%type <num> mcblocks mcblock mcblocks_opt
%type <num> frame definebutton definemc placemc initmc
%type <num> buttonevent buttonevents mcevent mcevents key property fileattr fileattrs
%type <num> opcode hex_list hexlist_opt
%type <num> urlmethod

%%

/* rules */
program
        : MOVIE MOVIENAME                       { 
                                                    if (mode != MODE_ASBYTECODE)
                                                        startUpdate($2);
                                                    else
                                                        yyerror("Only single action block can be assembled to __bytecode__, not the whole Flasm project");
                                                
                                                }
          actionblocks_opt END                  {   finishUpdate();     }
        | statements                            {
                                                    if (mode == MODE_ASBYTECODE)
                                                        writeASBytecode();
                                                    else
                                                        yyerror("Flasm project should start with movie 'moviename.swf'");
                                                }
        ;

actionblocks_opt
        : /* empty */                           {   $$ = 0;             }
        | actionblocks                          {   $$ = $1;            }
        ;

actionblocks
        : actionblock                           {   $$ = $1;            }
        | actionblocks actionblock              {   $$ = $1 + $2;       }
        ;

actionblock
        : frame                                 {   $$ = $1;    }
        | definebutton                          {   $$ = $1;    }
        | definemc                              {   $$ = $1;    }
        | placemc                               {   $$ = $1;    }
        | initmc                                {   $$ = $1;    }
        | impassets                             {   $$ = $1;    }
        | expassets                             {   $$ = $1;    }
        | COMPRESSED                            {   compressAfter = 1;          $$ = 0; }
        | PROTECT                               {   writeProtect("");           $$ = 0; }
        | PROTECT STRING                        {   writeProtect($2);           $$ = 0; }
        | ENABLEDEBUGGER                        {   writeEnableDebugger("");    $$ = 0; }
        | ENABLEDEBUGGER STRING                 {   writeEnableDebugger($2);    $$ = 0; }
        | ENABLEDEBUGGER2 STRING                {   writeEnableDebugger2($2);   $$ = 0; }
        | METADATA STRING                       {   writeMetadata($2);          $$ = 0; }
        | FILEATTRIBUTES fileattrs              {   writeFileAttrs($2);         $$ = 0; }
        | SCRIPTLIMITS RECURSION INTEGER TIMEOUT INTEGER
                                                {
                                                    if ($3>65535)
                                                        yyerror("Recursion depth out of range");
                                                    if ($5>65535)
                                                        yyerror("Timeout out of range");
                                                    writeScriptLimits((unsigned int)$3, (unsigned int)$5);
                                                    $$ = 0;
                                                }
        ;

frame
        : FRAME INTEGER statements_opt END      {
                                                    $$ = $3;
                                                    /*action end*/
                                                    $$ += writeByte(0);
                                                    writeDoAction();
                                                    $$ = 0;
                                                }
        ;

initmc
        : INITMOVIECLIP INTEGER statements_opt END
                                                {
                                                    $$ = $3;
                                                    /*action end*/
                                                    $$ += writeByte(0);
                                                    writeInitMC($2);
                                                    $$ = 0;
                                                }
        ;

definebutton
        : DEFINEBUTTON INTEGER                  {   writeButtonStart($2); }
          buttoneventblocks_opt END             {   $$ = $4; writeButtonEnd();  }
        ;

buttoneventblocks_opt
        : /* empty */                           {   $$ = 0;         }
        | buttoneventblocks                     {   $$ = $1;        }
        ;

buttoneventblocks
        : buttoneventblock                      {   $$ = $1;        }
        | buttoneventblocks buttoneventblock    {   $$ = $1 + $2;   }
        ;

buttoneventblock
        : ON                                    {   curEvent = 0;   }
          buttonevents statements_opt END       {
                                                    $$ = $4;
                                                    /*event action end*/
                                                    $$ += writeByte(0);
                                                    writeButtonEvent((unsigned int)curEvent);
                                                }
        ;

buttonevents
        : buttonevent                           {   $$ = $1;  curEvent += $1;   }
        | buttonevents ',' buttonevent          {   $$ += $3; curEvent += $3;   }
        ;

buttonevent
        : BIDLETOOVERUP                         {   $$ = 0x01;  }
        | BOVERUPTOIDLE                         {   $$ = 0x02;  }
        | BOVERUPTOOVERDOWN                     {   $$ = 0x04;  }
        | BOVERDOWNTOOVERUP                     {   $$ = 0x08;  }
        | BOVERDOWNTOOUTDOWN                    {   $$ = 0x10;  }
        | BOUTDOWNTOOVERDOWN                    {   $$ = 0x20;  }
        | BOUTDOWNTOIDLE                        {   $$ = 0x40;  }
        | BIDLETOOVERDOWN                       {   $$ = 0x80;  }
        | BOVERDOWNTOIDLE                       {   $$ = 0x100; }
        | KEYPRESS key                          {   $$ = $2<<9; }
        ;

key
        : _LEFT                                 {   $$ = 1;     }
        | _RIGHT                                {   $$ = 2;     }
        | _HOME                                 {   $$ = 3;     }
        | _END                                  {   $$ = 4;     }
        | _INSERT                               {   $$ = 5;     }
        | _DELETE                               {   $$ = 6;     }
        | _BACKSPACE                            {   $$ = 8;     }
        | _ENTER                                {   $$ = 13;    }
        | _UP                                   {   $$ = 14;    }
        | _DOWN                                 {   $$ = 15;    }
        | _PAGEUP                               {   $$ = 16;    }
        | _PAGEDOWN                             {   $$ = 17;    }
        | _TAB                                  {   $$ = 18;    }
        | _ESCAPE                               {   $$ = 19;    }
        | _SPACE                                {   $$ = 32;    }
        | STRING                                {   $$ = $1[0]; }
        | HEX                                   {
                                                    if (xtoi($1)>0x7f)
                                                        yyerror("key code should be not greater than 0x7F");
                                                    $$ = (char)xtoi($1);
                                                }
        ;

register
        : REGISTER INTEGER                      {
                                                    /* only 255 regs can be allocated within function2,
                                                       r:0 being the first and r:254 the last;
                                                       outside 4 global registers exist */
                                                    if ($2 >= 255)
                                                        yyerror("Register number out of range");
                                                    if ((byte)$2 > 3 && curFunc < 0)
                                                        warning("Local registers r:4 to r:255 work in function2 context only");
                                                    /* if needed, increase the number of registers
                                                       to allocate for function2 */
                                                    if (curFunc >= 0  &&  $2 + 1 > (long int) numRegisters[curFunc])
                                                        numRegisters[curFunc] = $2 + 1;                                               
                                                    $$ = $2;
                                                }

        | REGISTER STRING                       {
                                                    unsigned int m;
                                                    int r = -1;
                                                    
                                                    if (curFunc >= 0) {
                                                        for(m = 1; m < numRegisters[curFunc]; ++m) {
                                                            if (regfunc_args[curFunc][m] != NULL && !strcmp($2, regfunc_args[curFunc][m])) {
                                                                r = m;
                                                                break;
                                                            }
                                                        }
                                                    }

                                                    if (r == -1)
                                                        yyerror("Register alias not found");
                                                    
                                                    $$ = r;
                                                }
        ;

definemc
        : DEFINEMOVIECLIP INTEGER               {   writeDefineMCStart($2);   }
          mcblocks_opt END                      {   $$ = $4; writeDefineMCEnd();    }
        ;

mcblocks_opt
        : /* empty */                           {   $$ = 0;     }
        | mcblocks                              {   $$ = $1;    }
        ;

mcblocks
        : mcblock                               {   $$ = $1;        }
        | mcblocks mcblock                      {   $$ = $1 + $2;   }
        ;

mcblock
        : frame                                 {   $$ = $1;    }
        | placemc                               {   $$ = $1;    }
        ;

placemc
        : PLACEMOVIECLIP INTEGER mcname_opt     {   allEvents = 0;  writePlaceMCStart($2);       }
          mceventblocks_opt END                 {   $$ = $5;        writePlaceMCEnd(allEvents);  }
        ;

mcname_opt
        : /* empty */                           {   $$ = "";    }
        | AS STRING                             {   $$ = $2;    }
        ;

mceventblocks_opt
        : /* empty */                           {   $$ = 0;     }
        | mceventblocks                         {   $$ = $1;    }
        ;

mceventblocks
        : mceventblock                          {   $$ = $1;      }
        | mceventblocks mceventblock            {   $$ = $1 + $2; }
        ;

mceventblock
        : ONCLIPEVENT mcevents                  {
                                                    curEvent = $2;
                                                    allEvents |= curEvent;
                                                }

          statements_opt END                    {
                                                    $$ = $4;
                                                    /*event action end*/
                                                    $$ += writeByte(0);
                                                    writeOnClipEvent(curEvent);
                                                    curEvent = 0;
                                                }
        ;

mcevents
        : mcevent                               {   $$ = $1;    curEvent += $1; }
        | mcevents ',' mcevent                  {   $$ += $3;   curEvent += $3; }
        ;

mcevent
        : /* empty */                           {   yyerror("Missing mc event condition"); }
        | MCLOAD                                {   $$ = 0x01;      }
        | MCENTERFRAME                          {   $$ = 0x02;      }
        | MCUNLOAD                              {   $$ = 0x04;      }
        | MCMOUSEMOVE                           {   $$ = 0x08;      }
        | MCMOUSEDOWN                           {   $$ = 0x10;      }
        | MCMOUSEUP                             {   $$ = 0x20;      }
        | MCKEYDOWN                             {   $$ = 0x40;      }
        | MCKEYUP                               {   $$ = 0x80;      }
        | MCDATA                                {   $$ = 0x100;     }
        | MCINITIALIZE                          {   $$ = 0x200;     }
        | MCPRESS                               {   $$ = 0x400;     }
        | MCRELEASE                             {   $$ = 0x800;     }
        | MCRELEASEOUTSIDE                      {   $$ = 0x1000;    }
        | MCROLLOVER                            {   $$ = 0x2000;    }
        | MCROLLOUT                             {   $$ = 0x4000;    }
        | MCDRAGOVER                            {   $$ = 0x8000;    }
        | MCDRAGOUT                             {   $$ = 0x10000;   }
        | KEYPRESS key                          {   $$ = 0x20000; writeByte($2);    }
        | MCCONSTRUCT                           {   $$ = 0x40000;   }
        ;

fileattrs
        : fileattr                              {   $$ = $1;    }
        | fileattrs ',' fileattr                {   $$ += $3;   }
        ;
      
fileattr
        : ATTRUSENETWORK                        {   $$ = 0x01;  }
        | ATTRRELATIVEURLS                      {   $$ = 0x02;  }
        | ATTRSUPPRESSCROSSDOMAINCACHE          {   $$ = 0x04;  }
        | ATTRACTIONSCRIPT3                     {   $$ = 0x08;  }
        | ATTRHASMETADATA                       {   $$ = 0x10;  }
        | INTEGER                               {   $$ = $1;    }
        ;

catch_opt
        : /* empty */                           {   $$ = 0;     }
        | CATCH statements_opt                  {   $$ = $2;    }
        ;

finally_opt
        : /* empty */                           {   $$ = 0;     }
        | FINALLY statements_opt                {   $$ = $2;    }
        ;

trycatchfinally 
        : TRY name_opt                          {
                                                    $<num>$ = writeByte(SWFACTION_TRY);
                                                    /* action length */
                                                    $<num>$ += writeShort(strlen($2)+8);
                                                    /* zero flag */
                                                    $<num>$ += writeByte(0);
                                                    /* zero try length */
                                                    $<num>$ += writeShort(0);
                                                    /* zero catch length */
                                                    $<num>$ += writeShort(0);
                                                    /* zero finally length */
                                                    $<num>$ += writeShort(0);
                                                    /* error variable name */
                                                    $<num>$ += writeString($2);
                                                }

        statements_opt                          {   $<num>$ = $<num>3 + $4; patchLength($<num>$ - 6,  $4);    }
        catch_opt                               {   $<num>$ = $<num>5 + $6; patchLength($<num>$ - 8,  $6);    }
        finally_opt                             {   $<num>$ = $<num>7 + $8; patchLength($<num>$ - 10, $8);    }
        END                                     {
                                                    byte flag = 0;
                                                    $$ = $<num>9;
                                                    if ($6>0)
                                                        flag = flag + 1;
                                                    if ($8>0)
                                                        flag = flag + 2;
                                                    patchFlag($$ - 4, flag);
                                                }

        | TRY register                          {
                                                    $<num>$ = writeByte(SWFACTION_TRY);
                                                    /* action length */
                                                    $<num>$ += writeShort(8);
                                                    /* zero flag */
                                                    $<num>$ += writeByte(0);
                                                    /* zero try length */
                                                    $<num>$ += writeShort(0);
                                                    /* zero catch length */
                                                    $<num>$ += writeShort(0);
                                                    /* zero finally length */
                                                    $<num>$ += writeShort(0);
                                                    /* error register number */
                                                    $<num>$ += writeByte((byte) $2);
                                                }

        statements_opt                          {   $<num>$ = $<num>3 + $4; patchLength($<num>$ - 6,  $4);    }
        catch_opt                               {   $<num>$ = $<num>5 + $6; patchLength($<num>$ - 8,  $6);    }
        finally_opt                             {   $<num>$ = $<num>7 + $8; patchLength($<num>$ - 10, $8);    }
        END                                     {
                                                    byte flag = 4;
                                                    $$ = $<num>9;
                                                    if ($6>0)
                                                        flag = flag + 1;
                                                    if ($8>0)
                                                        flag = flag + 2;
                                                    patchFlag($$ - 4, flag);
                                                }
        ;

statements
        : statement                             {   $$ = $1;        }
        | statements statement                  {   $$ = $1 + $2;   }
        ;

statement
        : label                                 {   $$ = 0;     }
        | function                              {   $$ = $1;    }
        | function2                             {   $$ = $1;    }
        | with                                  {   $$ = $1;    }
        | settarget                             {   $$ = $1;    }
        | settargetexpression                   {   $$ = $1;    }
        | ifframeloaded                         {   $$ = $1;    }
        | ifframeloadedexpression               {   $$ = $1;    }
        | trycatchfinally                       {   $$ = $1;    }
        | opcode                                {   $$ = $1;    }
        ;


label
        : LABEL                                 {   addLabel($1);   }
        ;

/* this is weird/dumb- now we're returning the number of args instead
   of their length, and the args are stored in a global array func_args. */

function_args
        : /* empty */                           {   $$ = 0; }
        | STRING                                {   func_args[0] = $1;  $$ = 1;         }
        | function_args ',' STRING              {   func_args[$1] = $3; $$ = $1 + 1;    }
        ;

statements_opt
        : /* empty */                           {   $$ = 0;     }
        | statements                            {   $$ = $1;    }
        ;

name_opt
        : /* empty */                           {   $$ = "";    }
        | STRING                                {   $$ = $1;    }
        ;

function
        : FUNCTION name_opt                     {
                                                    $<num>$ = writeByte(SWFACTION_DEFINEFUNCTION);
                                                    /* zero block length */
                                                    $<num>$ += writeShort(0);
                                                    $<num>$ += writeString($2);
                                                }

          '(' function_args ')'                 {
                                                    unsigned int i;
                                                    numArgs = $5;

                                                    $<num>$ = $<num>3 + writeShort(numArgs);

                                                    for(i = 0; i < numArgs; ++i)
                                                        $<num>$ += writeString(func_args[i]);

                                                    /* zero function length */
                                                    $<num>$ += writeShort(0);
                                                    /* patch block length */
                                                    patchLength($<num>$-3, $<num>$-3);
                                                }

          statements_opt END                    {
                                                    $$ = $<num>7 + $8;
                                                    /* patch function length */
                                                    patchLength($8, $8);
                                                }
        ;

regarg
        : REGISTER INTEGER '=' STRING           {
                                                    numArgs++;
                                                    $$ = writeByte((byte) $2);

                                                    if ($2 == 0)
                                                        yyerror("Function argument can't be stored in r:0");

                                                    if ($2 + 1 >= MAX_REGISTERS)
                                                        yyerror("Too many registers");
                                                    
                                                    if (numArgs >= MAX_FUNCARGS)
                                                        yyerror("Too many function arguments");

                                                    if (regfunc_args[curFunc][$2] != NULL)
                                                        yyerror("Duplicate register");

                                                    regfunc_args[curFunc][$2] = $4;

                                                    if ($2 + 1 > (long int) numRegisters[curFunc])
                                                        numRegisters[curFunc] = $2 + 1;

                                                    /* if parameter is stored in register, may skip its name */
                                                    if ($2 > 0 && clearregisterargs && mode >= MODE_UPDATE)
                                                        $$ += writeString("");
                                                    else
                                                        $$ += writeString($4);
                                                }
        | STRING                                {
                                                    numArgs++;
                                                    /* r:0 - not stored in register */
                                                    $$ = writeByte(0);
                                                    $$ += writeString($1);
                                                }
        ;

regarglist
        : /* empty */                           {   $$ = 0;         }
        | regarg                                {   $$ = $1;        }
        | regarglist ',' regarg                 {   $$ = $1 + $3;   }
        ;

autoregarg
        : REGISTER INTEGER '=' STRING           {
                                                    unsigned int preloadFlag = 0, i;

                                                    if ($2 == 0 || $2 > MAX_AUTO_REGS)
                                                        yyerror("Automatic values should be placed into consequent registers starting with r:1\nin this order: this, arguments, super, _root, _parent, _global");

                                                    if (regfunc_args[curFunc][$2] != NULL && strcmp(regfunc_args[curFunc][$2], $4))
                                                        yyerror("Duplicate register");

                                                    regfunc_args[curFunc][$2] = $4;

                                                    if ($2 + 1 > (long int) numRegisters[curFunc])
                                                        numRegisters[curFunc] = $2 + 1;
                                                    
                                                    for (i = 0; i < MAX_AUTO_REGS; i++) {
                                                        if (!strcmp($4, arNames[i])) {
                                                            preloadFlag = arPreFlags[i];
                                                            if (nosuppressFlags & (1 << i))
                                                                yyerror("Duplicate automatic value");
                                                            break;
                                                        }
                                                    }

                                                    if (preloadFlag == 0)
                                                        yyerror("Only automatic values (this, arguments, super, _root, _parent, _global) are allowed here");
                                                    
                                                    if ((autoregFlags & preloadFlag) != preloadFlag)
                                                        autoregFlags += preloadFlag;
                                                    else
                                                        yyerror("Duplicate automatic value");
                                                }
        | STRING                                {
                                                    unsigned int nosuppressFlag = 0, i;
                                                    for (i = 0; i < MAX_AUTO_REGS; i++) {
                                                        if (!strcmp($1, arNames[i])) {
                                                            nosuppressFlag = 1 << i;
                                                            if ((autoregFlags & arPreFlags[i]) == arPreFlags[i])
                                                                yyerror("Duplicate automatic value");
                                                            break;
                                                        }
                                                    }
                                                    if (nosuppressFlag == 0)
                                                        yyerror("Only automatic values (this, arguments, super, _root, _parent, _global) are allowed here");

                                                    if (!(nosuppressFlags & nosuppressFlag))
                                                        nosuppressFlags += nosuppressFlag;
                                                    else
                                                        yyerror("Duplicate automatic value");
                                                }
        ;

autoregarglist
        : /* empty */                           {   $$ = 0;         }
        | autoregarg                            {   $$ = 1;         }
        | autoregarglist ',' autoregarg         {   $$ = $1 + 1;    }
        ;

function2
        : FUNCTION2 name_opt                    {
                                                    $<num>$ = writeByte(SWFACTION_DEFINEFUNCTION2);
                                                    /* zero block length */
                                                    $<num>$ += writeShort(0);
                                                    /* function name */
                                                    $<num>$ += writeString($2);
                                                    curFunc++;
                                                    memset(regfunc_args[curFunc], 0, sizeof (regfunc_args[curFunc]));
                                                    numArgs = 0;
                                                    /* zero num of function arguments */
                                                    $<num>$ += writeShort(numArgs);
                                                    /* allocate zero registers */
                                                    numRegisters[curFunc] = 0;
                                                    $<num>$ += writeByte(numRegisters[curFunc]);
                                                    /* zero automatic register flags */
                                                    $<num>$ += writeShort(0);
                                                }

          '(' regarglist ')'                    {
                                                    $<num>$ = $<num>3 + $5;
                                                    /* patch num of function arguments */
                                                    patchLength($5 + 3, numArgs);
                                                    autoregFlags = 0;
                                                    nosuppressFlags = 0;
                                                }
          
          '(' autoregarglist ')'                {
                                                    byte curautoreg = 1;
                                                    unsigned int i;

                                                    $<num>$ = $<num>7;
                                                    /* zero body length */
                                                    $<num>$ += writeShort(0);
                                                    
                                                    /* make sure auto registers are allocated in the right order */
                                                    for (i = 0; i < MAX_AUTO_REGS; i++) {
                                                        if ((autoregFlags & arPreFlags[i]) == arPreFlags[i]) {
                                                            if (regfunc_args[curFunc][curautoreg] != NULL && !strcmp(regfunc_args[curFunc][curautoreg], arNames[i]))
                                                                curautoreg++;
                                                            else
                                                                yyerror("Automatic values should be placed into consequent registers starting with r:1\nin this order: this, arguments, super, _root, _parent, _global");
                                                        }
                                                        else if (!(nosuppressFlags & (1 << i)))
                                                            autoregFlags += arSupFlags[i];
                                                    }

                                                    /* patch automatic register flags */
                                                    patchLength($<num>$ - $<num>3, autoregFlags);
                                                    /* patch block length */
                                                    patchLength($<num>$ - 3, $<num>$ - 3);
                                                }

          statements_opt END                    {
                                                    $$ = $<num>11 + $12;

                                                    /* patch number of registers to allocate */
                                                    if (numRegisters[curFunc] < MAX_REGISTERS)
                                                        patchFlag($$ - $<num>3 + 2, (byte) numRegisters[curFunc]);
                                                    else
                                                        yyerror("Too many registers.");

                                                    /* patch function length */
                                                    patchLength($12, $12);
                                                    curFunc--;
                                                }
        ;

with
        : WITH                                  {
                                                    $<num>$ = writeByte(SWFACTION_WITH);
                                                    /* length of with action */
                                                    $<num>$ += writeShort(2);
                                                    /* length of with block - will be patched */
                                                    $<num>$ += writeShort(0);
                                                }
                                                
          statements_opt END                    { 
                                                    $$ = $<num>2 + $3;
                                                    patchLength($3, $3);
                                                }

        ;

settarget
        : SETTARGET STRING                      {
                                                    $<num>$ = writeByte(SWFACTION_SETTARGET);
                                                    $<num>$ += writeShort(strlen($2)+1);
                                                    $<num>$ += writeString($2);
                                                }

          statements_opt END                    {
                                                    $$ = $4 + writeByte(SWFACTION_SETTARGET);
                                                    $$ += $<num>3 + writeShort(1);
                                                    $$ += writeByte(0);
                                                }
        ;

settargetexpression
        : SETTARGETEXPR                         {   $<num>$ = writeByte(SWFACTION_SETTARGETEXPRESSION);  }
          statements_opt END                    {
                                                    $$ = $3 + writeByte(SWFACTION_SETTARGET);
                                                    $$ += $<num>2 + writeShort(1);
                                                    $$ += writeByte(0);
                                                }
        ;

ifframeloadedexpression
        : IFFRAMELOADEDEXPR                     {
                                                    if (frameloadedStart>-1)
                                                        yyerror("IfFrameLoaded actions can't be nested");
                                                    $<num>$ = writeByte(SWFACTION_IFFRAMELOADEDEXPRESSION);
                                                    $<num>$ += writeShort(1);
                                                    $<num>$ += writeByte(0);
                                                    frameloadedStart = numActions;
                                                }

          statements_opt END                    { 
                                                    $$ = $<num>2 + $3;
                                                    patchFrameLoaded($3, numActions-frameloadedStart);
                                                    frameloadedStart = -1;
                                                }
        ;

ifframeloaded
        : IFFRAMELOADED INTEGER                 {
                                                    if (frameloadedStart>-1)
                                                        yyerror("IfFrameLoaded actions can't be nested");
                                                    $<num>$ = writeByte(SWFACTION_IFFRAMELOADED);
                                                    $<num>$ += writeShort(3);
                                                    $<num>$ += writeShort($2);
                                                    $<num>$ += writeByte(0);
                                                    frameloadedStart = numActions;
                                                }
                                         
          statements_opt END                    { 
                                                    $$ = $<num>3 + $4;
                                                    patchFrameLoaded($4, numActions-frameloadedStart);
                                                    frameloadedStart = -1;
                                                }
        ;

impassets
        : IMPORTASSETS FROM STRING              {   numAssets = 0;  }
          impassetsblocks_opt END               {
                                                    $$ = $5;
                                                    writeImportAssets($3, numAssets);
                                                    $$ = 0;
                                                }
        ;

impassetsblocks_opt
        : /* empty */                           {   $$ = 0;         }
        | impassetsblocks                       {   $$ = $1;        }
        ;

impassetsblocks
        : impassetsblock                        {   $$ = $1;        }
        | impassetsblocks impassetsblock        {   $$ = $1 + $2;   }
        ;

impassetsblock
        : STRING AS INTEGER                     {
                                                    $$ = strlen($1)+3;
                                                    writeShort($3);
                                                    writeString($1);
                                                    numAssets++;
                                                }
        ;

expassets
        : EXPORTASSETS                          {   numAssets = 0;  }
          expassetsblocks_opt END               {
                                                    $$ = $3;
                                                    writeExportAssets(numAssets);
                                                    $$ = 0;
                                                }
        ;

expassetsblocks_opt
        : /* empty */                           {   $$ = 0;         }
        | expassetsblocks                       {   $$ = $1;        }
        ;

expassetsblocks
        : expassetsblock                        {   $$ = $1;        }
        | expassetsblocks expassetsblock        {   $$ = $1 + $2;   }
        ;

expassetsblock
        : INTEGER AS STRING                     {
                                                    $$ = strlen($3)+3;
                                                    writeShort($1);
                                                    writeString($3);
                                                    numAssets++;
                                                }
        ;

push_item
        : STRING                                {   $$ = writePushString($1);   }

        | CONSTANT                              {
                                                    if ($1 < 256) {
                                                        /* constant, 1-byte reference */
                                                        $$ = writeByte(0x08);
                                                        $$ += writeByte((byte)$1);
                                                    }
                                                    else {
                                                        /* constant, 2-byte reference */
                                                        $$ = writeByte(0x09);
                                                        $$ += writeShort($1);
                                                    }
                                                }
        | property                              {
                                                    $$ = writeByte(0x01);
                                                    $$ += writeFloat((float)$1);
                                                }

        | FLOAT                                 {
                                                    float f;
                                                    sscanf($1, "%f", &f);
                                                    $$ = writeByte(0x01);
                                                    $$ += writeFloat(f);
                                                }

        | _NANF                                 {
                                                    $$ = writeByte(0x01);
                                                    $$ += writeFloat(0.0f/0.0f);
                                                }

        | POSITIVE_INFINITYF                    {
                                                    $$ = writeByte(0x01);
                                                    $$ += writeFloat(1.0/0.0f);
                                                }

        | NEGATIVE_INFINITYF                    {
                                                    $$ = writeByte(0x01);
                                                    $$ += writeFloat(-1.0/0.0f);
                                                }

        | HEX                                   {
                                                    unsigned long int li = xtoi($1);
                                                    if (li > 65535)
                                                        yyerror("Hex number should be unsigned integer in the range from 0x0000 to 0xFFFF");

                                                    $$ = writeByte(0x07);
                                                    $$ += writeLongInt(xtoi($1));
                                                }

        | INTEGER                               {
                                                    $$ = writeByte(0x07);
                                                    $$ += writeLongInt($1);
                                                }

        | DOUBLE                                {
                                                    $$ = writeByte(0x06);
                                                    $$ += writeDouble(atof($1));
                                                }

        | _NAN                                  {
                                                    $$ = writeByte(0x06);
                                                    $$ += writeDouble(0.0/0.0);
                                                }

        | POSITIVE_INFINITY                     {
                                                    $$ = writeByte(0x06);
                                                    $$ += writeDouble(1.0/0.0);
                                                }

        | NEGATIVE_INFINITY                     {
                                                    $$ = writeByte(0x06);
                                                    $$ += writeDouble(-1.0/0.0);
                                                }

        | TRUEVAL                               {
                                                    $$ = writeByte(0x05);
                                                    $$ += writeByte(1);
                                                }

        | FALSEVAL                              {
                                                    $$ = writeByte(0x05);
                                                    $$ += writeByte(0);
                                                }

        | NULLVAL                               {   $$ = writeByte(2);  }

        | UNDEFVAL                              {   $$ = writeByte(3);  }

        | register                              {
                                                    $$ = writeByte(0x04);
                                                    $$ += writeByte((byte)$1);
                                                }
        ;

property
        : X_PROPERTY                            {   $$ = 0;     }
        | Y_PROPERTY                            {   $$ = 1;     }
        | XSCALE_PROPERTY                       {   $$ = 2;     }
        | YSCALE_PROPERTY                       {   $$ = 3;     }
        | CURRENTFRAME_PROPERTY                 {   $$ = 4;     }
        | TOTALFRAMES_PROPERTY                  {   $$ = 5;     }
        | ALPHA_PROPERTY                        {   $$ = 6;     }
        | VISIBLE_PROPERTY                      {   $$ = 7;     }
        | WIDTH_PROPERTY                        {   $$ = 8;     }
        | HEIGHT_PROPERTY                       {   $$ = 9;     }
        | ROTATION_PROPERTY                     {   $$ = 10;    }
        | TARGET_PROPERTY                       {   $$ = 11;    }
        | FRAMESLOADED_PROPERTY                 {   $$ = 12;    }
        | NAME_PROPERTY                         {   $$ = 13;    }
        | DROPTARGET_PROPERTY                   {   $$ = 14;    }
        | URL_PROPERTY                          {   $$ = 15;    }
        | HIGHQUALITY_PROPERTY                  {   $$ = 16;    }
        | FOCUSRECT_PROPERTY                    {   $$ = 17;    }
        | SOUNDBUFTIME_PROPERTY                 {   $$ = 18;    }
        | QUALITY_PROPERTY                      {   $$ = 19;    }
        | XMOUSE_PROPERTY                       {   $$ = 20;    }
        | YMOUSE_PROPERTY                       {   $$ = 21;    }
        ;

push_list
        : push_item                             {   $$ = $1;    }
        | push_list ',' push_item               {   $$ += $3;   }
        ;

constant_list
        : STRING                                {   addConstant($1);    }
        | constant_list ',' STRING              {   addConstant($3);    }
        ;

constant_list_opt
        : /* empty */                           {   }
        | constant_list                         {   }
        ;

hex_list
        : HEX                                   {
                                                    if (xtoi($1)>0xff)
                                                        yyerror("Action data must be a byte list");
                                                    $$ = writeByte((char)xtoi($1));
                                                }

        | hex_list ',' HEX                      {
                                                    if (xtoi($3)>0xff)
                                                        yyerror("Action data must be a byte list");
                                                    $$ += writeByte((char)xtoi($3));
                                                }
        ;

hexlist_opt
        : /* empty */                           {   $$ = 0;     }
        | HEXDATA hex_list                      {   $$ = $2;    }
        ;

urlmethod
        : /* empty */                           {   $$ = 0;     }
        | GET                                   {   $$ = 1;     }
        | POST                                  {   $$ = 2;     }
        ;

opcode
        : CONSTANTPOOL                          {   nConstants = 0;         }
          constant_list_opt                     {   $$ = writeConstants();  }

        | PUSH                                  {
                                                    $<num>$ = writeByte(SWFACTION_PUSHDATA);
                                                    /* length */
                                                    $<num>$ += writeShort(0);
                                                }

          push_list                             {   
                                                    $$ = $<num>2 + $3;
                                                    patchLength($3, $3);
                                                }

        | SWFACTION HEX                         {
                                                    if (xtoi($2)>0xff)
                                                        yyerror("Action code out of range");
                                                    $<num>$ = writeByte((char)xtoi($2));
                                                    if (xtoi($2)>=0x80)
                                                    /* length */
                                                    $<num>$ += writeShort(0);
                                                }

          hexlist_opt                           {
                                                    if (($4>0) && (xtoi($2)>=0x80))
                                                        patchLength($4, $4);
                                                    $$ = $<num>3 + $4;
                                                }

        | SETREGISTER register                  {
                                                    $$ = writeByte(SWFACTION_SETREGISTER);
                                                    $$ += writeShort(1);
                                                    $$ += writeByte((byte)$2);
                                                }

        | STRICTEQUALS                          {   $$ = writeByte(SWFACTION_STRICTEQUALS);     }
        | GREATERTHAN                           {   $$ = writeByte(SWFACTION_GREATERTHAN);      }
        | ENUMERATEVALUE                        {   $$ = writeByte(SWFACTION_ENUMERATEVALUE);   }
        | INSTANCEOF                            {   $$ = writeByte(SWFACTION_INSTANCEOF);       }
        | NEXTFRAME                             {   $$ = writeByte(SWFACTION_NEXTFRAME);        }
        | PREVFRAME                             {   $$ = writeByte(SWFACTION_PREVFRAME);        }
        | PLAY                                  {   $$ = writeByte(SWFACTION_PLAY);             }
        | STOP                                  {   $$ = writeByte(SWFACTION_STOP);             }
        | TOGGLEQUALITY                         {   $$ = writeByte(SWFACTION_TOGGLEQUALITY);    }
        | STOPSOUNDS                            {   $$ = writeByte(SWFACTION_STOPSOUNDS);       }
        | CALLFUNCTION                          {   $$ = writeByte(SWFACTION_CALLFUNCTION);     }
        | RETURN                                {   $$ = writeByte(SWFACTION_RETURN);           }
        | NEWMETHOD                             {   $$ = writeByte(SWFACTION_NEWMETHOD);        }
        | CALLMETHOD                            {   $$ = writeByte(SWFACTION_CALLMETHOD);       }
        | BITWISEAND                            {   $$ = writeByte(SWFACTION_BITWISEAND);       }
        | BITWISEOR                             {   $$ = writeByte(SWFACTION_BITWISEOR);        }
        | BITWISEXOR                            {   $$ = writeByte(SWFACTION_BITWISEXOR);       }
        | MODULO                                {   $$ = writeByte(SWFACTION_MODULO);           }
        | NEWADD                                {   $$ = writeByte(SWFACTION_NEWADD);           }
        | NEWLESSTHAN                           {   $$ = writeByte(SWFACTION_NEWLESSTHAN);      }
        | NEWEQUALS                             {   $$ = writeByte(SWFACTION_NEWEQUALS);        }
        | TONUMBER                              {   $$ = writeByte(SWFACTION_TONUMBER);         }
        | TOSTRING                              {   $$ = writeByte(SWFACTION_TOSTRING);         }
        | INCREMENT                             {   $$ = writeByte(SWFACTION_INCREMENT);        }
        | DECREMENT                             {   $$ = writeByte(SWFACTION_DECREMENT);        }
        | TYPEOF                                {   $$ = writeByte(SWFACTION_TYPEOF);           }
        | TARGETPATH                            {   $$ = writeByte(SWFACTION_TARGETPATH);       }
        | ENUMERATE                             {   $$ = writeByte(SWFACTION_ENUMERATE);        }
        | DELETE                                {   $$ = writeByte(SWFACTION_DELETE);           }
        | DELETE2                               {   $$ = writeByte(SWFACTION_DELETE2);          }
        | NEW                                   {   $$ = writeByte(SWFACTION_NEW);              }
        | INITARRAY                             {   $$ = writeByte(SWFACTION_INITARRAY);        }
        | INITOBJECT                            {   $$ = writeByte(SWFACTION_INITOBJECT);       }
        | GETMEMBER                             {   $$ = writeByte(SWFACTION_GETMEMBER);        }
        | SETMEMBER                             {   $$ = writeByte(SWFACTION_SETMEMBER);        }
        | SHIFTLEFT                             {   $$ = writeByte(SWFACTION_SHIFTLEFT);        }
        | SHIFTRIGHT                            {   $$ = writeByte(SWFACTION_SHIFTRIGHT);       }
        | SHIFTRIGHT2                           {   $$ = writeByte(SWFACTION_SHIFTRIGHT2);      }
        | VAR                                   {   $$ = writeByte(SWFACTION_VAR);              }
        | VAREQUALS                             {   $$ = writeByte(SWFACTION_VAREQUALS);        }
        | OLDADD                                {   $$ = writeByte(SWFACTION_ADD);              }
        | SUBTRACT                              {   $$ = writeByte(SWFACTION_SUBTRACT);         }
        | MULTIPLY                              {   $$ = writeByte(SWFACTION_MULTIPLY);         }
        | DIVIDE                                {   $$ = writeByte(SWFACTION_DIVIDE);           }
        | OLDEQUALS                             {   $$ = writeByte(SWFACTION_EQUALS);           }
        | OLDLESSTHAN                           {   $$ = writeByte(SWFACTION_LESSTHAN);         }
        | FSCOMMAND2                            {   $$ = writeByte(SWFACTION_FSCOMMAND2);       }
        | LOGICALAND                            {   $$ = writeByte(SWFACTION_LOGICALAND);       }
        | LOGICALOR                             {   $$ = writeByte(SWFACTION_LOGICALOR);        }
        | LOGICALNOT LOGICALNOT                 {
                                                    if (mode >= MODE_UPDATE) {
                                                        /* strip double nots */
                                                        $$ = 0;
                                                        numActions -= 2;
                                                    }
                                                    else {
                                                        $$ = writeByte(SWFACTION_LOGICALNOT);
                                                        $$ += writeByte(SWFACTION_LOGICALNOT);
                                                    }
                                                }
        | LOGICALNOT                            {   $$ = writeByte(SWFACTION_LOGICALNOT);       }
        | STRINGEQ                              {   $$ = writeByte(SWFACTION_STRINGEQ);         }
        | STRINGLENGTH                          {   $$ = writeByte(SWFACTION_STRINGLENGTH);     }
        | SUBSTRING                             {   $$ = writeByte(SWFACTION_SUBSTRING);        }
        | INT                                   {   $$ = writeByte(SWFACTION_INT);              }
        | DUP                                   {   $$ = writeByte(SWFACTION_DUP);              }
        | SWAP                                  {   $$ = writeByte(SWFACTION_SWAP);             }
        | POP                                   {   $$ = writeByte(SWFACTION_POP);              }
        | GETVARIABLE                           {   $$ = writeByte(SWFACTION_GETVARIABLE);      }
        | SETVARIABLE                           {   $$ = writeByte(SWFACTION_SETVARIABLE);      }
        | STRINGCONCAT                          {   $$ = writeByte(SWFACTION_STRINGCONCAT);     }
        | GETPROPERTY                           {   $$ = writeByte(SWFACTION_GETPROPERTY);      }
        | SETPROPERTY                           {   $$ = writeByte(SWFACTION_SETPROPERTY);      }
        | DUPLICATECLIP                         {   $$ = writeByte(SWFACTION_DUPLICATECLIP);    }
        | REMOVECLIP                            {   $$ = writeByte(SWFACTION_REMOVECLIP);       }
        | TRACE                                 {   $$ = writeByte(SWFACTION_TRACE);            }
        | STARTDRAGMOVIE                        {   $$ = writeByte(SWFACTION_STARTDRAGMOVIE);   }
        | STOPDRAGMOVIE                         {   $$ = writeByte(SWFACTION_STOPDRAGMOVIE);    }
        | STRINGLESSTHAN                        {   $$ = writeByte(SWFACTION_STRINGLESSTHAN);   }
        | STRINGGREATERTHAN                     {   $$ = writeByte(SWFACTION_STRINGGREATERTHAN);}
        | RANDOM                                {   $$ = writeByte(SWFACTION_RANDOM);           }
        | MBLENGTH                              {   $$ = writeByte(SWFACTION_MBLENGTH);         }
        | ORD                                   {   $$ = writeByte(SWFACTION_ORD);              }
        | CHR                                   {   $$ = writeByte(SWFACTION_CHR);              }
        | GETTIMER                              {   $$ = writeByte(SWFACTION_GETTIMER);         }
        | MBSUBSTRING                           {   $$ = writeByte(SWFACTION_MBSUBSTRING);      }
        | MBORD                                 {   $$ = writeByte(SWFACTION_MBORD);            }
        | MBCHR                                 {   $$ = writeByte(SWFACTION_MBCHR);            }
        | IMPLEMENTS                            {   $$ = writeByte(SWFACTION_IMPLEMENTS);       }
        | EXTENDS                               {   $$ = writeByte(SWFACTION_EXTENDS);          }
        | THROW                                 {   $$ = writeByte(SWFACTION_THROW);            }
        | CAST                                  {   $$ = writeByte(SWFACTION_CAST);             }

        | CALLFRAME                             {
                                                    $$ = writeByte(SWFACTION_CALLFRAME);
                                                    $$ += writeShort(0);
                                                }

        | GOTOANDSTOP                           {   
                                                    $$ = writeByte(SWFACTION_GOTOEXPRESSION);
                                                    $$ += writeShort(1);
                                                    $$ += writeByte(0);
                                                }

        | GOTOANDSTOP SKIP INTEGER              {   
                                                    $$ = writeByte(SWFACTION_GOTOEXPRESSION);
                                                    $$ += writeShort(3);
                                                    $$ += writeByte(2);
                                                    $$ += writeShort($3);
                                                }

        | GOTOANDPLAY                           {
                                                    $$ = writeByte(SWFACTION_GOTOEXPRESSION);
                                                    $$ += writeShort(1);
                                                    $$ += writeByte(1);
                                                }

        | GOTOANDPLAY SKIP INTEGER              {
                                                    $$ = writeByte(SWFACTION_GOTOEXPRESSION);
                                                    $$ += writeShort(3);
                                                    $$ += writeByte(3);
                                                    $$ += writeShort($3);
                                                }

        | GOTOLABEL STRING                      {
                                                    $$ = writeByte(SWFACTION_GOTOLABEL);
                                                    $$ += writeShort(strlen($2)+1);
                                                    $$ += writeString($2);
                                                }

        | BRANCHALWAYS STRING                   {
                                                    $$ = writeByte(SWFACTION_BRANCHALWAYS);
                                                    $$ += writeShort(2);
                                                    $$ += branchTarget($2);
                                                }

        | BRANCHALWAYS INTEGER                  {
                                                    $$ = writeByte(SWFACTION_BRANCHALWAYS);
                                                    $$ += writeShort(2);
                                                    $$ += addNumLabel($2);
                                                }

        | BRANCHIFTRUE STRING                   {
                                                    $$ = writeByte(SWFACTION_BRANCHIFTRUE);
                                                    $$ += writeShort(2);
                                                    $$ += branchTarget($2);
                                                }

        | BRANCHIFTRUE INTEGER                  {
                                                    $$ = writeByte(SWFACTION_BRANCHIFTRUE);
                                                    $$ += writeShort(2);
                                                    $$ += addNumLabel($2);
                                                }

        | GOTOFRAME INTEGER                     {
                                                    $$ = writeByte(SWFACTION_GOTOFRAME);
                                                    $$ += writeShort(2);
                                                    $$ += writeShort($2);
                                                }

        | GETURL STRING STRING                  {
                                                    $$ = writeByte(SWFACTION_GETURL);
                                                    $$ += writeShort(strlen($2)+strlen($3)+2);
                                                    $$ += writeString($2); 
                                                    $$ += writeString($3);
                                                }

        | GETURL2 urlmethod                     {
                                                    $$ = writeByte(SWFACTION_GETURL2);
                                                    $$ += writeShort(1);
                                                    $$ += writeByte($2);
                                                }

        | LOADVARIABLES urlmethod               {
                                                    $$ = writeByte(SWFACTION_GETURL2);
                                                    $$ += writeShort(1);
                                                    $$ += writeByte(0xc0 + $2);
                                                }

        | LOADVARIABLESNUM urlmethod            {
                                                    $$ = writeByte(SWFACTION_GETURL2);
                                                    $$ += writeShort(1);
                                                    $$ += writeByte(0x80 + $2);
                                                }

        | LOADMOVIE urlmethod                   {
                                                    $$ = writeByte(SWFACTION_GETURL2);
                                                    $$ += writeShort(1);
                                                    $$ += writeByte(0x40 + $2);
                                                }

        | LOADMOVIENUM urlmethod                {
                                                    $$ = writeByte(SWFACTION_GETURL2);
                                                    $$ += writeShort(1);
                                                    $$ += writeByte($2);
                                                }

        | STRICTMODE ON                         {
                                                    $$ = writeByte(SWFACTION_STRICTMODE);
                                                    $$ += writeShort(1);
                                                    $$ += writeByte(1);
                                                }

        | STRICTMODE OFF                        {
                                                    $$ = writeByte(SWFACTION_STRICTMODE);
                                                    $$ += writeShort(1);
                                                    $$ += writeByte(0);
                                                }

        | MOVIE STRING                          {   yyerror("Movie declaration inside of the action block");    }
        ;
