GCC = arm-elf-gcc
AS = arm-elf-as

all: test1.arm

%.arm: %.o
	arm-elf-gcc -o $@ $<

%.o: %.s
	arm-elf-as $< -o $@
