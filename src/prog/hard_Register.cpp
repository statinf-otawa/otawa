/*
 *	$Id$
 *	Register class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-07, IRIT UPS.
 * 
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software 
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <otawa/hard/Register.h>
#include <elm/util/Formatter.h>
#include <elm/io/BlockInStream.h>
#include <elm/util/VarArg.h>

using namespace elm;

namespace otawa { namespace hard {

// RegisterFormatter
class RegisterFormatter: private util::Formatter {
	io::BlockInStream templ;
	io::Output output;
	int index;
protected:
	virtual int process(io::OutStream& out, char chr) {
		output.setStream(out);
		switch(chr) {
		case 'd':
			output << index;
			return DONE;
		case 'a':
		case 'A':
			output << (char)(chr + index);
			return DONE;
		default:
			return REJECT;
		}
	}
public:
	RegisterFormatter(CString _template)
	: templ(_template) {
	}
	
	String make(int _index) {
		StringBuffer buf;
		index = _index;
		templ.reset();
		format(templ, buf.stream());
		return buf.toString();
	}
};

/**
 * @class Register
 * @ingroup hard
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
 * Build a new register.
 * @param name		Register name.
 * @param number	Register number.
 * @param bank		Owner bank.
 */
Register::Register(const elm::String& name, Register::kind_t kind,
int size):
	_number(-1),
	_kind(kind),
	_size(size),
	_name(name),
	_bank(0),
	pfnum(-1)
{
	ASSERT(kind != NONE);
	ASSERT(size > 0);
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
 * @fn elm::String& Register::name(void) const;
 * Get the name of the register.
 * @return Register name.
 */


/**
 * @fn kind_t Register::kind(void) const;
 * Get the kind of a register.
 */


/**
 * @class RegBank
 * @ingroup hard
 * This class represents a bank of registers.
 */


/**
 * Buil a new register bank.
 * @param name		Name of the bank.
 * @param kind		Kind of register (may be NONE for melted bank).
 * @param size		Size in bits of the register (may be -1 for melted bank).
 */
RegBank::RegBank(CString name, Register::kind_t kind, int size)
: _name(name), _kind(kind), _size(size) {
}


/**
 * Buil a new register bank.
 * @param name		Name of the bank.
 * @param kind		Kind of register (may be NONE for melted bank).
 * @param size		Size in bits of the register (may be -1 for melted bank).
 * @param count		Count of registers.
 */
RegBank::RegBank(CString name, Register::kind_t kind, int size, int count)
: _name(name), _kind(kind), _size(size), _regs(count) {
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


/**
 * @class PlainBank
 * @ingroup hard
 * A plain bank is a register bank whose registers have the same size and the
 * same type. It represents the usual integer or floating-point banks.
 */


/**
 * Buila new plain bank.
 * @param name		Name of the bank.
 * @param kind		Kind of registers in the bank.
 * @param size		Size in bits of registers in the bank.
 * @param pattern	Pattern for naming registers in the bank : %d for
 * 					decimal numbering, %a for lower-case alphabetic numbering,
 * 					%A for upper-case alphabetic numbering.
 * @param count		Count of registers.
 */
PlainBank::PlainBank(elm::CString name, Register::kind_t kind, int size,
elm::CString pattern, int count)
: RegBank(name, kind, size, count) {
	RegisterFormatter format(pattern);
	for(int i = 0; i < count; i++)
		set(i, new Register(format.make(i), kind, size));
}


/**
 */
PlainBank::~PlainBank(void) {
	for(int i = 0; i < _regs.count(); i++)
		delete _regs[i];
}


/**
 * @class MeltedBank
 * @ingroup hard
 * A melted bank may contains registers with different sizes and kinds.
 * It is useful for grouping state registers.
 */


/**
 * Build a melted bank with the registers pass in the variable list arguments
 * (ended by null).
 * @param name	Name of the bank.
 * @param ...	List of registers in the bank.
 */
MeltedBank::MeltedBank(elm::CString name, ...)
: RegBank(name, Register::NONE, 0, 0) {
	int cnt = 0;
	VARARG_BEGIN(args, name);
		while(args.next<Register *>())
			cnt++;
	VARARG_END
	_regs.allocate(cnt);
	VARARG_BEGIN(args, name);
		for(int i = 0; i < cnt; i++)
			set(i, args.next<Register *>());
	VARARG_END	
}


/*
 */
MeltedBank::~MeltedBank(void) {
	//delete [] (Register **)_regs.table();
}

/**
 * @fn int Register::platformNumber(void) const;
 * Gives a number which is unique for this
 * platform. Each register, in a platform, is associated with a unique
 * number ranging from 0 to Platform::regCount()-1. This number may be used as
 * index to table or to bit vectors matching the overall registers.
 * @return	Platform number of the register.
 */ 

} } // otawa::hard
