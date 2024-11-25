/*
 *	mkff utility
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-08, IRIT UPS.
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

#include <elm/checksum/Fletcher.h>
#include <elm/data/Vector.h>
#include <elm/io.h>

#include <elm/io/InFileStream.h>
#include <elm/io/OutFileStream.h>
#include <elm/options.h>
#include <elm/option/ValueOption.h>
#include <elm/sys/Path.h>
#include <elm/sys/System.h>
#include <elm/xom/Serializer.h>
#include <elm/option/ValueOption.h>

#include <otawa/app/Application.h>
#include <otawa/cfg.h>
#include <otawa/cfg/CFGChecker.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/cfg/features.h>
#include <otawa/cfgio/features.h>
#include <otawa/flowfact/ContextualLoopBound.h>
#include <otawa/flowfact/features.h>
#include <otawa/otawa.h>
#include <otawa/proc/DynFeature.h>
#include <otawa/proc/DynProcessor.h>
#include <otawa/prop/DynIdentifier.h>
#include <time.h>
#include <otawa/cfgio/Output.h>
#include <otawa/cfg/ContextTree.h>
#include <otawa/display/CFGDecorator.h>
#include <otawa/flowfact/FlowFactLoader.h>

#include "display_MultipleDotDisplayer.h"
#include "display_MKFFDotDisplayer.h"


using namespace elm;
using namespace otawa;

const char *noreturn_labels[] = {
	"_exit",
	"exit",
	0
};

const char *nocall_labels[] = {
	"__eabi",	// ppc-eabi-*
	"_main",	// tricore-*-*
	0
};


/**
 * @addtogroup commands
 * @section mkff mkff Command
 *
 * This command is used to generate F4 file template file template (usually
 * suffixed by a @e .ff) to pass flow facts
 * to OTAWA. Currently, only constant loop bounds are supported as flow facts.
 * Look the @ref f4 documentation for more details.
 *
 * @par SYNTAX
 * @code
 * $ mkff binary_file function1 function2 ...
 * @endcode
 *
 * mkff builds the .ff loop statements for each function calling sub-tree for
 * the given binary file. If no function name is given, only the main()
 * function is processed.
 *
 * The loop statement are indented according their depth in the context tree
 * and displayed with the current syntax:
 * @code
 * // Function function_name
 * loop loop1_address ?;
 *  loop loop1_1_address ?;
 *    loop loop_1_1_address ?;
 * loop loop2_address ?;
 * @endcode
 * The "?" question marks must be replaced by the maximum loop bound in order
 * to get valid .ff files. A good way to achieve this task is to use the
 * @ref dumpcfg command to get  a graphical display of the CFG.
 *
 * @par Example
 * @code
 * $ mkff fft1
 * // Function main
 * loop 0x100006c0 ?;
 *
 * // Function fft1
 * loop 0x10000860 ?;
 * loop 0x10000920 ?;
 *  loop 0x10000994 ?;
 *    loop 0x10000a20 ?;
 * loop 0x10000bfc ?;
 *  loop 0x10000d18 ?;
 * loop 0x10000dc0 ?;
 *
 * // Function sin
 * loop 0x10000428 ?;
 * loop 0x1000045c ?;
 * loop 0x10000540 ?;
 * @endcode
 *
 * @par Other information
 *
 * mkff has the ability to produce automatically other commands to handle
 * problematic or exotic flow fact structures:
 * @li false control instruction (branching to the next instruction to get
 * the PC values on some architecture),
 * @li undirect branches (switch-like construction, function pointer calls),
 * @li non-returning functions (like exit(), _exit()),
 * @li problematic initialization (like __eabi on EABI based platforms,
 * _main for tricore).
 *
 * @par Usage
 * In very complex programs, it may be required to launch mkff several times.
 *
 * As mkff may detect unsolved indirect branches (function pointer call or
 * swicth-like statements, the first phase consist to fill this kind
 * information and to relaunch mkff to scan unreachable parts of the program.
 * Possibly, some parts may also be cut to tune the WCET computation.
 *
 * As an example, we want to build the flow facts of the program xxx.
 * -# generate a first version: @c{$ mkff xxx > xxx.ff},
 * -# if required, fix the non-loop directives and removes the loop directives,
 * -# generate a new version: @c{$ mkff xxx >> xxx.ff},
 * -# while it remains unfixed non-loop,  restart at step 2.
 *
 * In the second phase, you must fix the loop directives, that is, to replace
 * the question marks '?' by actual loop iteration bounds.
 */

// Marker for recorded symbols
static Identifier<bool> RECORDED("recorded", false);


/**
 * Find a name for the current CFG. If there is a label, use it.
 * Else build an identifier based on its address.
 * @param cfg	Current CFG.
 * @return		Matching name.
 */
inline string nameOf(CFG *cfg) {
	string label = cfg->label();
	if(!label)
		label = _ << "0x" << cfg->address();
	else
		label = "\"" + label + "\"";
	return label;
}


/**
 * Make an address for F4 file.
 * @param cfg	Current CFG.
 * @param addr	Address of the item relative to the CFG.
 * @return		String representing the address.
 */
inline string makeAddress(CFG *cfg, Address addr) {
	string label = cfg->label();
	if(!label)
		return _ << "0x" << addr;
	else {
		int offset = cfg->address() - addr;
		StringBuffer buf;
		buf << label;
		if(offset) {
			if(offset >= 0)
				buf << '+';
			else
				buf << '-';
		}
		buf << io::hex(offset);
		return buf.toString();
	}
}


/**
 * Interface for printing FFXs.
 */
class Printer {
public:
	Printer(WorkSpace *ws, bool debug): _debug(debug), _ws(ws) { }
	virtual ~Printer(void) { }
	virtual void printNoReturn(Output& out, string label) = 0;
	virtual void printNoCall(Output& out, string label) = 0;
	virtual void printNoCall(Output& out, Address addr) = 0;
	virtual void printMultiBranch(Output& out, CFG *cfg, Inst *inst, Vector<Address>* va = 0) = 0;
	virtual void printMultiCall(Output& out, CFG *cfg, Inst *inst, Vector<Address>* va = 0) = 0;
	virtual void printIgnoreControl(Output& out, CFG *cfg, Inst *inst) = 0;
	virtual void printNoBlock(Output& out, Address addr) = 0;
	virtual void printIsReturn(Output& out, Address addr) = 0;
	virtual void startComment(Output& out) = 0;
	virtual void endComment(Output& out) = 0;
	virtual void printHeader(Output& out) = 0;
	virtual void printFooter(Output& out) = 0;
	virtual void printCheckSum(Output& out, Path path, t::uint32 sum) = 0;
	virtual void startFunction(Output& out, CFG *cfg) = 0;
	virtual void endFunction(Output& out) = 0;
	virtual void startLoop(Output& out, CFG *cfg, Inst *inst, bool contextual, Vector<SynthBlock*>& callContext) = 0;
	virtual void endLoop(Output& out, bool contextual, Vector<SynthBlock*>& callContext) = 0;

protected:
	bool printSourceLine(Output& out, Address address) {
		if(!_debug)
			return false;
		Option<Pair< cstring, int> > loc = _ws->process()->getSourceLine(address);
		if(loc)
			out << (*loc).fst << ":" << (*loc).snd;
		return bool(loc);
	}

