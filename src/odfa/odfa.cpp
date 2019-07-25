/*
 *	odfa -- OTAWA dataflow analysis dislayer
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2011, IRIT UPS.
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

#include <elm/io/BlockInStream.h>
#include <elm/option/ListOption.h>
#include <elm/option/SwitchOption.h>

#include <otawa/app/Application.h>
#include <otawa/cfg/features.h>
#include <otawa/data/clp/features.h>
#include <otawa/data/clp/ClpAnalysis.h>
#include <otawa/data/clp/SymbolicExpr.h>
#include <otawa/display/CFGOutput.h>
#include <otawa/hard.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/prog/sem.h>
#include <otawa/prog/Symbol.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/stack/features.h>
#include <otawa/stack/AccessedAddress.h>

using namespace elm;
using namespace otawa;

Identifier<bool> BEFORE("", false);
Identifier<bool> AFTER("", false);
Identifier<bool> SEM("", false);
Identifier<bool> FILTER("", false);

/**
 * @addtogroup commands
 * @section odfa odfa Command
 *
 * ODFA allows to launch data flow analysis in a stand-alone way
 * and to display the results. For the time being, only the CLP analysis
 * is supported but more will be added later.
 *
 * @par SYNTAX
 * @code
 * $ odfa binary_file function1 function2 ...
 * @endcode
 *
 * The following options are provided:
 * @li -c, --clp -- perform CLP analysis,
 * @li --before -- display the data values before the basic blocks,
 * @li --after -- display the data values after the basic blocks,
 * @li -s, --sem -- display semantic instructions,
 * @li -F, --filter -- display filters (for the CLP analysis),
 * @li -S, --stats -- display statistics of the analysis,
 * @li -C, --cfg -- dump the CFG in .dot format (including data values),
 * @li -r, --reg REGISTER=VALUE -- add an initialization register.
 *
 */


/**
 * Generic interface for displaying the result of analyses.
 */
class Displayer {
public:
	friend class ODFA;
	virtual ~Displayer(void) { }

	/**
	 * Called once for each handled work space. Displayer can perform requirement
	 * of analysis at this point.
	 * @param ws	Current work space.
	 */
	virtual void prepare(WorkSpace *ws, const PropList& props) { }

	/**
	 * Called to display the state before BB.
	 * @param out	Current output.
	 * @param bb	Basic to display input state for.
	 */
	virtual void displayBefore(io::Output& out, BasicBlock *bb) = 0;

	/**
	 * Called to display the state after BB.
	 * @param out	Current output.
	 * @param bb	Basic to display output state for.
	 */
	virtual void displayAfter(io::Output& out, BasicBlock *bb) = 0;

	/**
	 * Called to display filters associated with a condition of basic blocks.
	 * @param out	Current output.
	 * @param bb	Basic block to display filters for.
	 */
	virtual void displayFilters(io::Output& out, BasicBlock *bb) { }

	/**
	 * Called to display statistics.
	 */
	virtual void displayStats(io::Output& out) { }

	/**
	 * Start the traversal of a basic block with computation.
	 * @param bb	Started basic block.
	 */
	virtual void start(BasicBlock *bb) { }

	/**
	 * Go to next instruction.
	 */
	virtual void next(void) { }

	/**
	 * Display the current state.
	 * @param out	Current output.
	 */
	virtual void display(io::Output& out) { }

	/**
	 * Ensure that addresses of memory accesses has been built.
	 * @param ws	Workspace to work on.
	 * @param props	Configuration properties.
	 */
	virtual void buildAddresses(WorkSpace *ws, const PropList& props) { }
};


// CLP textual displayer
class CLPDisplayer: public Displayer {
protected:

	virtual void prepare(WorkSpace *ws, const PropList& props) {
		_ws = ws;
		ws->require(otawa::clp::CLP_ANALYSIS_FEATURE, props);
	}

 	virtual void displayBefore(io::Output& out, BasicBlock *bb) {
 		clp::State state = clp::STATE_IN(bb);
 		state.print(out, _ws->process()->platform());
 		out<< io::endl;
 	}

 	virtual void displayAfter(io::Output& out, BasicBlock *bb) {
 		clp::State state = clp::STATE_OUT(bb);
 		state.print(out, _ws->process()->platform());
 		out<< io::endl;
 	}

