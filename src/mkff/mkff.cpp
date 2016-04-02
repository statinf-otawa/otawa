/*
 *	$Id$
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

#include <elm/io.h>
#include <elm/options.h>
#include <elm/option/StringOption.h>
#include <elm/genstruct/Vector.h>
#include <otawa/cfg.h>
#include <otawa/otawa.h>
#include <otawa/util/ContextTree.h>
#include <elm/checksum/Fletcher.h>
#include <elm/io/InFileStream.h>
#include <elm/system/Path.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/util/FlowFactLoader.h>
#include <otawa/flowfact/features.h>
#include <otawa/cfg/CFGChecker.h>
#include <otawa/app/Application.h>
#include <otawa/flowfact/ContextualLoopBound.h>
#include <otawa/dynbranch/features.h>
#include <otawa/display/CFGOutput.h>

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
 * Compute an address for an item, relative to the container CFG if
 * possible.
 * @param CFG		Container CFG.
 * @param address	Address of the item.
 * @param xml		Use XML output.
 * @return			String representing the address of the instruction in F4.
 */
/*inline string addressOf(CFG *cfg, Address address, bool xml = false) {
	string label = cfg->label();
	if(!label) {
		if(xml)
			return _ << "address=\"0x" << address << "\"";
		else
			return _ << "0x" << address;
	}
	t::uint32 offset = address - cfg->address();
	StringBuffer buf;
	if(xml) {
		buf << "label=\"" << label << "\" offset=\"";
	}
	else
		buf << '"' << label << '"';
	if(offset > 0)
		buf << " + 0x" << io::hex(offset);
	else
		buf << " - 0x" << io::hex(-offset);
	if(xml)
		buf << "\"";
	return buf.toString();
}*/


/**
 * Compute an address for an instruction, relative to the container CFG if
 * possible.
 * @param CFG	Container CFG.
 * @param inst	Instruction to get address of.
 * @return		String representing the address of the instruction in F4.
 */
/*inline string addressOf(CFG *cfg, Inst *inst) {
	return addressOf(cfg, inst->address());
}*/


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
	virtual void printMultiBranch(Output& out, CFG *cfg, Inst *inst, Vector<Address>* va = 0) = 0;
	virtual void printMultiCall(Output& out, CFG *cfg, Inst *inst, Vector<Address>* va = 0) = 0;
	virtual void printIgnoreControl(Output& out, CFG *cfg, Inst *inst) = 0;
	virtual void startComment(Output& out) = 0;
	virtual void endComment(Output& out) = 0;
	virtual void printHeader(Output& out) = 0;
	virtual void printFooter(Output& out) = 0;
	virtual void printCheckSum(Output& out, Path path, t::uint32 sum) = 0;
	virtual void startFunction(Output& out, CFG *cfg) = 0;
	virtual void endFunction(Output& out) = 0;
	virtual void startLoop(Output& out, CFG *cfg, Inst *inst) = 0;
	virtual void endLoop(Output& out) = 0;

protected:
	void printSourceLine(Output& out, Address address) {
		if(!_debug)
			return;
		Option<Pair< cstring, int> > loc = _ws->process()->getSourceLine(address);
		if(loc)
			out << (*loc).fst << ":" << (*loc).snd;
	}

	void printIndent(Output& out, int n) {
		for(int i = 0; i < n; i++)
			out << '\t';
	}

