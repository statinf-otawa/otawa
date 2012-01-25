/*
 *	$Id$
 *	FlowFactLoader analyzer implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-07, IRIT UPS.
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

#include <stdio.h>
#include <otawa/util/FlowFactLoader.h>
#include <otawa/prog/WorkSpace.h>
#include <elm/checksum/Fletcher.h>
#include <elm/io/InFileStream.h>
#include <elm/io/BufferedInStream.h>
#include <otawa/flowfact/features.h>
#include <elm/xom.h>
#include <elm/xom/XIncluder.h>
#include <elm/io/BlockInStream.h>
#include <elm/system/Path.h>
#include <otawa/prop/DeletableProperty.h>
#include <otawa/prog/File.h>

// Externals
extern FILE *util_fft_in;

namespace otawa {

extern int fft_line;


/**
 * @defgroup ff Flow Facts
 *
 * This module addresses the problem of the use of flow facts in OTAWA.
 * As this kind of information may be provided by the user or external tools,
 * OTAWA accepts several formats of input :
 * @li @ref f4 is a textual format with specific commands,
 * @li @ref ffx is an XML format.
 *
 * During the work of OTAWA, the load of this file is performed using a special
 * code processor: @ref otawa::util::FlowFactLoader.
 */

/**
 * @page ffx Flow Fact XML format
 * @ingroup ff
 *
 * The format FFX, usually with the extension @c{.ffx}, provide the expressivity
 * of XML applied to the expression of flow facts. The top level element must
 * be @c{flowfacts}:
 *
 * @code
 * <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
 * <flowfacts>
 *	...
 * </flowfacts>
 * @endcode
 *
 * @par Addressing of program parts
 *
 * The different element of this XML flow fact description usually designs
 * some location in the program code. There are is different ways to describe
 * code locations (as shown below) and the syntax, based on attributes of the
 * elements, is shared by most flow fact elements.
 *
 * In the following location forms, the used integers may have the following
 * forms:
 * @li a decimal integer,
 * @li prefixed by '0', an octal integer,
 * @li prefixed by '0[xX]', an hexadecimal integer.
 *
 * @code
 * <element address="ADDRESS"/>
 * @endcode
 * This is the simplest way to design a code piece just by its address. This
 * address represents an integer coded as above. As this way is not very human
 * compliant, you may prefer one of the following form.
 *
 * @code
 * <element label="LABEL" offset="OFFSET"/>
 * @endcode
 * This form provide a bit more portability and usability as the previous one.
 * The address is represented as a label found in the code (usually a function
 * label) and a signed integer offset to add to or substract from the label address.
 * This location may be useful for symbol found in libraries. As the actual
 * address is not known before program linkage, the location may be used
 * and remains valid after linkage.
 *
 * @code
 * <element source="SOURCE_FILE" line="LINE"/>
 * @endcode
 * This is the most advanced to locate code using the source file name and
 * the line of the designed program part. Yet, according to the way the code
 * is written (many source constructor on one line or use of macros) and
 * to the performed optimization, this method may be imprecise as a	a source line
 * may match differents parts in the code.
 *
 * In the remaining of the document, the location defines above are identified
 * by the keyword @c LOCATION in the element attributes.
 *
 * @par Top level elements
 *
 * Inside the @c flowfacts elements, the FFX format accepts the following
 * components.
 *
 * @code
 * <noreturn LOCATION/>
 * @endcode
 * This flow fact inform that the located function never return. This may be
 * helpful for function as the C library "_exit".
 *
 * @code
 * <nocall LOCATION/>
 * @endcode
 * When OTAWA encounters a call to the located function, it will be ignored
 *
 * @code
 * <function LOCATION>
 * 	CONTENT
 * </function>
 * @endcode
 * This flow fact informs that the CONTENT elements are applied to the
 * located function. This element is mainly used to build function call
 * contextual flow fact.
 *
 * @code
 * <loop LOCATION maxcount="INTEGER" totalcount="INTEGER"/>
 * @endcode
 * This element allows to put iteration limits on bounds. The LOCATION designs
 * the header of the loop. @c maxcount gives the maximal number of iteration
 * per loop start and @c totalcount gives the total number of loop iteration
 * for the whole program execution. At least one of them must be given.
 *
 * @code
 * <ignore-entry name="STRING"/>
 * @endcode
 * In some executable files, symbols are marked as function entries but are not,
 * possibly causing problems in instruction decoding. This problem may be avoided
 * by marking such symbols with this directive.
 *
 * @par Content of functions and calls
 *
 * The @ function element content may be either @c loop elements as given
 * above or @c call  elements.
 *
 * @code
 * <call LOCATION>
 * 	CONTENT
 * </call>
 * @endcode
 * This elements represents a function call inside an existing function. The
 * LOCATION gives the address of the called function. The content is the same
 * as of a @c function, that is, @c loop and @c call elements. The embedding
 * of calls allows to build function call contexts to make the loop bounds
 * context aware and tighten the WCET.
 */

