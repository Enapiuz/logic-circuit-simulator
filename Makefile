SRCDIR := src/
NAME := vc
CC := clang
CFLAGS := -c -Wall

SOURCES := $(shell find $(SRCDIR) -name "*.c")

.PHONY: all library clean

all:
	@$(CC) $(SOURCES) -o $(NAME) -framework OpenCL -std=c11

library:
	@$(CC) $(SOURCES) -o $(NAME).so -shared -undefined dynamic_lookup -framework OpenCL -std=c11

clean:
	rm -f $(NAME) $(NAME).so
