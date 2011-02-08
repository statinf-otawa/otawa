/*
 *	$Id$
 *	Base declaration interface.
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
#include <elm/types.h>
#include <elm/type_info.h>

// Base types
namespace elm { namespace t {
typedef uint8 byte;
typedef uint32 size;
typedef uint32 mask;
} }

namespace otawa {
using namespace elm;

// Address class
class Address {
public:
	typedef t::size page_t;
	typedef t::size offset_t;
	static Address null;
	static const page_t null_page = elm::type_info<page_t>::max;

	// Constructors
	inline Address(void): pg(null_page), off(0) { }
	inline Address(offset_t offset): pg(0), off(offset) { }
	inline Address(page_t page, offset_t offset)
		: pg(page), off(offset) { }
	inline Address(const Address& address)
		: pg(address.pg), off(address.off) { }

	// Accessors
	inline page_t page(void) const { return pg; }
	inline offset_t offset(void) const { return off; }
	inline offset_t operator*(void) const { return offset(); }
	inline bool isNull(void) const { return pg == null_page; }
	inline operator offset_t(void) const { return offset(); }

	// Assignment
	inline Address& operator=(const Address& address)
		{ pg = address.pg; off = address.off; return *this; }
	inline Address& operator=(offset_t offset)
		{ pg = 0; off = offset; return *this; }
	inline Address& operator+=(int offset)
		{ ASSERT(!offset || !isNull()); off += offset; return *this; }
	inline Address& operator+=(t::size offset)
		{ ASSERT(!offset || !isNull()); off += offset; return *this; }
	inline Address& operator-=(int offset)
		{ ASSERT(!offset || !isNull()); off -= offset; return *this; }
	inline Address& operator-=(t::size offset)
		{ ASSERT(!offset || !isNull()); off -= offset; return *this; }

	// Operations
	inline Address operator+(t::int32 offset) const
		{ ASSERT(!offset || !isNull()); return Address(pg, off + offset); }
	inline Address operator+(t::uint32 offset) const
		{ ASSERT(!offset || !isNull()); return Address(pg, off + offset); }
	inline Address operator-(t::int32 offset) const
		{ ASSERT(!offset || !isNull()); return Address(pg, off - offset); }
	inline Address operator-(t::uint32 offset) const
		{ ASSERT(!offset || !isNull()); return Address(pg, off - offset); }
	inline offset_t operator-(const Address& address) const {
		ASSERT(!isNull() && !address.isNull());
		ASSERT(pg == address.pg);
		return off - address.off;
	}

	// Comparisons
	inline bool equals(const Address& address) const
		{ return pg == address.pg && off == address.off; }
	inline int compare(const Address& address) const {
		ASSERT(pg == address.pg);
		if(off < address.off) return -1;
		else if(off > address.off) return 1;
		else return 0;
	}
	inline bool operator==(const Address& addr) const { return equals(addr); }
	inline bool operator!=(const Address& addr) const { return !equals(addr); }
	inline bool operator<(const Address& addr) const { ASSERT(pg == addr.pg); return off < addr.off; }
	inline bool operator<=(const Address& addr) const { ASSERT(pg == addr.pg); return off <= addr.off; }
	inline bool operator>(const Address& addr) const { ASSERT(pg == addr.pg); return off > addr.off; }
	inline bool operator>=(const Address& addr) const { ASSERT(pg == addr.pg); return off >= addr.off; }


	// Deprecated
	inline offset_t address(void) const { return off; }

private:
	page_t pg;
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
elm::io::Output& operator<<(elm::io::Output& out, Address addr);


// Exception class
class Exception: public elm::MessageException {
public:
	Exception(void);
	Exception(const String& message);
};

// time measurement
typedef signed long long time_t;

} // otawa

// Useful ELM predefinitions
namespace elm {
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
