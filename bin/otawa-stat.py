#!/usr/bin/env python3

import argparse
import datetime
import os.path
import re
import shutil
import sys
import xml.etree.ElementTree as ET

BLOCK_ENTRY = 0
BLOCK_EXIT = 1
BLOCK_CODE = 2
BLOCK_CALL = 3

class RGB:
    
    def __init__(self, r, g, b):
        self.val = "#%02x%02x%02x" % (r, g,b)

    def __str__(self):
        return self.val

WHITE = RGB(255, 255, 255)
BLACK = RGB(0, 0, 0)

COLORS = [
    RGB(234,231,255),
    RGB(214,207,255),
    RGB(192,183,255),
    RGB(171,158,255),
    RGB(161,148,250),
    RGB(155,142,245),
    RGB(140,125,237),
    RGB(123,108,227),
    RGB(113,98,221)
]
COLOR_TH = 4

def escape_dot(s):
	"""Escape a string to be compatible with dot."""
	return s. \
		replace("{", "\\{").\
		replace("}", "\\}").\
		replace("\n", "").\
		replace("\r", "")

def escape_html(s):
	"""Escape a string to be compatible with HTML text."""
	return s. \
		replace("<", "&lt;"). \
		replace(">", "&gt;"). \
		replace("&", "&amp;"). \
		replace(" ", "&nbsp;"). \
		replace("\t", "&nbsp;&nbsp;&nbsp;&nbsp;")


def background(ratio):
    return COLORS[round(ratio * (len(COLORS) - 1))]

def foreground(ratio):
    if ratio * (len(COLORS) - 1) < COLOR_TH:
        return BLACK
    else:
        return WHITE


def error(msg):
    sys.stderr.write("ERROR: %s\n" % msg)

def fatal(msg):
    error(msg)
    exit(1)


class SourceManager:
	"""The source manager manges sources of the processed program."""
	
	def __init__(self, paths = ["."]):
		self.paths = paths
		self.sources = {}
	
	def load_source(self, path, alias = None):
		"""Try to load the source from the file system."""
		try:
			return list(open(path, "r"))
		except OSError:
			return None
	
	def find_source(self, path):
		"""Lookup for a source file. If not already loaded, lookup
		for the source using the lookup paths."""
		try:
			return self.sources[path]
		except KeyError:
			lines = None
			if os.path.isabs(path):
				lines = self.load_source(path)
			else:
				for p in self.paths:
					p = os.path.join(p, path)
					lines = self.load_source(p, path)
					if lines != None:
						self.sources[p] = lines
						break
			self.sources[path] = lines
			return lines

	def get_line(self, path, line):
		"""Get the line corresponding to the given source path and line.
		If not found, return None."""
		lines = self.find_source(path)
		if lines == None:
			return None
		else:
			try:
				return lines[line - 1]
			except IndexError:
				return None


class Data:
    
    def __init__(self):
        self.data = { }
    
    def set_val(self, id, val):
        self.data[id] = val
    
    def get_val(self, id):
        try:
            r = self.data[id]
            return r
        except KeyError:
            return 0
    
    def set_max(self, id, val):
        if val != 0:
            try:
                self.data[id] = max(self.data[id], val)
            except KeyError:
                self.data[id] = val

    def add_val(self, id, val):
        if val != 0:
            try:
                self.data[id] = self.data[id] + val
            except KeyError:
                self.data[id] = val


class Block(Data):
    
    def __init__(self, type, id):
        Data.__init__(self)
        self.type = type
        self.id = id
        self.next = []
        self.pred = []
        self.lines = []
    
    def collect(self, id, val, addr, size):
        return 0
    

class BasicBlock(Block):
    
    def __init__(self, id, base, size, lines = []):
        Block.__init__(self, BLOCK_CODE, id)
        self.base = base
        self.size = size
        self.lines = lines

    def collect(self, id, val, addr, size):
        if self.base <= addr and addr < self.base + self.size:
            self.add_val(id, val)


class CallBlock(Block):

    def __init__(self, id, callee):
        Block.__init__(self, BLOCK_CALL, id)
        self.callee = callee


class Edge(Data):
    
    def __init__(self, src, snk):
        Data.__init__(self)
        self.src = src
        src.next.append(self)
        self.snk = snk
        snk.prev = self


