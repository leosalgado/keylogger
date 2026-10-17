CC = gcc
CFLAGS = -Wall -Wextra

keylogger: main.c keys.c keys.h
	$(CC) $(CFLAGS) main.c keys.c -o keylogger

clean:
	rm -f keylogger keylog_*.txt

.PHONY: clean
