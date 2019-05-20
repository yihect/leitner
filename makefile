##############################################
#	Makefile for ltsys app
#
###############################################

MEMMAN_HOME = $(shell pwd)
BIN_PATH	= $(MEMMAN_HOME)/bin
INC_PATH	= $(MEMMAN_HOME)/inc

CROSS_TOOL =
AS = $(CROSS_TOOL)as
CC = $(CROSS_TOOL)gcc
LD = $(CROSS_TOOL)ld
OD = $(CROSS_TOOL)objdump
OC = $(CROSS_TOOL)objcopy
AR = $(CROSS_TOOL)ar

RM  	     := rm -f

################################################


################################################
app			:= $(BIN_PATH)/ltsys

sources := 	utest/AllTests.c \
		utest/CuTest.c \
		utest/glue_tests.c \
		utest/mempool_tests.c \
		utest/objpool_tests.c \
		utest/cvspool_tests.c \
		utest/objvec_tests.c \
		utest/bitmap_tests.c \
		utest/idr_tests.c \
		utest/parser_tests.c \
		src/parser.c \
		src/ltsys.c \
		src/rbtree.c \
		src/bitmap.c \
		src/idr.c \
		src/vs.c \
		src/glue.c \
		src/mempool.c \
		src/objpool.c \
		src/cvspool.c \
		src/objvec.c \
		src/util.c \
		src/log.c \
		src/main.c

################################################
objects		:= $(subst .c,.o,$(sources))
dependencies 	:= $(subst .c,.d,$(sources))
include_dirs 	:= $(INC_PATH)

CFLAGS += -std=gnu99 -I $(include_dirs) -lm -L/usr/lib/x86_64-linux-gnu/ -lreadline -lncurses -g

################################################
.PHONY:	all $(app)
all: $(app)

$(app): $(objects)
	$(CC)  $^ $(CFLAGS) -o $@


clean:
	$(RM)  $(app) $(objects) $(dependencies)

################################################


%.d: %.c
	$(SHELL) -ec '$(CC) -M $(CFLAGS) $< | sed '\''s,$(notdir $*).o,& $@,g'\'' > $@'

-include $(sources:.c=.d)




