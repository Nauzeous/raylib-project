CC = gcc
CFLAGS = -Wall -Wextra -O3 

# Libraries
LIBS = -lm -lraylib 

# Source files (add more .c files here if you have them)
SRCS = main.c
OBJS = $(SRCS:.c=.o)

# Output executable name
TARGET = main

# Default target
all: $(TARGET)

# Link the program
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean built files
clean:
	rm -f $(OBJS) $(TARGET)

# Run the program
run: $(TARGET)
	./$(TARGET)

# Phony targets
.PHONY: all clean run
