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
#include <elm/string/Formatter.h>
#include <elm/io/BlockInStream.h>
#include <elm/util/VarArg.h>

using namespace elm;

namespace otawa { namespace hard {

// RegisterFormatter
class RegisterFormatter: private Formatter {
public:
	RegisterFormatter(string _template, int base = 0)
	: _s(_template), _templ(_template), _index(-1), _base(base) {
	}

	String make(int index) {
		StringBuffer buf;
		_index = index;
		_templ.reset();
		format(_templ, buf.stream());
		return buf.toString();
	}

protected:
	virtual int process(io::OutStream& out, char chr) {
		_out.setStream(out);
		switch(chr) {
		case 'd':
			_out << _index;
			return DONE;
		case 'a':
		case 'A':
			_out << (char)(chr + _index);
			return DONE;
		case 'D':
			_out << (_index + _base);
			return DONE;
		default:
			return REJECT;
		}
	}

private:
	string _s;
	io::BlockInStream _templ;
	io::Output _out;
	int _index, _base;
};

/**
 * @class Register
 * Objects of this class are simple machine register, more accurately
 * unbreakable atomic registers.
 *
 * The preferred way to declare a register is a the user of maker class
 * the provides the most flexiblity:
 * @code
 * #include <otawa/hard/Register.h
 * using namespace otawa;
 *
 * hard::Register PC(hard::Register::Make("PC").alias("R15").kind(hard::Register::ADDR));
 * @endcode
 *
 * There is two ways to declare a register bank :
 * @li the register are automatically generated from the bank description,
 * @li the register are defined separately and passed to the
 * bank constructor.
 *
 * @ingroup hard
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
Register::Register(string name, Register::kind_t kind,
int size):
	_number(-1),
	_kind(kind),
	_size(size),
	_name(name),
	_bank(0),
	_id(-1)
{
	ASSERT(kind != NONE);
	ASSERT(size > 0);
}

/**
 * Build a register based on a maker class. This allows to
 * define more shorly a register or to provide more detail
 * as alias names.
 */
Register::Register(const Make& make):
	_number(-1),
	_kind(make._kind),
	_size(make._size),
	_name(make._name),
	_bank(0),
	_id(-1)
{
	ASSERT(_kind != NONE);
	ASSERT(_size > 0);
	_aliases = make._aliases;
}

/**
 * @fn const List<string>& aliases(void);
 * Get the list of alias names for the current register.
 * @return	List of alias names.
 */

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
 * @fn int Register::id(void) const;
 * Gives a number which is unique for this
 * platform. Each register, in a platform, is associated with a unique
 * number ranging from 0 to Platform::regCount()-1. This number may be used as
 * index to table or to bit vectors matching the overall registers.
 * @return	Platform number of the register.
 */


/**
 * @fn int Register::platformNumber(void) const;
 * @deprecated	Use id() instead.
 */


/**
 * @class RegBank::Make
 * A maker class used to initialize a register bank (@ref RegBank).
 *
 * This object must be used as below:
 * @code
 * #include <otawa/hard/Register.h>
 * using namespace otawa;
 *
 * hard::Register PC(hard::Register::Make("PC").init1(val1).init2(val2)...);
 * @endcode
 *
 * After the Make construction, as many calls to Make functions can be
 * performed to build the maker passed to the hard::Register constructor.
 *
 * @ingroup hard
 */

/**
 * Generates several registers in one short. To produce logical unique names,
 * the name and aliases of the passed register maker can contain escape sequences:
 * * %d for decimal numbering,
 * * %a for lower-case alphabetic numbering,
 * * %A for upper-case alphabetic numbering,
 * * %D for absolute numbering (relative to the full register bank).
 *
 * @param count		Number of register to generate.
 * @param pattern	Pattern for naming registers in the bank :
 * @param make		Description of the register.
 */
RegBank::Make& RegBank::Make::gen(int count, const Register::Make& make) {
	int base = _regs.count();
	RegisterFormatter fname(make.getName(), base);
	for(int i = 0; i < count; i++) {
		Register::Make m(fname.make(i));
		m.kind(make.getKind()).size(make.getSize());
		for(auto a = *make.getAliases(); a(); a++) {
			RegisterFormatter f(*a, base);
			m.alias(f.make(i));
		}
		Register *r = new Register(m);
		_regs.add(r);
		_alloc.add(r);
	}
	return *this;
}


/**
 * @class RegBank
 * This class represents a bank of registers. The preferred way
 * to build a register bank is to either use a derived class,
 * or to use a maker class.
 *
 * @code
 * #include <otawa/hard/Register.h>
 * using namespace otawa;
 *
 * hard::Register SP(hard::Register::Make("SP"));
 * hard::Register PC(hard::Register::Make("PC").kind(hard::Register::ADDR));
 * hard::Register SR(hard::Register::Make("SR").kind(hard::Register::BITS));
 * hard::RegBank MISC(hard::RegBank::Make("MISC")
 * 		.add(SP)
 * 		.add(PC)
 * 		.add(SR));
 * @endcode
 *
 * @ingroup hard
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
 * Build a register using a maker class. The registers are number
 * in the order they have been passed to the maker instance
 * (function Make::add()).
 *
 * @param make	Maker object to initialize the register bank.
 */
RegBank::RegBank(const Make& make) {
	init(make);
}


/**
 * Default empty constructor.
 */
RegBank::RegBank(void):
	_kind(Register::INT),
	_size(32)
{
}


/**
 */
RegBank::~RegBank(void) {
	for(auto r = *_alloc; r(); r++)
		delete *r;
}


/**
 * Initialize the register bank.
 */
void RegBank::init(const Make& make) {

	// init basic attributes
	_name = make._name;
	_kind = make._kind;
	_size = make._size;
	_alloc = make._alloc;

	// init register list
	_regs = AllocArray<Register *>(make._regs.count());
	int i = _regs.count() - 1;
	for(auto r = *make._regs; r(); r++, i--) {
		_regs[i] = *r;
		r->_bank = this;
		set(i, *r);
	}
}


/**
 * @fn cstring RegBank::name(void) const;
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
 * @fn const AllocArray<Register *>& RegBank::registers(void) const;
 * Get the list of registers in the bank.
 * @return	List of registers.
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
PlainBank::PlainBank(cstring name, Register::kind_t kind, int size, cstring pattern, int count) {
	Make m(name);
	m.kind(kind).size(size);
	RegisterFormatter format(pattern);
	for(int i = 0; i < count; i++)
		m.add(new Register(format.make(i), kind, size));
	init(m);
}


/**
 */
PlainBank::~PlainBank(void) {
	for(int i = 0; i < _regs.count(); i++)
		delete _regs[i];
}


/**
 * @class MeltedBank
 * A melted bank may contains registers with different sizes and kinds.
 * It is useful for grouping state registers.
 *
 * @ingroup hard
 * @deprecated	Redundant with new RegBank construction with maker.
 */


/**
 * Build a melted bank with the registers passed in the variable list arguments
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
	_regs = AllocArray<Register *>(cnt);
	VARARG_BEGIN(args, name);
		for(int i = 0; i < cnt; i++)
			set(i, args.next<Register *>());
	VARARG_END	
}


} } // otawa::hard
