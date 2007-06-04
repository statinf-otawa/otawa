#!/usr/bin/python

import sys
import re

line_re = re.compile("(\w+)\s+(.*)");
empty_re = re.compile("\s*$");
comment_re = re.compile("\s*#");

def inc(cnd, cnts):
	if cnts.has_key(cnd):
		cnts[cnd] = cnts[cnd] + 1
	else:
		cnts[cnd] = 1;


# Perform the generation
def gen(cnts):
	for cnd in cnts.keys():
		cnd_text = ""
		if cnd:
			cnd_text = "if(" + cnd + ") "
		print "\t" + cnd_text + "t += " + str(cnts[cnd]) + ";"


# Scan the string
def scan(exp, begin = True, end = False):
	cnts = { }
	alt = 0;	
	for c in exp:
		if c == "f" or c == "g" or c == "i" or c == "n" or c == "P" or c == "r":
			inc("", cnts)
		elif c == "U":
			inc("stack_to_ext8(inst)", cnts)
		elif c == "V":
			inc("vector_to_ext8(inst)", cnts)
	gen(cnts)


# Default case
def manage_default(exps):
	first = 1;
	for exp in exps:
		if not first:
			print "\n\told_t = t;\n\tt = 0;"
		scan(exp)
		if first:
			first = 0;
		else:
			print "\tif(old_t > t) t = old_t;"			


# Branch case
def manage_branch(exps):
	print "\tif(is_taken) {"
	scan(exps[0])
	print "\told_t = t;\n\tt=0;\n\t}"
	print "\tif(is_not_taken) {"
	scan(exps[1])
	print "\t}\n\tif(old_t > t) t = old;"


# Rep case
def manage_rep(exps):
	print "\tcnt = COUNT(exp);"
	print "\tif(cnt < 0) throw Exception(\"unbounded repeat instruction at %p\", (int)inst->address());"
	scan(exps[0])
	print "\told_t = t;\n\tt = 0;"
	scan(exps[1]);
	print "\tt += old_t + cnt * t;"
	scan(exps[2])
	scan(exps[3])


# Process the file
cnt = 0;
syms = []
for line in sys.stdin:
	cnt = cnt + 1;
	#print line
	if empty_re.match(line) or comment_re.match(line):
		continue
	match = line_re.match(line)
	if not match:
		print "WARNING:" + str(cnt) + ": garbage ignored"
	else:
		name = match.group(1)
		exp = match.group(2)
		syms.append(name)
		#print "name = " + name + ", exp = " + exp
		print "static int time_" + name + "(Inst *inst) {"
		print "\tint t = 0, old_t;"
		print "\t// " + exp
		exps = exp.split(",")
		if exp.startswith("[bra]"):
			manage_branch(exps)
		elif exp.startswith("[rep]"):
			manage_rep(exps)
		else:
			manage_default(exps)
		print "\treturn t;\n}\n"


# Generate the final table
print "static s12x_time_t times[] = {"
first = 1
for sym in syms:
	print "\t{" + sym + ", time_" + sym + " },"
print "};"
