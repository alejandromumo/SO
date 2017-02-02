.PHONY: all clean cleanall

CC = gcc
CFLAGS = -Wall -Wextra -ggdb
LDLIBS = -lpthread

OBJS = logger.o simulation.o dining-room.o philosopher.o waiter.o utils.o
SYMBOLS = -DASCII_SYMBOLS # -DUTF8_SYMBOLS1, -DUTF8_SYMBOLS2, -DASCII_SYMBOLS

all: simulation

simulation: $(OBJS)
	$(CC) $(CFLAGS) $(SYMBOLS) $(OBJS) $(LDLIBS) -o simulation

%.o: %.c %.h
	$(CC) $(CFLAGS) $(SYMBOLS) $(LDLIBS) -c $<

clean:
	rm -fv $(OBJS)

cleanall: clean
	rm -fv simulation
