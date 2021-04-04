MKDIR=mkdir -p
RM=rm -rf

VERSION=$(shell cat VERSION)

SRC=source/
INC=$(SRC)include/
BUILD=build/
BIN=$(BUILD)bin/

GCC=gcc
CFLAGS=-Wall -I$(INC) -O2 -DHMAP_VERSION=\"$(VERSION)\"
LFLAGS=-lpthread -lm

SRCS=$(SRC)hmap.c \
     $(SRC)hmap_op.c
OBJS=$(patsubst $(SRC)%.c,$(BUILD)%.o,$(SRCS))

all: hmap

hmap: $(BIN) $(OBJS)
	$(GCC) $(CFLAGS) $(EXTRA_CFLAGS) $(OBJS) $(LFLAGS) -o $(BIN)$@

$(BUILD)%.o: $(SRC)%.c $(BUILD)
	$(GCC) -c $(CFLAGS) $(EXTRA_CFLAGS) $< -o $@

$(BUILD):
	$(MKDIR) $@

$(BIN): $(BUILD)
	$(MKDIR) $@

clean:
	$(RM) $(BUILD)

.PHONY: clean
.PHONY: all
