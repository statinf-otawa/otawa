/*
 *	$Id$
 *	Register and RegBank classes interface
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
#ifndef OTAWA_HARD_REGISTER_H
#define OTAWA_HARD_REGISTER_H

#include <assert.h>
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
	inline int number(void) const;
	inline RegBank *bank(void) const;
	inline elm::String& name(void);
	inline kind_t kind(void) const;
	inline int size(void) const;
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
protected:
	elm::CString _name;
	Register::kind_t _kind;
	int _size;
	elm::genstruct::Table<Register *> _regs;
	RegBank(elm::CString name, Register::kind_t kind, int size, int count, 
		Register **regs);
	inline ~RegBank(void) { };
	inline void set(int index, Register *reg);
public:
	inline elm::CString name(void) const;
	inline Register::kind_t kind(void) const;
	inline int size(void) const;
	inline int count(void) const;
	inline Register *get(int index) const;
	inline Register *operator[](int index) const;
	inline const elm::genstruct::Table<Register *>& registers() const;
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


// Register inlines
inline int Register::number(void) const {
	return _number;
}

inline RegBank *Register::bank(void) const {
	return _bank;
}

inline Register::kind_t Register::kind(void) const {
	return _kind;
}

inline int Register::size(void) const {
	return _size;
}

inline elm::String& Register::name(void) {
	return _name;
}

inline elm::io::Output& operator<<(elm::io::Output& out, Register *reg) {
	out << reg->name();
	return out;
}


// RegBank inlines
inline elm::CString RegBank::name(void) const {
	return _name;
}

inline Register::kind_t RegBank::kind(void) const {
	return _kind;
}

inline int RegBank::size(void) const {
	return _size;
}

inline int RegBank::count(void) const {
	return _regs.count();
}

inline Register *RegBank::get(int index) const {
	assert(index < _regs.count());
	return _regs[index];
}

inline Register *RegBank::operator[](int index) const {
	return get(index);
}

inline const elm::genstruct::Table<Register *>& RegBank::registers() const {
	return _regs;
}

inline void RegBank::set(int index, Register *reg) {
	assert(index < _regs.count());
	reg->_number = index;
	_regs[index] = reg;
	reg->_bank = this;
}

} } // otawa::hard

#endif // OTAWA_HARD_REGISTER_H
