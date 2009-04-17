
ARCH=powerpc-linux-gnu-
CFLAGS=-g3 -gdwarf-2
LDFLAGS=-static
CLEAN=bench.elf

all: bench.elf test

test: 
	../../src/oipet/oipet bench.elf -C

bench.elf: bench.c
	$(ARCH)gcc $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -rf $(CLEAN)
