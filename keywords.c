/* ANSI-C code produced by gperf version 3.0.3 */
/* Command-line: gperf --language=ANSI-C -t -T -E -o -k '1,$,2,5' -S8 keywords.gperf  */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

#line 1 "keywords.gperf"

/*
flasm, command line assembler & disassembler of flash actionscript bytecode
Copyright (c) 2001 Opaque Industries, (c) 2002-2007 Igor Kogan
All rights reserved. See LICENSE.TXT for terms of use.
*/

#include <string.h>
#include "assembler.tab.h"
#include "util.h"
#ifdef MEMWATCH
#include "memwatch.h"
#endif
struct keyword *in_word_set(register const char *str, register unsigned int len);
/* maximum key range = 697, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash (register const char *str, register unsigned int len)
{
  static unsigned short asso_values[] =
    {
      706, 706, 706, 706, 706, 706, 706, 706, 706, 706,
      706, 706, 706, 706, 706, 706, 706, 706, 706, 706,
      706, 706, 706, 706, 706, 706, 706, 706, 706, 706,
      706, 706, 706, 706, 706, 706, 706, 706, 706, 706,
      706, 706, 706, 706, 706, 706, 706, 706, 706, 706,
       30,   0, 706, 706, 706, 706, 706, 706, 706, 706,
      706, 706, 706, 706, 706, 706, 706, 706, 706, 706,
      706, 706, 706, 706, 706, 706, 706, 706, 706, 706,
      706, 706, 706, 706, 706, 706, 706, 706, 706, 706,
      706, 706, 706, 706, 706, 255, 706,  40, 205, 239,
       45,   0,  40, 155, 190,  75, 706, 235, 125, 105,
       15,  60,  50, 134,   5,  30,   0, 190, 105, 175,
      210,  75, 706, 706, 706, 706, 706, 706, 706, 706,
      706, 706, 706, 706, 706, 706, 706, 706, 706, 706,
      706, 706, 706, 706, 706, 706, 706, 706, 706, 706,
      706, 706, 706, 706, 706, 706, 706, 706, 706, 706,
      706, 706, 706, 706, 706, 706, 706, 706, 706, 706,
      706, 706, 706, 706, 706, 706, 706, 706, 706, 706,
      706, 706, 706, 706, 706, 706, 706, 706, 706, 706,
      706, 706, 706, 706, 706, 706, 706, 706, 706, 706,
      706, 706, 706, 706, 706, 706, 706, 706, 706, 706,
      706, 706, 706, 706, 706, 706, 706, 706, 706, 706,
      706, 706, 706, 706, 706, 706, 706, 706, 706, 706,
      706, 706, 706, 706, 706, 706, 706, 706, 706, 706,
      706, 706, 706, 706, 706, 706, 706, 706, 706, 706,
      706, 706, 706, 706, 706, 706
    };
  register int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[4]];
      /*FALLTHROUGH*/
      case 4:
      case 3:
      case 2:
        hval += asso_values[(unsigned char)str[1]];
      /*FALLTHROUGH*/
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