class CFG:
    
    def __init__(self, id, label, ctx):
        self.id = id
        self.label = label
        self.ctx = ctx
        self.verts = []
        self.entry = None
        self.exit = None
        self.data = { }
    
    def add(self, block):
        self.verts.append(block)

    def collect(self, id, val, addr, size, ctx):
        if ctx == self.ctx:
            for b in self.verts:
                b.collect(id, val, addr, size)
        

class Task:
    
    def __init__(self, name):
        self.name = name
        self.entry = None
        self.cfgs = []
        self.data = { }
    
    def add(self, cfg):
        if self.entry == None:
            self.entry = cfg
        cfgs.append(cfg)
    
    def collect(self, id, val, addr, size, ctx):
        for g in self.cfgs:
            g.collect(id, val, addr, size, ctx)


class Decorator:
    
	def __init__(self, sman = SourceManager()):
		self.sman = sman
    
	def major(self):
		return None

	def start_cfg(self, cfg):
		pass

	def end_cfg(self, cfg):
		pass

	def cfg_label(self, cfg, out):
		pass

	def bb_body(self, bb, out):
		pass

	def bb_attrs(self, bb, out):
		pass

def read_cfg(path):
	cfg_path = os.path.join(path, "stats/cfg.xml")
	try:

		# open the file
		doc = ET.parse(cfg_path)
		root = doc.getroot()
		if root.tag != "cfg-collection":
			raise IOError("bad XML type")

		# prepare CFGS
		task = Task(path)
		cfg_map = {}
		for n in root.iter("cfg"):
			id = n.attrib["id"]
            
			# look for context
			try:
				ctx = n.attrib["context"]
			except KeyError:
				ctx = ""
				for p in n.iter("property"):
					try:
						if p.attrib["identifier"] == "otawa::CONTEXT":
							ctx = p.text
							break
					except KeyError:
						pass
            
			# build the CFG
			cfg = CFG(id, n.attrib["label"], ctx)
			task.cfgs.append(cfg)
			cfg_map[id] = cfg
			cfg.node = n
        
        # initialize the content of CFGs
		for cfg in task.cfgs:
			block_map = {}

			# fill the vertices of CFG
			for n in cfg.node:
				try:
					id = n.attrib["id"]
				except KeyError:
					continue
				if n.tag == "entry":
					b = Block(BLOCK_ENTRY, id)
					cfg.entry = b
				elif n.tag == "exit":
					b = Block(BLOCK_EXIT, id)
					cfg.exit = b
				elif n.tag == "bb":
					try:
						b = CallBlock(id, cfg_map[n.attrib["call"]])
					except KeyError:
						lines = []
						for l in n.iter("line"):
							lines.append((l.attrib["file"], int(l.attrib["line"])))
						b = BasicBlock(id, int(n.attrib["address"], 16), int(n.attrib["size"]), lines)
						b.lines = lines
				else:
					continue  
				cfg.add(b)
				block_map[id] = b
            
			# fill the edges of CFG
			for n in cfg.node.iter("edge"):
				Edge(block_map[n.attrib["source"]], block_map[n.attrib["target"]])

			cfg.node = None

		return task
	except ET.ParseError as e:
		error("error during CFG read: %s" % e)
	except KeyError:
		error("malformed CFG XML file")


def norm(name):
    return name.replace("-", "_")

