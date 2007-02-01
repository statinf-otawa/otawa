/*
 *	$Id$
 *	Copyright (c) 2005-07, IRIT UPS.
 *
 *	otawa;;gliss::Platform class interface.
 */
#ifndef OTAWA_GLISS_PLATFORM_H
#define OTAWA_GLISS_PLATFORM_H

#include <otawa/platform.h>

namespace otawa { namespace gliss {

// Platform class
class Platform: public hard::Platform {
public:
	static const Identification ID;
	Platform(const PropList& props = PropList::EMPTY);
	Platform(const Platform& platform, const PropList& props = PropList::EMPTY);

	// Registers
	static hard::Register CTR_reg;
	static hard::Register LR_reg;
	static hard::Register XER_reg;
	static const hard::PlainBank GPR_bank;
	static const hard::PlainBank FPR_bank;
	static const hard::PlainBank CR_bank;
	static const hard::MeltedBank MISC_bank;

	// otawa::Platform overload
	virtual bool accept(const Identification& id);
};

} } // otawa::gliss

#endif // OTAWA_GLISS_PLATFORM_H
