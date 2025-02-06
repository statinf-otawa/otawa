/*
 *	FlowFactLoader analyzer implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2016, IRIT UPS.
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

#include <elm/checksum/Fletcher.h>
#include <elm/io/InFileStream.h>
#include <elm/io/BufferedInStream.h>
#include <elm/xom.h>
#include <elm/xom/XIncluder.h>
#include <elm/io/BlockInStream.h>
#include <elm/sys/Path.h>

#include <otawa/flowfact/features.h>
#include <otawa/hard/Platform.h>
#include <otawa/prop/DeletableProperty.h>
#include <otawa/prog/File.h>
#include <otawa/prog/Process.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/flowfact/FlowFactLoader.h>

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
 *
 * Flow facts are usually hooked as properties to the program part
 * they apply to. Below is the list of properties set as flow facts.
 * @li @ref ACCESS_RANGE
 * @li @ref BRANCH_TARGET
 * @li @ref CALL_TARGET
 * @li @ref IGNORE_CONTROL
 * @li @ref IGNORE_ENTRY
 * @li @ref IGNORE_SEQ
 * @li @ref IS_RETURN
 * @li @ref MAX_ITERATION
 * @li @ref MIN_ITERATION
 * @li @ref NO_CALL
 * @li @ref NO_BLOCK
 * @li @ref NO_RETURN
 * @li @ref TOTAL_ITERATION
 * @li @ref PRESERVED
 * @li @ref NO_INLINE
 * @li @ref INLINING_POLICY
 * @li @ref ACCESS_RANGE
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
 * label) and a signed integer offset to add to or subtract from the label address.
 * This location may be useful for symbol found in libraries. As the actual
 * address is not known before program linkage, the location may be used
 * and remains valid after linkage.
 *
 * @code
 * <element symbol="SYMBOL" size="SIZE"/>
 * @endcode
 * This form provides a way to define a memory area in a single step.
 * The defined memory area begins by the address of @c symbol and its size is
 * @c size of it is defined or the size of the @c symbol.
 *
 * @code
 * <element source="SOURCE_FILE" line="LINE"/>
 * @endcode
 * This is the most advanced to locate code using the source file name and
 * the line of the designed program part. Yet, according to the way the code
 * is written (many source constructor on one line or use of macros) and
 * to the performed optimization, this method may be imprecise as a	a source line
 * may match different parts in the code.
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
 * When OTAWA encounters a call to the located function, it will be ignored.
 *
 * @code
 * <doinline LOCATION/>
 * @endcode
 * When virtualizing, calls to the located function will be inlined.
 *
 * @code
 * <noinline LOCATION/>
 * @endcode
 * When virtualizing, calls to the located function will not be inlined.
 *
 * @code
 * <inlining-on LOCATION/>
 * @endcode
 * When virtualizing the located function, default policy will be set to true.
 *
 * @li <b><tt>memory access INST_ADDRESS1 LOW_ADDRESS .. HIGH_ADDRESS [in STEP1 / STEP2 / ...] ;</tt></b> @n
 * Sets the memory accessed by the instruction at INST_ADDRESS
 * to the range from LOW_ADDRESS up to third HIGH_ADDRESS,
 * optionally in the given context.
 *
 * @li <b><tt>memory TYPE ADDRESS = VALUE ;</tt></b> @n
 * Sets the memory at ADDRESS to the VALUE of type TYPE.
 *
 * @code
 * <inlining-off LOCATION/>
 * @endcode
 * When virtualizing the located function, default policy will be set to false.
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
 * @code
 * <return LOCATION/>
 * @endcode
 * The instruction at the given location is considered as a return. This element
 * may be useful with languages where control management and computation is melted.
 * For example, the ARM allows to store the return address of a function call out of
 * the usual link register and move of a register value in the PC may as well be
 * considered as an indirect call as a return.
 *
 * @par Content of functions and calls
 *
 * The @c function element content may be either @c loop elements as given
 * above or @c call  elements.
 *
 * @code
 * <call LOCATION>
 * 	 CONTENT
 * </call>
 * @endcode
 * This element represents a function call inside an existing function. The
 * LOCATION gives the address of the called function. The content is the same
 * as of a @c function, that is, @c loop and @c call elements. The embedding
 * of calls allows to build function call contexts to make the loop bounds
 * context aware and tighten the WCET.
 *
 * @code
 * <multibranch LOCATION>
 *   <target LOCATION />
 *   ...
 * </multibranch>
 * @endcode
 * This element is used to resolve a complex control to several targets
 * (case of indirect branch found in switch compilation using tables).
 * The first LOCATION is the control instruction itself and the target child
 * elements locations represents the different possible targets.
 *
 * @code
 * <multicall LOCATION>
 *   <target LOCATION />
 *   ...
 * </multicall>
 * @endcode
 * This element is used to resolve a complex control to several targets
 * (case of function pointer calls).
 * The first LOCATION is the call instruction itself and the target child
 * elements locations represents the different possible targets.
 *
 * @code
 * <ignorecontrol LOCATION/>
 * @endcode
 * This element is used to ignore the control effect of the instruction at LOCATION.
 *
 * @code
 * <ignoreseq LOCATION/>
 * @endcode
 * This element is used to force instruction at LOCATION to be considered
 * as an unconditional branch.
 * Can only be put on a conditional branch instruction.
 *
 * @code
 * <state LOCATION>
 * 		<reg-value name="TEXT"> VALUE </reg-value>
 * 		<mem-value type="TEXT" LOCATION> VALUE </reg-value>
 * </state>
 * Provide the processor state at a given program point at LOCATION.
 * The processor state can be composed with register and memory values.
 * The register is specified with the name, e.g. name="r10".
 * The memory is specified with the type of access, and its location, e.g. type="uint32" address="0x80000003".
 *
 * @par Extension
 *
 * Top-level element support also the following extension elements
 * (not included in the FFX format):
 *
 * @code
 * <mem-access LOCATION>
 * 		<low LOCATION />
 * 		<high LOCATION />
 * </mem-access>
 * @endcode
 * @code
 * <mem-access LOCATION>
 *  	<area LOCATION />
 *  	...
 * </mem-access>
 * @endcode
 * Allow to bound the accessed addresses of a memory instruction.
 * When multiple areas are defined, they are joined to a single continuous area!
 * Mainly useful to help data cache analysis.
 *
 * @code
 * <reg-set name="TEXT"> VALUE </reg-set>
 * @endcode
 * Set the value of a register. <i>name</i> is the name of the register.
 *
 * @code
 * <mem-set type="TEXT" LOCATION> VALUE </mem-set>
 * @endcode
 * Set the value of a kind of memory. The LOCATION attributes gives
 * the address of the accessed memory and <i>type</i> defines the type of data
 * among <b>int8</b>, <b>int16</b>, <b>int32</b>, <b>int64</b>, <b>uint8</b>, <b>uint16</b>,
 * <b>uint32</b> and <b>uint64</b>.
 *
 * <b><i>VALUE</i></b> may be one of:
 * @code
 * 	<value LOCATION/>
 * @endcode
 * Declare a simple integer value or an address.
 *
 * @code
 * 	<low LOCATION/>
 * 	<high LOCATION/>
 * @endcode
 * A range of integer or of addresses.
 * @code
 * 	<value LOCATION step="INTEGER" count="INTEGER"/>
 * @endcode
 * Declare a CLP (b, d, n) of base the location attributes, step and count
 * and denotes the set { b + k d / 0 <= j <= n }.
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
 * In the following description, upper case words represents variable part
 * of the command, i.e. sub-rules of grammar. If such a token is prefixed
 * by a name followed by ":", this name is used to identify the token
 * in subsequent description.
 *
 * @par Directives
 *
 * @li <b><tt>loop ADDRESS COUNT ;</tt></b> @n
 * Declare that the loop whose header start at the
 * given address has the given maximal bound.
 *
 * @li <b><tt>loop ADDRESS ... in STEP1 / STEP2 / ...;</tt></b> @n
 * Same as above but limit the loop bound to the given calling context.
 * STEP1 is the address of the function calling the loop, STEP2 is the
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
 * Force the target instruction to be considered as an unconditional branch.
 * Can only be put on a conditional branch instruction.
 *
 * @li <b><tt>multibranch ADDRESS to ADDRESS, ADDRESS, ... ;</tt></b> @n
 * List the different tagets of a multi-target control instruction
 * (switch-like construction).
 *
 * @li <b><tt>multicall ADDRESS to ADDRESS, ADDRESS, ... ;</tt></b> @n
 * List the different tagets of a multi-target call instruction
 * (function pointer call).
 *
 * @li <b><tt>nocall FUNCTION_ADDRESS ;</tt></b> @n
 * Process each call to the given function as a non-control instruction.
 * It may be useful to remove call to intrusive initialization function like
 * "__eabi" in main.
 * (function pointer call).
 *
 * @li <b><tt>noblock FUNCTION_ADDRESS ;</tt></b> @n
 * Remove this block from the CFG, and those that post-dominate it.
 * Removing a block that is post-dominated by the EXIT node is an error.
 *
 * @li <b><tt>doinline FUNCTION_ADDRESS ;</tt></b> @n
 * When virtualizing, calls to the given function will be inlined.
 *
 * @li <b><tt>noinline FUNCTION_ADDRESS ;</tt></b> @n
 * When virtualizing, calls to the given function will not be inlined.
 *
 * @li <b><tt>inlining-on FUNCTION_ADDRESS ;</tt></b> @n
 * When virtualizing the given function, default inlining policy
 * will be set to true.
 *
 * @li <b><tt>inlining-off FUNCTION_ADDRESS ;</tt></b> @n
 * When virtualizing the given function, default inlining policy
 * will be set to false.
 *
 * @li <b><tt>preserve ADDRESS ;</tt></b> @n
 * Ensure that flow fact loader will not change the addressed instruction.
 *
 * @li <b><tt>ignore entry STRING;</tt></b> @n
 * In some executable files, symbols are marked as function entries but are not,
 * possibly causing problems in instruction decoding. This problem may be avoided
 * by marking such symbols with this directive.
 *
 * @li <b><tt>library;</tt></b> @n
 * This command is marker informing that the current flow fact file is applied to
 * a library. The main effect is that if a label does not exist in the current executable,
 * instead of raising a fatal error, the matching flow fact is quietly ignored
 * considering that the label of the library is not linked in the current
 * executable.
 *
 * @li <b><tt>force branch ADDRESS ;</tt></b> @n
 * Force the instruction matching the address to be processed as a simple branch
 * whatever its former kind is: not a control at all, a return or a call.
 *
 * @li <b><tt>force call ADDRESS ;</tt></b> @n
 * Force the instruction matching the address to be processed as a call control
 * whatever its former kind is: not a control at all, a return or a simple branch.
 *
 * @par Syntactic items
 *
 * COUNT may be a combination of:
 *	* INTEGER -- represent the maximum number of iteration of the loop,
 *	* <b><tt>max</tt></b> INTEGER -- represent the maximum number of iteration of the loop (for each loop startup),
 *	* <b><tt>total</tt></b> INTEGER -- represent the total number of iteration of the loop (for the complete task execution),
 *	* <b><tt>min</tt></b> INTEGER -- represent the minimum number of iteration of the loop (for each loop startup).
 *
 * FUNCTION_ADDRESS may have one of the following form:
 * @li INTEGER: integer address,
 * @li STRING: double-quoted string that must match a program label,
 *
 * STEP may have one of the following form:
 * @li ADDRESS: simple address of a function,
 * @li @ ADDRESS: address of instruction calling the previous function.
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
 * loop "f" + 0x24 10 in "main" / "g1" / "f";
 * loop "f" + 0x24 max 20 total 50 in "main" / "g2" / "f";
 * @endcode
 *
 * @par To Come
 * @li "branch ADDRESS to ADDRESS ;"
 * @li "recursive ADDRESS max COUNT min COUNT ;"
 * @li "entry ADDRESS ;"
 * @li "entry NAME ;"
 *
 * @par Extensions
 * Flow fact files are very useful file to allow user to give more detail
 * on the behaviour of a program.
 *
 * @li <b><tt>memory access <i>iaddr: ADDRESS</i> .. <i>lo: ADDRESS</i> .. <i>hi: ADDRESS</i> ;</tt></b> @n
 * Definition of the range <i>lo</i> to <i>hi</i> (exclusive) for the memory instruction at address <i>iaddr</i>.
 *
 * @li <b><tt>reg <i>NAME</i> = <i>DATA</i> ;</tt></b>
 * Allows to assign a value to a register. This value may be a simple integer or an address.
 *
 * @li <b><tt>memory [<i>TYPE</i>] <i>ADDRESS</i> = <i>DATA</i> ;<tt></b>
 * Assign a value in memory.<i>TYPE</i> may be one <tt>int8</tt>, <tt>uint8</tt>, <tt>int816</tt>, <tt>uint16</tt>,
 * <tt>int32</tt>, <tt>uint32</tt>, <tt>int64</tt>, <tt>uint64</tt>, <tt>float32</tt> or <tt>float64</tt>.
 * <i>DATA</i> supports several forms described below.
 *
 * <i>DATA</i> a bit complex but allows a lot of flexibility:
 * @li <b><i>ADDRESS</i></b> -- the easier form, a simple integer or an address,
 * @li <b>[ <i>ADDRESS</i> .. <i>ADDRESS</i> ]</b> -- an integer or address interval,
 * @li <b>( <i>ADDRESS</i>, <i>INTEGER<i>, <i>INTEGER</i> )</b> -- a CLP triple (b, d, n) representing the integer set { b + k d / 0 <= k <= n }.
 */