 	virtual void displayFilters(io::Output& out, BasicBlock *bb) {
 		Vector<se::SECmp *> reg_filters = se::REG_FILTERS(bb);
 		Vector<se::SECmp *> addr_filters = se::ADDR_FILTERS(bb);
		if(reg_filters.length() != 0 || addr_filters.length() != 0) {
			out << "\tFILTERS = \n";
			for(int i = 0; i < reg_filters.count(); i++) {
				out << "\n\t\t";
				reg_filters[i]->print(out, _ws->process()->platform());
			}
			for(int i = 0; i < addr_filters.count(); i++) {
				out << "\n\t\t";
				addr_filters[i]->print(out, _ws->process()->platform());
			}
		}
 	}

 	virtual void displayStats(io::Output& out) {
		/*cerr << "machine instructions: " << clpa.get_nb_inst() << io::endl;
		cerr << "semantic instructions: " << clpa.get_nb_sem_inst() << io::endl;
		cerr << "sets: " << clpa.get_nb_set() << io::endl;
		cerr << "sets to T: " << clpa.get_nb_top_set() << io::endl;
		cerr << "stores: " << clpa.get_nb_store() << io::endl;
		cerr << "stores of T: " << clpa.get_nb_top_store() << io::endl;
		cerr << "stores at T: " << clpa.get_nb_top_store_addr() << io::endl;
		cerr << "loads: " << clpa.get_nb_load() << io::endl;
		cerr << "loads of T: " << clpa.get_nb_top_load() << io::endl;
		cerr << "loads at T: " << clpa.get_nb_load_top_addr() << io::endl;
		cerr << "filters: " << clpa.get_nb_filters() << io::endl;
		cerr << "filters to T: " << clpa.get_nb_top_filters() << io::endl;

		cerr << "precise sets: " << (100 - (float(clpa.get_nb_inst() + clpa.get_nb_top_store() + clpa.get_nb_load_top_addr()) * 100 / (clpa.get_nb_sem_inst() + clpa.get_nb_store() + clpa.get_nb_load()))) << io::endl;
		cerr << "precise addresses: " << (100 - (float(clpa.get_nb_top_store_addr() + clpa.get_nb_load_top_addr()) * 100 / (clpa.get_nb_store() + clpa.get_nb_load()))) << io::endl;
		cerr << "precise filters: " << (100 - (float(clpa.get_nb_top_filters()) * 100 / clpa.get_nb_filters())) << io::endl;*/
 	}

private:
 	WorkSpace *_ws;
};


/**
 * Displayer for stack analysis.
 */
class StackDisplayer: public Displayer {
public:

	StackDisplayer(WorkSpace *ws): i(0) { }
	~StackDisplayer(void) { if(i) delete i; }

	virtual void prepare(WorkSpace *ws, const PropList& props) {
		ws->require(otawa::STACK_ANALYSIS_FEATURE, props);
		i = new stack::Iter(ws);
	}

	virtual void displayBefore(io::Output& out, BasicBlock *bb) {
		if(bb->isEnd())
			return;
		i->start(bb);
		out << i->state() << io::endl;
	}

	virtual void displayAfter(io::Output& out, BasicBlock *bb) {
		if(bb->isEnd())
			return;
		i->start(bb);
		while(!i->ended())
			i->next();
		out << *i->state() << io::endl;
	}

	virtual void start(BasicBlock *bb) {
		i->start(bb);
	}

	virtual void next(void) {
		i->nextInst();
	}

	virtual void display(io::Output& out) {
		out << i->state() << io::endl;
	}

	virtual void buildAddresses(WorkSpace *ws, const PropList& props) {
		ws->require(stack::ADDRESS_FEATURE, props);
	}

private:
	stack::Iter *i;
};


/**
 * All information to perform the display.
 */
class DisplayContext {
public:
	DisplayContext(WorkSpace *ws, const PropList& ps)
		: before(false), after(false), filter(false), sem(false), stats(false), insts(false), addrs(false), _ws(ws), _ps(ps) { }

	virtual ~DisplayContext(void) {
		for(int i = 0; i < disps.count(); i++)
			delete disps[i];
	}

	inline const Vector<Displayer *>& displayers(void) const { return disps; }

	void add(Displayer *d) {
		disps.add(d);
		d->prepare(_ws, _ps);
	}

	virtual void displayBefore(io::Output& out, BasicBlock *bb) {
		for(int i = 0; i < disps.count(); i++)
			disps[i]->displayBefore(out, bb);
	}

	virtual void displayAfter(io::Output& out, BasicBlock *bb) {
		for(int i = 0; i < disps.count(); i++) {
			out << "\t\t";
			disps[i]->displayAfter(out, bb);
		}
	}

