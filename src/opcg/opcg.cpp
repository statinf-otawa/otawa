/*
 *	opcg command
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2010, IRIT UPS.
 * 
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software 
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <elm/assert.h>
#include <elm/data/HashMap.h>
#include <elm/data/Vector.h>
#include <elm/data/VectorQueue.h>
#include <elm/options.h>

#include <otawa/otawa.h>
#include <otawa/pcg/PCG.h>
#include <otawa/display/Displayer.h>
#include <otawa/pcg/PCGBuilder.h>
#include <otawa/app/Application.h>

using namespace elm;
using namespace elm::option;
using namespace otawa;
using namespace otawa::display;


/**
 * @addtogroup commands
 * @section opcg opcg Command
 * 
 * opcg is a simple OTAWA tool to display the Program Call Graph (PCG) of
 * a binary program.
 * 
 * @par Syntax
 * 
 * @code
 * $ opcg [options] binary_file [entry function]
 * @endcode
 * This program output in postscript the PCG of a binary program rooted by
 * the given function or, as a default, by the "main" function.
 * 
 * The output file use the entry function name where the file type extension is
 * appended. If you do not select a special file output and no entry function,
 * the output file is named "main.ps".
 * 
 * The options includes:
 * 
 * -o|--output path -- path of the file to output to.
 *
 * -S|--statistics -- display statistics about each subprogram.
 *
 * -T|--out-type @i type -- select the type of output (the @i type may be one of
 * 		ps, pdf, png, gif, jpg, svg, dot, view).
 */

// TODO re-enabled these options.
/*
* @li -I|--no_intern -- do not dump C internal functions (starting by '_' in the PCG).
*
* @li -c|--chain @i function -- bound the PCG to network between the entry function
* 		and the given @i function (useful when the PCG is too big).
*/


// Selection tag
Identifier<bool> SELECTED("", false);

class Stat {
public:
	Stat(void)
	:	bb_cnt(0),
		inst_cnt(0),
		bra_cnt(0),
		mem_cnt(0),
		caller_cnt(0),
		callee_cnt(0)
	{ }

	void process(Block *b) {

		// end case
		if(b->isEnd())
			return;

		// call case
		else if(b->isSynth())
			callee_cnt++;

		// basic block case
		else {

			// manage instruction
			BasicBlock *bb = b->toBasic();
			bb_cnt++;
			for(auto i: *bb) {
				inst_cnt++;
				if(i->isBranch())
					bra_cnt++;
				if(i->isMem())
					mem_cnt++;
			}

			// manage addresses
			addr_max = max(addr_max, bb->topAddress());
			addr_min = min(addr_min, bb->address());
		}
	}

	void process(CFG *g) {
		for(auto b: *g)
			process(b);
		caller_cnt += g->callers().count();
	}

	int bb_cnt;
	int inst_cnt;
	int bra_cnt;
	int mem_cnt;
	int caller_cnt;
	int callee_cnt;
	Address addr_max;
	Address addr_min;
};

p::id<Stat> STAT("");

class StatBuilder: public CFGProcessor {
public:
	static p::declare reg;
	StatBuilder(void): CFGProcessor(reg) { }

protected:

	void processCFG(WorkSpace *ws, CFG *cfg) override {
		(*STAT(cfg)).process(cfg);
	}

};

p::declare StatBuilder::reg = p::init("StatBuilder", Version(1, 0, 0))
	.extend<CFGProcessor>()
	.make<StatBuilder>();


// PCGDecorator class
class PCGDecorator: public display::GenDecorator<PCG> {
public:
	PCGDecorator(bool stat): _stat(stat) { }

	void decorate(PCG *graph, Text& caption, GraphStyle& style) const override {
		caption.out() << "PCG of " << graph->entry()->cfg()->label();
	}
	
	void decorate(PCG *graph, PCGBlock *vertex, Text& content, VertexStyle& style) const override {
		if(_stat) {
			content
				<< display::begin(display::TABLE)
				<< display::begin(display::ROW)
				<< display::begin(display::CELL);
		}
		content << vertex->getName();
		if(_stat) {

			// display intercell
			content
				<< display::end(display::CELL)
				<< display::end(display::ROW)
				<< display::hr
				<< display::begin(display::ROW)
				<< display::align::left
				<< display::begin(display::CELL)
				<< display::begin(display::SMALL);


			// display statistics
			const Stat& s = STAT(vertex->cfg());
			content
				<< s.addr_min << " - " << s.addr_max << display::br
				<< "BB: " << s.bb_cnt << display::br
				<< "Inst.: " << s.inst_cnt << display::br
				<< "Memory: " << s.mem_cnt << display::br
				<< "Branch: " << s.bra_cnt << display::br
				<< "Calls: " << s.callee_cnt << display::br
				<< "Called by: " << s.caller_cnt << display::br;

			// display table end
			content
				<< display::end(display::SMALL)
				<< display::end(display::CELL)
				<< display::end(display::ROW)
				<< display::end(display::TABLE);
		}
	}
	
	void decorate(PCG *graph, PCGEdge *edge, Text& label, EdgeStyle& style) const override {
	}

private:
	bool _stat;
};


/**
 * Manager for the application.
 */
class OPCG: public Application {
public:

	OPCG(void):	Application(Application::Make("opcg", Version(2, 0, 0))
		.author("F. Nemer, H. Cassé <casse@irit.fr>")
		.description("Draw the program call tree of the given executable.")
		.copyright("Copyright (c) 2018 IRIT - UPS")),
		no_int(SwitchOption::Make(*this).cmd("-I").cmd("--no-internal").description("do not include internal functions (starting with '_')")),
		chain(ValueOption<string>::Make(*this).cmd("-c").cmd("--chain").description("generate calling chain to a function").argDescription("function")),
		stat(SwitchOption::Make(*this).cmd("-S").cmd("--statistics").description("display statistics for each function")),
		out_type(this),
		out_path(this)
	{ }
	
protected:

	bool accept(PCGBlock *block) {
		if(!no_int)
			return true;
		string name = block->getName();
		return name && !name.startsWith("_");		
	}

	virtual void work(const string &entry, PropList &props) {

		// Build the PCG
		TASK_ENTRY(props) = entry;
		workspace()->run<PCGBuilder>(props);
		PCG *pcg = PROGRAM_CALL_GRAPH(workspace());
		ASSERT(pcg);

		// select part to display
#		if 0
		for(PCG::Iter block = pcg->blocks(); block; block++)
			SELECTED(block) = accept(block);
#		endif

		// compute the path
		sys::Path path;
		if(out_path)
			path = out_path;
		else
			path = workspace()->makeWorkDir() / "pcg";

		// Display the PCG
		PCGDecorator dec(stat);
		display::Displayer *disp = display::Provider::display(pcg, dec, out_type);
		disp->defaultVertex().shape= display::VertexStyle::SHAPE_MRECORD;
		disp->defaultVertex().margin = 0;
		disp->setPath(path);
		disp->process();
		delete disp;
	}
	
private:
	SwitchOption no_int;
	ValueOption<string> chain;
	SwitchOption stat;
	display::TypeOption out_type;
	display::OutputOption out_path;

	string prog_name;
	string entry;
};

OTAWA_RUN(OPCG);