/**
 * @class FlowFactLoader
 * This class is an abstract class to monitor the load of flow facts. To get
 * simple access to the flow fact files, a new class must be inherited from
 * this class and this class must implement the virtual "onXXX" methods.
 *
 * @par Required Features
 * @li @ref dfa::INITIAL_STATE_FEATURE
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
 * @li @ref otawa::FLOW_FACTS_IGNORE_UNKNOWN -- if set to true, the processing will not fails if one flow fact is unknown type
 * 		(default to false).
 *
 * @see
 * 		@ref f4 for more details on the flow facts files.
 * @ingroup ff
 * @author H. Cassé <casse@irit.fr>
 */

p::declare FlowFactLoader::reg = p::init("otawa::FlowFactLoader", Version(1, 4, 0))
	.maker<FlowFactLoader>()
	.require(dfa::INITIAL_STATE_FEATURE)
	.provide(FLOW_FACTS_FEATURE)
	.provide(MKFF_PRESERVATION_FEATURE);


/**
 * Build a flow fact loader for the given executable file.
 */
FlowFactLoader::FlowFactLoader(p::declare& r):
	Processor(r),
	_fw(0),
	checksummed(false),
	
	
	mandatory(false),
	ignore_unknown(false),
	lines_available(false),
	state(0),
	lib(false),
	
	currentCteNum(0),  
	numOfEdgeIntoCurrentCte(0),
	intoConflictPath(false)
{
}


/**
 */
void FlowFactLoader::configure (const PropList &props) {
	Processor::configure(props);
	for(Identifier<Path>::Getter path(props, FLOW_FACTS_PATH); path(); path++)
		paths.add(*path);
	for(Identifier<xom::Element *>::Getter node(props, FLOW_FACTS_NODES); node(); node++)
		nodes.add(*node);

	mandatory = FLOW_FACTS_MANDATORY(props);
	ignore_unknown = FLOW_FACTS_IGNORE_UNKNOWN(props);
}


/**
 */
void FlowFactLoader::setup(WorkSpace *ws) {
	state = dfa::INITIAL_STATE(ws);
}


/**
 * Load flow facts from the given file.
 * @param ws	Current workspace.
 * @param path	Path to the file to load flow facts from.
 */
void FlowFactLoader::load(WorkSpace *ws, const Path& path) {
	if(logFor(LOG_DEPS))
		log << "\tloading \"" << path << "\"\n";
	current = path;

	// load the file
	if(path.extension() == "ffx" || path.extension() == "xml")
		loadXML(path);
	else {
		if(path.extension() != "ff")
			log << "WARNING: no known extension to " << path << ": assuming F4 format.\n";
		loadF4(path);
	}

	// display warning if there is no checksum
	if(!checksummed && logFor(LOG_DEPS))
		warn("no checksum: flow facts and executable file may no match !");
}


/**
 */
void FlowFactLoader::processWorkSpace(WorkSpace *ws) {
	_fw = ws;

	// lines available ?
	lines_available = ws->isProvided(SOURCE_LINE_FEATURE);

	// Build the F4 file path
	if(paths)
		for(int i = 0; i < paths.length(); i++)
			load(ws, paths[i]);
	else {
		bool done = false;
		Vector<sys::Path> to_test;

		// build the list of paths

		Path path;

		// replace suffix with "ff"
		if(!done) {
			path = ws->process()->program()->name();
			path = path.setExtension("ff");
			done = path.isReadable();
		}

		// add suffix ".ff"
		if(!done) {
			path = ws->process()->program()->name() + ".ff";
			done = path.isReadable();
		}

		// replace suffix with ".ffx"
		if(!done) {
			path = ws->process()->program()->name();
			path = path.setExtension("ffx");
			done = path.isReadable();
		}

		// add suffix ".ffx"
		if(!done) {
			path = ws->process()->program()->name() + ".ffx";
			done = path.isReadable();
		}

		// Something found
		if(done)
			load(ws, path);
		else {
			warn(_ << "no flow fact file for " << ws->process()->program()->name());
		}
	}

	if(nodes)
		for(int i = 0; i < nodes.length(); i++) {
			ContextualPath cpath;
			scanXBody(nodes[i], cpath);
		}
}


