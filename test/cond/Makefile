ARCH=arm-linux-gnueabihf-
EXT=.arm
CC=$(ARCH)gcc
CFLAGS=-gdwarf-4
LDFLAGS=-static -nostartfiles

all: cond$(EXT)

cond$(EXT): cond.s
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)