	void printIndent(Output& out, int n) {
		for(int i = 0; i < n; i++)
			out << '\t';
	}

private:
	bool _debug;
	WorkSpace *_ws;
};

bool k(Inst* i) {
	return false;
}


/**
 * FF printer.
 */
class FFPrinter: public Printer {
public:
	FFPrinter(WorkSpace *ws, bool debug): Printer(ws, debug), indent(0) { }

	virtual void printNoReturn(Output& out, string label) {
		out << "noreturn \"" << label << "\";\n";
	}

	virtual void printNoCall(Output& out, string label) {
		out << "nocall \"" << label << "\";\n";
	}

	virtual void printNoCall(Output& out, Address addr) {
		out << "nocall 0x" << addr << ";\n";
	}

	/**
	 * Generate the target of multiple branch program structure
	 * @param out	The output destination
	 * @param CFG	Container CFG.
	 * @param inst	Instruction to get address of.
	 * @param va	The vector pointer containing the target addresses
	 */
	virtual void printMultiBranch(Output& out, CFG *cfg, Inst *inst, Vector<Address>* va) {
		out << "multibranch ";
		addressOf(out, cfg, inst->address());

		if(va) {
			out << " to "
				<< "\t// 0x" << inst->address() << " (";
			printSourceLine(out, inst->address());
			out << ")\n";
			for(Vector<Address>::Iter vai(*va); vai(); vai++) {
				out << "\t";
				addressOf(out, cfg, *vai);
				if(*vai == va->last())
					out << ";";
				else
					out << ",";
				out << "\t// 0x" << *vai << " (";
				printSourceLine(out, *vai);
				out << ") switch-like branch in " << nameOf(cfg) << io::endl;
			}
		}
		else if(IGNORE_CONTROL(inst)) {
			out << " has no target (infeasible path)."
				<< "\t// 0x" << inst->address() << " (";
			printSourceLine(out, inst->address());
			out << ") switch-like branch in " << nameOf(cfg) << io::endl;
		}
		else {
			out << " to ?;"
				<< "\t// 0x" << inst->address() << " (";
			printSourceLine(out, inst->address());
			out << ") switch-like branch in " << nameOf(cfg) << io::endl;
		}
		out << io::endl;
	}

	virtual void printMultiCall(Output& out, CFG *cfg, Inst *inst, Vector<Address>* va) {
		out << "multicall ";
		addressOf(out, cfg, inst->address());

		if(va) {
			out << " to "
				<< "\t// 0x" << inst->address() << " (";
			printSourceLine(out, inst->address());
			out << ")\n";
			for(Vector<Address>::Iter vai(*va); vai(); vai++) {
				out << "\t";
				addressOf(out, cfg, *vai);
				if(*vai == va->last())
					out << ";";
				else
					out << ",";
				out << "\t// 0x" << *vai << " (";
				printSourceLine(out, *vai);
				out << ") indirect call in " << nameOf(cfg) << io::endl;
			}
		}
		else if(IGNORE_CONTROL(inst)) {
			out << " has no target (infeasible path)."
				<< "\t// 0x" << inst->address() << " (";
			printSourceLine(out, inst->address());
			out << ") indirect call in " << nameOf(cfg) << io::endl;
		}
		else {
			out << " to ?;"
				<< "\t// 0x" << inst->address() << " (";
			printSourceLine(out, inst->address());
			out << ") indirect call in " << nameOf(cfg) << io::endl;
		}
		out << io::endl;
	}

	virtual void printIgnoreControl(Output& out, CFG *cfg, Inst *inst) {
		out << "ignorecontrol ";
		addressOf(out, cfg, inst->address());
		out << ";\t// " << nameOf(cfg) << " function\n";
	}

	virtual void printNoBlock(Output& out, Address addr) {
		out << "noblock 0x" << addr << ";\n";
	}

	virtual void printIsReturn(Output& out, Address addr) {
		out << "return 0x" << addr << ";\n";
	}

	virtual void startComment(Output& out) {
		out << "// ";
	}

	virtual void endComment(Output& out) {
		out << io::endl;
	}

	virtual void printHeader(Output& out) {
	}

	virtual void printFooter(Output& out) {
		cout << "\n";
	}

	virtual void printCheckSum(Output& out, Path path, t::uint32 sum) {
		cout << "checksum \"" << path.namePart()
			 << "\" 0x" << io::hex(sum) << ";\n";
	}

	virtual void startFunction(Output& out, CFG *cfg) {
		indent = -1;
		String label = cfg->label();
		if(!label)
			label = _ << "0x" << cfg->address();
		out << "// Function " << label << " (";
		this->printSourceLine(out, cfg->address());
		out << ")\n";
	}

	virtual void endFunction(Output& out) {
		out << io::endl;
	}

	virtual void startLoop(Output& out, CFG *cfg, Inst *inst, bool contextual, Vector<SynthBlock*>& callContext) {
		indent++;
		printIndent(out, indent);
		if(RECORDED(inst) || MAX_ITERATION(inst) != -1 || CONTEXTUAL_LOOP_BOUND(inst)) {
			out << "// loop ";
			addressOf(out, cfg, inst->address());
		}
		else {
			out << "loop ";
			addressOf(out, cfg, inst->address());
			out << " ? ";

			if(contextual) {
				out << "in";
				bool first = true;
				for(Vector<SynthBlock*>::Iter vsbi(callContext); vsbi(); vsbi++) {
					if(first)
						first = false;
					else
						out << " /";
					out << " \"" << vsbi->cfg()->name() << "\" @ ";
					out << "0x" << io::hex(vsbi->callInst()->address().offset()) ;
				}
				if(!first)
					out << " /";
				out << " \"" << cfg->name() << "\"";
			}


			out << "; // 0x" << io::hex(inst->address().offset());
		}
		out << " (";
		bool isFirst = ! printSourceLine(out, inst->address());
		if(inst->isRepeat())
			out << (isFirst ? "repeat instruction" : ", repeat instruction");
		out << ")\n";
	}

	virtual void endLoop(Output& out, bool contextual, Vector<SynthBlock*>& callContext) {
		indent--;
	}

private:
	int indent;

	void addressOf(io::Output& out, CFG *cfg, Address address) {
		string label = cfg->label();
		if(!label)
			out << "0x" << address;
		else {
			t::uint32 offset = address - cfg->address();
			out << '"' << label << '"';
			if(offset >= 0)
				out << " + 0x" << io::hex(offset);
			else
				out << " - 0x" << io::hex(-offset);
		}
	}
};


/**
 * FFX printer.
 */
class FFXPrinter: public Printer {
public:
	FFXPrinter(WorkSpace *ws, bool debug): Printer(ws, debug), indent(0) { }

