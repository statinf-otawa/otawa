/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	platform.cc -- platform class implementation.
 */

#include <otawa/platform.h>

namespace otawa {

/**
 * @class PlatformId
 * <p>This class represents a platform identifier that is composed by:</p>
 * @li architecture: processor family like 'x86' or 'powerpc'.
 * @li ABI or Application Binary Interface that defines the encoding of binary
 * and system call protocol like 'elf' or 'coff'.
 * @li machine that gives information about the hardware configuration of the
 * system, for example, 'pcat' or 'sim' for simulator.
 * <p>Each component may be datailed by one or many sub-components
 * separated by '/' slashes. For example, a full description of a classic
 * executable environment on Linux may be:
 * 'x86/i586-elf/linux/libc2.4-pcat'.</p>
 */

/**
 * Split the platform name into its components.
 */
void PlatformId::split(void) {
	
	// Find the architecture
	int start = 0, pos = name.indexOf('-');
	arch = name.substring(0, pos >= 0 ? pos : name.length());
	
	// Find the ABI
	if(pos >= 0) {
		start = pos + 1;
		pos = name.indexOf('-', start);
		abi = name.substring(start, pos >= 0 ? pos - start : name.length() - start);
	}
	
	// Find the machine
	if(pos >= 0) {
		start = pos + 1;
		pos = name.indexOf('-', start);
		mach = name.substring(start, pos >= 0 ? pos - start : name.length() - start);
	}
}

/**
 * Build a platform identifier from a C string.
 * @param _name	Name of the platform.
 */
PlatformId::PlatformId(CString _name): name(_name) {
	split();
}

/**
 * Build a platform identifier from a string.
 * @param _name	Name of the platform.
 */
PlatformId::PlatformId(const String& _name): name(_name) {
	split();
}

/**
 * Build an explicit platform identifier.
 * @param _arch		Archietcture.
 * @param _abi		ABI.
 * @param _mach	Machine.
 */
PlatformId::PlatformId(CString _arch, CString _abi, CString _mach)
: name(_arch + "-" + _abi + "-" + _mach) {
	split();
}

/**
 * @fn const String& PlatformId::getArch(void) const;
 * Get the architecture component of the platform identifier.
 * @return Architecture component.
 */

/**
 * @fn const String& PlatformId::getABI(void) const;
 * Get the ABI component of the platform identifier.
 * @return ABI component.
 */

/**
 * @fn const String& PlatformId::getMach(void) const;
 * Get the machine component of the platform identifier.
 * @return Machine component.
 */


/**
 * @class Platform
 * This class describes the platform supporting the execution of a program.
 * It provides information about the underlying hardware, execution and
 * programmation model, but also about the supporting OS.
 */

/**
 * @fn Platform::~Platform(void);
 * Procted for avoiding that anybody delete it while some resources are still
 * used.
 */

/**
 * @fn bool Platform::accept(const PlatformId& id);
 * Check if the platform handle the given platform description.
 * @param id	Platform identifier to check.
 * @return			True if the platform accepts this kind of architecture.
 */

/**
 * @fn bool Platform::accept(CString name);
 * Check if the platform accepts the given kind of architecture.
 * @param name of the acrhitecture.
 * @result True if the platform accepts the given architecture.
 */

/**
 * @fn bool Platform::accept(const String& name);
 * Check if the platform accepts the given kind of architecture.
 * @param name of the acrhitecture.
 * @result True if the platform accepts the given architecture.
 */


/**
 * @class RegBank
 * This class describes a register bank. It provides information about
 * the type of registers, the number of register and the number of bits in
 * the register.
 */

/**
 * @fn CString RegBank::name(void);
 * Get the generic name of the register. It may contain a special # character
 * that may be replaced by the number of a register.
 * @return Generic name.
 */

/**
 * @fn int RegBank::bits(void);
 * Get the number of bits in each register.
 * @return Number of bits in each register.
 */

/**
 * @fn int RegBank::count(void);
 * Get the number of register in a bank.
 * @return Number of bits.
 */

/**
 * @fn RegSet RegBank::unalias(Register reg);
 * As banks may be overriden due for alias, type or grouping purpose,
 * this method returns the lower or unaliased representation of the given register.
 * @param reg	Register to unalias.
 * @return			Lower representation of the register.
 */

/**
 * @fn Type *RegBank::type(void);
 * Get the type of the register in the bank.
 * @return Type of registers.
 */


/**
 * @class RegSet
 * This class is the unaliased representation of a register. As register may be
 * alaised by one or many other registers, the register set records the bank,
 * the number of the first unaliased register and the number of following
 * register aliased.
 */

/**
 * @fn RegSet::RegSet(RegBank *bank, int base, int size) ;
 * Build a register set from its description.
 * @param bank	Owner register bank.
 * @param base	Base register in the set.
 * @param size		Number of register in the set.
 */

/**
 * @fn RegSet::RegSet(const RegSet& set);
 * Build a register set by cloning.
 * @param set	Register set to clone.
 */

/**
 * @fn RegSet& RegSet::operator=(const RegSet& set);
 * Assignment for register set.
 * @param set	Register set to copy.
 * @return			This register set.
 */

/**
 * @fn RegBank *RegSet::bank(void) const;
 * Get the bank of the register set.
 * @return	Register set bank.
 */

/**
 * @fn int RegSet::base(void) const;
 * Get the number of the base register of the set.
 * @return	Number of the base register.
 */

/**
 * @fn int RegSet::size(void) const;
 * Get the number of registers in the set.
 * @return Number of registers in the set.
 */

/**
 * @fn bool RegSet::equals(const RegSet& set) const;
 * Test for equality.
 * @param set	Set to test with.
 * @return True if both are equals.
 */

/**
 * @fn bool RegSet::operator==(const RegSet& set) const;
 * Equality operator.
 * @param set	Set to test with.
 * @return True if both are equals.
 */
 
/**
 * @fn bool RegSet::operator!=(const RegSet& set) const;
 * Inequality operator.
 * @param set	Set to test with.
 * @return True if they are different.
 */

/**
 * @fn bool RegSet::override(const RegSet& set) const;
 * Test if bith set override, that is, if they have some register in common.
 * @return True if they ovveride.
 */


/**
 * @class Register
 * Represents a register on the platform. This register may be a true register
 * or an alias renaming or extending another register.
 */

/**
 * @fn Register::Register(RegBank *bank, int number);
 * Build a new register.
 * @param bank		Owner register bank.
 * @param number	Number of the register in the bank.
 */

/**
 * @fn Register::Register(const Register& reg);
 * Register building by copy.
 * @param reg	Register to copy.
 */

/**
 * @fn Register& Register::operator=(const Register& reg);
 * Assignment with register.
 * @param reg	Register to copy.
 * @return			This register.
 */

/**
 * @fn RegBank *Register::bank(void) const;
 * Get the bank of the register.
 * @return Register bank.
 */

/**
 * @fn int Register::number(void) const;
 * Get the register number in the bank.
 * @return Register number.
 */

/**
 * @fn bool Register::equals(const Register& reg);
 * Test if two register are equals, based on their unaliased form.
 * @return True if they are equals, false else.
 */

/**
 * @fn bool Register::operator==(const Register& reg);
 * Equality operator.
 * @param reg	Register to test.
 * @return			True if thet are equal.
 */

/**
 * @fn bool Register::operator!=(const Register& reg);
 * Inequality operator.
 * @param reg	Register to test.
 * @return			True if thet are not equal.
 */

/**
 * @fn bool Register::override(const Register& reg);
 * Test if both registers override, that is, if they have some sub-register
 * in common in the unaliased form.
 * @param reg	Register to test with.
 * @return			True if they override.
 */


};	// namespace otawa

