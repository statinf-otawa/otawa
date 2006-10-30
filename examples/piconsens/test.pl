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
	print "1 $bench: echo; ./piconsens -S 8 -s -p deg1.xml ~/Benchs/snu-rt/$bench/$bench; echo\n";
	print "2 $bench: echo; ./piconsens -S 8 -s -p deg2.xml ~/Benchs/snu-rt/$bench/$bench; echo\n";
	print "3 $bench: echo; ./piconsens -S 8 -s -p deg4.xml ~/Benchs/snu-rt/$bench/$bench; echo\n";
}