	virtual void printNoReturn(Output& out, string label) {
		out << "\t<noreturn label=\"" << label << "\"/>\n";
	}

	virtual void printNoCall(Output& out, string label) {
		out << "\t<nocall label=\"" << label << "\"/>\n";
	}

	virtual void printNoCall(Output& out, Address addr) {
		out << "\t<nocall address=\"0x" << addr << "\"/>\n";
	}

	virtual void printMulti(Output& out, CFG *cfg, Inst *inst, Vector<Address>* va) {
		if(va)
			for(Vector<Address>::Iter vai(*va); vai(); vai++) {
				out << "\t\t<target ";
				addressOf(out, cfg, *vai);
				out << "/> ";
				out << "<!-- 0x" << *vai << " (";
				printSourceLine(out, *vai);
				out << ") -->";
				out << "\n";
			} // end of for
	}

	virtual void printMultiBranch(Output& out, CFG *cfg, Inst *inst, Vector<Address>* va) {
		out << "\t<!-- switch-like branch (" << inst->address() << ") in " << nameOf(cfg) << " -->\n";
		out << "\t<multibranch ";
		addressOf(out, cfg, inst->address());
		out << ">\n";
		printMulti(out, cfg, inst, va);
		out << "\t</multibranch>\n";
	}

	virtual void printMultiCall(Output& out, CFG *cfg, Inst *inst, Vector<Address>* va) {
		out << "\t<!-- indirect call (" << inst->address() << ") in " << nameOf(cfg) << " -->\n";
		out << "\t<multicall ";
		addressOf(out, cfg, inst->address());
		out << ">\n";
		printMulti(out, cfg, inst, va);
		out	<< "\t</multicall>\n";
	}

	virtual void printIgnoreControl(Output& out, CFG *cfg, Inst *inst) {
		out << "\t!<-- " << nameOf(cfg) << " function -->\n"
			<< "\t<ignorecontrol ";
		addressOf(out, cfg, inst->address() - cfg->address());
		out << "/>\n";
	}

	virtual void printNoBlock(Output& out, Address addr) {
		out << "\t<noblock address=\"0x" << addr << "\"/>\n";
	}

	virtual void printIsReturn(Output& out, Address addr) {
		out << "\t<return address=\"0x" << addr << "\"/>\n";
	}

	virtual void startComment(Output& out) {
		out << "\t<!-- ";
	}

	virtual void endComment(Output& out) {
		out << "-->";
	}

	virtual void printHeader(Output& out) {
		out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
				"<flowfacts\n"
				"	xmlns:xi=\"http://www.w3.org/2001/XInclude\"\n"
				"	xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\">\n\n";
	}

	virtual void printFooter(Output& out) {
		out << "</flowfacts>\n";
	}

	virtual void printCheckSum(Output& out, Path path, t::uint32 sum) {
		// ignored in FFX
	}

	virtual void startFunction(Output& out, CFG *cfg) {
		indent = 1;
		out << "\t<function ";
		string label = cfg->label();
		if(label)
			out << " label=\"" << label << "\"";
		else
			out << " address=\"0x" << cfg->address() << "\"";
		out << "> <!-- 0x" << cfg->address() << " (";
		printSourceLine(out, cfg->address());
		out << ") -->\n";
	}

	virtual void endFunction(Output& out) {
		out << "\t</function>\n\n";
	}

	virtual void startLoop(Output& out, CFG *cfg, Inst *inst, bool contextual, Vector<SynthBlock*>& callContext) {
		indent++;
		printIndent(out, indent);
		out << "<loop ";
		addressOf(out, cfg, inst->address());
		if(!(RECORDED(inst) || MAX_ITERATION(inst) != -1 || CONTEXTUAL_LOOP_BOUND(inst)))
			out << " maxcount=\"NOCOMP\"";
		out << "> <!-- 0x" << inst->address() << " (";
		printSourceLine(out, inst->address());
		out << ") -->\n";
	}

	virtual void endLoop(Output& out, bool contextual, Vector<SynthBlock*>& callContext) {
		printIndent(out, indent);
		out << "</loop>\n";
		indent--;
	}

private:
	int indent;

	void addressOf(io::Output& out, CFG *cfg, Address address) {
		string label = cfg->label();
		if(!label)
			out << "address=\"0x" << address << "\"";
		else {
			t::uint32 offset = address - cfg->address();
			out << "label=\"" << label << "\" offset=\"";
			if(offset > 0)
				out << "0x" << io::hex(offset);
			else
				out << "-0x" << io::hex(-offset);
			out << "\"";
		}
	}
};


// ControlOutput processor
class QuestFlowFactLoader;
class ControlOutput: public CFGProcessor {
public:
	ControlOutput(Printer&, QuestFlowFactLoader *ffl);
protected:
	void setup(WorkSpace *ws) override;
	void cleanup(WorkSpace *ws) override;
	void processAll(WorkSpace *ws) override;
	void processCFG(WorkSpace *ws, CFG *cfg) override;
private:
	void prepare(WorkSpace *ws, CFG *cfg);
	bool one;
	Printer& _printer;
	QuestFlowFactLoader *init_state;
};


// FFOutput processor
class FFOutput: public CFGProcessor {
public:
	FFOutput(Printer& printer, bool removeDuplicatedTarget, bool context);

protected:
	void setup(WorkSpace *ws) override {
		has_debug = ws->isProvided(otawa::SOURCE_LINE_FEATURE);
	}

	void processCFG(WorkSpace *ws, CFG *cfg) override;
private:
	void scanFun(ContextTree *ctree);
	void scanLoop(CFG *cfg, ContextTree *ctree, int indent, Vector<SynthBlock*>& callContext);
	//void scanLoop(CFG *cfg, ContextTree *ctree, int indent);
	bool checkLoop(ContextTree *ctree);
	void scanTargets(CFG *cfg);

	void printSourceLine(Output& out, WorkSpace *ws, Address address) {
		if(!has_debug)
			return;
		Option<Pair< cstring, int> > loc = ws->process()->getSourceLine(address);
		if(loc)
			out << (*loc).fst << ":" << (*loc).snd << io::endl;
	}

	bool has_debug;
	Printer& _printer;
	Vector<Address> displayedInstructions; // used to prevent same instruction being displayed twice.
	bool _removeDuplicatedTarget;
	bool contextual;
};


// QuestFlowFactLoader class
class QuestFlowFactLoader: public FlowFactLoader {
public:
	static p::declare reg;
	QuestFlowFactLoader(void): FlowFactLoader(reg), check_summed(false) { }

	inline bool checkSummed(void) const { return check_summed; }

	Vector<Address>& getRecordedNoCall() { return recorded_nocall; }
	Vector<Address>& getRecordedNoBlock() { return recorded_noblock; }

protected:

	virtual void onCheckSum(const String& name, unsigned long sum) {
		FlowFactLoader::onCheckSum(name, sum);
		check_summed = true;
	}