	virtual void displayFilters(io::Output& out, BasicBlock *bb) {
		for(int i = 0; i < disps.count(); i++)
			disps[i]->displayFilters(out, bb);
	}

	virtual void displayStats(io::Output& out) {
		for(int i = 0; i < disps.count(); i++)
			disps[i]->displayStats(out);
	}

	virtual void start(BasicBlock *bb) {
		for(int i = 0; i < disps.count(); i++)
			disps[i]->start(bb);
	}

	virtual void next(void) {
		for(int i = 0; i < disps.count(); i++)
			disps[i]->next();
	}

	virtual void display(io::Output& out) {
		for(int i = 0; i < disps.count(); i++) {
			out << "\t\t";
			disps[i]->display(out);
		}
	}

	virtual void buildAddresses(WorkSpace *ws, const PropList& props) {
		for(int i = 0; i < disps.count(); i++)
			disps[i]->buildAddresses(ws, props);
	}

	bool before, after, filter, sem, stats, insts, addrs;

private:
	Vector<Displayer *> disps;
	WorkSpace *_ws;
	const PropList& _ps;
};


// CLPCFGOutput class
class CLPCFGOutput: public display::CFGOutput {
public:
	CLPCFGOutput(DisplayContext& context): ctx(context) { }

protected:
	virtual void genBBInfo(CFG *cfg, BasicBlock *bb, Output& out) {

		// display state before
		if(ctx.before) {
			out << "\n---\nbefore:";
			ctx.displayBefore(out, bb);
		}

		// display state after
		if(ctx.after) {
			out << "\n---\nafter:";
			ctx.displayAfter(out, bb);
		}

		// display filters
		if(ctx.filter) {
			out << "\n---\nfilters:";
			ctx.displayFilters(out, bb);
		}
	}

private:
	DisplayContext& ctx;
};


// Generic textual displayer
class TextualDisplayer: public BBProcessor {
public:
	TextualDisplayer(DisplayContext& context): ctx(context) { }

protected:

	virtual void processCFG(WorkSpace *ws, CFG *cfg) {
		out << "#function " << cfg->label() << io::endl;
		BBProcessor::processCFG(ws, cfg);
		out << io::endl;
	}

	virtual void processBB(WorkSpace *ws, CFG *cfg, Block *b) {

		// ends
		if(b->isEnd())
			out << b << io::endl;

		// synthetic block
		else if(b->isSynth()) {
		}

		// normal BB
		else {
			BasicBlock *bb = b->toBasic();
			out << "BB " << bb->index() << " (" << bb->address() << ")\n";
			if(ctx.before)
				ctx.displayBefore(out, bb);
			if(ctx.insts)
				ctx.start(bb);
			for(BasicBlock::InstIter inst(bb); inst(); inst++) {

				// display symbols
				for(Identifier<Symbol *>::Getter sym(*inst, SYMBOL); sym(); sym++)
					out << '\t' << sym->name() << io::endl;

				// disassemble the instruction
				out << "\t" << inst->address() << ' ' << *inst;
				if(inst->isControl() && inst->target())
					out << " (" << inst->target()->address() << ")";
				out << io::endl;

				// disassemble semantic instructions
				if(ctx.sem) {
					sem::Block block;
					inst->semInsts(block);
					for(int i = 0; i < block.count(); i++) {
						sem::Printer printer(ws->process()->platform());
						out << "\t\t\t";
						printer.print(out, block[i]);
						out << io::endl;
					}
				}

				// display state after
				if(ctx.insts) {
					ctx.next();
					ctx.display(out);
				}
			}

			// display state after basic block
			if(ctx.after) {
				if(ctx.filter)
					ctx.displayFilters(out, bb);
				ctx.displayAfter(out, bb);
			}

			// display accessed addresses
			if(ctx.addrs) {
				AccessedAddresses *as = otawa::ADDRESSES(bb);
				if(as)
					out << as << io::endl;
			}

		}
		out << io::endl;

	}

	DisplayContext& ctx;
};


class ODFA: public Application {
public:

