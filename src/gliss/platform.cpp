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
static const RegBank *banks[] = {
	&Platform::GPR_bank,
	&Platform::FPR_bank,
	&Platform::CR_bank,
	&Platform::MISC_bank
};
static const elm::genstruct::Table<const RegBank *> banks_table(banks, 4);


/**
 * GPR register bank.
 */
const PlainBank Platform::GPR_bank("GPR", Register::INT,  32, "r%d", 32);


/**
 * FPR register bank.
 */
const PlainBank Platform::FPR_bank("FPR", Register::FLOAT, 64, "fr%d", 32);


/**
 * CR register bank
 */
const PlainBank Platform::CR_bank("CR", Register::BITS, 4, "cr%d", 8);


/**
 * CTR register
 */
hard::Register Platform::CTR_reg("ctr", Register::BITS, 32);


/**
 * LR register
 */
hard::Register Platform::LR_reg("lr", Register::ADDR, 32);


/**
 * XER register
 */
hard::Register Platform::XER_reg("xer", Register::INT, 32);


/**
 * MISC register bank
 */
const hard::MeltedBank Platform::MISC_bank("MISC", &Platform::CTR_reg,
	&Platform::LR_reg, &Platform::XER_reg, 0);


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
Platform::Platform(const PropList& props): hard::Platform(ID, props) {
	_banks = &banks_table;
}


/**
 * Build a new platform by cloning.
 * @param platform	Platform to clone.
 * @param props		Configuration properties.
 */
Platform::Platform(const Platform& platform, const PropList& props)
: hard::Platform(platform, props) {
	_banks = &banks_table;
}


/**
 */
bool Platform::accept(const Identification& id) {
	return id.abi() == "elf" && id.architecture() == "powerpc";
}

} }	// otawa::gliss
