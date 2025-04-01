CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -fsanitize=address

sound_seg.o: sound_seg.c
	$(CC) $(CFLAGS) -fPIC -c sound_seg.c -o sound_seg.o

clean:
	rm -f *.o