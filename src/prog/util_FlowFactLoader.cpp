/*
 * $Id$
 * Copyright (c) 2005, IRIT-UPS <casse@irit.fr>
 * 
 * src/prog/util_FlowFactLoader.cpp -- FlowFactLoader class implementation.
 */

#include <stdio.h>
#include <otawa/util/FlowFactLoader.h>
#include <otawa/prog/FrameWork.h>
#include <elm/checksum/Fletcher.h>
#include <elm/io/InFileStream.h>

// Externals
extern FILE *util_fft_in;

namespace otawa {

/**
 * @page f4 F4 : Flow Facts File Format
 * 
 * This file format is used to store flow facts information, currently, the
 * loop bounds. The usual non-mandatory extension of F4 files is "ff".
 * 
 * F4 is a simple text format. It may contain comments, starting from a
 * '#' character to the end of line. Spaces and line format are not
 * meaningful for other commands.
 * 
 * Other commands includes :
 * 
 * @li "loop address count ;"
 * Declare that the loop whose header start at the
 * given address has the given maximal bound,

 * @li "checksum value ;"
 * This command is used to check the consistency
 * between the flow fact file and the matching executable file (it is the
 * Fletcher checksum of the executable file).
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
FlowFactLoader::FlowFactLoader(void): checksummed(false) {
}


/**
 * Launch the analysis of the associated flow fact file associated with the
 * current file.
 * @param fw	Current framework.
 */
void FlowFactLoader::run(FrameWork *fw, Path path) {
	_fw = fw;
	
	// Build the F4 file path
	if(!path) {
		File *file = fw->process()->program();
		elm::StringBuffer buffer;
		buffer.print(file->name());
		buffer.print(".ff");
		path = buffer.toString();
	}

	// Open the file
	util_fft_in = fopen(&path, "r");
	if(!util_fft_in) {
		onError("cannot open the constraint file \"%s\".", &path);
		return;
	}
	
	// Perform the parsing
	util_fft_parse(this);
	
	// Close all
	fclose(util_fft_in);
	
	// Display warning if there is no checksum
	if(!checksummed)
		onWarning("no checksum: flow facts and executable file may no match !");
}


/**
 * @fn void FlowFactLoader::onError(const char *fmt, ...)
 * This method is called when an error is encountered.
 * @param fmt	Message format a-la printf.
 * @param ...	Argument for the string format.
 */


/**
 * @fn void FlowFactLoader::onWarning(const char *fmt, ...)
 * This method is called when a warning need to be displayed.
 * @param fmt	Message format a-la printf.
 * @param ...	Argument for the string format.
 */


/**
 * @fn void FlowFactLoader::onLoop(address_t addr, int count)
 * This method is called when a loop is found.
 * @param addr	Address of the header of the loop.
 * @param count	Bound on the loop iterations.
 */
 
 
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
 * This property may be used in the configuration of a code processor
 * to pass the path of an F4 file containing flow facts.
 */
GenericIdentifier<Path> FLOW_FACTS_PATH("flow_facts_path", "", OTAWA_NS);

} // otawa
