/*
 *	$Id$
 *	FlowFactLoader class implementation
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

// Externals
extern FILE *util_fft_in;

namespace otawa {

using namespace elm::system;

extern int fft_line;


/**
 * @page f4 F4 : Flow Facts File Format
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
 * given address has the given maximal bound,

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
 * @see
 * 		@ref f4 for more details on the flow facts files.
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
	
	// Build the F4 file path
	if(!path)
		path = _ << fw->process()->program()->name() << ".ff";

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
	util_fft_parse(this);
	
	// Close all
	fclose(util_fft_in);
	
	// Display warning if there is no checksum
	if(!checksummed && isVerbose())
		warn("no checksum: flow facts and executable file may no match !");
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
 * This method is called when a loop is found.
 * @param addr	Address of the header of the loop.
 * @param count	Bound on the loop iterations.
 */
void FlowFactLoader::onLoop(address_t addr, int count) {
	Inst *inst = _fw->process()->findInstAt(addr);
	MAX_ITERATION(inst) = count;
}
 
 
 /**
  * Check the consistency of the flow facts with the executable.
  * @param name	Concerned file name.
  * @param sum	Flow fact checksum.
  */
void FlowFactLoader::onCheckSum(const String& name, unsigned long sum) {
	
	// Find the file
	for(Process::FileIter file(_fw->process()); file; file++) {
		Path cpath = file->name();
		if(cpath.namePart() == name) {
			
			// Compute the checksum
			checksum::Fletcher summer;
			io::InFileStream stream(file->name());
			summer.put(stream);
			unsigned long sum2 = summer.sum();
			//cout << io::hex(sum) << " = " << io::hex(sum2) << io::endl;
			if(sum2 != sum)
				onError("bad checksum: flow facts and executable does not match !");
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
	  throw ProcessorException(*this, _ << "no instruction at " << addr);
	IS_RETURN(inst) = true;
}


/**
 * This method is called each time a "noreturn ADDRESS" statement is found.
 * @param addr	Address of the entry of the function.
 */
void FlowFactLoader::onNoReturn(address_t addr) {
	Inst *inst = _fw->process()->findInstAt(addr);
	if(!inst)
	  throw ProcessorException(*this, _ << "no instruction at " << addr);
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
		throw ProcessorException(*this,
			_ << "label \"" << label << "\" does not exist.");
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
		throw ProcessorException(*this,
			_ << " no instruction at  " << address << ".");
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
		throw ProcessorException(*this,
			_ << " no instruction at  " << address << ".");
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
		throw ProcessorException(*this,
			_ << " no instruction at  " << address << ".");
	else
		IGNORE_CONTROL(inst) = true;			
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
		throw ProcessorException(*this,
			_ << " no instruction at  " << control << ".");
	
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
 * This property may be used in the configuration of a code processor
 * to pass the path of an F4 file containing flow facts.
 */
Identifier<Path> FLOW_FACTS_PATH("otawa::flow_facts_path", "");


/**
 * This feature ensures that the flow facts has been loaded.
 * Currrently, only the @ref otawa::util::FlowFactLoader provides this kind
 * of information from F4 files.
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
 * 
 * @par Hooked Properties
 * @li @ref PRESERVED
 */
Feature<FlowFactLoader> MKFF_PRESERVATION_FEATURE("otawa::MKFF_PRESERVATION_FEATURE");


/**
 * Put on a control flow instruction, this shows that this instruction
 * is equivalent to a function return. It may be useful with assembly providing
 * very complex ways to express a function return.
 * 
 * @par Hooks
 * @li @ref Inst (@ref otawa::util::FlowFactLoader)
 */
Identifier<bool> IS_RETURN("otawa::is_return", false);


/**
 * This annotation is put on the first instruction of functions that does not
 * never return. It is usually put on the C library "_exit" function.
 * 
 * @par Hooks
 * @li @ref Inst (@ref otawa::util::FlowFactLoader)
 */
Identifier<bool> NO_RETURN("otawa::no_return", false);


/**
 * Put on the first instruction of a loop, it gives the maximum number of
 * iteration of this loop.
 * 
 * @par Hooks
 * @li @ref Inst (@ref otawa::util::FlowFactLoader)
 */
Identifier<int> MAX_ITERATION("otawa::max_iteration", -1);


/**
 * In configuration of the FlowFactLoader, makes it fail if no flow fact
 * fail is available.
 */
Identifier<bool> FLOW_FACTS_MANDATORY("otawa.flow_facts_mandatory", false);


/**
 * Put on the first instruction of a function that must be no called.
 * @par Features
 * @li @ref FLOW_FACTS_FEATURE
 * @par Hooks
 * @li @ref Inst
 */
Identifier<bool> NO_CALL("otawa::NO_CALL", false);


/**
 * Put on a control instruction to prevent it to be interpreted as is.
 * @par Features
 * @li @ref FLOW_fACTS_FEATURE
 * @par Hooks
 * @li @ref Inst
 */
Identifier<bool> IGNORE_CONTROL("otawa::IGNORE_CONTROL", false);


/**
 * Put on instruction that may branch to several targets or whose target
 * computation cannot computed. There is one property
 * with this identifier for each branched target.
 * @par Features
 * @li @ref FLOW_fACTS_FEATURE
 * @par Hooks
 * @li @ref Inst
 */
Identifier<Address> BRANCH_TARGET("otawa::BRANCH_TARGET", Address());


/**
 * Put on instruction that must preserved from the mkff special flow-fact
 * detection.
 * @par Features
 * @li @ref MKFF_PRESERVATION_FEATURE
 * @par Hooks
 * @li @ref Inst
 */
Identifier<bool> PRESERVED("otawa::PRESERVED", false);

} // otawa
