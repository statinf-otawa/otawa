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
	
	Iterator<File *> file(*fw->files());
	elm::StringBuffer buffer;
	buffer.put(file->name());
	buffer.put(".ff");
	String path = buffer.toString();
	
	// Open the file
	util_fft_in = fopen(&path.toCString(), "r");
	if(!util_fft_in) {
		onError("ERROR: cannot open the constraint file \"%s\".", &path.toCString());
		return;
	}
	
	// Perform the parsing
	util_fft_parse(this);
	
	// Close all
	fclose(util_fft_in);
}

} // otawa