/**
 * Load an F4 file.
 * @param path	Path of the file.
 */
void FlowFactLoader::loadF4(const string& path) {
	if(logFor(LOG_DEPS))
		log << "\tloading " << path << io::endl;

	// Open the file
	util_fft_in = fopen(path.asSysString(), "r");
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
		_ << current << ": " << fft_line << ": " << message);
}


/**
 * This method is called when a warning need to be displayed.
 * @param fmt	Message.
 */
void FlowFactLoader::onWarning(const string& message) {
	warn(_ << current << ": " << fft_line << ": " << message);
}


/**
 * Called when an "ignore entry" command is found. This ensures that
 * the symbol is not considered as a code entry during decoding phase.
 * @param name	Name of the symbol.
 */
void FlowFactLoader::onIgnoreEntry(string name) {

	// look for the symbol
	for(Process::FileIter file(workspace()->process()); file(); file++) {
		Symbol *sym = file->findSymbol(name);
		if(sym) {
			IGNORE_ENTRY(sym) = true;
			return;
		}
	}

	// else produces a warning
	if(!lib)
		onWarning(_ << "symbol \"" << name << "\" cannot be found.");
}

/**
 * This method is called when a loop is found.
 * @param addr	Address of the header of the loop.
 * @param count	Bound on the loop iterations.
 */
void FlowFactLoader::onLoop(address_t addr, int count, int total, int min, const ContextualPath& path) {
	if(addr.isNull())
		return;

	// find the instruction
	Inst *inst = _fw->process()->findInstAt(addr);
	if(!inst)
		onError(_ << "unmarked loop because instruction at " << addr << " not found");

	// put the max iteration
	if(count >= 0) {
		int max = path(MAX_ITERATION, inst);
		if(max < count)
			max = count;
		path.ref(MAX_ITERATION, inst) = max;
		if(logFor(LOG_BB))
			log << "\t" << path << "(MAX_ITERATION," << inst->address() << ") = " << count << io::endl;
	}

	// put the total iteration
	if(total >= 0) {
		path.ref(TOTAL_ITERATION, inst) = total;
		if(logFor(LOG_BB))
			log << "\t" << path << "(TOTAL_ITERATION," << inst->address() << ") = " << total << io::endl;
	}

	// put the min iteration
	if(min >= 0) {
		path.ref(MIN_ITERATION, inst) = min;
		if(logFor(LOG_BB))
			log << "\t" << path << "(MIN_ITERATION," << inst->address() << ") = " << min << io::endl;
	}

}



/**
 * This method is called to count haw many conflict are present into an element.
 * @param  element	... element
 * @param count	number of inner conflict.
 */

int FlowFactLoader::containsNotALL(xom::Element *element){
	int nb=0;
	for(int i = 0; i < element->getChildCount(); i++) {
		xom::Node *child = element->getChild(i);
		if(child->kind() == xom::Node::ELEMENT) {
			xom::Element *elt = (xom::Element *)child;
			xom::String name = elt->getLocalName();
			if	(name == "not-all" && elt->getAttributeValue("seq").value() == "true") nb++;
		}
	}
	return nb;
}

/**
 * This method is called when a edges are infeaseable path.
 * @param edge_addr	vector of Address inpossible path.
 * @param ...count	Bound on the loop iterations.
 */
void FlowFactLoader::onInfeasablePath(  address_t addr,  const ContextualPath& path) {
	Address firtElement =addr;  
	// find the instruction
	Inst *inst = workSpace()->process()->findInstAt(firtElement);
	if(!inst)
		onError(_ << "unmarked instruction because instruction at " << firtElement << " not found");
		
		// put the infeasable path  
		++currentCteNum;
		numListOfUnclosedPath.push(currentCteNum);
		numOfEdgeIntoCurrentCte=0;
		LockPtr<ConflictOfPath  > ff  = path(INFEASABLE_PATH, inst);
		if (!ff) ff=new ConflictOfPath(path, currentCteNum) ;
		else ff->addConflictToPath( currentCteNum) ;
		path.ref(INFEASABLE_PATH, inst) = ff;
	}
 
/**
 * Load a memory range instruction.
 * @param iaddr		Instruction address.
 * @param lo		Low address (inclusive).
 * @param hi		High address (exclusive).
 * @param path		Current context.
 */
void FlowFactLoader::onMemoryAccess(Address iaddr, Address lo, Address hi, const ContextualPath& path) {
	if(iaddr.isNull())
		return;

	// find the instruction
	Inst *inst = _fw->process()->findInstAt(iaddr);
	if(!inst)
		onError(_ << "unmarked memory access because instruction at " << iaddr << " not found");

	// put the property
	path.ref(ACCESS_RANGE, inst) = pair(lo, hi);
	if(logFor(LOG_BB))
		log << "\t" << path << "(MEMORY_ACCESS," << iaddr << ") = [" << lo << ", " << hi << "]" << io::endl;
}


/**
 * Called when a register set is found.
 * @param name		Name of the register.
 * @param value		Value to set.
 */
void FlowFactLoader::onRegSet(string name, const dfa::Value& value) {
	onRegSet(state, name, value);
}

void FlowFactLoader::onRegSet(dfa::State* state, string name, const dfa::Value& value) {

	// find register
	const hard::Register *reg = workSpace()->process()->platform()->findReg(name);
	if(!reg)
		onError(_ << "no register named " << name);

	// set the value
	state->set(reg, value);
}


/**
 * Called to set a memory initialization.
 * @param addr		Address to initialize.
 * @param type		Type in memory.
 * @param value		Value to set.
 */
void FlowFactLoader::onMemSet(Address addr, const Type *type, const dfa::Value& value) {
	onMemSet(state, addr, type, value);
}

void FlowFactLoader::onMemSet(dfa::State* state, Address addr, const Type *type, const dfa::Value& value) {
	if(addr.isNull())
		return;
	state->record(dfa::MemCell(addr, type, value));
}


 /**
  * Check the consistency of the flow facts with the executable.
  * @param name	Concerned file name.
  * @param sum	Flow fact checksum.
  */
void FlowFactLoader::onCheckSum(const String& name, t::uint32 sum) {

	// Find the file
	for(Process::FileIter file(_fw->process()); file(); file++) {
		Path cpath = file->name();
		if(cpath.namePart() == name) {

			// Compute the checksum
			checksum::Fletcher summer;
			io::InFileStream stream(file->name());
			io::BufferedInStream buf(stream);
			summer.put(buf);
			t::uint32 sum2 = summer.sum();
			if(sum2 != sum)
				onWarning(_ << "bad checksum: flow facts and executable does not match\nff checksum: " << io::hex(sum) << "\nexecutable checksum: " << io::hex(sum2));
			checksummed = true;
			return;
		}
	}

	// Name not found
	onError(_ << "bad checksum: file not found: \"" << name << "\".");
}


/**
 * Called to inform that the current FFX file is a library.
 * The main effect is that, if a label cannot be resolved, the fact is
 * simply skipped and it does not cause an error.
 */
void FlowFactLoader::onLibrary(void) {
	lib = true;
}


/**
 * This method is called each a "return" statement is found.
 * @param addr	Address of the statement to mark as return.
 */
void FlowFactLoader::onReturn(address_t addr) {
	if(addr.isNull())
		return;
	Inst *inst = _fw->process()->findInstAt(addr);
	if(!inst)
		onError(_ << "no instruction at " << addr);
	if(logFor(LOG_INST))
		log << "\treturn at " << addr << io::endl;
	IS_RETURN(inst) = true;
}


/**
 * This method is called each time a "noreturn ADDRESS" statement is found.
 * @param addr	Address of the entry of the function.
 */
void FlowFactLoader::onNoReturn(address_t addr) {
	if(addr.isNull())
		return;
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
	if(!inst) {
		if(!lib)
			throw ProcessorException(*this, _ << " label \"" << name << "\" does not exist.");
	}
	else
		NO_RETURN(inst) = true;
}


/**
 * Get the address of the given label.
 * @param label					Label to look for.
 * @return						Matching address or null address if not found.
 */
Address FlowFactLoader::addressOf(const string& label) {
	Address res = _fw->process()->findLabel(label);
	if(res.isNull()) {
		if(lib)
			return Address::null;
		else
			onWarning(_ << "label \"" << label << "\" does not exist.");
	}
	return res;
}


/**
 * Called for the F4 production: "nocall ADDRESS".
 * @param address	Address of the instruction to work on.
 * @throw ProcessorException	If the instruction cannot be found.
 */
