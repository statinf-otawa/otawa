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

#include <otawa/app/Application.h>
#include <elm/option/SwitchOption.h>
#include <otawa/data/clp/ClpAnalysis.h>
#include <otawa/cfg/features.h>
#include <otawa/proc/BBProcessor.h>
#include <elm/option/ListOption.h>
#include <elm/io/BlockInStream.h>
#include <otawa/hard.h>
#include <otawa/prog/sem.h>
#include <otawa/data/clp/SymbolicExpr.h>

using namespace elm;
using namespace otawa;

Identifier<bool> BEFORE("", false);
Identifier<bool> AFTER("", false);
Identifier<bool> SEM("", false);
Identifier<bool> FILTER("", false);

// Generic textual displayer
class TextualDisplayer: public BBProcessor {
public:
	TextualDisplayer(void) { }
 	TextualDisplayer(AbstractRegistration& reg): BBProcessor(reg) { }

protected:

 	virtual void displayBefore(WorkSpace *ws, CFG *cfg, BasicBlock *bb) = 0;
 	virtual void displayAfter(WorkSpace *ws, CFG *cfg, BasicBlock *bb) = 0;

 	virtual void configure(const PropList& props) {
 		before = BEFORE(props);
 		after = AFTER(props);
 		if(!before && !after) {
 			before = true;
 			after = true;
 		}
 		sem = SEM(props);
 		filter = FILTER(props);
 	}

	virtual void processCFG(WorkSpace *ws, CFG *cfg) {
		out << "#function " << cfg->label() << io::endl;
		BBProcessor::processCFG(ws, cfg);
		out << io::endl;
	}

	virtual void processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {

		// ends
		if(bb->isEnd()) {
			if(bb->isEntry())
				out << "ENTRY\n";
			else
				out << "EXIT\n";
		}

		// normal BB
		else {
			out << "BB " << bb->number() << " (" << bb->address() << ")\n";
			if(before)
				displayBefore(ws, cfg, bb);
			for(BasicBlock::InstIter inst(bb); inst; inst++) {

				// display symbols
				for(Identifier<Symbol *>::Getter sym(inst, Symbol::ID); sym; sym++)
					out << '\t' << sym->name() << io::endl;

				// disassemble the instruction
				out << "\t\t" << *inst;
				if(inst->isControl() && inst->target())
					out << " (" << inst->target()->address() << ")";
				out << io::endl;

				// disassemble semantics instructions
				if(sem) {
					sem::Block block;
					inst->semInsts(block);
					for(int i = 0; i < block.count(); i++) {
						sem::Printer printer(ws->process()->platform());
						out << "\t\t\t";
						printer.print(out, block[i]);
						out << io::endl;
					}
				}
			}
			if(after) {
				if(filter) {
					Vector<se::SECmp *> reg_filters = se::REG_FILTERS(bb);
					Vector<se::SECmp *> addr_filters = se::ADDR_FILTERS(bb);
					if(reg_filters.length() != 0 || addr_filters.length() != 0) {
						out << "\tFILTERS = \n";
						for(int i = 0; i < reg_filters.count(); i++) {
							out << "\t\t";
							reg_filters[i]->print(out, ws->process()->platform());
							out << "\n";
						}
						for(int i = 0; i < addr_filters.count(); i++) {
							out << "\t\t";
							addr_filters[i]->print(out, ws->process()->platform());
							out << "\n";
						}
					}
				}
				displayAfter(ws, cfg, bb);
			}

		}
		out << io::endl;

	}

	bool after, before, sem, filter;
};

// CLP textual displayer
class CLPDisplayer: public TextualDisplayer {
protected:

 	virtual void displayBefore(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {
 		clp::State state = CLP_STATE_IN(bb);
 		state.print(out, ws->process()->platform());
 		out<< io::endl;
 	}

 	virtual void displayAfter(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {
 		clp::State state = CLP_STATE_OUT(bb);
 		state.print(out, ws->process()->platform());
 		out<< io::endl;
 	}

};


class ODFA: public Application {
public:

	ODFA(void)
	: Application(
		"odfa",
		Version(1, 0, 0),
		"DataFlow Analysis Displayer",
		"H. Cassé <casse@irit.fr>"),
	clp(*this, option::cmd, "-c", option::cmd, "--clp", option::help, "display CLP information", option::end),
	list(*this, option::cmd, "-l", option::cmd, "--list", option::help, "display the list of registers", option::end),
	before(*this, option::cmd, "--before", option::help, "display state before the BB", option::end),
	after(*this, option::cmd, "--after", option::help, "display state after the BB", option::end),
	sem(*this, option::cmd, "-s", option::cmd, "--sem", option::help, "display semantics instructions", option::end),
	filter(*this, option::cmd, "-f", option::cmd, "--filter", option::help, "display filters", option::end),
	inits(*this, option::cmd, "-r", option::cmd, "--reg", option::help, "add an initialization register", option::arg_desc, "REGISTER=VALUE", option::end)
	{ }

protected:

	void work(const string& entry, PropList& props) throw (elm::Exception) {
		if(list)
			performList();
		else if(clp)
			performCLP(props);
	}

private:

	void performList(void) {

		// display platform
		hard::Platform *pf = workspace()->process()->platform();
		cout << "PLATFORM: " << pf->identification() << io::endl;

		// display registers
		const hard::Platform::banks_t& banks = pf->banks();
		const hard::Register *reg = 0;
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

	void performCLP(PropList& props) throw (elm::Exception) {

		// initialization
		fillRegs(props);
		BEFORE(props) = before;
		AFTER(props) = after;
		SEM(props) = sem;
		FILTER(props) = filter;

		// perform the analysis
		require(otawa::VIRTUALIZED_CFG_FEATURE);
		require(otawa::CLP_ANALYSIS_FEATURE);

		// display the CFG
		CLPDisplayer displayer;
		displayer.process(workspace(), props);
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
				ClpAnalysis::INITIAL(props) = pair(reg, Address(v));
				break;
			default:
				throw option::OptionException(_ << "unsupported register kind for initialization: " << s);
			}
		}
	}

	option::SwitchOption clp, list, before, after, sem, filter;
	option::ListOption<string> inits;
};

int main(int argc, char **argv) {
	ODFA app;
	return app.run(argc, argv);
}