/**
 * @page f4 F4 : Flow Facts File Format
 * @ingroup ff
 *
 * This file format is used to store flow facts information, currently, the
 * loop bounds. The usual non-mandatory extension of F4 files is "ff".
 *
 * @par Lexical Level
 *
 * F4 is a simple plain text format. It may contain comments a-la C++, that is, one
 * line comments prefixed by "//" or enclosed comments between "/ *" and "* /"."
 * Spaces and line format are not meaningful for other commands.
 *
 * @par Directives
 *
 * @li <b><tt>loop ADDRESS COUNT ;</tt></b> @n
 * Declare that the loop whose header start at the
 * given address has the given maximal bound.
 *
 * @li <b><tt>loop ADDRESS ... in ADDRESS1 / ADDRESS2 / ...;</tt></b> @n
 * Same as above but limit the loop bound to the given calling context.
 * ADDRESS1 is the address of the function calling the loop, ADDRESS2 is the
 * address of the function calling the former one and so on. It is not
 * required to go back to the task entry point. In the latter case, the bound
 * will be applied to any call context whose prefix matches the given one.
 *
 * @li <b><tt>loop ADDRESS [max MAX] [total TOTAL] [in ...]</tt></b> @n
 * In addition to select the MAX number of iterations per startup of a loop,
 * this form of <b><tt>loop</tt></b> allows also to select the TOTAL number
 * of iterations for the whole task run.
 *
 * @li <b><tt>checksum VALUE ;</tt></b> @n
 * This command is used to check the consistency
 * between the flow fact file and the matching executable file (it is the
 * Fletcher checksum of the executable file).
 *
 * @li <b><tt>return ADDRESS ;</tt></b> @n
 * Mark a complex control instruction as equivallent to sub-program return.
 *
 * @li <b><tt>noreturn FUNCTION_ADDRESS ;</tt></b> @n
 * Mark the function as non-returning like the standard C library "exit".
 *
 * @li <b><tt>ignorecontrol ADDRESS ;</tt></b> @n
 * Force to ignore the control effect of the addressed instruction.
 *
 * @li <b><tt>ignoreseq ADDRESS ;</tt></b> @n
 * Force the target instruction to be considered to be considered as an unconditional branch.
 * Can only be put on a conditional branch instruction.
 *
 * @li <b><tt>multibranch ADDRESS to ADDRESS, ADDRESS, ... ;</tt></b> @n
 * List the different tagets of a multi-target control instruction
 * (function pointer call, switch-like construction).
 *
 * @li <b><tt>nocall FUNCTION_ADDRESS ;</tt></b> @n
 * Process each call to the given function as a non-control instruction.
 * It may be useful to remove call to intrusive initialization function like
 * "__eabi" in main.
 *
 * @li <b><tt>preserve ADDRESS ;</tt></b> @n
 * Ensure that flow fact loader will not change the addressed instruction.
 *
 * @li <b><tt>ignore entry STRING;</tt></b> @n
 * In some executable files, symbols are marked as function entries but are not,
 * possibly causing problems in instruction decoding. This problem may be avoided
 * by marking such symbols with this directive.
 *
 *
 * @par Syntactic items
 *
 * FUNCTION_ADDRESS may have one of the following form:
 * @li INTEGER: integer address,
 * @li STRING: double-quoted string that must match a program label,
 *
 * ADDRESS may have one of the following form:
 * @li INTEGER: integer address,
 * @li STRING: double-quoted string that must match a program label,
 * @li STRING <b><tt>+</tt></b> INTEGER: address relative to a label,
 * @li STRING <b><tt>-</tt></b> INTEGER: address back-relative to a label.
 *
 * INTEGER may have one of the following form:
 * @li {DECIMAL_DIGIT}+: decimal integer
 * @li <b><tt>0</tt></b>{OCTAL_DIGIT}+: octal integer
 * @li <b><tt>0x</tt></b>{HEXADECIMAL_DIGIT}+: hexadecimal integer
 * @li <b><tt>0b</tt></b>{BINARY_DIGIT}+: binary integer
 *
 * @par Examples
 *
 * This example shows a program composed of two function, "main" and "ludcmp"
 * containing nested loops.
 *
 * @code
 * checksum "ludcmp.elf" 0xaa9b6952;
 *
 * // Function main
 * loop "main" + 0x40 6;
 *  loop "main" + 0x70 6;
 *
 * // Function ludcmp
 * loop "ludcmp" + 0x80 5;
 *   loop "ludcmp" + 0x108 5;
 *     loop "ludcmp" + 0x170 5;
 *   loop "ludcmp" + 0x2e0 5;
 *     loop "ludcmp" + 0x344 5;
 * loop "ludcmp" + 0x4a4 5;
 *   loop "ludcmp" + 0x4e0 5;
 * loop "ludcmp" + 0x654 5;
 *   loop "ludcmp" + 0x6a0 5;
 * @endcode
 *
 * This second example shows the use of contextual loop bounds, that is,
 * bounds only applied according the function call context. In this example,
 * the loop in "f" iterates 10 or 20 times according "f"'s caller being
 * "g1" or "g2". In addition, this flow facts states that the loop in the
 * second state iterates at most 50 times for the whole task run.
 *
 * @code
 * checksum "test.arm" 0xdac4ee63;
 *
 * // Function f
 * loop "f" + 0x24 10 in "f" / "g1" / "main";
 * loop "f" + 0x24 max 20 total 50 in "f" / "g2" / "main";
 * @endcode
 *
 * @par To Come
 * @li "branch ADDRESS to ADDRESS ;"
 * @li "loop ADDRESS max COUNT min COUNT ;"
 * @li "recursive ADDRESS max COUNT min COUNT ;"
 * @li "entry ADDRESS ;"
 * @li "entry NAME ;"
 */