void FlowFactLoader::onNoCall(Address address) {
	if(address.isNull())
		return;
	Inst *inst = _fw->process()->findInstAt(address);
	if(!inst)
		onError(_ << " no instruction at  " << address << ".");
	else
		NO_CALL(inst) = true;
}


/**
 * Called for the F4 production: "noblock ADDRESS".
 * @param address	Address of the instruction to work on.
 * @throw ProcessorException	If the instruction cannot be found.
 */
void FlowFactLoader::onNoBlock(Address address) {
	if(address.isNull())
		return;
	Inst *inst = _fw->process()->findInstAt(address);
	if(!inst)
		onError(_ << " no instruction at  " << address << ".");
	else
		NO_BLOCK(inst) = true;
}


/**
 * Called for the F4 production: "force-branch ADDRESS".
 * @param address	Address of the instruction to work on.
 * @throw ProcessorException	If the instruction cannot be found.
 */
void FlowFactLoader::onForceBranch(Address address) {
	if(address.isNull())
		return;
	Inst *inst = _fw->process()->findInstAt(address);
	if(!inst)
		onError(_ << " no instruction at  " << address << ".");
	else {
		Inst::kind_t k;
		if(!inst->hasProp(ALT_KIND))
			k = inst->kind();
		else
			k = ALT_KIND(inst);
		ALT_KIND(inst) = (k & ~(Inst::IS_CALL | Inst::IS_RETURN)) | Inst::IS_CONTROL;
	}
}

/**
 * Called for the F4 production: "force-call ADDRESS".
 * @param address	Address of the instruction to work on.
 * @throw ProcessorException	If the instruction cannot be found.
 */
void FlowFactLoader::onForceCall(Address address) {
	if(address.isNull())
		return;
	Inst *inst = _fw->process()->findInstAt(address);
	if(!inst)
		onError(_ << " no instruction at  " << address << ".");
	else {
		Inst::kind_t k;
		if(!inst->hasProp(ALT_KIND))
			k = inst->kind();
		else
			k = ALT_KIND(inst);
		ALT_KIND(inst) = (k & ~Inst::IS_RETURN) | (Inst::IS_CONTROL | Inst::IS_CALL);
	}
}


/**
 * Called for the F4 production: "noinline ADDRESS" or "inline ADDRESS".
 *
 * @param address	Address of the instruction to work on.
 * @param no_inline	The boolean value to be set in NO_INLINE
 * @throw ProcessorException	If the instruction cannot be found.
 */
void FlowFactLoader::onNoInline(Address address, bool no_inline, const ContextualPath& path) {
	Inst *inst = _fw->process()->findInstAt(address);
	if(!inst)
		onError(_ << " no instruction at  " << address << ".");
	else
		path.ref(NO_INLINE, inst) = no_inline;

	if(logFor(LOG_BB))
		log << "\t" << path << "(NO_INLINE," << address << ") = " << no_inline << io::endl;
}


/**
 * Called for the F4 production: "inlining-on ADDRESS"
 * or "inlining-off ADDRESS".
 *
 * @param address	Address of the instruction to work on.
 * @param policy	The boolean value to be set in INLINING_POLICY
 * @throw ProcessorException	If the instruction cannot be found.
 */
void FlowFactLoader::onSetInlining(Address address, bool policy, const ContextualPath& path) {
	Inst *inst = _fw->process()->findInstAt(address);
	if(!inst)
		onError(_ << " no instruction at  " << address << ".");
	else
		path.ref(INLINING_POLICY, inst) = policy;

	if(logFor(LOG_BB))
		log << "\t" << path << "(INLINING_POLICY," << address << ") = " << policy << io::endl;
}


/**
 * Called for the F4 production: "preserver ADDRESS".
 * @param address	Address of instruction to preserve.
 * @throw ProcessorException	If the instruction cannot be found.
 */
void FlowFactLoader::onPreserve(Address address) {
	if(address.isNull())
		return;
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
	if(address.isNull())
		return;
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
	if(address.isNull())
		return;
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
void FlowFactLoader::onMultiBranch(Address control, const Vector<Address>& targets) {
	if(control.isNull())
		return;

	// Find the instruction
	Inst *inst = _fw->process()->findInstAt(control);
	if(!inst)
		onError(_ << " no instruction at  " << control << ".");

	// List of targets
	if(logFor(LOG_BB))
		log << "\tmultibranch at " << control << " to "
			<< io::list(targets, ", ") << io::endl;
	for(int i = 0; i < targets.length(); i++)
		if(!targets[i].isNull()) {
			bool found = false;
			for(Identifier<Address>::Getter target(inst, BRANCH_TARGET); target(); target++)
				if (target.item() == targets[i]) {
					found = true;
					break;
				}
			if (!found)
				BRANCH_TARGET(inst).add(targets[i]);
		}
}


/**
 * Called for the F4 construction "multicall ADDRESS to ADDRESS, ...".
 * @param control	Multi-call instruction address.
 * @param target	List of targets.
 */
void FlowFactLoader::onMultiCall(Address control, const Vector<Address>& targets) {
	if(control.isNull())
		return;

	// Find the instruction
	Inst *inst = _fw->process()->findInstAt(control);
	if(!inst)
		onError(_ << " no instruction at  " << control << ".");

	// List of targets
	if(logFor(LOG_BB))
		log << "\tmultibranch at " << control << " to "
			<< io::list(targets, ", ") << io::endl;
	for(int i = 0; i < targets.length(); i++)
		if(!targets[i].isNull()) {
			bool found = false;
			for(Identifier<Address>::Getter target(inst, CALL_TARGET); target(); target++)
				if (target.item() == targets[i]) {
					found = true;
					break;
				}
			if (!found)
				CALL_TARGET(inst).add(targets[i]);
		}
}


/**
 * This method is called each the production "loop ADDRESS ?" is found.
 * It usually emit an error.
 * @param addr	Address of the loop entry.
 */
void FlowFactLoader::onUnknownLoop(Address addr) {
	if(!ignore_unknown)
		onError(_ << "no bound for loop at " << addr);
}


/**
 * This method is called each the production "multibranch ADDRESS to ?" is found.
 * It usually emit an error.
 * @param control	Address of the control instruction.
 */
void FlowFactLoader::onUnknownMultiBranch(Address control) {
	if(!ignore_unknown)
		onError(_ << "undefined targets for multi-branch at " << control);
}


/**
 * This method is called each the production "multicall ADDRESS to ?" is found.
 * It usually emit an error.
 * @param control	Address of the control instruction.
 */
void FlowFactLoader::onUnknownMultiCall(Address control) {
	if(!ignore_unknown)
		onError(_ << "undefined targets for multi-call at " << control);
}

/** 
 * Scan a not_all XML element.
 * @param element	... element
 * @param cpath		contextual path
 */
 
void FlowFactLoader::scanEdge(xom::Element* edge,  ContextualPath& cpath  ){
	
	    EdgeInfoOfConflict * edgeInfo = new EdgeInfoOfConflict();
	    //LockPtr<EdgeInfoOfConflict> edgeInfo(new EdgeInfoOfConflict());
		String countSrcStr = edge->getAttributeValue("src").value();
		String countDstStr = edge->getAttributeValue("dst").value();
		if (countSrcStr.charAt(1) == 'x' && countSrcStr.charAt(0) == '0'){
			countSrcStr = countSrcStr.substring(2, (countSrcStr.length())-2); // addr format : "0x123456"
			countDstStr = countDstStr.substring(2, (countDstStr.length())-2); // addr format : "0x123456"
		}
		else if (countSrcStr == "*"){ countSrcStr = "0";	}

		unsigned int source = 0, dest = 0;
		sscanf(countSrcStr.chars(), "%x", &source);
		sscanf(countDstStr.chars(), "%x", &dest);			
 		if(source) {
			edgeInfo->setSource( MemArea(source, 4).address());	
			 
			numOfEdgeIntoCurrentCte++;
			edgeInfo->getInfoOfConflicts().push(Pair<int,int>(currentCteNum, numOfEdgeIntoCurrentCte));			 
			if(dest) {
				edgeInfo->setTarget(MemArea(dest, 4).address());
				// Find the instruction
 				Inst *inst = _fw->process()->findInstAt(edgeInfo->getSource());
				if(!inst)
					onError(_ << " no instruction at  " << edgeInfo->getSource() << ".");
				LockPtr<ListOfEdgeConflict > max  = cpath(EDGE_OF_INFEASABLE_PATH_I, inst);
				
 				int trouve =false;
				for(int k55=0;max &&!trouve && k55<max->length();k55++){	
						LockPtr<EdgeInfoOfConflict> ct  = max->get(k55); //->item(k55);  
						if (ct->getSource() == edgeInfo->getSource() && edgeInfo->getTarget()  == ct->getTarget() ) { 
							trouve=true;
							 
							ct->getInfoOfConflicts().push(Pair<int,int>(currentCteNum, numOfEdgeIntoCurrentCte));
						 
							cpath.ref(EDGE_OF_INFEASABLE_PATH_I, inst) = max; 
						}
				}		
				if (!trouve){
					if (!max) max = new ListOfEdgeConflict();

					max->push(edgeInfo);
					cpath.ref(EDGE_OF_INFEASABLE_PATH_I, inst) = max; 
				}		
			}
		}  				 
} 
	

/**
 * Load the flow facts from an XML file.
 * @param path	Path of the XML file.
 */
void FlowFactLoader::loadXML(const string& path) {

	// open the file
	xom::Builder builder;
	xom::Document *doc = builder.build(path.asSysString());
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
 * "nocall", "noblock", "doinline", "noinline", "inlining-on", "inlining-off",
 * "flowfacts", "ignore-entry", "multibranch", "multicall", "ignorecontrol",
 * "ignoreseq", "mem-access", "mem-set", "reg-set".
 * @param body	Content of the file.
 * @param cpath	Contextual path.
 */
void FlowFactLoader::scanXBody(xom::Element *body, ContextualPath& cpath) {
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
				Address addr = scanAddress(element, cpath).address();
				if(!addr.isNull())
					onNoReturn(addr);
				else
					onWarning(_ << "ignoring this because its address cannot be determined: " << xline(element));
			}
			else if(name == "return") {
				Address addr = scanAddress(element, cpath).address();
				if(!addr.isNull())
					onReturn(addr);
				else
					onWarning(_ << "ignoring this because its address cannot be determined: " << xline(element));
			}
			else if(name == "nocall") {
				Address addr = scanAddress(element, cpath).address();
				if(!addr.isNull())
					onNoCall(addr);
				else
					onWarning(_ << "ignoring this because its address cannot be determined: " << xline(element));
			}
			else if(name == "noblock") {
				Address addr = scanAddress(element, cpath).address();
				if(!addr.isNull())
					onNoBlock(addr);
				else
					onWarning(_ << "ignoring this because its address cannot be determined: " << xline(element));
			}
			else if(name == "doinline")
				scanNoInline(element, cpath, false);
			else if(name == "noinline")
				scanNoInline(element, cpath, true);
			else if(name == "inlining-on")
				scanSetInlining(element, cpath, true);
			else if(name == "inlining-off")
				scanSetInlining(element, cpath, false);
			else if(name == "flowfacts")
				scanXBody(element, cpath);
			else if(name == "ignore-entry")
				scanIgnoreEntry(element);
			else if(name == "multibranch")
				scanMultiBranch(element, cpath);
			else if(name == "multicall")
				scanMultiCall(element, cpath);
			else if(name == "ignorecontrol")
				scanIgnoreControl(element, cpath);
			else if(name == "ignoreseq")
				scanIgnoreSeq(element, cpath);
			else if(name == "mem-access")
				scanMemAccess(element);
			else if(name == "mem-set")
				scanMemSet(element, state);
			else if(name == "reg-set")
				scanRegSet(element, state);
			else if(name == "state")
				scanXState(element);
			else if(name == "not-all") 	{		 	
					if (intoConflictPath)  {
						onWarning(_ << " Error of ffx not all into not all: " << xline(element));
						return;
					}
					else  scanXNotAll(element, cpath);  
			}
			else if (name == "iteration") { 
						scanXContent( element,   cpath);
				}
			else	 if(intoConflictPath&&name == "edge") 	{	 				
					 scanEdge(element, cpath); 
			} 
			else
				warn(_ << "garbage at \"" << xline(child) << "\"");
		}
	}
}

			
void FlowFactLoader::scanXNotAll(xom::Element *element, ContextualPath& cpath){
 	 
 	intoConflictPath = true;
 	if (element->getAttributeValue("seq").value() == "true"){
		currentCteNum++;
		numOfEdgeIntoCurrentCte=0;
		scanXContent( element,   cpath);
	} 
 	intoConflictPath = false;
}