	virtual void onUnknownLoop(Address addr) { record(addr); }
	virtual void onUnknownMultiBranch(Address control) { record(control); }
	virtual void onIgnoreControl(Address address) {
		FlowFactLoader::onIgnoreControl(address);
		record(address);
	}
	virtual void onMultiBranch(Address control, const Vector< Address > &target) {
		FlowFactLoader::onMultiBranch(control, target);
		record(control);
	}
	virtual void onMultiCall(Address control, const Vector< Address > &target) {
		FlowFactLoader::onMultiCall(control, target);
		record(control);
	}
	virtual void onReturn(address_t addr) {
		FlowFactLoader::onReturn(addr);
		record(addr);
	}
	virtual void onNoCall(Address addr) {
		FlowFactLoader::onNoCall(addr);
		record(addr);
		recorded_nocall.add(addr);
	}
	virtual void onNoBlock(address_t addr, bool ignore_seq) {
		FlowFactLoader::onNoBlock(addr, ignore_seq);
		record(addr);
		recorded_noblock.add(addr);
	}

private:

	void record(Address addr) {
		Inst *inst = workSpace()->findInstAt(addr);
		if(!inst)
			onWarning(_ << "no instruction at " << addr);
		else
			RECORDED(inst) = true;
	}

	bool check_summed;

	//required as ControlOutput::processCFG only runs through the processed CFG, and so not the marked as nocall
	//  without it the further generated flowfact file is missing some nocall directives and reports errors where there
	//  shouldn't be
	Vector<Address> recorded_nocall;

	//required as ControlOutput::processCFG only runs through BB that don't have the NO_BLOCK attribute, and so BB marked
	//  as NO_BLOCK are not part of the CFG, and these attributes can't be rewritten in the new FF(X)
	Vector<Address> recorded_noblock;
};


p::declare QuestFlowFactLoader::reg = p::init("QuestFlowFactLoader", Version(1, 0, 0))
	.maker<FlowFactLoader>()
	.require(dfa::INITIAL_STATE_FEATURE)
	.provide(FLOW_FACTS_FEATURE)
	.provide(MKFF_PRESERVATION_FEATURE);


// Command class
class Command: public Application {
public:
	Command();
protected:
	void work(PropList &props) override;
private:
	struct GeneratedCFGType {
		enum CFGType {
			DEFAULT			= 0x00,
			MULTIDOT 		= 0x01,
			INLINED 		= 0x02,
			VIRTUALIZED		= 0x04,
			COLORED			= 0x08,
			SHOW_CLP		= 0x10,
			NO_SOURCE		= 0x20
		};
	};
	void generateCFGs(String path, int type = GeneratedCFGType::MULTIDOT);
	void generateXMLs(String path, int type, PropList &props);
	void analyzeBranches(PropList& props);

	option::Switch contextual, xml, dynbranch, /* modularized in the future */ outputCFG, outputInlinedCFG, outputVirtualizedCFG, removeDuplicatedTarget,
		showBlockProps, rawoutput, forFun, slicing, cfg4PS, cfg4LR, lightSlicing, debugging, nosource, debugSlicing, outputCFGXML, outputSimpleCFGXML;
};


/**
 *	@param path		where the CFGs will be stored
 *	@param single	single CFG output
 */
void Command::generateCFGs(String path, int type) {
	const CFGCollection& coll = **otawa::INVOLVED_CFGS(workspace()); // obtain the CFG Collection for outputing
	for(CFGCollection::Iter cfg(coll); cfg(); cfg++) {
		//display::AbstractGraph* ag;
		display::Decorator* d;

		// we can customize the CFG here
#if 0
		if(type & GeneratedCFGType::INLINED) {
			ag = new display::InlinedCFG(**cfg);
			d = new display::InlinedCFGDecorator(workspace());
		}
		else {
#endif
			//ag = new display::DisplayedCFG(**cfg);
			if(type & GeneratedCFGType::COLORED)
				d = new mkff::MKFFDotDecorator(workspace(), true, nosource | (type & GeneratedCFGType::NO_SOURCE), type & GeneratedCFGType::SHOW_CLP);
			else
				d = new mkff::MKFFDotDecorator(workspace(), false, nosource | (type & GeneratedCFGType::NO_SOURCE), type & GeneratedCFGType::SHOW_CLP);
#if 0
		}
#endif

		// obtain the displayer
		display::Displayer *disp = display::Provider::display(*cfg, *d, display::OUTPUT_RAW_DOT);

		// set up the path
		Path dir;
		if(path.length() == 0)
			dir = Path(".");
		else
			dir = Path(path);

		if(!dir.exists())
			sys::System::makeDir(dir);

		if(cfg->index() == 0)
			disp->setPath(dir / "index.dot");
		else
			disp->setPath(dir / string(_ << cfg->index() << "_" << cfg->name() << ".dot"));
		disp->process();
		delete disp;
		delete d;

		if((type & GeneratedCFGType::INLINED) || (type & GeneratedCFGType::VIRTUALIZED)) // if only output a single CFG, then break at the first iteration
			break;
	}
}


class XMLOutput: public otawa::cfgio::Output {
public:
	static p::declare reg;
	XMLOutput(void);
protected:
	virtual void configure(const PropList& props);
	virtual void processWorkSpace(WorkSpace *ws);
	virtual void processCFG(WorkSpace *ws, CFG *cfg);
	virtual void processBB(WorkSpace *ws, CFG *cfg, Block *b);
};

XMLOutput::XMLOutput(void): otawa::cfgio::Output() {
}


void XMLOutput::configure(const PropList& props) {
	otawa::cfgio::Output::configure(props);
}


void XMLOutput::processWorkSpace(WorkSpace *ws) {
	BBProcessor::processWorkSpace(ws);
}


void XMLOutput::processCFG(WorkSpace *ws, CFG *cfg) {
	// initial log
	if(logFor(LOG_DEPS))
		for(avl::Set<const AbstractIdentifier *>::Iter id(ids); id(); id++)
			log << "\tproperty " << id->name() << " include in the output\n";

	// build the root node
	root = new xom::Element("cfg-collection");

	cfgio::Output::processCFG(ws, cfg);

	// open output
	io::OutFileStream *file = 0;
	io::OutStream *out = &io::out;

	Path p = path;
	if(cfg->index() == 0)
		p = p / "index.xml";
	else
		p = p / string(_ << cfg->index() << "_" << cfg->name() << ".xml");

	if(p) {
		file = new OutFileStream(p);
		if(!file->isReady()) {
			cstring msg = file->lastErrorMessage();
			delete file;
			throw ProcessorException(*this, _ << "cannot open \"" << p << "\": " << msg);
		}
		out = file;
	}

	// output the XML
	xom::Document doc(root);
	xom::Serializer serial(*out);
	serial.write(&doc);
	serial.flush();

	// close file if needed
	if(file)
		delete file;
}

