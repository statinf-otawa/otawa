/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/heptane/Loader.h -- implementation for heptane::HeptLoader class.
 */

#include <elm/io.h>
#include "Process.h"
#include "Loader.h"

namespace otawa { namespace heptane {

/**
 * @class Loader
 * Implementation of Heptane loader.
 */


// otawa::Loader overload
CString Loader::getName(void) const {
	return Loader::LOADER_NAME_Heptane;
}


// otawa::Loader overload
otawa::Process *Loader::create(Manager *_man, PropList& props) {
	return new Process(_man , props);
}

}	// heptane

// PPC Heptane Loader entry point
static heptane::Loader heptane_loader;
otawa::Loader& otawa::Loader::LOADER_Heptane_PowerPC = heptane_loader;

} // otawa
