MKDIR=mkdir -p
RM=rm -rf
CP=cp -r

VERSION=$(shell cat VERSION)

SRC=source/
INC=$(SRC)include/
BUILD=build/
BIN=$(BUILD)bin/

INST_DIR=/usr/local/bin/

GCC=gcc
CFLAGS=-Wall -I$(INC) -O2 -DHMAP_VERSION=\"$(VERSION)\"
LFLAGS=-lpthread -lm

SRCS=$(SRC)hmap.c \
     $(SRC)hmap_op.c
OBJS=$(patsubst $(SRC)%.c,$(BUILD)%.o,$(SRCS))

all: hmap

install: hmap
	$(CP) $(BIN)hmap $(INST_DIR)

hmap: $(BIN) $(OBJS)
	$(GCC) $(CFLAGS) $(EXTRA_CFLAGS) $(OBJS) $(LFLAGS) -o $(BIN)$@

$(BUILD)%.o: $(SRC)%.c $(BUILD)
	$(GCC) -c $(CFLAGS) $(EXTRA_CFLAGS) $< -o $@

$(BUILD):
	$(MKDIR) $@

$(BIN): $(BUILD)
	$(MKDIR) $@

clean:
	@$(RM) $(BUILD)

.PHONY: clean
.PHONY: all
