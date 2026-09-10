CC = gcc
CFLAGS = -Wall -Wextra -std=c11

TARGET = scheduler
SRC = escalonamento.c
HDR = escalonamento.h

$(TARGET): $(SRC) $(HDR)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) *.out