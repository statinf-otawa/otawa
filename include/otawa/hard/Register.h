/*
 *	$Id$
 *	Register and RegBank classes interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-08, IRIT UPS.
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
#ifndef OTAWA_HARD_REGISTER_H
#define OTAWA_HARD_REGISTER_H

#include <elm/assert.h>
#include <elm/string.h>
#include <elm/genstruct/Table.h>
#include <elm/io.h>

namespace otawa { namespace hard {
	
// Predeclaration of classes
class Platform;
class Register;
class RegBank;

// Register class
class Register {
public:
	typedef enum kind_t {
		NONE = 0,
		ADDR,
		INT,
		FLOAT,
		BITS
	} kind_t;

	Register(const elm::String& name, kind_t kind, int size);
	inline int number(void) const { return _number; }
	inline RegBank *bank(void) const { return _bank; }
	inline const elm::String& name(void) const { return _name; }
	inline kind_t kind(void) const { return _kind; }
	inline int size(void) const { return _size; }
	inline int platformNumber(void) const { return pfnum; }

private:
	friend class Platform;
	friend class RegBank;
	int _number;
	kind_t _kind;
	int _size;
	elm::String _name;
	RegBank *_bank;
	int pfnum;
};

// RegBank class
class RegBank {
	friend class Register;
public:
	inline elm::CString name(void) const { return _name; }
	inline Register::kind_t kind(void) const { return _kind; }
	inline int size(void) const { return _size; }
	inline int count(void) const { return _regs.count(); }
	inline Register *get(int index) const { ASSERT(index < _regs.count()); return _regs[index]; }
	inline Register *operator[](int index) const { return get(index); }
	inline const elm::genstruct::Table<Register *>& registers() const { return _regs; }
protected:
	elm::CString _name;
	Register::kind_t _kind;
	int _size;
	elm::genstruct::AllocatedTable<Register *> _regs;
	RegBank(elm::CString name, Register::kind_t kind, int size);
	RegBank(elm::CString name, Register::kind_t kind, int size, int count);
	inline ~RegBank(void) { };
	inline void set(int index, Register *reg)
		{ ASSERT(index < _regs.count()); reg->_number = index; _regs[index] = reg; reg->_bank = this; }
};


// PlainBank class
class PlainBank: public RegBank {
public:
	PlainBank(elm::CString name, Register::kind_t kind, int size,
		elm::CString pattern, int count);
	~PlainBank(void);
};


// MeltedBank class
class MeltedBank: public RegBank {
public:
	MeltedBank(elm::CString name, ...);
	~MeltedBank(void);
};

inline elm::io::Output& operator<<(elm::io::Output& out, Register *reg) {
	out << reg->name();
	return out;
}


} } // otawa::hard

#endif // OTAWA_HARD_REGISTER_H
