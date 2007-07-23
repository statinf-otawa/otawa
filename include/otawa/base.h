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
	typedef unsigned long offset_t;
	static Address null;

	// Constructors
	inline Address(void): pg(0), off(0) { }
	inline Address(offset_t offset): pg(0), off(offset) { }
	inline Address(page_t page, offset_t offset)
		: pg(page), off(offset) { }
	inline Address(const Address& address)
		: pg(address.pg), off(address.off) { }

	// Accessors
	inline page_t page(void) const { return pg; }
	inline offset_t offset(void) const { return off; }
	inline offset_t operator*(void) const { return offset(); }
	inline bool isNull(void) const { return this == &null; }
	inline operator offset_t(void) const { return offset(); }

	// Assignment
	inline Address& operator=(const Address& address)
		{ pg = address.pg; off = address.off; return *this; }
	inline Address& operator=(offset_t offset)
		{ pg = 0; off = offset; return *this; }
	inline Address& operator+=(int offset)
		{ off += offset; return *this; }
	inline Address& operator+=(size_t offset)
		{ off += offset; return *this; }
	inline Address& operator-=(int offset)
		{ off -= offset; return *this; }
	inline Address& operator-=(size_t offset)
		{ off -= offset; return *this; }

	// Operations
	inline Address operator+(int offset) const
		{ return Address(pg, off + offset); }
	inline Address operator+(size_t offset) const
		{ return Address(pg, off + offset); }
	inline Address operator-(int offset) const
		{ return Address(pg, off + offset); }
	inline Address operator-(size_t offset) const
		{ return Address(pg, off + offset); }
	inline offset_t operator-(const Address& address) const {
		ASSERT(pg == address.pg);
		return off - address.off;
	}

	// Comparisons
	inline bool equals(const Address& address)
		{ return pg == address.pg && off == address.off; }
	inline int compare(const Address& address) {
		ASSERT(pg == address.pg);
		return off - address.off;
	}

	// Deprecated
	inline offset_t address(void) const { return off; }
	
private:
	unsigned long pg;
	offset_t off;
};
typedef Address address_t;


// Format
namespace fmt {
	inline elm::io::IntFormat address(address_t addr) {
		return elm::io::right(elm::io::width(8, elm::io::pad('0',
			elm::io::hex((int)addr.offset()))));
	}
}


// Address display
inline elm::io::Output& operator<<(elm::io::Output& out, Address addr) {
	if(addr.page())
		out << addr.page() << ':';
	out << fmt::address(addr.offset());
	return out;
}


// Exception class
class Exception: public elm::MessageException {
public:
	Exception(void);
	Exception(const String& message);
};

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
			{ return key.page() + key.offset(); }
		static inline bool equals (const otawa::Address &key1, const otawa::Address &key2)
			{ return key1 == key2; }
	};
} // elm

#endif	// OTAWA_BASE_H