	ODFA(void)
	: Application(
		"odfa",
		Version(2, 0, 0),
		"DataFlow Analysis Displayer",
		"H. Cassé <casse@irit.fr>"),
	clp		(option::SwitchOption::Make(*this).cmd("-c").cmd("--clp")		.description("display CLP information")),
	list	(option::SwitchOption::Make(*this).cmd("-l").cmd("--list")		.description("display the list of registers")),
	before	(option::SwitchOption::Make(*this).cmd("-B").cmd("--before")	.description("display state before the BB")),
	after	(option::SwitchOption::Make(*this).cmd("-A").cmd("--after")		.description("display state after the BB")),
	sem		(option::SwitchOption::Make(*this).cmd("-s").cmd("--sem")		.description("display semantic instructions")),
	filter	(option::SwitchOption::Make(*this).cmd("-F").cmd("--filter")	.description("display filters")),
	stats	(option::SwitchOption::Make(*this).cmd("-S").cmd("--stats")		.description("display statistics of the analysis")),
	cfg		(option::SwitchOption::Make(*this).cmd("-C").cmd("--cfg")		.description("dump the CFG in .dot format")),
	stack	(option::SwitchOption::Make(*this).cmd("-P").cmd("--stack")		.description("perform stack analysis and display results")),
	insts	(option::SwitchOption::Make(*this).cmd("-I").cmd("--insts")		.description("display state after each instruction")),
	addrs	(option::SwitchOption::Make(*this).cmd("-a").cmd("--address")	.description("display addresses of memory access")),
	inits	(option::ListOption<string>::Make(this).cmd("-r").cmd("--reg")	.description("add an initialization register").argDescription("REGISTER=VALUE"))
	{ }

protected:

	void work(const string& entry, PropList& props) {
		if(list)
			performList();
		else
			performAnalysis(props);
	}

private:

	void performList(void) {

		// display platform
		hard::Platform *pf = workspace()->process()->platform();
		cout << "PLATFORM: " << pf->identification() << io::endl;

		// display registers
		const hard::Platform::banks_t& banks = pf->banks();
		for(int i = 0; i < banks.count(); i++) {
			bool fst = true;
			cout << "BANK: " << banks[i]->name() << io::endl;
			for(int j = 0; j < banks[i]->count(); j++) {
				if(fst)
					fst = false;
				else
					cout << ", ";
				cout << banks[i]->get(j)->name();
			}
			cout << io::endl;
		}
	}

	void performAnalysis(PropList& props) {
		fillRegs(props);

		// perform the analysis
		// TODO
		/*if(workspace()->isProvided(DELAYED_FEATURE)
		|| workspace()->isProvided(DELAYED2_FEATURE))
			require(DELAYED_CFG_FEATURE);*/
		require(otawa::VIRTUALIZED_CFG_FEATURE); //should shift to require(otawa::COLLECTED_CFG_FEATURE);


		// prepare analysis
		DisplayContext ctx(workspace(), props);
		ctx.before = *before || !*after;
		ctx.after = (*after || !*before) && !insts;
		ctx.sem = sem;
		ctx.filter = filter;
		ctx.insts = insts;
		ctx.addrs = addrs;
		if(clp)
			ctx.add(new CLPDisplayer());
		if(stack)
			ctx.add(new StackDisplayer(workspace()));
		if(addrs)
			ctx.buildAddresses(workspace(), props);

		// display the CFG
		if(cfg) {
			display::CFGOutput::KIND(props) = display::OUTPUT_DOT;
			CLPCFGOutput *output = new CLPCFGOutput(ctx);
			workspace()->run(output, props);
		}
		else {
			TextualDisplayer *displayer = new TextualDisplayer(ctx);
			workspace()->run(displayer, props);
		}

	}

	void fillRegs(PropList& props) {
		for(int i = 0; i < inits.count(); i++) {

			// scan the string
			string s = inits[i];
			int p = s.indexOf('=');
			if(p < 0)
				throw option::OptionException(_ << "bad format in -reg argument: " + s);
			string reg_name = s.substring(0, p), init = s.substring(p + 1);

			// find the register
			hard::Platform *pf = workspace()->process()->platform();
			const hard::Register *reg = pf->findReg(reg_name);
			if(!reg)
				throw option::OptionException(_ << " no register named " << reg_name << " in platform " << pf->identification());

			// initialize the register
			io::BlockInStream ins(init);
			io::Input in(ins);
			switch(reg->kind()) {
			case hard::Register::INT:
				t::uint32 v;
				in >> v;
				//clp::Analysis::INITIAL(props).add( pair(reg, Address(v)) );
				break;
			default:
				throw option::OptionException(_ << "unsupported register kind for initialization: " << s);
			}
		}
	}

	option::SwitchOption clp, list, before, after, sem, filter, stats, cfg, stack, insts, addrs;
	option::ListOption<string> inits;
};

OTAWA_RUN(ODFA);