/**
 * Scan a noinline/doinline XML element.
 * @param element	ignoreseq element
 * @param cpath		contextual path
 */
void FlowFactLoader::scanNoInline(xom::Element *element, ContextualPath& cpath, bool no_inline) {
	MemArea mem_area = scanAddress(element, cpath);
	if(mem_area.isNull()) {
		onWarning(_ << "ignoreseq ignored at " << xline(element) << " ... address cannot be determined");
		return;
	}

	Inst *inst = workSpace()->process()->findInstAt(mem_area.address());
	while (inst && !inst->isControl()
			&& inst->address() <= mem_area.lastAddress())
		inst = workSpace()->process()->findInstAt(inst->topAddress());
	if (!inst || inst->address() > mem_area.lastAddress()) {
		onWarning(_ << "ignoreseq ignored at " << xline(element) << " ... no control found");
		return;
	}

	this->onNoInline(inst->address(), no_inline, cpath);
}


/**
 * Scan a inlining-off/inlining-on XML element.
 * @param element	ignoreseq element
 * @param cpath		contextual path
 */
void FlowFactLoader::scanSetInlining(xom::Element *element, ContextualPath& cpath, bool policy) {
	MemArea mem_area = scanAddress(element, cpath);
	if(mem_area.isNull()) {
		onWarning(_ << "ignoreseq ignored at " << xline(element) << " ... address cannot be determined");
		return;
	}

	Inst *inst = workSpace()->process()->findInstAt(mem_area.address());
	while (inst && !inst->isControl()
			&& inst->address() <= mem_area.lastAddress())
		inst = workSpace()->process()->findInstAt(inst->topAddress());
	if (!inst || inst->address() > mem_area.lastAddress()) {
		onWarning(_ << "ignoreseq ignored at " << xline(element) << " ... no control found");
		return;
	}

	this->onSetInlining(inst->address(), policy, cpath);
}


/**
 * Scan a multibranch XML element composed of items with a different location each.
 * @param element	multibranch element
 * @param cpath		contextual path
 */
void FlowFactLoader::scanMultiBranch(xom::Element *element, ContextualPath& cpath) {
	MemArea mem_area = scanAddress(element, cpath);
	if(mem_area.isNull()) {
		onWarning(_ << "multibranch ignored at " << xline(element) << " ... address cannot be determined");
		return;
	}

	Inst *inst = workSpace()->process()->findInstAt(mem_area.address());
	while (inst && !inst->isBranch()
			&& inst->address() <= mem_area.lastAddress())
		inst = workSpace()->process()->findInstAt(inst->topAddress());
	if (!inst || inst->address() > mem_area.lastAddress()) {
		onWarning(_ << "multibranch ignored at " << xline(element) << " ... no branch found");
		return;
	}

	Vector<Address> targets;
	xom::Elements *items = element->getChildElements("target");
	for(int i = 0; i < items->size(); i++) {
		MemArea target_area = scanAddress(items->get(i), cpath);
		if(target_area.isNull())
			onWarning(_ << "target of multibranch ignored at " << xline(items->get(i)));
		else
			targets.add(target_area.address());
	}
	delete items;
	this->onMultiBranch(inst->address(), targets);
}


/**
 * Scan a multicall XML element composed of items with a different location each.
 * @param element	multicall element
 * @param cpath		contextual path
 */
void FlowFactLoader::scanMultiCall(xom::Element *element, ContextualPath& cpath) {
	MemArea mem_area = scanAddress(element, cpath);
	if(mem_area.isNull()) {
		onWarning(_ << "multicall ignored at " << xline(element) << " ... address cannot be determined");
		return;
	}

	Inst *inst = workSpace()->process()->findInstAt(mem_area.address());
	while (inst && !inst->isCall()
			&& inst->address() <= mem_area.lastAddress())
		inst = workSpace()->process()->findInstAt(inst->topAddress());
	if (!inst || inst->address() > mem_area.lastAddress()) {
		onWarning(_ << "multicall ignored at " << xline(element) << " ... no call found");
		return;
	}

	Vector<Address> targets;
	xom::Elements *items = element->getChildElements("target");
	for(int i = 0; i < items->size(); i++) {
		MemArea target_area = scanAddress(items->get(i), cpath);
		if(target_area.isNull())
			onWarning(_ << "target of multicall ignored at " << xline(items->get(i)));
		else
			targets.add(target_area.address());
	}
	delete items;
	this->onMultiCall(inst->address(), targets);
}


/**
 * Scan a ignorecontrol XML element.
 * @param element	ignorecontrol element
 * @param cpath		contextual path
 */