#ifdef __GNUC__
__inline
#ifdef __GNUC_STDC_INLINE__
__attribute__ ((__gnu_inline__))
#endif
#endif
struct keyword *
in_word_set (register const char *str, register unsigned int len)
{
  enum
    {
      TOTAL_KEYWORDS = 219,
      MIN_WORD_LENGTH = 2,
      MAX_WORD_LENGTH = 28,
      MIN_HASH_VALUE = 9,
      MAX_HASH_VALUE = 705
    };

  static struct keyword wordlist[] =
    {
#line 140 "keywords.gperf"
      {"true",                   0,              TRUEVAL},
#line 219 "keywords.gperf"
      {"trace",                  1,              TRACE},
#line 188 "keywords.gperf"
      {"enumerate",              1,              ENUMERATE},
#line 187 "keywords.gperf"
      {"enumeratevalue",         1,              ENUMERATEVALUE},
#line 120 "keywords.gperf"
      {"enterframe",             0,              MCENTERFRAME},
#line 163 "keywords.gperf"
      {"return",                 1,              RETURN},
#line 268 "keywords.gperf"
      {"recursion",              0,              RECURSION},
#line 175 "keywords.gperf"
      {"newlessthan",            1,              NEWLESSTHAN},
#line 196 "keywords.gperf"
      {"setmember",              1,              SETMEMBER},
#line 161 "keywords.gperf"
      {"setregister",            1,              SETREGISTER},
#line 35 "keywords.gperf"
      {"frame",                 -1,              FRAME},
#line 190 "keywords.gperf"
      {"delete",                 1,              DELETE},
#line 131 "keywords.gperf"
      {"release",                0,              MCRELEASE},
#line 184 "keywords.gperf"
      {"decrement",              1,              DECREMENT},
#line 132 "keywords.gperf"
      {"releaseoutside",         0,              MCRELEASEOUTSIDE},
#line 19 "keywords.gperf"
      {"protect",                0,              PROTECT},
#line 30 "keywords.gperf"
      {"end",                    0,              END},
#line 147 "keywords.gperf"
      {"nextframe",              1,              NEXTFRAME},
#line 164 "keywords.gperf"
      {"newmethod",              1,              NEWMETHOD},
#line 206 "keywords.gperf"
      {"or",                     1,              LOGICALOR},
#line 59 "keywords.gperf"
      {"negative_infinityf",     0,              NEGATIVE_INFINITYF},
#line 222 "keywords.gperf"
      {"stringlessthan",         1,              STRINGLESSTHAN},
#line 223 "keywords.gperf"
      {"stringgreaterthan",      1,              STRINGGREATERTHAN},
#line 207 "keywords.gperf"
      {"not",                    1,              LOGICALNOT},
#line 47 "keywords.gperf"
      {"settarget",              2,              SETTARGET},
#line 213 "keywords.gperf"
      {"setvariable",            1,              SETVARIABLE},
#line 191 "keywords.gperf"
      {"delete2",                1,              DELETE2},
#line 42 "keywords.gperf"
      {"try",                    1,              TRY},
#line 153 "keywords.gperf"
      {"stop",                   1,              STOP},
#line 141 "keywords.gperf"
      {"false",                  0,              FALSEVAL},
#line 29 "keywords.gperf"
      {"definebutton",           0,              DEFINEBUTTON},
#line 48 "keywords.gperf"
      {"settargetexpr",          2,              SETTARGETEXPR},
#line 254 "keywords.gperf"
      {"attrrelativeurls",             0,        ATTRRELATIVEURLS},
#line 36 "keywords.gperf"
      {"on",                    -1,              ON},
#line 211 "keywords.gperf"
      {"int",                    1,              INT},
#line 256 "keywords.gperf"
      {"attractionscript3",            0,        ATTRACTIONSCRIPT3},
#line 255 "keywords.gperf"
      {"attrsuppresscrossdomaincache", 0,        ATTRSUPPRESSCROSSDOMAINCACHE},
#line 183 "keywords.gperf"
      {"increment",              1,              INCREMENT},
#line 154 "keywords.gperf"
      {"stopsounds",             1,              STOPSOUNDS},
#line 262 "keywords.gperf"
      {"as",                     0,              AS},
#line 205 "keywords.gperf"
      {"and",                    1,              LOGICALAND},
#line 148 "keywords.gperf"
      {"prevframe",              1,              PREVFRAME},
#line 56 "keywords.gperf"
      {"negative_infinity",      0,              NEGATIVE_INFINITY},
#line 172 "keywords.gperf"
      {"newadd",                 1,              NEWADD},
#line 226 "keywords.gperf"
      {"ord",                    1,              ORD},
#line 263 "keywords.gperf"
      {"post",                   0,              POST},
#line 136 "keywords.gperf"
      {"dragout",                0,              MCDRAGOUT},
#line 130 "keywords.gperf"
      {"press",                  0,              MCPRESS},
#line 216 "keywords.gperf"
      {"setproperty",            1,              SETPROPERTY},
#line 135 "keywords.gperf"
      {"dragover",               0,              MCDRAGOVER},
#line 28 "keywords.gperf"
      {"definemovieclip",        0,              DEFINEMOVIECLIP},
#line 127 "keywords.gperf"
      {"data",                   0,              MCDATA},
#line 77 "keywords.gperf"
      {"target_property",        0,              TARGET_PROPERTY},
#line 134 "keywords.gperf"
      {"rollout",                0,              MCROLLOUT},
#line 171 "keywords.gperf"
      {"add",                    1,              NEWADD},
#line 133 "keywords.gperf"
      {"rollover",               0,              MCROLLOVER},
#line 78 "keywords.gperf"
      {"framesloaded_property",  0,              FRAMESLOADED_PROPERTY},
#line 269 "keywords.gperf"
      {"timeout",                0,              TIMEOUT},
#line 266 "keywords.gperf"
      {"off",                    0,              OFF},
#line 80 "keywords.gperf"
      {"droptarget_property",    0,              DROPTARGET_PROPERTY},
#line 174 "keywords.gperf"
      {"lessthan",               1,              NEWLESSTHAN},
#line 95 "keywords.gperf"
      {"idletooverdown",         0,              BIDLETOOVERDOWN},
#line 200 "keywords.gperf"
      {"var",                    1,              VAR},
#line 267 "keywords.gperf"
      {"from",                   0,              FROM},
#line 71 "keywords.gperf"
      {"rotation_property",      0,              ROTATION_PROPERTY},
#line 264 "keywords.gperf"
      {"get",                    0,              GET},
#line 21 "keywords.gperf"
      {"enabledebugger",         0,              ENABLEDEBUGGER},
#line 193 "keywords.gperf"
      {"initobject",             1,              INITOBJECT},
#line 37 "keywords.gperf"
      {"onclipevent",           -1,              ONCLIPEVENT},
#line 158 "keywords.gperf"
      {"pop",                    1,              POP},
#line 58 "keywords.gperf"
      {"positive_infinityf",     0,              POSITIVE_INFINITYF},
#line 195 "keywords.gperf"
      {"getmember",              1,              GETMEMBER},
#line 218 "keywords.gperf"
      {"removeclip",             1,              REMOVECLIP},
#line 204 "keywords.gperf"
      {"divide",                 1,              DIVIDE},
#line 122 "keywords.gperf"
      {"mousemove",              0,              MCMOUSEMOVE},
#line 128 "keywords.gperf"
      {"initialize",             0,              MCINITIALIZE},
#line 45 "keywords.gperf"
      {"ifframeloadedexpr",      1,              IFFRAMELOADEDEXPR},
#line 181 "keywords.gperf"
      {"tonumber",               1,              TONUMBER},
#line 189 "keywords.gperf"
      {"instanceof",             1,              INSTANCEOF},
#line 185 "keywords.gperf"
      {"typeof",                 1,              TYPEOF},
#line 88 "keywords.gperf"
      {"idletooverup",           0,              BIDLETOOVERUP},
#line 22 "keywords.gperf"
      {"enabledebugger2",        0,              ENABLEDEBUGGER2},
#line 176 "keywords.gperf"
      {"greaterthan",            1,              GREATERTHAN},
#line 208 "keywords.gperf"
      {"stringeq",               1,              STRINGEQ},
#line 179 "keywords.gperf"
      {"newequals",              1,              NEWEQUALS},
#line 123 "keywords.gperf"
      {"mousedown",              0,              MCMOUSEDOWN},
#line 192 "keywords.gperf"
      {"new",                    1,              NEW},
#line 220 "keywords.gperf"
      {"startdrag",              1,              STARTDRAGMOVIE},
#line 235 "keywords.gperf"
      {"geturl2",                1,              GETURL2},
#line 23 "keywords.gperf"
      {"metadata",               0,              METADATA},
#line 24 "keywords.gperf"
      {"fileattributes",         0,              FILEATTRIBUTES},
#line 55 "keywords.gperf"
      {"positive_infinity",      0,              POSITIVE_INFINITY},
#line 212 "keywords.gperf"
      {"getvariable",            1,              GETVARIABLE},
#line 173 "keywords.gperf"
      {"oldlessthan",            1,              OLDLESSTHAN},
#line 46 "keywords.gperf"
      {"ifframeloaded",          1,              IFFRAMELOADED},
#line 194 "keywords.gperf"
      {"initarray",              1,              INITARRAY},
#line 248 "keywords.gperf"
      {"fscommand2",             1,              FSCOMMAND2},
#line 224 "keywords.gperf"
      {"random",                 1,              RANDOM},
#line 244 "keywords.gperf"
      {"implements",             1,              IMPLEMENTS},
#line 124 "keywords.gperf"
      {"mouseup",                0,              MCMOUSEUP},
#line 73 "keywords.gperf"
      {"focusrect_property",     0,              FOCUSRECT_PROPERTY},
#line 96 "keywords.gperf"
      {"overdowntoidle",         0,              BOVERDOWNTOIDLE},
#line 25 "keywords.gperf"
      {"importassets",           0,              IMPORTASSETS},
#line 182 "keywords.gperf"
      {"tostring",               1,              TOSTRING},
#line 197 "keywords.gperf"
      {"shiftleft",              1,              SHIFTLEFT},
#line 198 "keywords.gperf"
      {"shiftright",             1,              SHIFTRIGHT},
#line 74 "keywords.gperf"
      {"soundbuftime_property",  0,              SOUNDBUFTIME_PROPERTY},
#line 202 "keywords.gperf"
      {"subtract",               1,              SUBTRACT},
#line 119 "keywords.gperf"
      {"load",                   0,              MCLOAD},
#line 221 "keywords.gperf"
      {"stopdrag",               1,              STOPDRAGMOVIE},
#line 27 "keywords.gperf"
      {"placemovieclip",         0,              PLACEMOVIECLIP},
#line 186 "keywords.gperf"
      {"targetpath",             1,              TARGETPATH},
#line 92 "keywords.gperf"
      {"overdowntooutdown",      0,              BOVERDOWNTOOUTDOWN},
#line 228 "keywords.gperf"
      {"gettimer",               1,              GETTIMER},
#line 215 "keywords.gperf"
      {"getproperty",            1,              GETPROPERTY},
#line 209 "keywords.gperf"
      {"stringlength",           1,              STRINGLENGTH},
#line 49 "keywords.gperf"
      {"function",               1,              FUNCTION},
#line 152 "keywords.gperf"
      {"play",                   1,              PLAY},
#line 26 "keywords.gperf"
      {"exportassets",           0,              EXPORTASSETS},
#line 34 "keywords.gperf"
      {"initmovieclip",         -1,              INITMOVIECLIP},
#line 157 "keywords.gperf"
      {"swap",                   1,              SWAP},
#line 199 "keywords.gperf"
      {"shiftright2",            1,              SHIFTRIGHT2},
#line 245 "keywords.gperf"
      {"extends",                1,              EXTENDS},
#line 149 "keywords.gperf"
      {"gotoframe",              1,              GOTOFRAME},
#line 108 "keywords.gperf"
      {"_enter",                 0,              _ENTER},
#line 50 "keywords.gperf"
      {"function2",              1,              FUNCTION2},
#line 155 "keywords.gperf"
      {"togglequality",          1,              TOGGLEQUALITY},
#line 91 "keywords.gperf"
      {"overdowntooverup",       0,              BOVERDOWNTOOVERUP},
#line 265 "keywords.gperf"
      {"hexdata",                0,              HEXDATA},
#line 97 "keywords.gperf"
      {"keypress",               0,              KEYPRESS},
#line 243 "keywords.gperf"
      {"strictmode",             1,              STRICTMODE},
#line 76 "keywords.gperf"
      {"totalframes_property",   0,              TOTALFRAMES_PROPERTY},
#line 170 "keywords.gperf"
      {"oldadd",                 1,              OLDADD},
#line 247 "keywords.gperf"
      {"cast",                   1,              CAST},
#line 257 "keywords.gperf"
      {"attrhasmetadata",              0,        ATTRHASMETADATA},
#line 156 "keywords.gperf"
      {"dup",                    1,              DUP},
#line 54 "keywords.gperf"
      {"_nan",                   0,              _NAN},
#line 143 "keywords.gperf"
      {"undef",                  0,              UNDEFVAL},
#line 151 "keywords.gperf"
      {"geturl",                 1,              GETURL},
#line 69 "keywords.gperf"
      {"alpha_property",         0,              ALPHA_PROPERTY},
#line 178 "keywords.gperf"
      {"equals",                 1,              NEWEQUALS},
#line 121 "keywords.gperf"
      {"unload",                 0,              MCUNLOAD},
#line 238 "keywords.gperf"
      {"loadmovie",              1,              LOADMOVIE},
#line 84 "keywords.gperf"
      {"ymouse_property",        0,              YMOUSE_PROPERTY},
#line 114 "keywords.gperf"
      {"_escape",                0,              _ESCAPE},
#line 104 "keywords.gperf"
      {"_end",                   0,              _END},
#line 106 "keywords.gperf"
      {"_delete",                0,              _DELETE},
#line 129 "keywords.gperf"
      {"construct",              0,              MCCONSTRUCT},
#line 180 "keywords.gperf"
      {"strictequals",           1,              STRICTEQUALS},
#line 242 "keywords.gperf"
      {"gotoandstop",            1,              GOTOANDSTOP},
#line 125 "keywords.gperf"
      {"keydown",                0,              MCKEYDOWN},
#line 201 "keywords.gperf"
      {"varequals",              1,              VAREQUALS},
#line 261 "keywords.gperf"
      {"skip",                   0,              SKIP},
#line 66 "keywords.gperf"
      {"yscale_property",        0,              YSCALE_PROPERTY},
#line 44 "keywords.gperf"
      {"finally",                0,              FINALLY},
#line 94 "keywords.gperf"
      {"outdowntoidle",          0,              BOUTDOWNTOIDLE},
#line 231 "keywords.gperf"
      {"mbchr",                  1,              MBCHR},
#line 240 "keywords.gperf"
      {"callframe",              1,              CALLFRAME},
#line 112 "keywords.gperf"
      {"_pagedown",              0,              _PAGEDOWN},
#line 81 "keywords.gperf"
      {"url_property",           0,              URL_PROPERTY},
#line 236 "keywords.gperf"
      {"loadvariables",          1,              LOADVARIABLES},
#line 142 "keywords.gperf"
      {"null",                   0,              NULLVAL},
#line 110 "keywords.gperf"
      {"_down",                  0,              _DOWN},
#line 105 "keywords.gperf"
      {"_insert",                0,              _INSERT},
#line 160 "keywords.gperf"
      {"constants",              1,              CONSTANTPOOL},
#line 126 "keywords.gperf"
      {"keyup",                  0,              MCKEYUP},
#line 241 "keywords.gperf"
      {"gotoandplay",            1,              GOTOANDPLAY},
#line 93 "keywords.gperf"
      {"outdowntooverdown",      0,              BOUTDOWNTOOVERDOWN},
#line 214 "keywords.gperf"
      {"concat",                 1,              STRINGCONCAT},
#line 162 "keywords.gperf"
      {"callfunction",           1,              CALLFUNCTION},
#line 57 "keywords.gperf"
      {"_nanf",                  0,              _NANF},
#line 169 "keywords.gperf"
      {"modulo",                 1,              MODULO},
#line 177 "keywords.gperf"
      {"oldequals",              1,              OLDEQUALS},
#line 18 "keywords.gperf"
      {"compressed",             0,              COMPRESSED},
#line 20 "keywords.gperf"
      {"scriptlimits",           0,              SCRIPTLIMITS},
#line 111 "keywords.gperf"
      {"_pageup",                0,              _PAGEUP},
#line 89 "keywords.gperf"
      {"overuptoidle",           0,              BOVERUPTOIDLE},
#line 167 "keywords.gperf"
      {"bitwiseor",              1,              BITWISEOR},
#line 168 "keywords.gperf"
      {"bitwisexor",             1,              BITWISEXOR},
#line 217 "keywords.gperf"
      {"duplicateclip",          1,              DUPLICATECLIP},
#line 210 "keywords.gperf"
      {"substring",              1,              SUBSTRING},
#line 101 "keywords.gperf"
      {"_left",                  0,              _LEFT},
#line 90 "keywords.gperf"
      {"overuptooverdown",       0,              BOVERUPTOOVERDOWN},
#line 79 "keywords.gperf"
      {"name_property",          0,              NAME_PROPERTY},
#line 230 "keywords.gperf"
      {"mbord",                  1,              MBORD},
#line 239 "keywords.gperf"
      {"loadmovienum",           1,              LOADMOVIENUM},
#line 166 "keywords.gperf"
      {"bitwiseand",             1,              BITWISEAND},
#line 237 "keywords.gperf"
      {"loadvariablesnum",       1,              LOADVARIABLESNUM},
#line 159 "keywords.gperf"
      {"push",                   1,              PUSH},
#line 83 "keywords.gperf"
      {"xmouse_property",        0,              XMOUSE_PROPERTY},
#line 227 "keywords.gperf"
      {"chr",                    1,              CHR},
#line 165 "keywords.gperf"
      {"callmethod",             1,              CALLMETHOD},
#line 41 "keywords.gperf"
      {"with",                   1,              WITH},
#line 103 "keywords.gperf"
      {"_home",                  0,              _HOME},
#line 203 "keywords.gperf"
      {"multiply",               1,              MULTIPLY},
#line 65 "keywords.gperf"
      {"xscale_property",        0,              XSCALE_PROPERTY},
#line 102 "keywords.gperf"
      {"_right",                 0,              _RIGHT},
#line 234 "keywords.gperf"
      {"branchiftrue",           1,              BRANCHIFTRUE},
#line 113 "keywords.gperf"
      {"_tab",                   0,              _TAB},
#line 249 "keywords.gperf"
      {"swfaction",              1,              SWFACTION},
#line 68 "keywords.gperf"
      {"height_property",        0,              HEIGHT_PROPERTY},
#line 150 "keywords.gperf"
      {"gotolabel",              1,              GOTOLABEL},
#line 64 "keywords.gperf"
      {"y_property",             0,              Y_PROPERTY},
#line 70 "keywords.gperf"
      {"visible_property",       0,              VISIBLE_PROPERTY},
#line 253 "keywords.gperf"
      {"attrusenetwork",               0,        ATTRUSENETWORK},
#line 82 "keywords.gperf"
      {"quality_property",       0,              QUALITY_PROPERTY},
#line 233 "keywords.gperf"
      {"branchalways",           1,              BRANCHALWAYS},
#line 72 "keywords.gperf"
      {"highquality_property",   0,              HIGHQUALITY_PROPERTY},
#line 109 "keywords.gperf"
      {"_up",                    0,              _UP},
#line 225 "keywords.gperf"
      {"mblength",               1,              MBLENGTH},
#line 75 "keywords.gperf"
      {"currentframe_property",  0,              CURRENTFRAME_PROPERTY},
#line 67 "keywords.gperf"
      {"width_property",         0,              WIDTH_PROPERTY},
#line 115 "keywords.gperf"
      {"_space",                 0,              _SPACE},
#line 246 "keywords.gperf"
      {"throw",                  1,              THROW},
#line 63 "keywords.gperf"
      {"x_property",             0,              X_PROPERTY},
#line 232 "keywords.gperf"
      {"branch",                 1,              BRANCHALWAYS},
#line 43 "keywords.gperf"
      {"catch",                  0,              CATCH},
#line 229 "keywords.gperf"
      {"mbsubstring",            1,              MBSUBSTRING},
#line 107 "keywords.gperf"
      {"_backspace",             0,              _BACKSPACE}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= MIN_HASH_VALUE)
        {
          register struct keyword *resword;

          if (key < 239)
            {
              if (key < 138)
                {
                  if (key < 84)
                    {
                      switch (key - 9)
                        {
                          case 0:
                            resword = &wordlist[0];
                            goto compare;
                          case 1:
                            resword = &wordlist[1];
                            goto compare;
                          case 15:
                            resword = &wordlist[2];
                            goto compare;
                          case 20:
                            resword = &wordlist[3];
                            goto compare;
                          case 21:
                            resword = &wordlist[4];
                            goto compare;
                          case 22:
                            resword = &wordlist[5];
                            goto compare;
                          case 25:
                            resword = &wordlist[6];
                            goto compare;
                          case 32:
                            resword = &wordlist[7];
                            goto compare;
                          case 35:
                            resword = &wordlist[8];
                            goto compare;
                          case 37:
                            resword = &wordlist[9];
                            goto compare;
                          case 41:
                            resword = &wordlist[10];
                            goto compare;
                          case 42:
                            resword = &wordlist[11];
                            goto compare;
                          case 43:
                            resword = &wordlist[12];
                            goto compare;
                          case 45:
                            resword = &wordlist[13];
                            goto compare;
                          case 50:
                            resword = &wordlist[14];
                            goto compare;
                          case 53:
                            resword = &wordlist[15];
                            goto compare;
                          case 54:
                            resword = &wordlist[16];
                            goto compare;
                          case 55:
                            resword = &wordlist[17];
                            goto compare;
                          case 60:
                            resword = &wordlist[18];
                            goto compare;
                          case 63:
                            resword = &wordlist[19];
                            goto compare;
                          case 64:
                            resword = &wordlist[20];
                            goto compare;
                          case 65:
                            resword = &wordlist[21];
                            goto compare;
                          case 68:
                            resword = &wordlist[22];
                            goto compare;
                          case 69:
                            resword = &wordlist[23];
                            goto compare;
                          case 70:
                            resword = &wordlist[24];
                            goto compare;
                          case 72:
                            resword = &wordlist[25];
                            goto compare;
                          case 73:
                            resword = &wordlist[26];
                            goto compare;
                          case 74:
                            resword = &wordlist[27];
                            goto compare;
                        }
                    }
                  else
                    {
                      switch (key - 84)
                        {
                          case 0:
                            resword = &wordlist[28];
                            goto compare;
                          case 1:
                            resword = &wordlist[29];
                            goto compare;
                          case 3:
                            resword = &wordlist[30];
                            goto compare;
                          case 4:
                            resword = &wordlist[31];
                            goto compare;
                          case 7:
                            resword = &wordlist[32];
                            goto compare;
                          case 8:
                            resword = &wordlist[33];
                            goto compare;
                          case 9:
                            resword = &wordlist[34];
                            goto compare;
                          case 13:
                            resword = &wordlist[35];
                            goto compare;
                          case 14:
                            resword = &wordlist[36];
                            goto compare;
                          case 15:
                            resword = &wordlist[37];
                            goto compare;
                          case 16:
                            resword = &wordlist[38];
                            goto compare;
                          case 18:
                            resword = &wordlist[39];
                            goto compare;
                          case 19:
                            resword = &wordlist[40];
                            goto compare;
                          case 20:
                            resword = &wordlist[41];
                            goto compare;
                          case 23:
                            resword = &wordlist[42];
                            goto compare;
                          case 27:
                            resword = &wordlist[43];
                            goto compare;
                          case 29:
                            resword = &wordlist[44];
                            goto compare;
                          case 30:
                            resword = &wordlist[45];
                            goto compare;
                          case 33:
                            resword = &wordlist[46];
                            goto compare;
                          case 36:
                            resword = &wordlist[47];
                            goto compare;
                          case 37:
                            resword = &wordlist[48];
                            goto compare;
                          case 39:
                            resword = &wordlist[49];
                            goto compare;
                          case 41:
                            resword = &wordlist[50];
                            goto compare;
                          case 45:
                            resword = &wordlist[51];
                            goto compare;
                          case 46:
                            resword = &wordlist[52];
                            goto compare;
                          case 48:
                            resword = &wordlist[53];
                            goto compare;
                          case 49:
                            resword = &wordlist[54];
                            goto compare;
                        }
                    }
                }
              else
                {
                  if (key < 187)
                    {
                      switch (key - 138)
                        {
                          case 0:
                            resword = &wordlist[55];
                            goto compare;
                          case 3:
                            resword = &wordlist[56];
                            goto compare;
                          case 4:
                            resword = &wordlist[57];
                            goto compare;
                          case 5:
                            resword = &wordlist[58];
                            goto compare;
                          case 6:
                            resword = &wordlist[59];
                            goto compare;
                          case 10:
                            resword = &wordlist[60];
                            goto compare;
                          case 11:
                            resword = &wordlist[61];
                            goto compare;
                          case 15:
                            resword = &wordlist[62];
                            goto compare;
                          case 16:
                            resword = &wordlist[63];
                            goto compare;
                          case 19:
                            resword = &wordlist[64];
                            goto compare;
                          case 20:
                            resword = &wordlist[65];
                            goto compare;
                          case 21:
                            resword = &wordlist[66];
                            goto compare;
                          case 22:
                            resword = &wordlist[67];
                            goto compare;
                          case 23:
                            resword = &wordlist[68];
                            goto compare;
                          case 25:
                            resword = &wordlist[69];
                            goto compare;
                          case 30:
                            resword = &wordlist[70];
                            goto compare;
                          case 31:
                            resword = &wordlist[71];
                            goto compare;
                          case 32:
                            resword = &wordlist[72];
                            goto compare;
                          case 33:
                            resword = &wordlist[73];
                            goto compare;
                          case 36:
                            resword = &wordlist[74];
                            goto compare;
                          case 37:
                            resword = &wordlist[75];
                            goto compare;
                          case 39:
                            resword = &wordlist[76];
                            goto compare;
                          case 40:
                            resword = &wordlist[77];
                            goto compare;
                          case 42:
                            resword = &wordlist[78];
                            goto compare;
                          case 43:
                            resword = &wordlist[79];
                            goto compare;
                          case 44:
                            resword = &wordlist[80];
                            goto compare;
                          case 47:
                            resword = &wordlist[81];
                            goto compare;
                          case 48:
                            resword = &wordlist[82];
                            goto compare;
                        }
                    }
                  else
                    {
                      switch (key - 187)
                        {
                          case 0:
                            resword = &wordlist[83];
                            goto compare;
                          case 1:
                            resword = &wordlist[84];
                            goto compare;
                          case 2:
                            resword = &wordlist[85];
                            goto compare;
                          case 6:
                            resword = &wordlist[86];
                            goto compare;
                          case 7:
                            resword = &wordlist[87];
                            goto compare;
                          case 10:
                            resword = &wordlist[88];
                            goto compare;
                          case 11:
                            resword = &wordlist[89];
                            goto compare;
                          case 12:
                            resword = &wordlist[90];
                            goto compare;
                          case 15:
                            resword = &wordlist[91];
                            goto compare;
                          case 19:
                            resword = &wordlist[92];
                            goto compare;
                          case 24:
                            resword = &wordlist[93];
                            goto compare;
                          case 26:
                            resword = &wordlist[94];
                            goto compare;
                          case 27:
                            resword = &wordlist[95];
                            goto compare;
                          case 28:
                            resword = &wordlist[96];
                            goto compare;
                          case 29:
                            resword = &wordlist[97];
                            goto compare;
                          case 33:
                            resword = &wordlist[98];
                            goto compare;
                          case 35:
                            resword = &wordlist[99];
                            goto compare;
                          case 36:
                            resword = &wordlist[100];
                            goto compare;
                          case 37:
                            resword = &wordlist[101];
                            goto compare;
                          case 40:
                            resword = &wordlist[102];
                            goto compare;
                          case 41:
                            resword = &wordlist[103];
                            goto compare;
                          case 42:
                            resword = &wordlist[104];
                            goto compare;
                          case 43:
                            resword = &wordlist[105];
                            goto compare;
                          case 44:
                            resword = &wordlist[106];
                            goto compare;
                          case 46:
                            resword = &wordlist[107];
                            goto compare;
                          case 47:
                            resword = &wordlist[108];
                            goto compare;
                          case 51:
                            resword = &wordlist[109];
                            goto compare;
                        }
                    }
                }
            }
          else
            {
              if (key < 340)
                {
                  if (key < 291)
                    {
                      switch (key - 239)
                        {
                          case 0:
                            resword = &wordlist[110];
                            goto compare;
                          case 1:
                            resword = &wordlist[111];
                            goto compare;
                          case 3:
                            resword = &wordlist[112];
                            goto compare;
                          case 4:
                            resword = &wordlist[113];
                            goto compare;
                          case 7:
                            resword = &wordlist[114];
                            goto compare;
                          case 8:
                            resword = &wordlist[115];
                            goto compare;
                          case 14:
                            resword = &wordlist[116];
                            goto compare;
                          case 15:
                            resword = &wordlist[117];
                            goto compare;
                          case 18:
                            resword = &wordlist[118];
                            goto compare;
                          case 19:
                            resword = &wordlist[119];
                            goto compare;
                          case 20:
                            resword = &wordlist[120];
                            goto compare;
                          case 22:
                            resword = &wordlist[121];
                            goto compare;
                          case 23:
                            resword = &wordlist[122];
                            goto compare;
                          case 25:
                            resword = &wordlist[123];
                            goto compare;
                          case 27:
                            resword = &wordlist[124];
                            goto compare;
                          case 30:
                            resword = &wordlist[125];
                            goto compare;
                          case 34:
                            resword = &wordlist[126];
                            goto compare;
                          case 37:
                            resword = &wordlist[127];
                            goto compare;
                          case 38:
                            resword = &wordlist[128];
                            goto compare;
                          case 39:
                            resword = &wordlist[129];
                            goto compare;
                          case 40:
                            resword = &wordlist[130];
                            goto compare;
                          case 41:
                            resword = &wordlist[131];
                            goto compare;
                          case 42:
                            resword = &wordlist[132];
                            goto compare;
                          case 44:
                            resword = &wordlist[133];
                            goto compare;
                          case 46:
                            resword = &wordlist[134];
                            goto compare;
                          case 49:
                            resword = &wordlist[135];
                            goto compare;
                          case 50:
                            resword = &wordlist[136];
                            goto compare;
                          case 51:
                            resword = &wordlist[137];
                            goto compare;
                        }
                    }
                  else
                    {
                      switch (key - 291)
                        {
                          case 0:
                            resword = &wordlist[138];
                            goto compare;
                          case 3:
                            resword = &wordlist[139];
                            goto compare;
                          case 4:
                            resword = &wordlist[140];
                            goto compare;
                          case 5:
                            resword = &wordlist[141];
                            goto compare;
                          case 8:
                            resword = &wordlist[142];
                            goto compare;
                          case 9:
                            resword = &wordlist[143];
                            goto compare;
                          case 11:
                            resword = &wordlist[144];
                            goto compare;
                          case 13:
                            resword = &wordlist[145];
                            goto compare;
                          case 16:
                            resword = &wordlist[146];
                            goto compare;
                          case 17:
                            resword = &wordlist[147];
                            goto compare;
                          case 20:
                            resword = &wordlist[148];
                            goto compare;
                          case 25:
                            resword = &wordlist[149];
                            goto compare;
                          case 26:
                            resword = &wordlist[150];
                            goto compare;
                          case 27:
                            resword = &wordlist[151];
                            goto compare;
                          case 28:
                            resword = &wordlist[152];
                            goto compare;
                          case 29:
                            resword = &wordlist[153];
                            goto compare;
                          case 31:
                            resword = &wordlist[154];
                            goto compare;
                          case 32:
                            resword = &wordlist[155];
                            goto compare;
                          case 34:
                            resword = &wordlist[156];
                            goto compare;
                          case 37:
                            resword = &wordlist[157];
                            goto compare;
                          case 38:
                            resword = &wordlist[158];
                            goto compare;
                          case 41:
                            resword = &wordlist[159];
                            goto compare;
                          case 42:
                            resword = &wordlist[160];
                            goto compare;
                          case 43:
                            resword = &wordlist[161];
                            goto compare;
                          case 44:
                            resword = &wordlist[162];
                            goto compare;
                          case 46:
                            resword = &wordlist[163];
                            goto compare;
                          case 47:
                            resword = &wordlist[164];
                            goto compare;
                        }
                    }
                }
              else
                {
                  if (key < 444)
                    {
                      switch (key - 340)
                        {
                          case 0:
                            resword = &wordlist[165];
                            goto compare;
                          case 1:
                            resword = &wordlist[166];
                            goto compare;
                          case 2:
                            resword = &wordlist[167];
                            goto compare;
                          case 5:
                            resword = &wordlist[168];
                            goto compare;
                          case 6:
                            resword = &wordlist[169];
                            goto compare;
                          case 15:
                            resword = &wordlist[170];
                            goto compare;
                          case 16:
                            resword = &wordlist[171];
                            goto compare;
                          case 18:
                            resword = &wordlist[172];
                            goto compare;
                          case 19:
                            resword = &wordlist[173];
                            goto compare;
                          case 21:
                            resword = &wordlist[174];
                            goto compare;
                          case 22:
                            resword = &wordlist[175];
                            goto compare;
                          case 27:
                            resword = &wordlist[176];
                            goto compare;
                          case 29:
                            resword = &wordlist[177];
                            goto compare;
                          case 30:
                            resword = &wordlist[178];
                            goto compare;
                          case 33:
                            resword = &wordlist[179];
                            goto compare;
                          case 44:
                            resword = &wordlist[180];
                            goto compare;
                          case 45:
                            resword = &wordlist[181];
                            goto compare;
                          case 46:
                            resword = &wordlist[182];
                            goto compare;
                          case 58:
                            resword = &wordlist[183];
                            goto compare;
                          case 65:
                            resword = &wordlist[184];
                            goto compare;
                          case 67:
                            resword = &wordlist[185];
                            goto compare;
                          case 70:
                            resword = &wordlist[186];
                            goto compare;
                          case 71:
                            resword = &wordlist[187];
                            goto compare;
                          case 94:
                            resword = &wordlist[188];
                            goto compare;
                          case 95:
                            resword = &wordlist[189];
                            goto compare;
                          case 97:
                            resword = &wordlist[190];
                            goto compare;
                          case 99:
                            resword = &wordlist[191];
                            goto compare;
                        }
                    }
                  else
                    {
                      switch (key - 444)
                        {
                          case 0:
                            resword = &wordlist[192];
                            goto compare;
                          case 6:
                            resword = &wordlist[193];
                            goto compare;
                          case 9:
                            resword = &wordlist[194];
                            goto compare;
                          case 11:
                            resword = &wordlist[195];
                            goto compare;
                          case 12:
                            resword = &wordlist[196];
                            goto compare;
                          case 17:
                            resword = &wordlist[197];
                            goto compare;
                          case 20:
                            resword = &wordlist[198];
                            goto compare;
                          case 24:
                            resword = &wordlist[199];
                            goto compare;
                          case 26:
                            resword = &wordlist[200];
                            goto compare;
                          case 30:
                            resword = &wordlist[201];
                            goto compare;
                          case 31:
                            resword = &wordlist[202];
                            goto compare;
                          case 32:
                            resword = &wordlist[203];
                            goto compare;
                          case 35:
                            resword = &wordlist[204];
                            goto compare;
                          case 46:
                            resword = &wordlist[205];
                            goto compare;
                          case 47:
                            resword = &wordlist[206];
                            goto compare;
                          case 50:
                            resword = &wordlist[207];
                            goto compare;
                          case 54:
                            resword = &wordlist[208];
                            goto compare;
                          case 79:
                            resword = &wordlist[209];
                            goto compare;
                          case 81:
                            resword = &wordlist[210];
                            goto compare;
                          case 85:
                            resword = &wordlist[211];
                            goto compare;
                          case 86:
                            resword = &wordlist[212];
                            goto compare;
                          case 101:
                            resword = &wordlist[213];
                            goto compare;
                          case 166:
                            resword = &wordlist[214];
                            goto compare;
                          case 201:
                            resword = &wordlist[215];
                            goto compare;
                          case 220:
                            resword = &wordlist[216];
                            goto compare;
                          case 237:
                            resword = &wordlist[217];
                            goto compare;
                          case 261:
                            resword = &wordlist[218];
                            goto compare;
                        }
                    }
                }
            }
          return 0;
        compare:
          {
            register const char *s = resword->name;

            if (*str == *s && !strcmp (str + 1, s + 1))
              return resword;
          }
        }
    }
  return 0;
}
