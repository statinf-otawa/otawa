/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	gliss/Platform.h -- gliss::Platform class interface.
 */
#ifndef OTAWA_GLISS_PLATFORM_H
#define OTAWA_GLISS_PLATFORM_H

#include <otawa/platform.h>

namespace otawa { namespace gliss {

// Platform class
class Platform: public ::otawa::Platform {
public:
	static const Identification ID;
	static Platform platform;
	Platform(const PropList& props = PropList::EMPTY);
	Platform(const Platform& platform, const PropList& props = PropList::EMPTY);

	// otawa::Platform overload
	virtual bool accept(const Identification& id);
};

} } // otawa::gliss

#endif // OTAWA_GLISS_PLATFORM_H
