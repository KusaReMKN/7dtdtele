SHELL   = /bin/sh
CC      = gcc
PROGRAM = 7dtdtelemod
OBJS    = main.o mknconf.o
CFLAGS  = -O2

.PHONY: all
all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) -o $@ $^

.PHONY: clean
clean:
	$(RM) $(OBJS) $(PROGRAM)