void FlowFactLoader::scanIgnoreControl(xom::Element *element, ContextualPath& cpath) {
	MemArea mem_area = scanAddress(element, cpath);
	if(mem_area.isNull()) {
		onWarning(_ << "ignorecontrol ignored at " << xline(element) << " ... address cannot be determined");
		return;
	}

	Inst *inst = workSpace()->process()->findInstAt(mem_area.address());
	while (inst && !inst->isControl()
			&& inst->address() <= mem_area.lastAddress())
		inst = workSpace()->process()->findInstAt(inst->topAddress());
	if (!inst || inst->address() > mem_area.lastAddress()) {
		onWarning(_ << "ignorecontrol ignored at " << xline(element) << " ... no control found");
		return;
	}

	this->onIgnoreControl(inst->address());
}


/**
 * Scan a ignoreseq XML element.
 * @param element	ignoreseq element
 * @param cpath		contextual path
 */
void FlowFactLoader::scanIgnoreSeq(xom::Element *element, ContextualPath& cpath) {
	MemArea mem_area = scanAddress(element, cpath);
	if(mem_area.isNull()) {
		onWarning(_ << "ignoreseq ignored at " << xline(element) << " ... address cannot be determined");
		return;
	}

	Inst *inst = workSpace()->process()->findInstAt(mem_area.address());
	while (inst && !inst->isControl()
			&& inst->address() <= mem_area.lastAddress())
		inst = workSpace()->process()->findInstAt(inst->topAddress());
	if (!inst || inst->address() > mem_area.lastAddress()) {
		onWarning(_ << "ignoreseq ignored at " << xline(element) << " ... no control found");
		return;
	}

	this->onIgnoreSeq(inst->address());
}


/**
 * Scan a value in the given element.
 * @param element	Element to scan value in.
 */
dfa::Value FlowFactLoader::scanValue(xom::Element *element) {

	// look for value element
	xom::Element *velem = element->getFirstChildElement("value");
	if(velem) {

		// get the initial address
		ContextualPath c;
		Address addr = this->scanAddress(velem, c).address();

		// look for step and count
		Option<long> base = scanInt(velem, "base");
		Option<long> step = scanInt(velem, "step");
		Option<long> count = scanInt(velem, "count");
		if(!step || !count || !base)
			return dfa::Value(addr.offset());
		else if((!step || !count) && base)
			return dfa::Value(*base);
		else if(base)
			return dfa::Value(*base, *step, *count);
		else
			return dfa::Value(addr.offset(), *step, *count);
	}

	// scan for high and low
	xom::Element *helem = element->getFirstChildElement("high");
	xom::Element *lelem = element->getFirstChildElement("low");
	if(helem && lelem) {
		ContextualPath c;
		Address haddr = scanAddress(helem, c).address();
		Address laddr = scanAddress(lelem, c).address();
		return dfa::Value(laddr.offset(), haddr.offset());
	}

	// unknown value
	onError(_ << "unknown value at " << xline(element));
	return dfa::Value();
}


/**
 * Scan for a memory access instruction.
 * @param element	Element to look in.
 */
void FlowFactLoader::scanMemAccess(xom::Element *element) {
	Address eaddr, haddr, laddr;

	// look for area
	xom::Elements *areas = element->getChildElements("area");

	// look for high and low
	xom::Element *helem = element->getFirstChildElement("high");
	xom::Element *lelem = element->getFirstChildElement("low");

	if(areas->size() <= 0 && (!helem || !lelem))
		onError(_ << "malformed mem-acess at " << xline(element));

	// build the memory access
	ContextualPath c;
	if (areas->size() > 0) {
		// use only area(s) by joining them if needed
		MemArea area = scanAddress(areas->get(0), c);
		for (int i = 1; i < areas->size(); ++i)
			area.join(scanAddress(areas->get(i), c));
		haddr = area.topAddress();
		laddr = area.address();
	} else {
		// use only high and low
		haddr = Address(scanAddress(helem, c).address());
		laddr = Address(scanAddress(lelem, c).address());
	}
	eaddr = Address(scanAddress(element, c).address());
	this->onMemoryAccess(eaddr, laddr, haddr, c);
}


/**
 * Load a register set instruction.
 * @param element	Element to get information from.
 */
void FlowFactLoader::scanRegSet(xom::Element *element, dfa::State* state) {

	// get the name
	Option<xom::String> name = element->getAttributeValue("name");
	if(!name)
		onError(_ << "name attribute required at " << xline(element));

	// get the value
	dfa::Value val;
	Option<xom::String> value = element->getAttributeValue("value");
	if(value)
		val = dfa::Value::parse(*value);
	else
		val = scanValue(element);

	// perform the instruction
	onRegSet(state, *name, val);
}


/**
 * Load a memory set.
 * @param element	Element to get information from.
 */
void FlowFactLoader::scanMemSet(xom::Element *element, dfa::State* state) {

	// list of known types
	static struct {
		cstring id;
		const Type *type;
	} map[] = {
		{ "int8", 	&Type::int8_type },
		{ "int16", 	&Type::int16_type },
		{ "int32", 	&Type::int32_type },
		{ "int64", 	&Type::int64_type },
		{ "uint8", 	&Type::uint8_type },
		{ "uint16", &Type::uint16_type },
		{ "uint32", &Type::uint32_type },
		{ "uint64",	&Type::uint64_type },
		{ "", 0 }
	};

	// get the type
	Option<xom::String> name = element->getAttributeValue("type");
	if(!name)
		onError(_ << "a type attribute is required at " << xline(element));
	const Type *type = 0;
	for(int i = 0; map[i].type; i++)
		if(*name == map[i].id) {
			type = map[i].type;
			break;
		}
	if(!type)
		onError(_ << "unknown type " << *name << " at " << xline(element));

	// get the address
	ContextualPath c;
	Address addr = this->scanAddress(element, c).address();


	// get the value
	dfa::Value val;
	Option<xom::String> value = element->getAttributeValue("value");
	if(value)
		val = dfa::Value::parse(*value);
	else
		val = scanValue(element);

	// perform the instruction
	onMemSet(state, addr, type, val);
}


/**
 * Scan a function XML element.
 * @param element	Element of the function.
 * @param path		Context path to access the function.
 */
void FlowFactLoader::scanXFun(xom::Element *element, ContextualPath& path) {
	Address addr;

	// look for the name attribute
	Option<xom::String> val = element->getAttributeValue("name");
	if(val) {
		addr = addressOf(*val);
		if(addr.isNull())
			throw ProcessorException(*this,
				_ << " no function named \"" << *val << "\" from " << xline(element));
	}

	// or an address
	else {
		addr = scanAddress(element, path).address();
		if(addr.isNull()) {
			onWarning(_ << "ignoring this function whose address cannot be found: "<< xline(element));
			return;
		}
	}

	// get the address
	Inst *inst = _fw->process()->findInstAt(addr);
	if(!inst)
		throw ProcessorException(*this,
			_ << " no instruction at  " << addr << " from " << xline(element));
	path.push(ContextualStep::FUNCTION, addr);
	
	
	int nb =containsNotALL	 (element);
	if (nb>0) {
		if (!intoConflictPath) {
			if (!numListOfUnclosedPath.isEmpty() && !addr.isNull() ){
							 LockPtr<ListOfEndConflict>  ff  = path(INFEASABLE_PATH_END, inst);
							if (!numListOfUnclosedPath.isEmpty() && !ff) /* new vector*/ 
								 ff =  new ListOfEndConflict();
								 
							while(!numListOfUnclosedPath.isEmpty() ){
								int numOfUnclosedPath=numListOfUnclosedPath.pop();
								ff->push(numOfUnclosedPath);
							}
							path.ref(INFEASABLE_PATH_END, inst) = ff;
						
						 
					}
			numListOfUnclosedPath.clear();
		}
		int num = currentCteNum;
		int nbE = numOfEdgeIntoCurrentCte;
		for (int j=0; j<nb;j++) this->onInfeasablePath( inst->address(),   path);
	    currentCteNum=num;
	    numOfEdgeIntoCurrentCte=nbE;
	}
	

	// scan the content
	scanXContent(element, path);
	path.pop();
}


/**
 * Scan a call element that may have possibly two forms:
 * @li contains a list of called functions,
 * @li contains a function content (compatibility with old syntax).
 */