private:
	bool _debug;
	WorkSpace *_ws;
};



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
			for(Vector<Address>::Iterator vai(*va); vai; vai++) {
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
			for(Vector<Address>::Iterator vai(*va); vai; vai++) {
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

	virtual void startLoop(Output& out, CFG *cfg, Inst *inst) {
		indent++;
		printIndent(out, indent);
		if(RECORDED(inst) || MAX_ITERATION(inst) != -1 || CONTEXTUAL_LOOP_BOUND(inst)) {
			out << "// loop ";
			addressOf(out, cfg, inst->address());
			out << " (";
			printSourceLine(out, inst->address());
			out << ")\n";
		}
		else
			out << "loop ";
			addressOf(out, cfg, inst->address());
			out << " ?; // " << inst->address() << " (";
			printSourceLine(out, inst->address());
			out << ")\n";
	}

	virtual void endLoop(Output& out) {
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
			if(offset > 0)
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

	virtual void printMulti(Output& out, CFG *cfg, Inst *inst, Vector<Address>* va) {
		if(va)
			for(Vector<Address>::Iterator vai(*va); vai; vai++) {
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

	virtual void startComment(Output& out) {
		out << "\t<!-- ";
	}

	virtual void endComment(Output& out) {
		out << "-->";
	}

	virtual void printHeader(Output& out) {
		cout << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
				"<flowfacts\n"
				"	xmlns:xi=\"http://www.w3.org/2001/XInclude\"\n"
				"	xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\">\n\n";
	}

	virtual void printFooter(Output& out) {
		cout << "</flowfacts>\n";
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

	virtual void startLoop(Output& out, CFG *cfg, Inst *inst) {
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

	virtual void endLoop(Output& out) {
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
class ControlOutput: public CFGProcessor {
public:
	ControlOutput(Printer& printer);
protected:
	virtual void setup(WorkSpace *ws);
	virtual void cleanup(WorkSpace *ws);
	virtual void processCFG(WorkSpace *ws, CFG *cfg);
private:
	void prepare(WorkSpace *ws, CFG *cfg);
	bool one;
	Printer& _printer;
};


// FFOutput processor
class FFOutput: public CFGProcessor {
public:
	FFOutput(Printer& printer, bool removeDuplicatedTarget);
protected:
	virtual void setup(WorkSpace *ws) {
		has_debug = ws->isProvided(otawa::SOURCE_LINE_FEATURE);
	}

	virtual void processCFG(WorkSpace *ws, CFG *cfg);
private:
	void scanFun(ContextTree *ctree);
	void scanLoop(CFG *cfg, ContextTree *ctree, int indent);
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
};


// QuestFlowFactLoader class
class QuestFlowFactLoader: public FlowFactLoader {
public:
	static p::declare reg;
	QuestFlowFactLoader(void): FlowFactLoader(reg), check_summed(false) { }

	inline bool checkSummed(void) const { return check_summed; }

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

private:

	void record(Address addr) {
		Inst *inst = workSpace()->findInstAt(addr);
		if(!inst)
			onError(_ << "no instruction at " << addr);
		RECORDED(inst) = true;
	}

	bool check_summed;
};


p::declare QuestFlowFactLoader::reg = p::init("QuestFlowFactLoader", Version(1, 0, 0))
	.maker<FlowFactLoader>();


// Command class
class Command: public Application {
public:
	Command(void);
protected:
	virtual void work(PropList &props) throw(elm::Exception);
private:
	option::SwitchOption xml, dynbranch, /* modularized in the future */ outputCFG, outputInlinedCFG, outputVirtualizedCFG, removeDuplicatedTarget, showBlockProps, rawoutput, forFun;
};


/**
 */
void Command::work(PropList &props) throw(elm::Exception) {

	// configure the CFG collection
	TASK_ENTRY(props) = arguments()[0];
	for(int i = 1; i < arguments().length(); i++)
		CFGCollector::ADDED_FUNCTION(props).add(arguments()[i].toCString());
	CFGChecker::NO_EXCEPTION(props) = true;

	// Enable the dynamic branch
	if(dynbranch) {
		class CFGOutput: public otawa::display::CFGOutput { // for simple CFG output facility
		public:
			void processCharacters(StringBuffer& sb, Output& out) {
				String tempString = sb.toString();
				for(int i = 0; i < tempString.length(); i++){
					char c = tempString[i];
					if(c == '{' || c == '}' || c == '|' || c == '\\' || c == '"') { // adding '\' as the escape character
						out << '\\';
						out << c;
					}
					else if(c == '<')
						out << "&lt;";
					else if(c == '>')
						out << "&gt;";
					else
						out << c;
				}
			}
			CFGOutput(bool _showProp, bool _forFun): otawa::display::CFGOutput(), showProp(_showProp), forFun(_forFun) { }
			inline void genBBInfo(CFG *cfg, Block *bb, Output& out) {
				if(!showProp)
					return;
				out << " | ";
				for(PropList::Iter prop(bb); prop; prop++) {
					out << prop->id()->name() << " = ";
					StringBuffer temp;
					prop->id()->print(temp, prop);
					processCharacters(temp, out);
					out << "<br ALIGN=\"LEFT\"/>";
				}
			}
			inline void genEdgeInfo(CFG *cfg, otawa::Edge *edge, Output& out) { /* nothing on the edge */ }

			void genBBLabel(CFG *cfg, Block *b, Output& out) {
				// display title
				out << b;
				// special of entry, exit or synthetic
				if(b->isEnd() || b->isSynth())
					return;
				BasicBlock *bb = b->toBasic();
				// this is used by the AbstractDrawer::Vertex::setup as the separator between the title and the content of the node body
				out << "\n---\n";
				// make body
				cstring file;
				int line = 0;
				system::StopWatch watch;
				if(forFun) { // get the random seed
					watch.start();
					watch.stop();
					srand (watch.startTime());
				}
				for(BasicBlock::InstIter inst(bb); inst; inst++){ // the body:
					// display labels
					for(Identifier<String>::Getter label(inst, FUNCTION_LABEL); label; label++)
						out << *label << ":<br ALIGN=\"LEFT\"/>";
					for(Identifier<String>::Getter label(inst, otawa::LABEL); label; label++)
						out << *label << ":<br ALIGN=\"LEFT\"/>";
					Option<Pair<cstring, int> > info = workspace()->process()->getSourceLine(inst->address());
					if(info) {
						if((*info).fst != file || (*info).snd != line) { // only output once if a file:line is of many instructions
							file = (*info).fst;
							line = (*info).snd;
							out << file << ":" << line << "<br ALIGN=\"LEFT\"/>";
						}
					}
					else {
						file = "";
						line = 0;
					}
					// display the instruction
					if(forFun)
						out << "<Font color=\"#" << hex(rand()%255) << hex(rand()%255) << hex(rand()%255) << "\">";
					out << "0x" << ot::address(inst->address()) << ":  ";
					// inst->dump(out);
					StringBuffer temp;
					inst->dump(temp);
					processCharacters(temp, out);
					if(forFun)
						out << "</Font>";
					out << "<br ALIGN=\"LEFT\"/>";
				}
				// give special format for Entry and Exit
				genBBInfo(cfg, bb, out);
			}
		private:
			bool showProp, forFun;
		};

		otawa::display::CFGOutput::INLINING(props) = outputInlinedCFG;
		otawa::display::CFGOutput::VIRTUALIZED(props) = outputVirtualizedCFG;
		if(!otawa::display::CFGOutput::KIND(props).exists())
			otawa::display::CFGOutput::KIND(props) = otawa::display::OUTPUT_DOT;
		if(!otawa::display::CFGOutput::PATH(props).exists())
			otawa::display::CFGOutput::PATH(props) = ".";
		if(rawoutput)
			otawa::display::CFGOutput::KIND(props) = otawa::display::OUTPUT_RAW_DOT;
		CFGOutput::RAW_BLOCK_INFO(props) = true;

		int iteration = 0;
		bool branchDetected = false;
		bool first = true;
		do {
			if(first)
				first = false;
			else {
				workspace()->invalidate(COLLECTED_CFG_FEATURE);
				workspace()->require(COLLECTED_CFG_FEATURE, props);
			} // end of the first time

			if(outputInlinedCFG || outputVirtualizedCFG || outputCFG) {
				string iterationString = _ << iteration << "_";
				otawa::display::CFGOutput::PREFIX(props) = iterationString;
				CFGOutput(showBlockProps, forFun).process(workspace(), props);
			}

			otawa::dynbranch::NEW_BRANCH_TARGET_FOUND(workspace()) = false; // clear the flag
			workspace()->require(otawa::dynbranch::DYNBRANCH_FEATURE, props);
			// the loop goes on searching new branch target when there is a new target found
			branchDetected = otawa::dynbranch::NEW_BRANCH_TARGET_FOUND(workspace());

			iteration++;
		} while(branchDetected);

		if(outputInlinedCFG || outputVirtualizedCFG || outputCFG) {
			string iterationString = _ << iteration << "_";
			otawa::display::CFGOutput::PREFIX(props) = iterationString;
			CFGOutput(showBlockProps, forFun).process(workspace(), props);
		}
	}

	// Load flow facts and record unknown values
	QuestFlowFactLoader ffl;
	ffl.process(workspace(), props);

	// determine printer
	Printer *p;
	if(xml)
		p = new FFXPrinter(workspace(), true);
	else
		p = new FFPrinter(workspace(), true);

	// printer header
	p->printHeader(cout);

	// Build the checksums of the binary files
	if(!ffl.checkSummed()) {
		for(Process::FileIter file(workspace()->process()); file; file++) {
			checksum::Fletcher sum;
			io::InFileStream stream(file->name());
			sum.put(stream);
			elm::system::Path path = file->name();
			p->printCheckSum(cout, path, sum.sum());
		}
		cout << io::endl;
	}

	// display low-level flow facts
	ControlOutput ctrl(*p);
	ctrl.process(workspace(), props);

	// display the context tree
	FFOutput out(*p, removeDuplicatedTarget);
	out.process(workspace(), props);

	// output footer for XML
	p->printFooter(cout);

	// cleanup at end
	delete p;
}


/**
 * Build the command.
 */
Command::Command(void):
	otawa::Application(
		"mkff",
		Version(1, 1, 0),
		"Hugues Cassé <casse@irit.fr>",
		"Generate a flow fact file for an application.",
		"Copyright (c) 2005-15, IRIT - UPS"),
		xml(*this, option::cmd, "-x", option::cmd, "--ffx", option::description, "activate FFX output", option::end),
		dynbranch(*this, option::cmd, "-D", option::cmd, "--dynbranch", option::description, "check for dynamic branches", option::end),
		outputCFG(*this, option::cmd, "-C", option::cmd, "--cfg_output", option::description, "output cfg in a given fashion (otawa::display::CFGOutput::PATH, KIND)", option::end),
		outputInlinedCFG(*this, option::cmd, "-I", option::cmd, "--inlined_cfg", option::description, "the output cfg is inlined (otawa::display::CFGOutput::INLINED = true), implies -C", option::end),
		outputVirtualizedCFG(*this, option::cmd, "-V", option::cmd, "--virtualized_cfg", option::description, "the output cfg is virtualized (otawa::display::CFGOutput::VIRTUALIZED = true), implies -C -I", option::end),
		removeDuplicatedTarget(*this, option::cmd, "-S", option::cmd, "--no_repeat_multibranch", option::description, "do not output the multi-call/branch with the same target addresses", option::end),
		showBlockProps(*this, option::cmd, "-P", option::cmd, "--show_block_props", option::description, "shows the properties of the block", option::end),
		rawoutput(*this, option::cmd, "-R", option::cmd, "--raw_output", option::description, "generate raw dot output file (without calling dot)", option::end),
		forFun(*this, option::cmd, "-F", option::cmd, "--for_fun", option::description, "the generated dot files will be colourful :)", option::end)
{
}


/**
 * Display the flow facts.
 */
FFOutput::FFOutput(Printer& printer, bool removeDuplicatedTarget): CFGProcessor("FFOutput", Version(1, 0, 0)), has_debug(false), _printer(printer), _removeDuplicatedTarget(removeDuplicatedTarget) {
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
	scanTargets(cfg);
}

/**
 * Find the instruction with multiple branch/call target
 * @param cfg The current working CFG
 */
void FFOutput::scanTargets(CFG *cfg) {
	for(CFG::BlockIter bi = cfg->blocks(); bi; bi++) {

		// only treats BB
		if(!bi->isBasic())
			continue;

		BasicBlock* bb = bi->toBasic();
		Inst* lastInst = bb->last();

		if(BRANCH_TARGET(lastInst).exists()) {
			Vector<Address> va;
			for(Identifier<Address>::Getter target(lastInst, BRANCH_TARGET); target; target++)
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
			for(Identifier<Address>::Getter target(lastInst, CALL_TARGET); target; target++)
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

		// Scan the loop
		scanLoop(ctree->cfg(), ctree, 0);

		// Displayer footer
		_printer.endFunction(out);
	}
}


/**
 * Scan a context tree for displaying its loop flow facts.
 * @param cfg		Container CFG.
 * @param ctree		Current context tree.
 * @param indent	Current indentation.
 */
void FFOutput::scanLoop(CFG *cfg, ContextTree *ctree, int indent) {
	ASSERT(ctree);

	for(ContextTree::ChildrenIterator child(ctree); child; child++) {
		ASSERT(child->kind() != ContextTree::FUNCTION);

		// Process loop
		if(child->kind() == ContextTree::LOOP) {

			/* for(int i = 0; i < indent; i++)
				cout << "  ";
			BasicBlock::InstIter inst(child->bb());
			if(RECORDED(inst) || MAX_ITERATION(inst) != -1 || CONTEXTUAL_LOOP_BOUND(inst))
				cout << "// loop " << addressOf(cfg, child->bb()->address()) << io::endl;
			else
				cout << "loop " << addressOf(cfg, child->bb()->address()) << " ?; // "
					 << child->bb()->address() << io::endl;*/

			_printer.startLoop(out, cfg, child->bb()->first());
			scanLoop(cfg, child, indent + 1);
			_printer.endLoop(out);
		}
	}
}


bool FFOutput::checkLoop(ContextTree *ctree) {
	for(ContextTree::ChildrenIterator child(ctree); child; child++) {
		ASSERT(child->kind() != ContextTree::FUNCTION);
		if(child->kind() == ContextTree::LOOP) {
			BasicBlock::InstIter inst(child->bb());
			if((!RECORDED(inst) && MAX_ITERATION(inst) == -1)
			|| checkLoop(child))
				return true;
		}
	}
	return false;
}


/**
 * @class ControlOutput
 * Output information about the control.
 */


/**
 * Constructor.
 */
ControlOutput::ControlOutput(Printer& printer)
: CFGProcessor("ControlOutput", Version(1, 1, 0)), one(false), _printer(printer) {
}


/**
 */
void ControlOutput::setup(WorkSpace *ws) {
	one = false;
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
	for(CFG::BlockIter bi = cfg->blocks(); bi; bi++) {
		if(!bi->isBasic())
			continue;
		BasicBlock* bb = bi->toBasic();

		//for(BasicBlock::InstIter inst(bb); inst; inst++)
		Inst *inst = bb->last();
		if(inst->isControl()
		&& !inst->isReturn()
		&& !RECORDED(inst)
		&& !PRESERVED(inst)) {

			// Undefined branch target
			if(!inst->target()) {
				if(!BRANCH_TARGET(inst).exists() && !CALL_TARGET(inst).exists()) {
					prepare(ws, cfg);
					cstring type, com;
					if(inst->isCall())
						_printer.printMultiCall(out, cfg, inst);
					else
						_printer.printMultiBranch(out, cfg, inst);
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
