/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	loader.cpp -- GLISS loader classes interface.
 */

#include <elm/io/io.h>
//#include <otawa/manager.h>
#define ISS_DISASM
#include <otawa/gliss.h>

namespace otawa { namespace gliss {


/**
 * @class Loader
 * Implementation of the Otawa Loader class for GLISS PowerPC.
 */


/**
 * Alias table.
 */
static CString table[] = {
	"elf_20"
};
static elm::genstruct::Table<CString> ppc_aliases(table, 1);
 

/**
 * Build a new loader.
 */
Loader::Loader(void)
: otawa::Loader("ppc", Version(1, 0, 0), OTAWA_LOADER_VERSION, ppc_aliases) {
}


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
otawa::Process *Loader::load(Manager *man, CString path, PropList& props) {
	otawa::Process *proc = create(man, props);
	if(!proc->loadProgram(path)) {
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
 * @return		Created process.
 */
otawa::Process *Loader::create(Manager *man, PropList& props) {
	return new Process(man, props);
}


/**
 * @internal Identifier giving access to the GLISS state of the loaded program.
 */
GenericIdentifier<state_t *> GLISS_STATE("ppc.gliss_state", 0);

} }	// otawa::gliss


// PPC GLISS Loader entry point
otawa::gliss::Loader ppc_plugin;
otawa::Loader& otawa::Loader::LOADER_Gliss_PowerPC = ppc_plugin;
elm::system::Plugin& OTAWA_LOADER_HOOK = ppc_plugin;


