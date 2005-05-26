/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/heptane/Loader.h -- interface for heptane::HeptLoader class.
 */
#ifndef OTAWA_HEPTANE_LOADER_H
#define OTAWA_HEPTANE_LOADER_H

#include <otawa/gliss.h>

//#define LOAD_OUT(txt) txt
#define LOAD_OUT(txt)

namespace otawa { namespace heptane {

// HeptLoader class
class Loader: public gliss::Loader {
public:
	inline Loader(void) { LOAD_OUT(cout << "INIT Heptane\n"); };
	inline ~Loader(void) { LOAD_OUT(cout << "FINISH Heptane\n"); };
	
	// gliss::Loader overload
	virtual CString getName(void) const;
	virtual otawa::Process *create(Manager *_man, PropList& props);
};
	
} } // otawa::heptane

#endif // OTAWA_LOADER_HEPTLOADER_H