/**
 * @class FlowFactLoader
 * This class is an abstract class to monitor the load of flow facts. To get
 * simple access to the flow fact files, a new class must be inherited from
 * this class and this class must implement the virtual "onXXX" methods.
 *
 * @par Provided features
 * @li @ref otawa::FLOW_FACTS_FEATURE
 * @li @ref otawa::MKFF_PRESERVATION_FEATURE
 *
 * @par Configuration
 * @li @ref otawa::FLOW_FACTS_PATH -- select the flow fact to load (if not present, the flow fact name is obtained
 * 		by replacing extension or by appending ".ff" or ".ffx").
 * @li @ref otawa::FLOW_FACTS_MANDATORY -- if set to true, the processing fails if one loop bound is not available
 * 		(default to false).
 *
 * @see
 * 		@ref f4 for more details on the flow facts files.
 * @ingroup ff
 * @author H. Cassé <casse@irit.fr>
 */


/**
 * Build a flow fact loader for the given executable file.
 */
FlowFactLoader::FlowFactLoader(void):
	Processor("otawa::util::FlowFactLoader", Version(1, 0, 0)),
	checksummed(false)
{
	provide(FLOW_FACTS_FEATURE);
	provide(MKFF_PRESERVATION_FEATURE);
}


/**
 * Constructor for inheritance.
 * @param name		Name of the processor.
 * @param version	Version of the processor.
 */
FlowFactLoader::FlowFactLoader(const string& name, const Version& version):
	Processor(name, version), checksummed(false)
{
	provide(FLOW_FACTS_FEATURE);
	provide(MKFF_PRESERVATION_FEATURE);
}


/**
 */
void FlowFactLoader::configure (const PropList &props) {
	Processor::configure(props);
	path = FLOW_FACTS_PATH(props);
	mandatory = FLOW_FACTS_MANDATORY(props);
}


/**
 */
void FlowFactLoader::processWorkSpace(WorkSpace *fw) {
	_fw = fw;
	bool xml = false;

	// lines available ?
	lines_available = fw->isProvided(SOURCE_LINE_FEATURE);

	// Build the F4 file path
	elm::system::Path file_path = path;
	if(file_path) {
		if(file_path.extension() == "ffx")
			xml = true;
	}
	else {
		bool done = false;

		// replace suffix with "ff"
		if(!done) {
			file_path = fw->process()->program()->name();
			file_path = file_path.setExtension("ff");
			done = file_path.isReadable();
		}

		// add suffix ".ff"
		if(!done) {
			file_path = fw->process()->program()->name() + ".ff";
			done = file_path.isReadable();
		}

		// replace suffix with ".ffx"
		if(!done) {
			xml = true;
			file_path = fw->process()->program()->name();
			file_path = file_path.setExtension("ffx");
			done = file_path.isReadable();
		}

		// add suffix ".ffx"
		if(!done) {
			xml = true;
			file_path = fw->process()->program()->name() + ".ffx";
			done = file_path.isReadable();
		}

		// Something found
		if(!done) {
			warn(_ << "no flow fact file for " << fw->process()->program()->name());
			return;
		}
	}

	// process the file
	if(isVerbose())
		log << "\tloading \"" << file_path << "\"\n";
	if(!xml)
		loadF4(file_path);
	else
		loadXML(file_path);

	// Display warning if there is no checksum
	if(!checksummed && isVerbose())
		warn("no checksum: flow facts and executable file may no match !");
}


/**
 * Load an F4 file.
 * @param path	Path of the file.
 */
