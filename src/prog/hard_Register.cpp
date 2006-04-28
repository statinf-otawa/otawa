/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	otawa/hardware/Register.h -- Register classes interface.
 */

#include <otawa/hard/Register.h>

using namespace elm;

namespace otawa { namespace hard {
	
/**
 * @class Register
 * Objects of this class are simple machine register, more accurately
 * unbreakable atomic registers. If you architecture provides register spanning
 * on multiple atomic register, use the SpanReg class for representing them.
 * @par
 * The spanning registers are rarely used : most instruction register accessors
 * use the atomic Register class splitting the logical spanning register into
 * its component atomic registers.
 * @par
 * There is three ways to declare a register bank :
 * @li the register are automatically generated from the bank description,
 * @li the bank is first defined and the register are defined by hand with
 * a reference on the bank,
 * @li the register are defined in a table and the table is passed to the 
 * bank constructor.
 */


/**
 * @enum Register::kind_t
 * This enumeration represents the differents kind of value found in hardware
 * registers.
 */


/**
 * @var Register::kind_t Register::NONE
 * Usually only used as null value for register kinds.
 */


/**
 * @var Register::kind_t Register::ADDR
 * A register specialized for containing an address.
 */


/**
 * @var Register::kind_t Register::INT
 * A register specialized for containing an integer value.
 */


/**
 * @var Register::kind_t Register::FLOAT
 * A register specialized for containing float value.
 */


/**
 * @var Register::kind_t Register::BITS
 * This kind defines registers not specialized in other kinds. This kind
 * represents special purpose registers as status register.
 */


/**
 * Build a new register. The name of the register is automatically built using
 * the bank name and appending the decimal register number.
 * @param number	Number of the register.
 * @param bank		Owner bank.
 */
Register::Register(int number, RegBank *bank): _number(number), _bank(bank) {
	assert(number >= 0);
	if(bank) {
		assert(number < bank->count());
		bank->_regs[number] = this;
	}
}


/**
 * Build a new register.
 * @param name		Register name.
 * @param number	Register number.
 * @param bank		Owner bank.
 */
Register::Register(const elm::String& name, int number, RegBank *bank)
: _number(number), _name(name), _bank(bank) {
	assert(number >= 0);
	if(bank) {
		assert(number < bank->count());
		bank->_regs[number] = this;
	}
}


/**
 * @fn int Register::number(void) const;
 * Get the register number.
 * @return	Register number.
 */


/**
 * @fn RegBank *Register::bank(void) const;
 * Get the owner bank of the register.
 * @return Register bank.
 */


/**
 * Get the name of the register.
 * @return Register name.
 */
elm::String& Register::name(void) {
	if(!_name) {
		assert(_bank);
		StringBuffer buf;
		buf << _bank->name();
		buf << _number;
		_name = buf.toString();
	}
	return _name;
}


/**
 * @fn kind_t Register::kind(void) const;
 * Get the kind of a register.
 */


/**
 * @class RegBank
 * This class represents a bank of registers.
 */


/**
 * Build a register bank with a predefined list of registers.
 * @param name	Bank name (used for building automatically the register names).
 * @param kind	Kind of registers in the bank. Registers of a bank must have the
 * same kind.
 * @param size	Size in bits of the registers in the bank. Registers of a bank
 * must have the same size.
 * @param count	Count of registers in the bank.
 * @param regs	Array of size count containing the registers.
 */
RegBank::RegBank(elm::CString name, Register::kind_t kind, int size, int count,
Register **regs): _name(name), _kind(kind), _size(size), _regs(regs, count),
flags(0) {
	assert(kind != Register::NONE);
	assert(size > 0);
	assert(count > 0);
	
	// Link registers
	for(int i = 0; i < count; i++)
		regs[i]->_bank = this;
}


/**
 * Build a register bank automatically (paramerter fill = true) or by
 * delaying the insertion of register (parameter fill = false).
 * @param name	Bank name (used for building automatically the register names).
 * @param kind	Kind of registers in the bank. Registers of a bank must have the
 * same kind.
 * @param size	Size in bits of the registers in the bank. Registers of a bank
 * must have the same size.
 * @param count	Count of registers in the bank.
 * @param fill	If true, the register are automatically built. Else the user
 * must defines register thereafter passing to their constructor the bank
 * reference.
 */
RegBank::RegBank(elm::CString name, Register::kind_t kind, int size, int count,
bool fill): _name(name), _kind(kind), _size(size),
_regs(new Register *[count], count), flags(FLAG_FreeTable) {
	assert(kind != Register::NONE);
	assert(size > 0);
	assert(count > 0);
	
	if(fill) {
		flags |= FLAG_FreeRegs;
		for(int i = 0; i < count; i++)
			new Register(i, this);
	}
}


/**
 */
RegBank::~RegBank(void) {
	if(flags & FLAG_FreeTable) {
		if(flags & FLAG_FreeRegs)
			for(int i = 0; i < _regs.count(); i++)
				delete _regs[i];
		delete _regs.table();
	}
}


/**
 * @fn elm::CString RegBank::name(void) const;
 * Get the name of the bank.
 * @return	Bank name.
 */


/**
 * @fn Register::kind_t RegBank::kind(void) const;
 * Get the kind of registers in the bank.
 * @return	Bank register kind.
 */


/**
 * @fn int RegBank::size(void) const;
 * Get the size, in bits, of the registers in the bank.
 * @return	Bank register size.
 */


/**
 * @fn int RegBank::count(void) const;
 * Get the count of register in the bank.
 * @return	Bank registers count.
 */


/**
 * @fn Register *RegBank::get(int number) const;
 * Get a register from the bank.
 * @param number	Number of the register to get.
 * @return			Register matching the given number.
 */


/**
 * @fn Register *RegBank::operator[](int index) const;
 * Short to RegBank::get().
 */

} } // otawa::hard
