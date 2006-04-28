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
#include <otawa/hard/Register.h>

using namespace otawa::hard;

namespace otawa { namespace gliss {

/**
 * Register banks.
 */
static RegBank GPR_bank("r", Register::INT,  32, 32, true);
static RegBank FPR_bank("fr", Register::FLOAT, 64, 32, true);
static RegBank *banks[] = {
	&GPR_bank,
	&FPR_bank
};
static elm::genstruct::Table<RegBank *> banks_table(banks, 2);


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
	_banks = &banks_table;
}


/**
 * Build a new platform by cloning.
 * @param platform	Platform to clone.
 * @param props		Configuration properties.
 */
Platform::Platform(const Platform& platform, const PropList& props)
: ::otawa::Platform(platform, props) {
	_banks = &banks_table;
}


/**
 */
bool Platform::accept(const Identification& id) {
	return id.abi() == "elf" && id.architecture() == "powerpc";
}

} }	// otawa::gliss