void FlowFactLoader::loadF4(const string& path) throw(ProcessorException) {
	if(isVerbose())
		log << "\tloading " << path << io::endl;

	// Open the file
	util_fft_in = fopen(&path, "r");
	if(!util_fft_in) {
		string msg = _ << "cannot open the constraint file \"" << path << "\".";
		if(mandatory)
			throw ProcessorException(*this, msg);
		else {
			warn(msg);
			return;
		}
	}

	// Perform the parsing
	fft_line = 1;
	try {
		util_fft_parse(this);
		fclose(util_fft_in);
	}
	catch(...) {
		throw;
	}
}


/**
 * This method is called when an error is encountered.
 * @param message				Message of the error.
 * @throw ProcessorException	With the given message and position in the file.
 */
void FlowFactLoader::onError(const string& message) {
	throw ProcessorException(*this,
		_ << path << ": " << fft_line << ": " << message);
}


/**
 * This method is called when a warning need to be displayed.
 * @param fmt	Message.
 */
void FlowFactLoader::onWarning(const string& message) {
	warn(_ << path << ": " << fft_line << ": " << message);
}


/**
 * Called when an "ignore entry" command is found. This ensures that
 * the symbol is not considered as a code entry during deconding phase.
 * @param name	Name of the symbol.
 */
void FlowFactLoader::onIgnoreEntry(string name) {

	// look for the symbol
	for(Process::FileIter file(workspace()->process()); file; file++) {
		Symbol *sym = file->findSymbol(name);
		if(sym) {
			IGNORE_ENTRY(sym) = true;
			return;
		}
	}

	// else produces a warning
	onWarning(_ << "symbol \"" << name << "\" cannot be found.");
}


/**
 * This method is called when a loop is found.
 * @param addr	Address of the header of the loop.
 * @param count	Bound on the loop iterations.
 */
void FlowFactLoader::onLoop(
	address_t addr,
	int count,
	int total,
	const ContextualPath& path)
{

	// find the instruction
	Inst *inst = _fw->process()->findInstAt(addr);
	if(!inst)
		onError(_ << "unmarked loop because instruction at " << addr << " not found");

	// put the max iteration
	if(count >= 0) {
		int old_max = path(MAX_ITERATION, inst);
		path.ref(MAX_ITERATION, inst) = max(old_max, count);
		if(isVerbose())
			log << "\t" << path << "(MAX_ITERATION," << inst->address() << ") = " << count << io::endl;
	}

	// put the total iteration
	if(total >= 0) {
		path.ref(TOTAL_ITERATION, inst) = total;
		if(isVerbose())
			log << "\t" << path << "(TOTAL_ITERATION," << inst->address() << ") = " << total << io::endl;
	}

	// no path bound
	/*if(!path) {
		if(count >= 0) {
			MAX_ITERATION(inst) = count;
			if(isVerbose())
				log << "\tMAX_ITERATION(" << inst->address() << ") = " << count << io::endl;
		}
		if(total >= 0) {
			TOTAL_ITERATION(inst) = total;
			if(isVerbose())
				log << "\tTOTAL_ITERATION(" << inst->address() << ") = " << total << io::endl;
		}
	}

	// bounds with path
	else {

		// Get the contextual loop bound object
		ContextualLoopBound *bound = CONTEXTUAL_LOOP_BOUND(inst);
		if(!bound) {
			bound = new ContextualLoopBound();
			inst->addProp(new DeletableProperty<ContextualLoopBound *>(CONTEXTUAL_LOOP_BOUND, bound));
			if(isVerbose())
				log << "\tcreatubt contextual loop bound at " << inst->address() << io::endl;
		}

		// Set the bounds
		if(count >= 0) {
			bound->addMax(path, count);
			if(isVerbose()) {
				log << "\tmax bound " << count << " to " << inst->address() << " in ";
				for(int i = 0; i < path.count(); i++)
					log << path[i] << "/";
				log << io::endl;
			}
		}
		if(total >= 0) {
			bound->addTotal(path, total);
			if(isVerbose()) {
				log << "\ttotal bound " << total << " to " << inst->address() << " in ";
				for(int i = 0; i < path.count(); i++)
					log << path[i] << "/";
				log << io::endl;
			}
		}
	}*/
}


 /**
  * Check the consistency of the flow facts with the executable.
  * @param name	Concerned file name.
  * @param sum	Flow fact checksum.
  */
void FlowFactLoader::onCheckSum(const String& name, t::uint32 sum) {

	// Find the file
	for(Process::FileIter file(_fw->process()); file; file++) {
		Path cpath = file->name();
		if(cpath.namePart() == name) {

			// Compute the checksum
			checksum::Fletcher summer;
			io::InFileStream stream(file->name());
			io::BufferedInStream buf(stream);
			summer.put(buf);
			t::uint32 sum2 = summer.sum();
			if(sum2 != sum)
				onError(_ << "bad checksum: flow facts and executable does not match\nff checksum: " << io::hex(sum) << "\nexecutable checksum: " << io::hex(sum2));
			checksummed = true;
			return;
		}
	}

	// Name not found
	onError(_ << "bad checksum: file not found: \"" << name << "\".");
}