void XMLOutput::processBB(WorkSpace *ws, CFG *cfg, Block *b) {
	// add the basic
	if(!b->isEnd()) {

		// make the BB element
		xom::Element *bb_node = new xom::Element("bb");
		string _id = id(b);
		string num = _ << b->index();
		cfg_node->insertChild(bb_node, last_bb++);
		bb_node->addAttribute(new xom::Attribute("id", _id.asNullTerminated()));
		bb_node->addAttribute(new xom::Attribute("number", num.asNullTerminated()));

		// add the cafe babe instruction
		xom::Element *inst_node = new xom::Element("inst");
		bb_node->appendChild(inst_node);
		string addr = _ << "0xCAFEBABE";
		inst_node->addAttribute(new xom::Attribute("address", addr.asNullTerminated()));
	}

	// add the output edges
	for(Block::EdgeIter edge = b->outs(); edge(); edge++) {
		xom::Element *edge_node = new xom::Element("edge");
		cfg_node->appendChild(edge_node);
		string source = id(edge->source());
		edge_node->addAttribute(new xom::Attribute("source", source.asNullTerminated()));
		string target = id(edge->target());
		edge_node->addAttribute(new xom::Attribute("target", target.asNullTerminated()));
		processProps(edge_node, **edge);
	}
}

p::declare XMLOutput::reg = p::init("mkff::XMLOutput", Version(1, 0, 0))
	.maker<XMLOutput>()
	.base(otawa::cfgio::Output::reg);

class DetailedXMLOutput: public XMLOutput {
public:
	static p::declare reg;
	DetailedXMLOutput(void) : XMLOutput() {}
protected:
	virtual void configure(const PropList& props) { XMLOutput::configure(props); }
	virtual void processWorkSpace(WorkSpace *ws) { XMLOutput::processWorkSpace(ws); }
	virtual void processBB(WorkSpace *ws, CFG *cfg, Block *b) {
		// add the basic
		if(!b->isEnd()) {

			// make the BB element
			xom::Element *bb_node = new xom::Element("bb");
			string _id = id(b);
			string num = _ << b->index();
			cfg_node->insertChild(bb_node, last_bb++);
			bb_node->addAttribute(new xom::Attribute("id", _id.asNullTerminated()));
			bb_node->addAttribute(new xom::Attribute("number", num.asNullTerminated()));

			// add the each instruction
			if(b->isBasic()) {
				for(BasicBlock::InstIter inst= b->toBasic()->insts(); inst(); inst++) {
					xom::Element *inst_node = new xom::Element("inst");
					bb_node->appendChild(inst_node);
					string addr = _ << "0x" << inst->address();
					inst_node->addAttribute(new xom::Attribute("address", addr.asNullTerminated()));
				}
			}
		}

		// add the output edges
		for(Block::EdgeIter edge = b->outs(); edge(); edge++) {
			xom::Element *edge_node = new xom::Element("edge");
			cfg_node->appendChild(edge_node);
			string source = id(edge->source());
			edge_node->addAttribute(new xom::Attribute("source", source.asNullTerminated()));
			string target = id(edge->target());
			edge_node->addAttribute(new xom::Attribute("target", target.asNullTerminated()));
			processProps(edge_node, **edge);
		}
	}
};

p::declare DetailedXMLOutput::reg = p::init("mkff::DetailedXMLOutput", Version(1, 0, 0))
	.maker<DetailedXMLOutput>()
	.base(XMLOutput::reg);

void Command::generateXMLs(String path, int type, PropList &props) {

	// set up the path
	Path dir;
	if(path.length() == 0)
		dir = Path(".");
	else
		dir = Path(path);
	// create the directory if it does not exist
	if(!dir.exists())
		sys::System::makeDir(dir);

	DynIdentifier<Path> xmlOutputFolder("otawa::cfgio::OUTPUT");
	Path p(path);
	xmlOutputFolder(props) = p;

	if(type == 1) {
		DynProcessor dis("mkff::XMLOutput");
		dis.process(workspace(), props);
	}
	else {
		DynProcessor dis("mkff::DetailedXMLOutput");
		dis.process(workspace(), props);
	}
}


///
void Command::analyzeBranches(PropList& props) {
	int iteration = 0; // the nth time of the iteration
	bool branchDetected = false; // assuming there is no new branched detected, will be changed by the results of dynamic branch resolution
	bool first = true; // the first iteration
	do {
		if(first)
			first = false;
		else {
			workspace()->invalidate(COLLECTED_CFG_FEATURE);

			if(slicing) {
#ifndef USE_PLUGINS
				DynFeature f1("otawa::oslice::UNKNOWN_TARGET_COLLECTOR_FEATURE");
				workspace()->invalidate(f1);
				DynFeature f2("otawa::oslice::SLICER_FEATURE");
				workspace()->invalidate(f2);
#else
				workspace()->invalidate(otawa::oslice::UNKNOWN_TARGET_COLLECTOR_FEATURE);
				workspace()->invalidate(otawa::oslice::SLICER_FEATURE);
#endif
			}

			workspace()->require(COLLECTED_CFG_FEATURE, props); // rebuild the CFG
		} // end of the first time

		if(outputCFG)
			generateCFGs(String("") << iteration << "_iteration" /*, GeneratedCFGType::MULTIDOT*/);

		if(outputCFGXML)
			generateXMLs(String("") << iteration << "_iteration_xml" , 0, props);

		if(outputSimpleCFGXML)
			generateXMLs(String("") << iteration << "_iteration_xml_s" , 1, props);


		// before performing the analysis, maybe it is better to slice away the unnecessary ?
		if(slicing || lightSlicing) {

			if(debugSlicing) {
				DynIdentifier<int> sliceDebugLevel("otawa::oslice::DEBUG_LEVEL");
				sliceDebugLevel(props) = 0xFFFF;
			}

			workspace()->require(DynFeature("otawa::oslice::UNKNOWN_TARGET_COLLECTOR_FEATURE"), props);

			if(cfg4PS) {
				DynIdentifier<bool> cfgOutput("otawa::oslice::CFG_OUTPUT");
				cfgOutput(props) = true;
				DynIdentifier<String> slicingCFGOutput("otawa::oslice::SLICING_CFG_OUTPUT_PATH");
				slicingCFGOutput(props) = (String("./") << iteration << "_slicing");
				DynIdentifier<String> slicedCFGOutput("otawa::oslice::SLICED_CFG_OUTPUT_PATH");
				slicedCFGOutput(props) = (String("./") << iteration << "_sliced");
			}

			if(slicing)
				workspace()->require(DynFeature("otawa::oslice::SLICER_FEATURE"), props);
			else if(lightSlicing)
				workspace()->require(DynFeature("otawa::oslice::LIGHT_SLICER_FEATURE"), props);

			if(outputCFG || cfg4LR)
				generateCFGs(String("") << iteration << "_sliced" /*, GeneratedCFGType::MULTIDOT*/);

			if(outputCFGXML) // generate the xml file for future uses
				generateXMLs(String("") << iteration << "_sliced_xml", 0, props);

			if(outputSimpleCFGXML)
				generateXMLs(String("") << iteration << "_sliced_xml_s", 1, props);


#define REDUCE_LOOP
#ifdef REDUCE_LOOP
			workspace()->require(otawa::REDUCED_LOOPS_FEATURE, props);

			{
				int sum = 0;
				int sumCFG = 0;
				int sumB = 0;
				const CFGCollection* cfgc = INVOLVED_CFGS(workspace());
				for(CFGCollection::Iter cfg(cfgc); cfg(); cfg++) {
					sumCFG++;
					for(CFG::BlockIter bi = cfg->blocks(); bi(); bi++) {
						sumB++;
						if(bi->isBasic())
							sum = sum + bi->toBasic()->count();
						else
							continue;
					} // for each BB
				} // for each CFG
				elm::cerr << "[mkff] After loop reduction: " << sumCFG << " CFGs, " << sumB << " Blocks, " << sum << " instructions" << io::endl;
			}

			//generateCFGs(String("") << iteration << "_reduced", 0);

			// output the CFG after loop reduction
			if(outputCFG || cfg4LR) {
				generateCFGs(String("") << iteration << "_reduced",
					(outputInlinedCFG?GeneratedCFGType::INLINED:GeneratedCFGType::DEFAULT) |
					(forFun?GeneratedCFGType::COLORED:GeneratedCFGType::DEFAULT));
			}

			if(outputCFGXML)
				generateXMLs(String("") << iteration << "_reduced_xml", 0, props);

			if(outputSimpleCFGXML)
				generateXMLs(String("") << iteration << "_reduced_xml_s", 1, props);

#endif
		} // end of slicing

		// STEP: dynamic branch analysis
		// to ensure that the unknown block does not generate top values which wipes out the whole state
		// clp::UNKOWN_BLOCK_EVALUATION(workspace()->process()) = true;
		DynIdentifier<bool> CLP_UNKOWN_BLOCK_EVALUATION("otawa::clp::UNKOWN_BLOCK_EVALUATION");
		CLP_UNKOWN_BLOCK_EVALUATION(props) = true;


		// set it to false so the branch targets will be detected
		// otawa::dynbranch::NEW_BRANCH_TARGET_FOUND(workspace()) = false;
		DynIdentifier<bool> newBranchTargetFound("otawa::dynbranch::NEW_BRANCH_TARGET_FOUND");
		newBranchTargetFound(workspace()) = false;

		// workspace()->require(otawa::dynbranch::DYNBRANCH_FEATURE, props); // perform the analysis
		workspace()->require(DynFeature("otawa::dynbranch::DYNBRANCH_FEATURE"), props);

		// the loop goes on searching new branch target when there is a new target found
		//branchDetected = otawa::dynbranch::NEW_BRANCH_TARGET_FOUND(workspace());
		branchDetected = newBranchTargetFound(workspace());

		// clp::UNKOWN_BLOCK_EVALUATION(workspace()->process()) = false;
		CLP_UNKOWN_BLOCK_EVALUATION(props) = false;

		// show the result of the dynbranch analysis
		// generateCFGs(String("") << iteration << "_dynbranched", GeneratedCFGType::SHOW_CLP | GeneratedCFGType::NO_SOURCE);

		iteration++;
	} while(branchDetected);
}


