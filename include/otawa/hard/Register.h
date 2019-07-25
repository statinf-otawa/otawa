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
#include <elm/data/Array.h>
#include <elm/data/Array.h>
#include <elm/data/List.h>
#include <elm/io.h>

namespace otawa { namespace hard {
	
using namespace elm;

// Pre-declaration of classes
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

	class Make {
		friend Register;
	public:
		inline Make(string name): _name(name), _kind(INT), _size(32) { }
		inline Make(string name, const Make& m): _name(name), _kind(m._kind), _size(m._size) { }
		inline Make& kind(kind_t k) { _kind = k; return *this; }
		inline Make& size(int s) { _size = s; return *this; }
		inline Make& alias(string n) { _aliases.add(n); return *this; }

		inline string getName(void) const { return _name; }
		inline kind_t getKind(void) const { return _kind; }
		inline int getSize(void) const { return _size; }
		inline const List<string>& getAliases(void) const { return _aliases; }

	private:
		string _name;
		kind_t _kind;
		int _size;
		List<string> _aliases;
	};

	Register(string name, kind_t kind, int size);
	Register(const Make& make);
	inline int number(void) const { return _number; }
	inline RegBank *bank(void) const { return _bank; }
	inline const string& name(void) const { return _name; }
	inline kind_t kind(void) const { return _kind; }
	inline int size(void) const { return _size; }
	inline const List<string>& aliases(void) { return _aliases; }
	inline int id(void) const { return _id; }

	// deprecated
	inline int platformNumber(void) const { return id(); }

private:
	friend class Platform;
	friend class RegBank;
	int _number;
	kind_t _kind;
	int _size;
	string _name;
	RegBank *_bank;
	int _id;
	List<string> _aliases;
};


// RegBank class
class RegBank {
	friend class Register;
public:

	class Make {
		friend class RegBank;
	public:
		inline Make(cstring name): _name(name), _kind(Register::INT), _size(32) { }
		inline Make& kind(Register::kind_t kind) { _kind = kind; return *this; }
		inline Make& size(int size) { _size = size; return *this; }
		inline Make& add(hard::Register& reg) { _regs.add(&reg); return *this; }
		inline Make& add(hard::Register *reg) { _regs.add(reg); return *this; }
		Make& gen(int count, const Register::Make& make);
	private:
		cstring _name;
		Register::kind_t _kind;
		int _size;
		List<Register *> _regs;
		List<Register *> _alloc;
	};

	RegBank(const Make& make);
	~RegBank(void);
	inline cstring name(void) const { return _name; }
	inline Register::kind_t kind(void) const { return _kind; }
	inline int size(void) const { return _size; }
	inline int count(void) const { return _regs.count(); }
	inline Register *get(int index) const { ASSERT(0 <= index && index < _regs.count()); return _regs[index]; }
	inline Register *operator[](int index) const { return get(index); }
	inline const AllocArray<Register *>& registers(void) const { return _regs; }

protected:
	cstring _name;
	Register::kind_t _kind;
	int _size;
	AllocArray<Register *> _regs;
	List<Register *> _alloc;

	// deprecated
	RegBank(void);
	void init(const Make& make);
	RegBank(cstring name, Register::kind_t kind, int size);
	RegBank(cstring name, Register::kind_t kind, int size, int count);
	inline void set(int index, Register *reg)
		{ ASSERT(index < _regs.count()); reg->_number = index; _regs[index] = reg; reg->_bank = this; }
};


// PlainBank class
class PlainBank: public RegBank {
public:
	PlainBank(cstring name, Register::kind_t kind, int size, cstring pattern, int count);
	~PlainBank(void);
};


// MeltedBank class
class MeltedBank: public RegBank {
public:
	MeltedBank(cstring name, ...);
};

inline elm::io::Output& operator<<(elm::io::Output& out, Register *reg) {
	out << reg->name();
	return out;
}

} } // otawa::hard

#endif // OTAWA_HARD_REGISTER_H
