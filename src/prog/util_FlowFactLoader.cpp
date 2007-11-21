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

/**
 * @page f4 F4 : Flow Facts File Format
 * 
 * This file format is used to store flow facts information, currently, the
 * loop bounds. The usual non-mandatory extension of F4 files is "ff".
 * 
 * F4 is a simple text format. It may contain comments a-la C++, that is, one
 * line comments prefixed by "//" or enclosed comments between "/*" and "* /"."
 * Spaces and line format are not meaningful for other commands.
 * 
 * Other commands includes :
 * 
 * @li "loop ADDRESS COUNT ;"
 * Declare that the loop whose header start at the
 * given address has the given maximal bound,

 * @li "checksum VALUE ;"
 * This command is used to check the consistency
 * between the flow fact file and the matching executable file (it is the
 * Fletcher checksum of the executable file).
 * 
 * @li "return ADDRESS ;"
 * Mark a complex control instruction as equivallent to sub-program return.
 * 
 * @par To Come
 * @li "branch ADDRESS to ADDRESS ;"
 * @li "loop ADDRESS max COUNT min COUNT ;"
 * @li "multibranch ADDRESS to ADDRESS, ADDRESS, ... ;"
 * @li "recursive ADDRESS max COUNT min COUNT ;" 
 * @li "ignore ADDRESS ;"
 * @li "ignore call ADDRESS ;"
 * @li "ignore call NAME ;"
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
	checksummed(false) {
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
		else
			warn(msg);
	}
	
	// Perform the parsing
	util_fft_parse(this);
	
	// Close all
	fclose(util_fft_in);
	
	// Display warning if there is no checksum
	if(!checksummed && isVerbose())
		warn("no checksum: flow facts and executable file may no match !");
}


/**
 * This method is called when an error is encountered.
 * @param message	Message format a-la printf.
 * @param ...	Argument for the string format.
 */
void FlowFactLoader::onError(const char *fmt, ...) {
	VARARG_BEGIN(args, fmt)
		StringBuffer buf;
		buf.format(fmt, args);
		throw ProcessorException(*this, buf.toString());
	VARARG_END
}


/**
 * This method is called when a warning need to be displayed.
 * @param fmt	Message format a-la printf.
 * @param ...	Argument for the string format.
 */
void FlowFactLoader::onWarning(const char *fmt, ...) {
	VARARG_BEGIN(args, fmt)
		StringBuffer buf;
		buf.format(fmt, args);
		warn(buf.toString());
	VARARG_END	
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
	cout << "!!" << name << "!!" << io::endl;
	onError("bad checksum: file not found: \"%s\".", &name);	
}


/**
 * This method is called each a "return" statement is found.
 * @param addr	Address of the statement to mark as return.
 */
void FlowFactLoader::onReturn(address_t addr) {
	Inst *inst = _fw->process()->findInstAt(addr);
	IS_RETURN(inst) = true;
}


/**
 * This method is called each time a "noreturn ADDRESS" statement is found.
 * @param addr	Address of the entry of the function.
 */
void FlowFactLoader::onNoReturn(address_t addr) {
	Inst *inst = _fw->process()->findInstAt(addr);
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
Feature<FlowFactLoader> FLOW_FACTS_FEATURE;


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

} // otawa