/**
 * This method is called each a "return" statement is found.
 * @param addr	Address of the statement to mark as return.
 */
void FlowFactLoader::onReturn(address_t addr) {
	Inst *inst = _fw->process()->findInstAt(addr);
	if(!inst)
		onError(_ << "no instruction at " << addr);
	if(isVerbose())
		log << "\treturn at " << addr << io::endl;
	IS_RETURN(inst) = true;
}


/**
 * This method is called each time a "noreturn ADDRESS" statement is found.
 * @param addr	Address of the entry of the function.
 */
void FlowFactLoader::onNoReturn(address_t addr) {
	Inst *inst = _fw->process()->findInstAt(addr);
	if(!inst)
	  onError(_ << "no instruction at " << addr);
	NO_RETURN(inst) = true;
}


/**
 * This method is called each time a "noreturn NAME" statement is found.
 * @param name	Name of the function.
 */
void FlowFactLoader::onNoReturn(String name) {
	Inst *inst = _fw->process()->findInstAt(name);
	if(!inst)
		throw ProcessorException(*this,
			_ << " label \"" << name << "\" does not exist.");
	else
		NO_RETURN(inst) = true;
}


/**
 * Get the address of the given label.
 * @param label					Label to look for.
 * @return						Matching address.
 * @throw ProcessorException	If the label cannot be found.
 */
Address FlowFactLoader::addressOf(const string& label) {
	Address res = _fw->process()->findLabel(label);
	if(res.isNull())
		onError(_ << "label \"" << label << "\" does not exist.");
	else
		return res;
}


/**
 * Called for the F4 production: "nocall ADDRESS".
 * @param address	Address of the instruction to work on.
 * @throw ProcessorException	If the instruction cannot be found.
 */
void FlowFactLoader::onNoCall(Address address) {
	Inst *inst = _fw->process()->findInstAt(address);
	if(!inst)
		onError(_ << " no instruction at  " << address << ".");
	else
		NO_CALL(inst) = true;
}


/**
 * Called for the F4 production: "preserver ADDRESS".
 * @param address	Address of instruction to preserve.
 * @throw ProcessorException	If the instruction cannot be found.
 */
void FlowFactLoader::onPreserve(Address address) {
	Inst *inst = _fw->process()->findInstAt(address);
	if(!inst)
		onError(_ << " no instruction at  " << address << ".");
	else
		PRESERVED(inst) = true;
}


/**
 * Called for the F4 construction "ignorecontrol ADDRESS"
 * @param address	Address of the ignored instruction.
 */
void FlowFactLoader::onIgnoreControl(Address address) {
	Inst *inst = _fw->process()->findInstAt(address);
	if(!inst)
		onError(_ << " no instruction at  " << address << ".");
	else
		IGNORE_CONTROL(inst) = true;
}


/**
 * Called for the F4 construction "ignoreseq ADDRESS"
 * @param address	Address of the ignored instruction.
 */
void FlowFactLoader::onIgnoreSeq(Address address) {
	Inst *inst = _fw->process()->findInstAt(address);
	if(!inst)
		onError(_ << " no instruction at  " << address << ".");
	else
		IGNORE_SEQ(inst) = true;
}


/**
 * Called for the F4 construction "multibranch ADDRESS to ADDRESS, ...".
 * @param control	Multi-branch instruction address.
 * @param target	List of targets.
 */
void FlowFactLoader::onMultiBranch(
	Address control,
	const Vector<Address>& targets
) {
	// Find the instruction
	Inst *inst = _fw->process()->findInstAt(control);
	if(!inst)
		onError(_ << " no instruction at  " << control << ".");

	// List of targets
	for(int i = 0; i < targets.length(); i++)
		BRANCH_TARGET(inst).add(targets[i]);
}


/**
 * This method is called each the production "loop ADDRESS ?" is found.
 * It usually emit an error.
 * @param addr	Address of the loop entry.
 */
void FlowFactLoader::onUnknownLoop(Address addr) {
	onError(_ << "no bound for loop at " << addr);
}


/**
 * This method is called each the production "multibranch ADDRESS to ?" is found.
 * It usually emit an error.
 * @param control	Address of the control instruction.
 */
void FlowFactLoader::onUnknownMultiBranch(Address control) {
	onError(_ << "undefined targets for multi-branch at " << control);
}


/**
 * Load the flow facts from an XML file.
 * @param path	Path of the XML file.
 */
