#!/usr/bin/perl

# Generate list of tasks
@benchs = (
	"bs",
	"fft1",
	"fibcall",
	"jfdctint",
	"ludcmp",
	"matmul",
	"qurt",
	"crc",
	"fft1k",
	"fir",
	"insertsort",
	"lms",
	"minver",
	"select"
);

# Initialize tasks
foreach $bench (@benchs) {
	print "1S $bench\t:./piconsens ~/Benchs/snu-rt/$bench/$bench; echo\n";
	print "1D $bench\t:./piconsens -D 4 ~/Benchs/snu-rt/$bench/$bench; echo\n";
	print "1E $bench\t:./piconsens -E ~/Benchs/snu-rt/$bench/$bench; echo\n";
	print "2S $bench\t:./piconsens -p deg2.xml ~/Benchs/snu-rt/$bench/$bench; echo\n";
	print "2D $bench\t:./piconsens -D 4 -p deg2.xml ~/Benchs/snu-rt/$bench/$bench; echo\n";
	print "2E $bench\t:./piconsens -E -p deg2.xml ~/Benchs/snu-rt/$bench/$bench; echo\n";
	print "3S $bench\t:./piconsens -p deg4.xml ~/Benchs/snu-rt/$bench/$bench; echo\n";
	print "3D $bench\t:./piconsens -D 4 -p deg4.xml ~/Benchs/snu-rt/$bench/$bench; echo\n";
	print "3E $bench\t:./piconsens -E -p deg4.xml ~/Benchs/snu-rt/$bench/$bench; echo\n";
}