void FlowFactLoader::scanXCall(xom::Element *element, ContextualPath& path) {

	// look for functions
	xom::Elements *elems = element->getChildElements("function");

	// list of function form
	if(elems->size() != 0) {

		// get the address
		Address addr = scanAddress(element, path, true).address();
		if(addr.isNull()) {
			onWarning(_ << "ignoring this call whose address cannot be found: " << xline(element));
			return;
		}

		// scan the content
		path.push(ContextualStep::CALL, addr);
		for(int i = 0; i < elems->size(); i++)
			scanXFun(elems->get(i), path);
		path.pop();
	}

	// old form
	else {

		// get the address
		Address addr = scanAddress(element, path, true).address();
		if(addr.isNull()) {
			onWarning(_ << "ignoring this call whose address cannot be found: " << xline(element));
			return;
		}
		Inst *inst = _fw->process()->findInstAt(addr);
		if(!inst)
			throw ProcessorException(*this, _ << " no instruction at  " << addr << " from " << xline(element));
		path.push(ContextualStep::CALL, addr);

		// scan the content
		bool pushed = false;
		Option<xom::String> name = element->getAttributeValue("name");
		if(name) {
			Inst *i = workspace()->process()->findInstAt(*name);
			if(i) {
				path.push(ContextualStep::FUNCTION, i->address());
				pushed = true;
			}
		}
		scanXContent(element, path);

		// pop call
		if(pushed)
			path.pop();
		path.pop();
	}

	// cleanup
	delete elems;
}


 
void  FlowFactLoader::getQualifierAnd(xom::Element * element, Inst *inst, int *nbPath, bool nextLoop, 
ContextualPath& path,/*genstruct::*/Vector<LoopOfConflict>  &infoLoop  ){
	
	for(int i = 0; i < element->getChildCount(); i++) {
		xom::Node *child = element->getChild(i);
		if(child->kind() == xom::Node::ELEMENT) {
			xom::Element *elt = (xom::Element *)child;
			xom::String name = elt->getLocalName();
			if(name == "loop")
				getQualifierAnd(elt, inst, nbPath , true, path, infoLoop);
			 
			else if(name == "not-all") {		 				
					*nbPath = * nbPath+1;
					getQualifierAnd(elt, inst, nbPath , nextLoop, path, infoLoop);
			}
			else if (nextLoop == false && name == "iteration")
			{		String SrcStr = elt->getAttributeValue("number").value();
					LoopOfConflict::conflictLoopQualifier qualifier = ( (SrcStr == "*") ? LoopOfConflict::ALL_IT : LoopOfConflict::LAST_IT);//id constrain+qualifier vector
					LoopOfConflict info  (*nbPath, qualifier)  ;
					infoLoop.add(info);
					getQualifierAnd(elt,inst, nbPath , nextLoop, path, infoLoop);
			}
			else  getQualifierAnd(elt,inst, nbPath , nextLoop, path, infoLoop); 
		}
	}
}

 

// context version
void FlowFactLoader::scanXState(xom::Element *element, ContextualPath& path) {
	dfa::State* state = new dfa::State(*(workspace()->process()));
	Address addr = scanAddress(element, path).address();
	if(addr.isNull()) {
		onWarning(_ << "ignoring this loop whose address cannot be computed: " << xline(element));
		return;
	}
	Inst *inst = _fw->process()->findInstAt(addr);
	if(!inst)
		throw ProcessorException(*this,
			_ << " no instruction at  " << addr << " from " << xline(element));

	for(int i = 0; i < element->getChildCount(); i++) {
		xom::Node *child = element->getChild(i);
		if(child->kind() == xom::Node::ELEMENT) {
			xom::Element *element = (xom::Element *)child;
			xom::String name = element->getLocalName();
			if(name == "reg-value") {
				scanRegSet(element, state);
			}
			else if(name == "mem-value") {
				scanMemSet(element, state);
			}
		}
	}
	path.ref(PROVIDED_STATE, inst) = state;
	EXIST_PROVIDED_STATE(inst) = true;
}


// global version
void FlowFactLoader::scanXState(xom::Element *element) {
	ContextualPath path;
	dfa::State* state = new dfa::State(*(workspace()->process()));
	Address addr = scanAddress(element, path).address();
	if(addr.isNull()) {
		onWarning(_ << "ignoring this loop whose address cannot be computed: " << xline(element));
		return;
	}
	Inst *inst = _fw->process()->findInstAt(addr);
	if(!inst)
		throw ProcessorException(*this,
			_ << " no instruction at  " << addr << " from " << xline(element));

	for(int i = 0; i < element->getChildCount(); i++) {
		xom::Node *child = element->getChild(i);
		if(child->kind() == xom::Node::ELEMENT) {
			xom::Element *element = (xom::Element *)child;
			xom::String name = element->getLocalName();
			if(name == "reg-value") {
				scanRegSet(element, state);
			}
			else if(name == "mem-value") {
				scanMemSet(element, state);
			}
		}
	}
	PROVIDED_STATE(inst) = state;
	EXIST_PROVIDED_STATE(inst) = true;
}


/**
 * Scan a loop XML element.
 * @param element	Element of the loop.
 * @param path		Context path to access the loop.
 */
void FlowFactLoader::scanXLoop(xom::Element *element, ContextualPath& path) {

	// get the address
	Address addr = scanAddress(element, path).address();
	if(addr.isNull()) {
		onWarning(_ << "ignoring this loop whose address cannot be computed: " << xline(element));
		return;
	}
	Inst *inst = _fw->process()->findInstAt(addr);
	if(!inst)
		throw ProcessorException(*this,
			_ << " no instruction at  " << addr << " from " << xline(element));

	// get the information
	int nbPath =currentCteNum ;

	// find info loop du conflic
	LockPtr <ListOfLoopConflict > aaa= path.ref(LOOP_OF_INFEASABLE_PATH_I, inst) ;
	if (!aaa) 	aaa = new ListOfLoopConflict();
	
	getQualifierAnd(element,   inst , &nbPath, false, path , *aaa);
 
	if (aaa->length() > 0)path(LOOP_OF_INFEASABLE_PATH_I, inst) = aaa;  
    int nb =containsNotALL	 (element);
	if (nb>0) { 
		int nbE = numOfEdgeIntoCurrentCte;
		int num = currentCteNum;
		for (int j=0; j<nb;j++) this->onInfeasablePath( inst->address(),   path);
	    currentCteNum=num;
	    numOfEdgeIntoCurrentCte = nbE;
		 
	} 	
	if (aaa->length() == 0) {
		// get the information
		Option<long> max = scanBound(element, "maxcount");
		Option<long> total = scanBound(element, "totalcount");
		Option<long> min = scanBound(element, "mincount");
		if(!max && !total)
			warn(_ << "loop exists at " <<  addr << " but no bound at " << xline(element));
			
		onLoop(addr, (max ? *max : -1), (total ? *total : -1), (min ? *min : -1), path);
	}
 	//END ADD

	// look for content
	scanXContent(element, path);
}


/**
 * Look in the instruction matching the given (file, line) location for a call.
 * @param file		Source file path.
 * @param line		Source file line.
 * @param r			Address of the found call instruction.
 * @return			Number of found calls.
 */
int FlowFactLoader::findCall(cstring file, int line, Address& r) {
	int c = 0;

	// get areas
	Vector<Pair<Address, Address> > areas;
	workspace()->process()->getAddresses(file, line, areas);
	if(!areas)
		return 0;

	// look in areas
	for(int i = 0; i < areas.count(); i++) {
		Inst *inst = workspace()->findInstAt(areas[i].fst);
		ASSERT(inst != nullptr);
		do {
			if(inst->isCall()) {
				c++;
				r = inst->address();
			}
			inst = workspace()->findInstAt(inst->topAddress());
		} while(inst && inst->address() < areas[i].snd);
	}
	return c;
}


/**
 * Retrieve the address of an element (function, loop, ...) from its XML element.
 * @param element	Element to scan in.
 * @param path		Context path.
 * @param call		Location for a call is required.
 * @return			Address of the element or null if there is an error.
 */
MemArea FlowFactLoader::scanAddress(xom::Element *element, ContextualPath& path, bool call) {

	// look for "symbol" attribute
	Option<xom::String> sym = element->getAttributeValue("symbol");
	if (sym) {
		Symbol *symbol = _fw->process()->findSymbol(*sym);
		if (!symbol)
			return MemArea::null;
		Option<long> size = scanInt(element, "size");
		return MemArea(symbol->address(), size ? *size : symbol->size());
	}

	// look "address" attribute
	Option<unsigned long> res = scanUInt(element, "address");
	if(res)
		return MemArea(*res, 4);

	// look for "name" and "offset
	Option<xom::String> val = element->getAttributeValue("label");
	if(val) {
		Address addr = addressOf(*val);
		if(addr.isNull())
			return MemArea::null;
		Option<long> offset = scanInt(element, "offset");
		return MemArea(addr + (int)(offset ? *offset : 0), 4);
	}

	// look for lonely offset
	Option<long> offset = scanInt(element, "offset");
	if(offset) {
		if(!path)
			throw ProcessorException(*this,
				_ << "'offset' out of addressed element at " << xline(element));
		return MemArea(path[path.count() - 1].address() + (int)*offset, 4);
	}

	// look for source and line
	Option<xom::String> source = element->getAttributeValue("source");
	if(source) {
		Option<long> line = scanInt(element, "line");
		if(!line)
			throw ProcessorException(*this,
				_ << "no 'line' but a 'source' at " << xline(element));
		if(!call)
			return addressOf(*source, *line);
		else {
			Address a;
			int c = findCall(*source, *line, a);
			if(!c)
				throw ProcessorException(*this, _ << "no call found at " << *source << ":" << *line << " in " << xline(element));
			else if(c > 1)
				throw ProcessorException(*this, _ << "location ambiguous for call at " << *source << ":" << *line << " in " << xline(element));
			else
				return MemArea(a, 1);
		}
	}

	// it is an error
	throw ProcessorException(*this,
		_ << "no location at " << xline(element));
}


