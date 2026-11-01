CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -pedantic -O2
TARGET  = hms
SRCS    = main.c utils.c storage.c auth.c patient.c doctor.c appointment.c billing.c records.c dashboard.c
OBJS    = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c hms.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