def output_CFG(path, task, decorator, with_source = False):
    
    # make directory
    dir = os.path.join(path, "%s-cfg" % decorator.major())
    if os.path.exists(dir):
        shutil.rmtree(dir)
    os.mkdir(dir)
    
    # output each CFG
    fst = True
    for cfg in task.cfgs:

        # open file
        if fst:
            name = "index.dot"
            fst = False
        else:
            name = cfg.id + ".dot"
        out = open(os.path.join(dir, name), "w")
        decorator.start_cfg(cfg)

        # generate file
        out.write("digraph %s {\n" % cfg.id)
        for b in cfg.verts:
            out.write("\t%s [" % norm(b.id))
            if b.type == BLOCK_ENTRY:
                out.write("label=\"entry\"")
            elif b.type == BLOCK_EXIT:
                out.write("label=\"exit\"")
            elif b.type == BLOCK_CALL:
                out.write("URL=\"%s.dot\",label=\"call %s\",shape=\"box\"" % (b.callee.id, b.callee.label))
            else:
                num = b.id[b.id.find("-") + 1:]
                out.write("margin=0,shape=\"box\",label=<<table border='0' cellpadding='8px'><tr><td>BB %s (%s:%s)</td></tr><hr/><tr><td align='left'>" % (num, b.base, b.size))
                if with_source:
                    decorator.bb_source(b, out)
                    out.write("</td></tr><hr/><tr><td>")
                decorator.bb_label(b, out)
                out.write("</td></tr></table>>")
            decorator.bb_attrs(b, out)
            out.write("];\n")
        for b in cfg.verts:
            for e in b.next:
                out.write("\t%s -> %s;\n" % (norm(e.src.id), norm(e.snk.id)))
        out.write("label=<CFG: %s %s<br/>colorized by %s<br/>" % (cfg.label, cfg.ctx, decorator.major()))
        decorator.cfg_label(cfg, out)
        out.write("<BR/><I>Generated by otawa-stat.py (%s).</I><BR/><I>OTAWA framework - copyright (c) 2019, University of Toulouse</I>" % datetime.datetime.today())
        out.write(">;\n}")

        # close file
        decorator.end_cfg(cfg)
        out.close()


def read_stat(dir, task, stat):
	try:
		inp = open(os.path.join(dir, "stats", stat + ".csv"))
		for l in inp.readlines():
			fs = l[:-1].split("\t")
			assert len(fs) == 4
			task.collect(stat, int(fs[0]), int(fs[1], 16), int(fs[2]), "[%s]" % fs[3][1:-1])
	except OSError as e:
		fatal("cannot open statistics %s: %s." % (stat, e))


class BaseDecorator(Decorator):
    
	def __init__(self, main, task, stats):
		Decorator.__init__(self)
		self.main = main
		self.task = task
		self.stats = stats

		# assign maxes
		task.max = Data()
		task.sum = Data()
		for g in task.cfgs:
			g.max = Data()
			g.sum = Data()

        # compute maxes
		for g in task.cfgs:
			for b in g.verts:
				for id in self.stats:
					g.max.set_max(id, b.get_val(id))
					g.sum.add_val(id, b.get_val(id))
			for id in self.stats:
				task.max.set_max(id, g.max.get_val(id))
				task.sum.add_val(id, g.sum.get_val(id))
    
	def major(self):
		return self.main
    
	def bb_source(self, bb, out):
		if bb.lines != []:
			for (f, l) in bb.lines:
				t = self.sman.get_line(f, l)
				if t == None:
					t = ""
				out.write("%s:%d: %s<br align='left'/>" % (f, l, escape_html(t)))
    
	def bb_label(self, bb, out):
		for id in self.stats:
			val = bb.get_val(id)
			out.write("%s=%d (%3.2f%%)<br/>" % (id, val, val * 100. / self.task.sum.get_val(id)))
    
	def start_cfg(self, cfg):
		self.max = 0
		for b in cfg.verts:
			try:
				v = b.data[self.main]
				if v > self.max:
					self.max = v
			except KeyError:
				pass
		self.max = float(self.max)


class ColorDecorator(BaseDecorator):

    def __init__(self, main, task, stats):
        BaseDecorator.__init__(self, main, task, stats)

    def bb_attrs(self, bb, out):
        if bb.type == BLOCK_CALL:
            val = bb.callee.max.get_val(self.main)
        else:
            val = bb.get_val(self.main)
        ratio = float(val) / task.max.get_val(self.main)
        if ratio != 0:
            out.write(",fillcolor=\"%s\",style=\"filled\"" % background(ratio)) 
            out.write(",fontcolor=\"%s\"" % foreground(ratio))


def get_all_stats(dir):
    stats = []
    for f in os.listdir(dir):
        if f.endswith(".csv"):
            stats.append(f[:-4])
    return stats

class SyntaxColorizer:
    
    def colorize(self, line, out):
        out.write(line)

SYNTAX_COLS = { }

