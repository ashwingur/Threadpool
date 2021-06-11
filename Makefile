CC=gcc
CFLAGS=-Wall -std=gnu11 -g -fsanitize=address -pthread
.PHONY: test

test:
	$(CC) test.c threadpool.c -o test $(CFLAGS)
	./test

performance:
	time ./test