/**
 */
void Command::work(PropList &props) {

	clock_t mkfftime = clock();

	// configure the CFG collection
	Application::parseAddress(arguments()[0]); // make sure the entry symbol is valid
	TASK_ENTRY(props) = arguments()[0];
	for(int i = 1; i < arguments().length(); i++)
		ADDED_FUNCTION(props).add(arguments()[i].toCString());
	CFGChecker::NO_EXCEPTION(props) = true;

	// outputing the original CFG when required
	if(outputCFG) {
		workspace()->require(COLLECTED_CFG_FEATURE, props);
		generateCFGs(String("") << "begin" /*, GeneratedCFGType::MULTIDOT*/);
	}

	if(outputCFGXML)
		generateXMLs(String("") << "begin_xml" , 0, props);

	if(outputSimpleCFGXML)
		generateXMLs(String("") << "begin_xml_s" , 1, props);

	Printer *p = nullptr;

	// Load flow facts and record unknown values
	QuestFlowFactLoader ffl;
	workspace()->run(&ffl, props);

	if(!debugging) {
		// determine printer
		if(xml)
			p = new FFXPrinter(workspace(), true);
		else
			p = new FFPrinter(workspace(), true);

		// printer header
		p->printHeader(cout);

		// Build the checksums of the binary files
		if(!ffl.checkSummed()) {
			for(Process::FileIter file(workspace()->process()); file(); file++) {
				checksum::Fletcher sum;
				io::InFileStream stream(file->name());
				sum.put(stream);
				elm::sys::Path path = file->name();
				p->printCheckSum(cout, path, sum.sum());
			}
			cout << io::endl;
		}
	}

	// Enable the dynamic branch
	if(dynbranch)
		analyzeBranches(props);

	// get the CFG
	if(workspace()->isProvided(COLLECTED_CFG_FEATURE))
		workspace()->invalidate(COLLECTED_CFG_FEATURE); // clean the sliced CFG
	workspace()->require(COLLECTED_CFG_FEATURE, props); // the final full CFG

	if(outputCFG)
		generateCFGs(String("") << "final" /*, GeneratedCFGType::MULTIDOT*/);

	if(outputCFGXML)
		generateXMLs(String("") << "final_xml", 0, props);

	if(outputSimpleCFGXML)
		generateXMLs(String("") << "final_xml_s", 1, props);

	if(!debugging) {

		// display low-level flow facts
		ControlOutput *ctrl = new ControlOutput(*p, &ffl);
		workspace()->run(ctrl, props);

		// display the context tree
		FFOutput *ffout = new FFOutput(*p, removeDuplicatedTarget, contextual);
		workspace()->run(ffout, props);

		// output footer for XML
		p->printFooter(cout);

		// cleanup at end
		delete p;
	}
	else {

		class printer {
		public:
			void addressOf(io::Output& out, CFG *cfg, Address address) {
				string label = cfg->label();
				if(!label)
					out << "0x" << address;
				else {
					t::uint32 offset = address - cfg->address();
					out << '"' << label << '"';
					if(offset > 0)
						out << " + 0x" << io::hex(offset);
					else
						out << " - 0x" << io::hex(-offset);
				}
			}

			void printMulti(Output& out, CFG *cfg, Inst *inst, Vector<Address>* va, WorkSpace* _ws, String s) {
				out << s << " ";
				addressOf(out, cfg, inst->address());

				if(va->count()) {
					out << " to "
						<< "\t// 0x" << inst->address() << "\n";
					for(Vector<Address>::Iter vai(*va); vai(); vai++) {
						out << "\t";
						addressOf(out, cfg, *vai);
						if(*vai == va->last())
							out << ";";
						else
							out << ",";
						out << "\t// 0x" << *vai << " switch-like branch in " << nameOf(cfg) << io::endl;
					}
				}
				else if(IGNORE_CONTROL(inst))
					out << " has no target (infeasible path)." << "\t// 0x" << inst->address() << " switch-like branch in " << nameOf(cfg) << io::endl;
				else
					out << " to ?;" << "\t// 0x" << inst->address() << " switch-like branch in " << nameOf(cfg) << io::endl;
				out << io::endl;
			}
		};


		Vector<Address> displayedInstructions;
		const CFGCollection* cfgc = INVOLVED_CFGS(workspace());
		for(CFGCollection::Iter cfg(cfgc); cfg(); cfg++) {
			for(CFG::BlockIter bi = cfg->blocks(); bi(); bi++) {
				// only treats BB
				if(!bi->isBasic())
					continue;
				BasicBlock* bb = bi->toBasic();
				Inst* control = bb->control();

				if(control == nullptr || control->isReturn() || RECORDED(control)
				|| PRESERVED(control) || control->target() != nullptr)
					continue;

				if(BRANCH_TARGET(control).exists()) {
					Vector<Address> va;
					for(Identifier<Address>::Getter target(control, BRANCH_TARGET); target(); target++)
						va.push(*target);
					if(removeDuplicatedTarget) {
						// to prevent same instruction printed twice
						if(displayedInstructions.contains(control->address()))
							continue;
						else if(va)
							displayedInstructions.add(control->address());
					}
					printer().printMulti(elm::cout, *cfg, control, &va, workspace(), "multibranch");
				}
				else if(CALL_TARGET(control).exists()) {
					Vector<Address> va;
					for(Identifier<Address>::Getter target(control, CALL_TARGET); target(); target++)
						va.push(*target);
					if(removeDuplicatedTarget) {
						// to prevent same instruction printed twice
						if(displayedInstructions.contains(control->address()))
							continue;
						else if(va)
							displayedInstructions.add(control->address());
					}
					printer().printMulti(elm::cout, *cfg, control, &va, workspace(), "multicall");
				}
				else {
					Vector<Address> va;
					if(control->isCall())
						printer().printMulti(elm::cout, *cfg, control, &va, workspace(), "multicall");
					else
						printer().printMulti(elm::cout, *cfg, control, &va, workspace(), "multibranch");
				}

			} // for each BB
		} // for each CFG
	} // debugging

	mkfftime = clock() - mkfftime;
	elm::cerr << "mkff: " << mkfftime << " micro-seconds" << io::endl;

}


