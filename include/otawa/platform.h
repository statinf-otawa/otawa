/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	platform.h -- platform description classes.
 */
#ifndef OTAWA_PLATFORM_H
#define OTAWA_PLATFORM_H

#include <elm/string.h>

namespace otawa {
using namespace elm;

// Classes
class Register;
class RegBank;
class RegSet;

// RegBank class
class Type;
class RegBank {
public:
	virtual CString name(void) = 0;
	virtual int bits(void) = 0;
	virtual int count(void) = 0;
	virtual RegSet unalias(Register reg) = 0;
	virtual Type *type(void) = 0;
};

// RegSet class
class RegSet {
	RegBank *bnk;
	int _base, _size;
public:
	inline RegSet(RegBank *bank, int base, int size) : bnk(bank), _base(base), _size(size) { };
	inline RegSet(const RegSet& set): bnk(set.bnk), _base(set._base), _size(set._size) { };
	inline RegSet& operator=(const RegSet& set)
		{ bnk = set.bnk; _base = set._base; _size = set._size; };
	inline RegBank *bank(void) const { return bnk; };
	inline int base(void) const { return _base; };
	inline int size(void) const { return _size; };
	inline bool equals(const RegSet& set) const
		{ return bnk == set.bnk && _base == set._base && _size == set._size; };
	inline bool operator==(const RegSet& set) const { return equals(set); };
	inline bool operator!=(const RegSet& set) const { return !equals(set); };
	inline bool override(const RegSet& set) const
		{ return bnk == set.bnk
				&& ((_base < set._base && _base + _size >= set._base)
				|| _base <= set._base + set._size); };
};

// Register class
class Register {
	RegBank *bnk;
	int num;
public:
	inline Register(RegBank *bank, int number): bnk(bank), num(number) { };
	inline Register(const Register& reg): bnk(reg.bnk), num(reg.num) { };
	inline Register& operator=(const Register& reg)
		{ bnk = reg.bnk; num = reg.num; return *this; };
	inline RegBank *bank(void) const { return bnk; };
	inline int number(void) const { return num; };
	inline bool equals(const Register& reg)
		{ return (bnk->unalias(*this)).equals(reg.bnk->unalias(*this)); };
	inline bool operator==(const Register& reg) { return equals(reg); };
	inline bool operator!=(const Register& reg) { return !equals(reg); };
	inline bool override(const Register& reg)
		{ return (bnk->unalias(*this)).override(reg.bnk->unalias(*this)); };		
};

// Platform identifier
class PlatformId {
	String name;
	String arch;
	String abi;
	String mach;
	void split(void);
public:
	PlatformId(CString _name);
	PlatformId(const String& _name);
	PlatformId(CString arch, CString abi, CString os);
	inline const String& getArch(void) const { return arch; };
	inline const String& getABI(void) const { return abi; };
	inline const String& getMach(void) const { return mach; };
};


// Platform class
class Manager;
class Platform {
	friend class Manager;
	virtual ~Platform(void) = 0;
public:
	virtual bool accept(const PlatformId& id) = 0;
	inline bool accept(CString name) { return accept(PlatformId(name)); };
	inline bool accept(const String& name) { return accept(PlatformId(name)); };
};


}; // namespace otawa


#endif	// OTAWA_PLATFORM_H
