# ========

MK_ROOT		= $(dir $(realpath $(firstword $(MAKEFILE_LIST))))
MK_NAME		= hxdmp

# ========

CC			= cc
CFLAGS		= -Wall -Wextra -Werror -std=c99 -O2

# ========

OBJS		= $(SRCS:.c=.o)
SRCS		= $(MK_ROOT)$(MK_NAME).c

# ========

TARGET		= $(MK_ROOT)$(MK_NAME)

# ========

.PHONY : all

all : $(TARGET)

.PHONY : clean

clean :
	rm -f $(OBJS)
	rm -f $(TARGET)

# ========

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJS) : %.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $<

# ========
