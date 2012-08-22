UNAME = $(shell uname)
CC = gcc
CFLAGS = -g -Wall -O2
LIBS = -lz
OFILES = util.o keywords.o flasm.o unflasm.o lex.yy.o assembler.tab.o
GARBAGE = assembler.tab.* lex.yy.c memwatch.o gmon.out memwatch.log core

# if make debug, include memwatch and all symbols
ifneq (,$(findstring debug,$(MAKECMDGOALS)))
	CFLAGS += -DMEMWATCH -pg -p -pedantic -W -Wcast-align -Wcast-qual -Wshadow -Wnested-externs -Wstrict-prototypes -Waggregate-return -Wmissing-prototypes -Wpointer-arith
	OFILES += memwatch.o
else
	CFLAGS += -s
endif

# executable should not depend on cygwin.dll
ifneq (,$(findstring CYGWIN,$(UNAME)))
	CFLAGS += -mno-cygwin
endif

all:	flasm

debug:	flasm

clean:
	-rm -f ${OFILES} ${GARBAGE}

flasm:	${OFILES}
	${CC} $(CFLAGS) -o flasm ${OFILES} ${LIBS}

assembler.tab.c assembler.tab.h: assembler.y
	bison --defines --debug assembler.y

lex.yy.c: assembler.flex assembler.tab.h
	flex -i assembler.flex

keywords.c: keywords.gperf assembler.tab.h
	gperf --language=ANSI-C -t -T -E -o -k 1,$$,2,5 -S8 keywords.gperf > keywords.c