void FlowFactLoader::loadXML(const string& path) throw(ProcessorException) {

	// open the file
	xom::Builder builder;
	xom::Document *doc = builder.build(&path);
	if(!doc)
		throw ProcessorException(*this, _ << "cannot open " << path);

	// perform inclusion
	xom::XIncluder::resolveInPlace(doc);

	// scan the root element
	xom::Element *root = doc->getRootElement();
	ASSERT(root);
	if(root->getLocalName() != "flowfacts")
		throw ProcessorException(*this, _ << "bad flow fact format in " << path);
	ContextualPath cpath;
	scanXBody(root, cpath);
}


/**
 * Supports an XML flow fact content.
 * Supported elements includes "loop", "function", "noreturn", "return",
 * "nocall", "flowfacts".
 * @param body	Content of the file.
 * @param cpath	Contextual path.
 */
void FlowFactLoader::scanXBody(xom::Element *body, ContextualPath& cpath)
throw(ProcessorException) {
	for(int i = 0; i < body->getChildCount(); i++) {
		xom::Node *child = body->getChild(i);
		if(child->kind() == xom::Node::ELEMENT) {
			xom::Element *element = (xom::Element *)child;
			xom::String name = element->getLocalName();
			if(name == "loop")
				scanXLoop(element, cpath);
			else if(name == "function")
				scanXFun(element, cpath);
			else if(name == "noreturn") {
				Address addr = scanAddress(element, cpath);
				onNoReturn(addr);
			}
			else if(name == "return") {
				Address addr = scanAddress(element, cpath);
				onReturn(addr);
			}
			else if(name == "nocall") {
				Address addr = scanAddress(element, cpath);
				onNoCall(addr);
			}
			else if(name == "flowfacts")
				scanXBody(element, cpath);
			else if(name == "ignore-entry")
				scanIgnoreEntry(element);
			else
				warn(_ << "garbage at \"" << xline(child) << "\"");
		}
	}
}


/**
 * Scan a function XML element.
 * @param element	Element of the function.
 * @param path		Context path to access the function.
 */
void FlowFactLoader::scanXFun(xom::Element *element, ContextualPath& path)
throw(ProcessorException) {

	// get the address
	Address addr = scanAddress(element, path);
	Inst *inst = _fw->process()->findInstAt(addr);
	if(!inst)
		throw ProcessorException(*this,
			_ << " no instruction at  " << addr << " from " << xline(element));
	path.push(ContextualStep::FUNCTION, addr);

	// scan the content
	scanXContent(element, path);
	path.pop();
}


/**
 * Scan a loop XML element.
 * @param element	Element of the loop.
 * @param path		Context path to access the loop.
 */
void FlowFactLoader::scanXLoop(xom::Element *element, ContextualPath& path)
throw(ProcessorException) {

	// get the address
	Address addr = scanAddress(element, path);
	if(addr.isNull())
		return;
	Inst *inst = _fw->process()->findInstAt(addr);
	if(!inst)
		throw ProcessorException(*this,
			_ << " no instruction at  " << addr << " from " << xline(element));

	// get the information
	Option<long> max = scanBound(element, "maxcount");
	Option<long> total = scanBound(element, "totalcount");
	if(!max && !total)
		warn(_ << "loop exists at " <<  addr << " but no bound at " << xline(element));
	onLoop(addr, (max ? *max : -1), (total ? *total : -1), path);

	// look for content
	scanXContent(element, path);
}


/**
 * Retrieve the address of an element (function, loop, ...) from its XML element.
 * @param element	Element to scan in.
 * @param path		Context path.
 */
Address FlowFactLoader::scanAddress(xom::Element *element, ContextualPath& path)
throw(ProcessorException) {

	// look "address" attribute
	Option<unsigned long> res = scanUInt(element, "address");
	if(res)
		return *res;

	// look for "name" and "offset
	Option<xom::String> val = element->getAttributeValue("label");
	if(!val)
		val = element->getAttributeValue("name");
	if(val) {
		Address addr = addressOf(*val);
		Option<long> offset = scanInt(element, "offset");
		return addr + (int)(offset ? *offset : 0);
	}

	// look for lonely offset
	Option<long> offset = scanInt(element, "offset");
	if(offset) {
		if(!path)
			throw ProcessorException(*this,
				_ << "'offset' out of addressed element at " << xline(element));
		return path[path.count() - 1].address() + (int)*offset;
	}

	// look for source and line
	Option<xom::String> source = element->getAttributeValue("source");
	if(source) {
		Option<long> line = scanInt(element, "line");
		if(!line)
			throw ProcessorException(*this,
				_ << "no 'line' but a 'source' at " << xline(element));
		return addressOf(*source, *line);
	}

	// it is an error
	throw ProcessorException(*this,
		_ << "no location in loop at " << xline(element));
}


