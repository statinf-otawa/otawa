/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	gliss/platform.cpp -- implementation of the GLISS platform.
 */

#include <elm/debug.h>
#include <elm/io/io.h>
#define ISS_DISASM
#include <otawa/gliss.h>

namespace otawa { namespace gliss {

/**
 * Identification of the default platform.
 */
const Platform::Identification Platform::ID("powerpc-elf-");


/**
 * Default platform description.
 */
Platform Platform::platform;


/**
 * Build a new gliss platform with the given configuration.
 * @param props		Configuration properties.
 */
Platform::Platform(const PropList& props): ::otawa::Platform(ID, props) {
}


/**
 * Build a new platform by cloning.
 * @param platform	Platform to clone.
 * @param props		Configuration properties.
 */
Platform::Platform(const Platform& platform, const PropList& props)
: ::otawa::Platform(platform, props) {
}


/**
 */
bool Platform::accept(const Identification& id) {
	return id.abi() == "elf" && id.architecture() == "powerpc";
}

} }	// otawa::gliss
