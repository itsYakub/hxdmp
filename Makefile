# ========

MK_ROOT		= $(dir $(realpath $(firstword $(MAKEFILE_LIST))))
MK_NAME		= hxdmp

# ========

CC			= cc
CDBG		= gdb
CFLAGS		= -Wall -Wextra -Werror -std=c99 -O2
GFLAGS		= -O0 -ggdb3

# ========

OBJS		= $(SRCS:.c=.o)
SRCS		= $(MK_ROOT)$(MK_NAME).c

# ========

TARGET		= $(MK_ROOT)$(MK_NAME)
TARGETDBG	= $(TARGET)-dbg

# ========

.PHONY : all

all : $(TARGET)

.PHONY : clean

clean :
	rm -f $(OBJS)
	rm -f $(TARGET)

.PHONY : install

install : $(TARGET)
	cp -f $^ /usr/local/bin

.PHONY : uninstall

uninstall :
	rm -f $/usr/local/bin/$(MK_NAME)

.PHONY : debug

debug :
	$(CC) $(GFLAGS) -o $(TARGETDBG) $(SRCS)
	$(CDBG) $(TARGETDBG)
	rm -f $(TARGETDBG)

# ========

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJS) : %.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $<

# ========