/**
 * Build the command.
 */
Command::Command():
	otawa::Application(Make("mkff", Version(1, 1, 0))
			.author("Hugues Cassé <casse@irit.fr>")
			.description("Generate a flow fact file for an application.")
			.copyright("Copyright (c) 2005-17, IRIT - UPS")),
		contextual				(make_switch().cmd("-E").cmd("--context")				.help("enable contextual path output for loop bound")),
		xml						(make_switch().cmd("-x").cmd("--ffx")					.help("activate FFX output")),
		dynbranch				(make_switch().cmd("-D").cmd("--dynbranch")				.help("check for dynamic branches")),
		outputCFG				(make_switch().cmd("-C").cmd("--cfg_output")			.help("output cfg in a given fashion (otawa::display::CFGOutput::PATH, KIND)")),
		outputInlinedCFG		(make_switch().cmd("-I").cmd("--inlined_cfg")			.help("the output cfg is inlined (otawa::display::CFGOutput::INLINED = true), implies -C")),
		outputVirtualizedCFG	(make_switch().cmd("-V").cmd("--virtualized_cfg")		.help("the output cfg is virtualized (otawa::display::CFGOutput::VIRTUALIZED = true), implies -C -I")),
		removeDuplicatedTarget	(make_switch().cmd("-R").cmd("--no_repeat_multibranch")	.help("do not output the multi-call/branch with the same target addresses")),
		showBlockProps			(make_switch().cmd("-P").cmd("--show_block_props")		.help("shows the properties of the block")),
		rawoutput				(make_switch().cmd("-RO").cmd("--raw_output")			.help("generate raw dot output file (without calling dot)")),
		forFun					(make_switch().cmd("-F").cmd("--for_fun")				.help("the generated dot files will be colorful :)")),
		slicing					(make_switch().cmd("-T").cmd("--test")					.help("apply the slicing during dynamic branching analysis")),
		cfg4PS					(make_switch().cmd("-S").cmd("--show_cfg_SL")			.help("generate DOT files before and after program slicing (PS)")),
		cfg4LR					(make_switch().cmd("-L").cmd("--show_cfg_LR")			.help("generate DOT files before and after loop reduction (LR)")),
		lightSlicing			(make_switch().cmd("-LS").cmd("--light_slicing")		.help("apply the slicing (light) during dynamic branching analysis")),
		debugging				(make_switch().cmd("-DBG").cmd("--debugging")			.help("fast output generation")),
		nosource				(make_switch().cmd("-NS").cmd("--no_source")			.help("do not output source code in the generated CFGs")),
		debugSlicing			(make_switch().cmd("-DS").cmd("--debug_slicing")		.help("show the debugging message of slicing")),
		outputCFGXML			(make_switch().cmd("-X").cmd("--xml_output")			.help("generate XML files of each CFG for the initial, the iterations, and the final phases")),
		outputSimpleCFGXML		(make_switch().cmd("-Y").cmd("--simple_xml_output")		.help("generate simpler XML file for each CFG, this option generates empty blocks for understanding the structure of the CFGs"))
{
}


/**
 * Display the flow facts.
 */
FFOutput::FFOutput(Printer& printer, bool removeDuplicatedTarget, bool context): CFGProcessor("FFOutput", Version(1, 0, 0)), has_debug(false), _printer(printer), _removeDuplicatedTarget(removeDuplicatedTarget), contextual(context) {
	require(CONTEXT_TREE_BY_CFG_FEATURE);
}


/**
 */
void FFOutput::processCFG(WorkSpace *ws, CFG *cfg) {
	ASSERT(ws);
	ASSERT(cfg);
	ContextTree *ctree = CONTEXT_TREE(cfg);
	ASSERT(ctree);
	scanFun(ctree);
}

/**
 * Find the instruction with multiple branch/call target
 * @param cfg The current working CFG
 */
void FFOutput::scanTargets(CFG *cfg) {
	for(CFG::BlockIter bi = cfg->blocks(); bi(); bi++) {

		// only treats BB
		if(!bi->isBasic())
			continue;

		BasicBlock* bb = bi->toBasic();
		Inst* lastInst = bb->last();

		if(BRANCH_TARGET(lastInst).exists()) {
			Vector<Address> va;
			for(Identifier<Address>::Getter target(lastInst, BRANCH_TARGET); target(); target++)
				va.push(*target);

			if(_removeDuplicatedTarget) {
				// to prevent same instruction printed twice
				if(displayedInstructions.contains(lastInst->address())) {
					continue;
				}
				else if(va) {
					displayedInstructions.add(lastInst->address());
				}
			}

			_printer.printMultiBranch(out, cfg, lastInst, &va);
		}
		else if(CALL_TARGET(lastInst).exists()) {
			Vector<Address> va;
			for(Identifier<Address>::Getter target(lastInst, CALL_TARGET); target(); target++)
				va.push(*target);

			if(_removeDuplicatedTarget) {
				// to prevent same instruction printed twice
				if(displayedInstructions.contains(lastInst->address())) {
					continue;
				}
				else if(va) {
					displayedInstructions.add(lastInst->address());
				}
			}

			_printer.printMultiCall(out, cfg, lastInst, &va);
		}
	}
}

