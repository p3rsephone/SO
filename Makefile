CC = gcc
CFLAGS = -g -Wall

all: controller

controller: stringProcessing.o const filter spawn window
	$(CC) $(CFLAGS) -o controller stringProcessing.o controller.c

const: stringProcessing.o const.c
	$(CC) $(CFLAGS) -o const stringProcessing.o const.c

filter: stringProcessing.o filter.c
	$(CC) $(CFLAGS) -o filter stringProcessing.o filter.c

spawn: stringProcessing.o spawn.c
	$(CC) $(CFLAGS) -o spawn stringProcessing.o spawn.c

window: stringProcessing.o window.c
	$(CC) $(CFLAGS) -o window stringProcessing.o window.c

stringProcessing.o: stringProcessing.c stringProcessing.h
	$(CC) $(CFLAGS) -c stringProcessing.c

clean:
	$(RM) controller const filter spawn window fifo_1_in fifo_2_in fifo_1_out fifo_2_out *.o *~