/**
 * Scan an integer attribute.
 * @param element	Element to scan in.
 * @param name		Name of the element.
 * @return			Read element or none.
 */
Option<long> FlowFactLoader::scanInt(xom::Element *element, cstring name)
throw(ProcessorException) {
	Option<xom::String> val = element->getAttributeValue(name);
	if(!val)
		return none;
	io::BlockInStream buf(val);
	io::Input in(buf);
	long res;
	try {
		in >> res;
		return res;
	}
	catch(io::IOException& e) {
		throw ProcessorException(*this, _ << "bad formatted address(" << e.message() << ") at " << xline(element));
	}
}


Option<unsigned long> FlowFactLoader::scanUInt(xom::Element *element, cstring name)
throw(ProcessorException) {
	Option<xom::String> val = element->getAttributeValue(name);
	if(!val)
		return none;
	io::BlockInStream buf(val);
	io::Input in(buf);
	unsigned long res;
	try {
		in >> res;
		return res;
	}
	catch(io::IOException& e) {
		throw ProcessorException(*this, _ << "bad formatted address(" << e.message() << ") at " << xline(element));
	}
}


/**
 * Scan a a bound attribute.
 * @param element	Element to scan in.
 * @param name		Name of the element.
 * @return			Read element or none.
 */
Option<long> FlowFactLoader::scanBound(xom::Element *element, cstring name)
throw (ProcessorException) {
	Option<xom::String> val = element->getAttributeValue(name);
	if(!val)
		return none;
	if(val == "NOCOMP")
		return none;
	io::BlockInStream buf(val);
	io::Input in(buf);
	long res;
	try {
		in >> res;
		return res;
	}
	catch(io::IOException e) {
		throw ProcessorException(*this, _ << "bad formatted bound at " << xline(element));
	}
}


/**
 * Scan the content of 'function' or of a 'loop'.
 * @param element	Container element.
 * @param path		Current context path.
 */
void FlowFactLoader::scanXContent(xom::Element *element, ContextualPath& path)
throw(ProcessorException) {
	for(int i = 0; i < element->getChildCount(); i++) {
		xom::Node *child = element->getChild(i);
		if(child->kind() == xom::Node::ELEMENT) {
			xom::Element *element = (xom::Element *)child;
			xom::String name = element->getLocalName();
			if(name == "loop")
				scanXLoop(element, path);
			else if(name == "call")
				scanXFun(element, path);
			else if(name == "conditional")
				scanXConditional(element, path);
		}
	}
}


/**
 * Called to scan an "if" element.
 * @param element	"if" element.
 * @param path		Current contextual path.
 */
void FlowFactLoader::scanXConditional(xom::Element *element, ContextualPath& path)
throw(ProcessorException) {
	for(int i = 0; i < element->getChildCount(); i++) {
		xom::Node *child = element->getChild(i);
		if(child->kind() == xom::Node::ELEMENT) {
			xom::Element *element = (xom::Element *)child;
			xom::String name = element->getLocalName();
			if(name == "condition")
				scanXContent(element, path);
			else if(name == "case")
				scanXContent(element, path);
		}
	}
}


/**
 * Called when an "ignore-entry" elemet is found.
 * @param element	Current element.
 */
void FlowFactLoader::scanIgnoreEntry(xom::Element *element) {
	Option<xom::String> name = element->getAttributeValue("name");
	if(!name || !*name)
		onWarning(_ << xline(element) << ": no name given");
	else
		onIgnoreEntry(*name);
}


/**
 * Build an XML element position for user.
 * @param element	Current XML element.
 */
string FlowFactLoader::xline(xom::Node *element) {
	return _ << element->getBaseURI() << ":" << element->line();
}


/**
 * Get the address matching the given source file name and line.
 * @param file	Source file path.
 * @param line	Line in the source file.
 * @return		Matching first address.
 * @throw ProcessorException	If the file/line cannot be resolved to an address.
 */
Address FlowFactLoader::addressOf(const string& file, int line)
throw(ProcessorException) {
	if(!lines_available)
		onError("the current loader does not provide source line information");
	Vector<Pair<Address, Address> > addresses;
 	workSpace()->process()->getAddresses(file.toCString(), line, addresses);
 	if(!addresses) {
		warn(_ << "cannot find the source line " << file << ":" << line);
		return Address::null;
	}
 	if(isVerbose())
 		log << "\t" << file << ":" << line << " is " << addresses[0].fst << io::endl;
 	return addresses[0].fst;
}


/**
 * This property may be used in the configuration of a code processor
 * to pass the path of an F4 file containing flow facts.
 * @ingroup ff
 */
Identifier<Path> FLOW_FACTS_PATH("otawa::FLOW_FACTS_PATH", "");


