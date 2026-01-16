CC = clang
CFLAGS = -Wall -O2
TARGET = rpg

all:
	$(CC) $(CFLAGS) -o $(TARGET) rpg.c cJSON.c -lm

clean:
	rm -f $(TARGET)
