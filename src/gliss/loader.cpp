/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	loader.cpp -- GLISS loader classes interface.
 */

#include <elm/io/io.h>
#define ISS_DISASM
#include "gliss.h"

namespace otawa { namespace gliss {


/**
 * @class Loader
 * Implementation of the Otawa Loader class for GLISS PowerPC.
 */

/**
 * Get the name of the loader.
 * @return Loader name.
 */
CString Loader::getName(void) const {
	return LOADER_NAME_Gliss_PowerPC;
}

/**
 * Load a file with the current loader.
 * @param man		Caller manager.
 * @param path		Path to the file.
 * @param props	Properties.
 * @return	Created process or null if there is an error.
 */
Process *Loader::load(Manager *man, CString path, PropList& props) {
	Process *proc = create(man, props);
	if(!proc->loadFile(path)) {
		delete proc;
		return 0;
	}
	else
		return proc;
}


/**
 * Create an empty process.
 * @param man		Caller manager.
 * @param props	Properties.
 * @return	Created process.
 */
Process *Loader::create(Manager *man, PropList& props) {
	return new Process(man, props);
}


// PPC GLISS Loader entry point
static Loader static_loader;
otawa::Loader& otawa::Loader::LOADER_Gliss_PowerPC = otawa::gliss::static_loader;


} }	// otawa::gliss