/**
 * This feature ensures that the flow facts has been loaded.
 * Currrently, only the @ref otawa::util::FlowFactLoader provides this kind
 * of information from F4 files.
 * @ingroup ff
 *
 * @par Hooked Properties
 * @li @ref IS_RETURN
 * @li @ref NO_RETURN
 * @li @ref MAX_ITERATION
 */
Feature<FlowFactLoader> FLOW_FACTS_FEATURE("otawa::FLOW_FACTS_FEATURE");


/**
 * This feature ensures that preservation information used by mkff is put
 * on instruction.
 * @ingroup f4
 *
 * @par Hooked Properties
 * @li @ref PRESERVED
 */
Feature<FlowFactLoader> MKFF_PRESERVATION_FEATURE("otawa::MKFF_PRESERVATION_FEATURE");


/**
 * Put on a control flow instruction, this shows that this instruction
 * is equivalent to a function return. It may be useful with assembly providing
 * very complex ways to express a function return.
 * @ingroup ff
 *
 * @par Hooks
 * @li @ref Inst (@ref otawa::util::FlowFactLoader)
 */
Identifier<bool> IS_RETURN("otawa::IS_RETURN", false);


/**
 * This annotation is put on the first instruction of functions that does not
 * never return. It is usually put on the C library "_exit" function.
 * @ingroup ff
 *
 * @par Hooks
 * @li @ref Inst (@ref otawa::util::FlowFactLoader)
 */
Identifier<bool> NO_RETURN("otawa::NO_RETURN", false);


/**
 * Put on the first instruction of a loop, it gives the maximum number of
 * iteration of this loop.
 * @ingroup ff
 *
 * @par Hooks
 * @li @ref Inst (@ref otawa::util::FlowFactLoader)
 */
Identifier<int> MAX_ITERATION("otawa::MAX_ITERATION", -1);


/**
 * In configuration of the FlowFactLoader, makes it fail if no flow fact
 * fail is available.
 * @ingroup ff
 */
Identifier<bool> FLOW_FACTS_MANDATORY("otawa::FLOW_FACTS_MANDATORY", false);


/**
 * Put on the first instruction of a function that must be no called.
 * @par Features
 * @li @ref FLOW_FACTS_FEATURE
 * @par Hooks
 * @li @ref Inst
 * @ingroup ff
 */
Identifier<bool> NO_CALL("otawa::NO_CALL", false);


/**
 * Put on a control instruction to prevent it to be interpreted as is.
 * @par Features
 * @li @ref FLOW_FACTS_FEATURE
 * @par Hooks
 * @li @ref Inst
 * @ingroup ff
 */
Identifier<bool> IGNORE_CONTROL("otawa::IGNORE_CONTROL", false);


/**
 * Put on a control instruction to prevent to pass in sequence.
 * @par Features
 * @li @ref FLOW_FACTS_FEATURE
 * @par Hooks
 * @li @ref Inst
 * @ingroup ff
 */
Identifier<bool> IGNORE_SEQ("otawa::IGNORE_SEQ", false);


/**
 * Put on instruction that may branch to several targets or whose target
 * computation cannot computed. There is one property
 * with this identifier for each branched target.
 * @par Features
 * @li @ref FLOW_fACTS_FEATURE
 * @par Hooks
 * @li @ref Inst
 * @ingroup ff
 */
Identifier<Address> BRANCH_TARGET("otawa::BRANCH_TARGET", Address());


/**
 * Put on instruction that must preserved from the mkff special flow-fact
 * detection.
 * @par Features
 * @li @ref MKFF_PRESERVATION_FEATURE
 * @par Hooks
 * @li @ref Inst
 * @ingroup ff
 */
Identifier<bool> PRESERVED("otawa::PRESERVED", false);


/**
 * Put on the first instruction of a loop, it gives the minimal
 * number of iterations.
 *
 * @par Features
 * @li @ref MKFF_PRESERVATION_FEATURE
 * @par Hooks
 * @li @ref Inst
 * @ingroup ff
 */
Identifier<int> MIN_ITERATION("otawa::MIN_ITERATION", -1);


/**
 * Put on the first instruction of a loop, it gives the total
 * number of iterations during the whole execution of the program.
 *
 * @par Features
 * @li @ref MKFF_PRESERVATION_FEATURE
 * @par Hooks
 * @li @ref Inst
 * @ingroup ff
 */
Identifier<int> TOTAL_ITERATION("otawa::TOTAL_ITERATION", -1);


/**
 * Put on function symbol that must be ignored as function entry.
 * It mainly used to avoid problems with executable files containing
 * symbols marked as function that are in fact data.
 *
 * @par Features
 * @li @ref FLOW_FACTS_FEATURE
 *
 * @par Hooks
 * @li @ref Symbol
 */
Identifier<bool> IGNORE_ENTRY("otawa::IGNORE_ENTRY", false);


} // otawa
