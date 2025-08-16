CC       := gcc
CFLAGS   := -Wall -Wextra -O2 -g -pthread
LDFLAGS  := -pthread

SRCS     := mem_alloc.c main.c
OBJS     := $(SRCS:.c=.o)
TARGET   := mem_alloc_demo

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c mem_alloc.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
