/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/heptane/HeptLoader.h -- interface for HeptLoader class.
 */
#ifndef OTAWA_LOADER_HEPTLOADER_H
#define OTAWA_LOADER_HEPTLOADER_H

#include <gliss.h>

namespace heptane {

// HeptLoader class
class HeptLoader: public gliss::Loader {

	// gliss::Loader overload
	virtual CString getName(void) const;
	virtual Process *load(Manager *_man, CString path, PropList& props);
	virtual Process *create(Manager *_man, PropList& props);
};
	
}  // heptane

#endif // OTAWA_LOADER_HEPTLOADER_H
