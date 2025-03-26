CC = gcc
CFLAGS = -Wall -Wextra -O3 

LIBS = -lm -lraylib 

SRCS = main.c terraingen.c enemies.c quadtree.c noise.c
OBJS = $(SRCS:.c=.o)

TARGET = main

all: clean run

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
