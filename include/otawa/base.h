/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	base.h -- base definition for Otawa framework.
 */
#ifndef OTAWA_BASE_H
#define OTAWA_BASE_H

#include <stdarg.h>
#include <elm/io.h>
#include <elm/utility.h>
#include <elm/string.h>
#include <elm/util/VarArg.h>
#include <elm/util/strong_type.h>
#include <elm/util/AutoComparator.h>
#include <elm/util/HashKey.h>

namespace otawa {
using namespace elm;

// Base types
typedef unsigned char byte_t;
//STRONG_TYPE(address_t, unsigned long);
typedef unsigned long size_t;
typedef signed long offset_t;
typedef unsigned long mask_t;


// Address class
class Address: public AutoPartialComparator<Address> {
public:
	typedef unsigned long page_t;
	typedef unsigned long address_t;
	static Address null;

	// Constructors
	inline Address(void): pg(0), addr(0) { }
	inline Address(address_t address): pg(0), addr(address) { }
	inline Address(page_t page, address_t address)
		: pg(page), addr(address) { }
	inline Address(const Address& address)
		: pg(address.pg), addr(address.addr) { }

	// Accessors
	inline page_t page(void) const { return pg; }
	inline address_t address(void) const { return addr; }
	inline address_t operator*(void) const { return address(); }
	inline bool isNull(void) const { return !pg && !addr; }
	inline operator bool(void) const { return !isNull(); }

	// Assignment
	inline Address& operator=(const Address& address)
		{ pg = address.pg; addr = address.addr; return *this; }
	inline Address& operator=(address_t address)
		{ pg = 0; addr = address; return *this; }
	inline Address& operator+=(int offset)
		{ addr += offset; return *this; }
	inline Address& operator+=(size_t offset)
		{ addr += offset; return *this; }
	inline Address& operator-=(int offset)
		{ addr -= offset; return *this; }
	inline Address& operator-=(size_t offset)
		{ addr -= offset; return *this; }

	// Operations
	inline Address operator+(int offset) const
		{ return Address(pg, addr + offset); }
	inline Address operator+(size_t offset) const
		{ return Address(pg, addr + offset); }
	inline Address operator-(int offset) const
		{ return Address(pg, addr + offset); }
	inline Address operator-(size_t offset) const
		{ return Address(pg, addr + offset); }
	inline address_t operator-(const Address& address) const {
		ASSERT(pg == address.pg);
		return addr - address.addr;
	}

	// Comparisons
	inline bool equals(const Address& address)
		{ return pg == address.pg && addr == address.addr; }
	inline int compare(const Address& address) {
		ASSERT(pg == address.pg);
		return addr - address.addr;
	}
	
private:
	unsigned long pg;
	offset_t addr;
};
typedef Address address_t;


// Format
namespace fmt {
	inline elm::io::IntFormat address(address_t addr) {
		return elm::io::right(elm::io::width(8, elm::io::pad('0',
			elm::io::hex((int)addr.address()))));
	}
}


// Address display
inline elm::io::Output& operator<<(elm::io::Output& out, Address addr) {
	if(addr.page())
		out << addr.page() << ':';
	out << fmt::address(addr);
	return out;
}


// Exception class
class Exception: public elm::Exception {
	String msg;
protected:
	void build(elm::CString format, VarArg& args);
	inline void setMessage(elm::String message);
public:
	Exception(void);
	Exception(const String message);
	Exception(elm::CString format, elm::VarArg& args);
	Exception(elm::CString format, ...);
	virtual ~Exception(void);
	virtual String message(void) { return msg; };
};

// Inlines
inline void Exception::setMessage(elm::String message) {
	msg = message;
}

} // otawa

// Useful ELM predefinitions
namespace elm {
	namespace xom {
		class Element;
	} // xom
	
	template <>
	class HashKey<otawa::Address> {
	public:
		static inline unsigned long hash (const otawa::Address &key)
			{ return key.page() + key.address(); }
		static inline bool equals (const otawa::Address &key1, const otawa::Address &key2)
			{ return key1 == key2; }
	};
} // elm

#endif	// OTAWA_BASE_H
