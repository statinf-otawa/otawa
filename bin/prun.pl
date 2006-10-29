#!/usr/bin/perl

# Default
$dir = $ENV{"PWD"};
$tasks_file = "tasks.txt";
$hosts_file = "hosts.txt";

# Scan arguments
$pos = 0;
for($i = 0; $i < $#ARGV; $i++) {
	$arg = $ARGV[$i];
	if($arg =="-d") {
		$dir = $ARGV[$i + 1];
		$i++;
	}
	elsif($arg == "-t") {
		$tasks_file = $ARGV[$i + 1];
		$i++;
	}
	elsif($arg == "-h") {
		$hosts_file = $ARGV[$i + 1];
		$i++;
	}
	else {
		if($arg != "-h" && $arg != "--help") {
			print "ERROR: unknown argument \"$arg\"\n";
		}
		print "SYNTAX: prun.pl [-d PATH] [-t PATH] [-h PATH]\n";
		print "	-d PATH: base path to use on the host [default to current path],\n";
		print "	-t PATH: path to the tasks file [default to tasks.txt],\n";
		print "	-h PATH: path to the hosts file [default to hosts.txt],\n";
		exit 1
	}
}

# Load configuration
open TASKS,$tasks_file || die "cannot open \"$tasks_file\" !";
@tasks = ();
while(<TASKS>) {
	chomp;
	push(@tasks, $_);
#	print "$_";
}
open HOSTS,$hosts_file || die "cannot open \"$hosts_file\" !";
@procs = ();
while(<HOSTS>) {
	chomp;
	push(@procs, $_);
#	print "$_";
}

# Initialization
%children = ();
@free = ();
foreach $proc (@procs) {
	push(@free, $proc);
	#print("=> $proc\n");
}
unlink "result.out";


# Main loop
while(@tasks || %children) {

	# Assign task to a proc
	while(@tasks && @free) {
	
		# Get information
		($proc, @free) = @free;
		($task, @tasks) = @tasks;
		$use[$proc] = $task;
		$host = $proc;
		if($proc =~ /(.*),.*/) {
			$host = $1;
		}
		$msg = $task;
		if($task =~ /(.*):(.*)/) {
			$msg = $1;
			$task = $2;
		}
		print "RUNNING $proc($host): $task\n";
		
		# Launch the task
		$pid = fork();
		if($pid) {
			$children{$pid} = $proc;
		}
		else {
			exec "echo -n \"$msg\" > $proc.out; ssh -q $host 'cd $dir; $task' >> $proc.out"
		}
	}
	
	# Wait for an ended task
	$pid = wait();
	$proc = $children{$pid};
	print "ENDED $proc\n";
	push(@free, $proc);
	delete $children{$pid};
	system("cat $proc.out >> result.out");
}

# cleanup
foreach $proc (@procs) {
	unlink "$proc.out"
}