/**
 * Scan an integer attribute.
 * @param element	Element to scan in.
 * @param name		Name of the element.
 * @return			Read element or none.
 */
Option<long> FlowFactLoader::scanInt(xom::Element *element, cstring name) {
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
	return none;
}


Option<unsigned long> FlowFactLoader::scanUInt(xom::Element *element, cstring name) {
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
	return none;
}


/**
 * Scan a a bound attribute.
 * @param element	Element to scan in.
 * @param name		Name of the element.
 * @return			Read element or none.
 */
Option<long> FlowFactLoader::scanBound(xom::Element *element, cstring name) {
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
	catch(io::IOException& e) {
		throw ProcessorException(*this, _ << "bad formatted bound at " << xline(element));
	}
	return none;
}


/**
 * Scan the content of 'function' or of a 'loop'.
 * @param element	Container element.
 * @param path		Current context path.
 */
void FlowFactLoader::scanXContent(xom::Element *element, ContextualPath& path) {
	for(int i = 0; i < element->getChildCount(); i++) {
		xom::Node *child = element->getChild(i);
		if(child->kind() == xom::Node::ELEMENT) {
			xom::Element *element = (xom::Element *)child;
			xom::String name = element->getLocalName();
			if(name == "loop")
				scanXLoop(element, path);
			else if(name == "call")
				scanXCall(element, path);
			else if(name == "conditional")
				scanXConditional(element, path);
			else if(name == "function")
				this->scanXFun(element, path);
			else if(name == "state")
				scanXState(element, path);	
			else if(name == "not-all") {		 				
					if  (intoConflictPath) {
						onWarning(_ << " Error of ffx not all into not all: " << xline(element));
						return;
					}
					else this->scanXNotAll(element, path);  
			}
			else if (name == "iteration")
			{ 
					this->scanXContent( element,   path);
			}
			else if(intoConflictPath&&name == "edge") 	{	 				
					scanEdge(element, path);  
			}
		}
	}
}


/**
 * Called to scan an "if" element.
 * @param element	"if" element.
 * @param path		Current contextual path.
 */
void FlowFactLoader::scanXConditional(xom::Element *element, ContextualPath& path) {
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
MemArea FlowFactLoader::addressOf(const string& file, int line) {
	if(!lines_available)
		onError("the current loader does not provide source line information");

	Vector<Pair<Address, Address> > addresses;
 	workSpace()->process()->getAddresses(file.toCString(), line, addresses);
 	if(!addresses) {
		warn(_ << "cannot find the source line " << file << ":" << line);
		return MemArea::null;
	}

 	MemArea area(addresses[0].fst, addresses[0].snd);
 	for (int i = 1; i < addresses.length(); ++i)
 		area.join(MemArea(addresses[i].fst, addresses[i].snd));

	if (logFor(LOG_DEPS))
		log << "\t" << file << ":" << line << " is " << area << io::endl;

 	return area;
}


/**
 * This property may be used in the configuration of a code processor
 * to pass the path of an flow fact file containing flow facts.
 * Several properties with this identifier may be passed.
 * @ingroup ff
 */
Identifier<Path> FLOW_FACTS_PATH("otawa::FLOW_FACTS_PATH", "");


/**
 * This property may be used in the configuration of a code processor
 * to directly pass an xml element containing flow facts in @ref ffx format.
 * Several properties with this identifier may be passed.
 * @ingroup ff
 */
Identifier<xom::Element *> FLOW_FACTS_NODES("otawa::FLOW_FACTS_NODES");


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
p::feature FLOW_FACTS_FEATURE("otawa::FLOW_FACTS_FEATURE", new Maker<FlowFactLoader>());


/**
 * This feature ensures that preservation information used by mkff is put
 * on instruction.
 * @ingroup f4
 *
 * @par Hooked Properties
 * @li @ref PRESERVED
 */
p::feature MKFF_PRESERVATION_FEATURE("otawa::MKFF_PRESERVATION_FEATURE", new Maker<FlowFactLoader>());


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
 * Provide an alternative kind for an instruction, superseding
 * the current kind of the instruction.
 * @ingroup ff
 *
 * @par Hooks
 * @li @ref Inst (@ref otawa::util::FlowFactLoader)
 */
Identifier<Inst::kind_t> ALT_KIND("otawa::ALT_KIND", 0);


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
 * Associate a program point with a provided dfa::State.
 * @ingroup ff
 *
 * @par Hooks
 * @li @ref Inst (@ref otawa::util::FlowFactLoader)
 */
Identifier<dfa::State*> PROVIDED_STATE("otawa::PROVIDED_STATE", 0);
Identifier<bool> EXIST_PROVIDED_STATE("otawa::EXISTPROVIDED_STATE", false);


/**
 * In configuration of the FlowFactLoader, makes it fail if no flow fact
 * fail is available.
 * @ingroup ff
 */
Identifier<bool> FLOW_FACTS_MANDATORY("otawa::FLOW_FACTS_MANDATORY", false);

/**
 * In configuration of the FlowFactLoader, makes it ignore unknown flow facts
 * @ingroup ff
 */
Identifier<bool> FLOW_FACTS_IGNORE_UNKNOWN("otawa::FLOW_FACTS_IGNORE_UNKNOWN", false);


/**
 * Put on the first instruction of a function that must not be called.
 * @par Features
 * @li @ref FLOW_FACTS_FEATURE
 * @par Hooks
 * @li @ref Inst
 * @ingroup ff
 */
Identifier<bool> NO_CALL("otawa::NO_CALL", false);


/**
 * Put on the first instruction of a block that musst not be executed
 * @par Features
 * @li @ref FLOW_FACTS_FEATURE
 * @par Hooks
 * @li @ref Inst
 * @ingroup ff
 */
Identifier<bool> NO_BLOCK("otawa::NO_BLOCK", false);


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
 * Put on instruction that may call to several targets or whose target
 * computation cannot computed. There is one property
 * with this identifier for each called target.
 * @par Features
 * @li @ref FLOW_fACTS_FEATURE
 * @par Hooks
 * @li @ref Inst
 * @ingroup ff
 */
Identifier<Address> CALL_TARGET("otawa::CALL_TARGET", Address());


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
 * @ingroup ff
 */
Identifier<bool> IGNORE_ENTRY("otawa::IGNORE_ENTRY", false);


/**
 * Put on a memory access instruction, provides a range
 * of possible accesses.
 *
 * @par Features
 * @li @ref FLOW_FACTS_FEATURE
 *
 * @par Hooks
 * @li @ref Inst
 * @ingroup ff
 */
Identifier<Pair<Address, Address> > ACCESS_RANGE("otawa::ACCESS_RANGE");
/**
 * Put on an infeasable path begining and end mark
 * 
 *
 * @par Features
 * @li @ref FLOW_FACTS_FEATURE
 *
 * @par Hooks
 * @li @ref Inst
 * @ingroup ff
 */
Identifier< LockPtr<ConflictOfPath  >> INFEASABLE_PATH("otawa::INFEASABLE_PATH",NULL);


Identifier<  LockPtr<ListOfEndConflict> > INFEASABLE_PATH_END("otawa::INFEASABLE_PATH_END", NULL);  
 /** Put on an infeasable path loop
 * Mark a loop that contains or is into an path constrain
 *
 * @par Features
 * @li @ref FLOW_FACTS_FEATURE
 *
 * @par Hooks
 * @li @ref Inst
 * @ingroup ff
 */

 
Identifier <LockPtr <ListOfLoopConflict > >    LOOP_OF_INFEASABLE_PATH_I ("otawa::LOOP_OF_INFEASABLE_PATH_I", NULL) ;
 
/**
 * Put on an infeasable path edge
 * Mark a edge that  is into an path constrain  
 *
 * @par Features
 * @li @ref FLOW_FACTS_FEATURE
 *
 * @par Hooks
 * @li @ref Inst
 * @ingroup ff
 */

Identifier<LockPtr<ListOfEdgeConflict > > EDGE_OF_INFEASABLE_PATH_I ("otawa::EDGE_OF_INFEASABLE_PATH_I", NULL) ;

} // otawa
