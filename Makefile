# Copyright (C) 2017, Chris Simmonds (chris@2net.co.uk)
#
# If cross-compiling, CC must point to your cross compiler, for example:
# make CC=arm-linux-gnueabihf-gcc

CFLAGS = -Wall -g -pthread -I./inc
BIN 	:= ./bin
SRC 	:= ./src
SRCS  := $(wildcard $(SRC)/*.c)
OBJS  := $(patsubst $(SRC)/%.c, $(BIN)/%, $(SRCS))

all: $(OBJS)

$(BIN)/%: $(SRC)/%.c
	mkdir -p $(BIN)
	$(CC) $(CFLAGS) $< -o $@ -lrt

clean:
	rm -rf $(BIN)
