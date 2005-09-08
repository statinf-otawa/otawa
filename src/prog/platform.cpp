/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	platform.cc -- platform class implementation.
 */

#include <otawa/platform.h>

namespace otawa {

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

