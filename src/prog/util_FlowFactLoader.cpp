/*
 * $Id$
 * Copyright (c) 2005, IRIT-UPS <casse@irit.fr>
 * 
 * src/prog/util_FlowFactLoader.cpp -- FlowFactLoader class implementation.
 */

#include <stdio.h>
#include <otawa/util/FlowFactLoader.h>
#include <otawa/prog/FrameWork.h>

// Externals
extern FILE *util_fft_in;

namespace otawa {

/**
 * Launch the analysis of the associated flow fact file associated with the
 * current file.
 * @param fw	Current framework.
 */
void FlowFactLoader::run(FrameWork *fw) {
	
	File *file = fw->process()->program();
	elm::StringBuffer buffer;
	buffer.print(file->name());
	buffer.print(".ff");
	String path = buffer.toString();
	
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
}

} // otawa