class CColorizer:
    
    def __init__(self):
        self.re = re.compile("(^#[a-z]+)|" +
            "(if|else|for|while|switch|case|break|continue|do|return)|" +
            "(typedef|bool|int|char|float|double|short|long|signed|unsigned|struct|union|enum)|" +
            "(//.*)|" +
            "(/\*\*+/)|" + 
            "(/\*(\*[^/]|[^\*])*\*/)|" +
            "([a-zA-Z_0-9]+)")

    def colorize(self, line, out):
        while line:
            m = self.re.search(line)
            if not m:
                out.write(line)
                return
            out.write(line[:m.start()])
            if m.group(1):
                out.write("<font color='orange'><b>%s</b></font>" % m.group(1))
            elif m.group(2):
                out.write("<font color='red'><b>%s</b></font>" % m.group(2))
            elif m.group(3):
                out.write("<b>%s</b>" % m.group(3))
            elif m.group(4) or m.group(5) or m.group(6):
                out.write("<font color='green'><i>%s</i></font>" % m.group())
            else:
                out.write(m.group())
            line = line[m.end():]
                

for ext in ['.c', '.h', '.cpp', '.hpp', '.cc', '.hh']:
    SYNTAX_COLS[ext] = CColorizer

def output_html_sign(out):
    out.write("    <center><i>Generated by otawa-stat.py (%s).<br/><a href=\"http://www.otawa.fr\">OTAWA</a> framework - copyright (c) 2019, University of Toulouse.<i></center>\n"
        % datetime.datetime.today())


def output_sources(path, main, task, stats):
    
    # make directory
    dir = os.path.join(path, "src")
    if os.path.exists(dir):
        shutil.rmtree(dir)
    os.mkdir(dir)
    
    # collect sources and statistics
    sources = []
    lines = { }
    maxv = { s: 0 for s in stats }
    maxl = { }

    for g in task.cfgs:
        for b in g.verts:
            if b.type == BLOCK_CODE:
                for (f, l) in b.lines:
                    if f not in sources and os.path.isfile(f):
                        sources.append(f)
                        maxl[f] = 0
                    if f in sources:
                        for s in stats:
                            try:
                                v  = lines[(s, f, l)] + b.get_val(s)
                            except KeyError:
                                v = b.get_val(s)
                            lines[(s, f, l)] = v
                            maxv[s] = max(v, maxv[s])
                            maxl[f] = max(maxl[f], l)

        # output index
    out = open(os.path.join(dir, "index.html"), "w")
    out.write("<html><head><title>Task %s colored by %s</title></head><body>" % (task.name, main))
    out.write("<h1>Task %s colored by %s</h1>" % (task.name, main))
    out.write("<p>List of sources:</p><ul>")
    for f in sources:
        out.write("<li><a href=\"%s.html\">%s</a></li>" % (f, f))
    out.write("</ul>")
    output_html_sign(out)
    out.write("</body></html>")
    out.close()
    
    # output each file
    for f in sources:
        
        # select colorizer
        try:
            col = SYNTAX_COLS[os.path.splitext(f)[1]]()
        except KeyError:
            col = SyntaxColorizer()
        
        # begin of header
        out = open(os.path.join(dir, "%s.html" % f), "w")
        out.write("<html><head><title>%s colored for %s</title>" % (f, main))
        
        # output styles
        out.write("""
    <style>
        td {
            text-align: right;
            padding-left: 8pt;
            padding-right: 8pt;
        }
        td.source {
            text-align: left;
        }
        table {
            margin-top: 1em;
        }
    </style>

""")
    
        # output script
        out.write("""
    <script type="text/javascript">
""")
        out.write("        var labels = [")
        for s in stats:
            out.write("'%s', " % s)
        out.write("];\n")
        out.write("        var backgrounds = ['%s'" % WHITE)
        for c in COLORS:
            out.write(", '%s'" % c)
        out.write("];\n")
        out.write("        var foregrounds = ['%s'" % BLACK)
        for i in range(COLOR_TH):
            out.write(", '%s'" % BLACK)
        for i in range(len(COLORS) - COLOR_TH):
            out.write(", '%s'" % WHITE)
        out.write("];\n")
        out.write("""
        function colorize(backs, label) {
            document.getElementById("label").textContent = label;
            trs = document.getElementById("stats").getElementsByTagName("tr");
            for(i = 0; i < trs.length; i++) {
                trs[i].style.backgroundColor = backgrounds[backs[i]];
                trs[i].style.color = foregrounds[backs[i]];
            }
        }

""")
        for s in stats:
            out.write("        var s%d = [\n            0,\n" % stats.index(s))
            for l in range(maxl[f] + 1):
                try:
                    v = lines[(s, f, l + 1)]
                    if v == 0:
                        c = 0
                    else:
                        c = round(float(v) * (len(COLORS) - 1) / maxv[s]) + 1
                except KeyError:
                    c = 0
                out.write("            %d,\n" % c)
            out.write("        ];\n")
        out.write("""
    </script>
            """)
    
        # end of head
        out.write("</head><body>\n")
        
        # output details
        out.write("    <h1>%s</h1>\n" % f)
        out.write("""
    <p><a href=\"index.html\">Top</a><br/>
    <b>Task:</b> %s<br/>
    <b>Colored by:</b> <span id='label'>%s</span>
""" % (task.name, main))
        
        # output table begin
        out.write("    <table id=\"stats\">\n")
        out.write("    <tr><th>num.</th><th>source</th>")
        for s in stats:
            i = stats.index(s)
            out.write("<th><a href=\"javascript:colorize(s%d, '%s')\">%s</a></th>" % (i, s, s))
        out.write("</tr>\n")

        # read the source file
        inp = open(f)
        num = 0
        for l in inp.readlines():
            num = num + 1

            # prepare row
            if l.endswith("\n"):
                l = l[:-1]
            style = ""
            
            # compute indentation
            indent = 0
            while l:
                if l[0] == ' ':
                    indent = indent + 8
                elif l[0] == '\t':
                    indent = indent + 32
                else:
                    break
                l = l[1:]
            if indent:
                style = style + " padding-left: %spt;" % indent

            # display the line
            out.write("    <tr><td>%d</td><td class=\"source\"" % num)
            if style:
                out.write(" style=\"%s\"" % style)
            out.write(">")
            col.colorize(l, out)
            out.write("</td>");
            for s in stats:
                try:
                    v = lines[(s, f, num)]
                    out.write("<td>%d</td>" % v)
                except KeyError:
                    out.write("<td></td>")
            out.write("</tr>\n")
        
        # close source file
        inp.close()
        
        # generate tail
        out.write("    </table>\n")
        output_html_sign(out)
        out.write("    <script type='text/javascript'>colorize(s%d, '%s');</Script>\n" % (stats.index(main), main))
        out.write("<body></html>\n")
        out.close()


