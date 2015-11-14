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

#sources := 	utest/AllTests.c \
#			utest/CuTest.c \
#	utest/memman_tests.c 
sources := 	utest/AllTests.c \
  		utest/CuTest.c \
		utest/glue_tests.c \
	 	src/ltsys.c \
  		src/rbtree.c \
		src/vs.c \
		src/glue.c \
		src/util.c \
		src/main.c

################################################
objects		:= $(subst .c,.o,$(sources))
dependencies 	:= $(subst .c,.d,$(sources))
include_dirs 	:= $(INC_PATH)

CFLAGS += -I $(include_dirs) -lm -g

################################################
.PHONY:	all $(app) 
all: $(app) 

$(app): $(objects) 
	$(CC)  $(CFLAGS) $^ -o $@
		 

clean:
	$(RM)  $(app) $(objects) $(dependencies)

################################################


%.d: %.c	
	$(SHELL) -ec '$(CC) -M $(CFLAGS) $< | sed '\''s,$(notdir $*).o,& $@,g'\'' > $@'

-include $(sources:.c=.d)




