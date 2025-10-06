# Compiler and flags
CC = clang
CFLAGS = -Wall -I./src
LDFLAGS = -lcurl

# Sources and objects
SRC_DIR = src
SRCS = $(SRC_DIR)/email.c send_email.c
OBJS = $(SRCS:.c=.o)

# Output binary
TARGET = test_email

# Default build
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile .c into .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean object files but keep the executable
clean:
	rm -f $(OBJS)

# Full clean (also removes the executable)
distclean: clean
	rm -f $(TARGET)

.PHONY: all clean distclean