# parse arguments
parser = argparse.ArgumentParser(description = "Statistics display for OTAWA WCET")
parser.add_argument('task', type=str, default="main", help="task name")
parser.add_argument('--list', '-l', action="store_true", help="list available statistics")
parser.add_argument('--all', '-a', action="store_true", help="include all available statistics in output")
parser.add_argument('--no-color', action="store_true", help="do not use colors in output")
parser.add_argument('stats', nargs='*', type=str, help="statistics to display")
parser.add_argument('--color-stat', '-s', action="store", help="statistics used to color the output")
parser.add_argument('--source', '-S', action="store_true", help="output sources colored according to statistics")
parser.add_argument('--cfg', '-G', action="store_true", help="output CFG colored according to statistics")
args = parser.parse_args()
task = args.task
dir = args.task + "-otawa"


# get all avalable stats
stat_dir = os.path.join(dir, "stats")
if not os.path.isdir(stat_dir):
	sys.stderr.write("ERROR: no statistics generated!\n")
	exit(1)
stats = args.stats
if stats == []:
    if args.all:
        stats = get_all_stats(stat_dir)
    else:
        stats = ["ipet-total_time"]


# select statistics to display
main = stats[0]
if args.color_stat:
    main = args.color_stat
    if main not in stats:
        stats.append(main)
decorator = ColorDecorator
if args.no_color:
    decorator = BaseDecorator


# list statistics
if args.list:
	for f in get_all_stats(stat_dir):
		sys.stdout.write("%s\n" % f)

# produce output
else:

	# read the CFG
	task = read_cfg(dir)

	# collect the data
	for s in stats:
		read_stat(dir, task, s)

	# output the CFG
	if args.cfg:
		for s in stats:
			output_CFG(dir, task, decorator(s, task, stats), args.source)
	
	# output the sources
	else:
		output_sources(dir, main, task, stats)