/**
 * Process a function context tree node.
 * @param ctree	Function context tree node.
 */
void FFOutput::scanFun(ContextTree *ctree) {
	ASSERT(ctree);

	// Display header
	if(checkLoop(ctree)) {

		// Display header
		_printer.startFunction(out, ctree->cfg());

		// scan the loop
		Vector<SynthBlock*> callContext;
		scanLoop(ctree->cfg(), ctree, 0, callContext);

		// Displayer footer
		_printer.endFunction(out);
	}

	// Scan the dynamic branch
	scanTargets(ctree->cfg());

}


/**
 * Scan a context tree for displaying its loop flow facts.
 * @param cfg		Container CFG.
 * @param ctree		Current context tree.
 * @param indent	Current indentation.
 */
void FFOutput::scanLoop(CFG *cfg, ContextTree *ctree, int indent, Vector<SynthBlock*>& callContext) {
	ASSERT(ctree);

	// process sub-loops
	for(ContextTree::ChildrenIterator child(ctree); child(); child++) {
		ASSERT(child->kind() != ContextTree::FUNCTION);

		// Process loop
		if(child->kind() == ContextTree::LOOP) {

			if(contextual && indent == 0) {
				Vector<Vector<SynthBlock*> > vv;
				Vector<SynthBlock*> v;
				vv.add(v);
				Vector<SynthBlock*> p;

				for(auto ci: cfg->callers())
					vv[0].add(*ci);

				int currLevel = 0;

				if(vv[0].count() == 0) { // only main function
					_printer.startLoop(out, cfg, child->bb()->first(), contextual, p);
					scanLoop(cfg, *child, indent + 1, p);
					_printer.endLoop(out, contextual, p);
				}
				else {
					while(vv[currLevel].count()) {
						SynthBlock* sb = vv[currLevel].pop();
						if(p.contains(sb)) {
							elm::cerr << "Warning: processing recursive call " << sb->cfg()->name() << ", assert." << endl;
							ASSERT(0);
						}
						p.addFirst(sb);

						currLevel++;
						if(vv.count() == currLevel)
							vv.add(v);
						for(auto ci: sb->cfg()->callers())
							vv[currLevel].add(*ci);

						if(vv[currLevel].count() == 0) { // reaches the end
							_printer.startLoop(out, cfg, child->bb()->first(), contextual, p);
							scanLoop(cfg, *child, indent + 1, p);
							_printer.endLoop(out, contextual, p);
						}
						while(vv[currLevel].count() == 0 && currLevel > 0) {
							currLevel--;
							p.removeFirst();
						}

					}
				}
			} // only need to find all the context at the top level loop
			else {
				_printer.startLoop(out, cfg, child->bb()->first(), contextual, callContext);
				scanLoop(cfg, *child, indent + 1, callContext);
				_printer.endLoop(out, contextual, callContext);
			}
		}
	}

	// process REPEAT instructions
	for(ContextTree::BlockIterator b(ctree); b(); b++)
		if(b->isBasic())
			for(auto i: *b->toBasic())
				if(i->isRepeat()) {
					_printer.startLoop(out, cfg, i, contextual, callContext);
					_printer.endLoop(out, contextual, callContext);
				}

}


bool FFOutput::checkLoop(ContextTree *ctree) {

	// look for a loop
	for(ContextTree::ChildrenIterator child(ctree); child(); child++) {
		ASSERT(child->kind() != ContextTree::FUNCTION);
		if(child->kind() == ContextTree::LOOP) {
			BasicBlock::InstIter inst(child->bb());
			if((!RECORDED(*inst) && MAX_ITERATION(*inst) == -1)
			|| checkLoop(*child))
				return true;
		}
	}

	// look for a repeat instruction
	for(auto b: *ctree->cfg())
		if(b->isBasic())
			for(auto i: *b->toBasic())
				if(i->isRepeat())
					return true;

	return false;
}


/**
 * @class ControlOutput
 * Output information about the control.
 */


/**
 * Constructor.
 */
ControlOutput::ControlOutput(Printer& printer, QuestFlowFactLoader *ffl)
: CFGProcessor("ControlOutput", Version(1, 1, 0)), one(false), _printer(printer), init_state(ffl) {
}


/**
 */
void ControlOutput::setup(WorkSpace *ws) {
	one = false;
}

void ControlOutput::processAll(WorkSpace *ws) {
	for(Address addr : init_state->getRecordedNoCall())
		_printer.printNoCall(out, addr);
	for(Address addr : init_state->getRecordedNoBlock())
		_printer.printNoBlock(out, addr);
	CFGProcessor::processAll(ws);
}


/**
 */
void ControlOutput::processCFG(WorkSpace *ws, CFG *cfg) {

	// Look for labels
	Inst *inst = ws->findInstAt(cfg->address());
	if(!PRESERVED(inst)) {
		string label = cfg->label();
		if(label) {
			for(int i = 0; noreturn_labels[i]; i++)
				if(label == noreturn_labels[i])
					_printer.printNoReturn(out, label);
			for(int i = 0; nocall_labels[i]; i++)
				if(label == nocall_labels[i])
					_printer.printNoCall(out, label);
		}
	}

	// Look in BB
	//for(CFG::BBIterator bb(cfg); bb; bb++)
	for(CFG::BlockIter bi = cfg->blocks(); bi(); bi++) {
		if(!bi->isBasic())
			continue;
		BasicBlock* bb = bi->toBasic();

		//for(BasicBlock::InstIter inst(bb); inst; inst++)
		Inst *inst = bb->control();
		if(inst != nullptr
		&& !inst->isReturn()
		&& !RECORDED(inst)
		&& !PRESERVED(inst)) {

			// Undefined branch target
			if(!inst->target()) {
				if(!BRANCH_TARGET(inst).exists() && !CALL_TARGET(inst).exists()) {
					prepare(ws, cfg);
					cstring type, com;
					if(!inst->isCall())
						_printer.printMultiBranch(out, cfg, inst);
					else // if(inst->isCall())
						_printer.printMultiCall(out, cfg, inst);
				}
			}

			// call to next instruction
			else if(inst->isCall()
			&& inst->target()->address() == inst->topAddress()) {
				prepare(ws, cfg);
				_printer.printIgnoreControl(out, cfg, inst);
			}
		}
	}
}


/**
 * Prepare the displaty of some information.
 * @param ws		Current workspace.
 * @param cfg		Current CFG.
 */
void ControlOutput::prepare(WorkSpace *ws, CFG *cfg) {
	if(!one) {
		_printer.startComment(out);
		out <<  "Low-level flow facts";
		_printer.endComment(out);
		one = true;
	}
}


/**
 */
void ControlOutput::cleanup(WorkSpace *ws) {
	if(one)
		out << io::endl;
}

OTAWA_RUN(Command);
