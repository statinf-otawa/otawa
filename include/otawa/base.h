/*
 *	Base declaration interface.
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003, IRIT UPS.
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

#include <elm/io.h>
#include <elm/utility.h>
#include <elm/string.h>
#include <elm/util/VarArg.h>
#include <elm/util/strong_type.h>
#include <elm/hash.h>
#include <elm/types.h>
#include <elm/type_info.h>

// ELM definitions
namespace elm {
	namespace rtti { class Type; }
	namespace serial2 {
		class Serializer;
		class Unserializer;
	}
} // elm::serial2

namespace otawa {
using namespace elm;

namespace ot {
	typedef t::uint8 byte;
	typedef t::uint32 mask;
	typedef t::uint32 size;
	typedef t::int64 time;
}	// ot

// MemArea
class MemArea;

// Address class
class Address {
public:
	static rtti::Type& __type;

	typedef t::uint32 page_t;
	typedef t::uint32 offset_t;
	static Address null;
	static const page_t null_page = elm::type_info<page_t>::max;

	// Constructors
	inline Address(void): pg(null_page), off(0) { }
	inline Address(offset_t offset): pg(0), off(offset) { }
	inline Address(page_t page, offset_t offset): pg(page), off(offset) { }
	inline Address(const Address& address): pg(address.pg), off(address.off) { }
	inline Address roundDown(offset_t size) { return Address(pg, off & ~(size -1)); }
	inline Address roundUp(offset_t size) { return Address(pg, (off + size - 1) & ~(size -1)); }

	// Accessors
	inline page_t page(void) const { return pg; }
	inline offset_t offset(void) const { return off; }
	inline offset_t operator*(void) const { return offset(); }
	inline bool isNull(void) const { return pg == null_page; }
	inline Address mask(offset_t m) const { return Address(pg, off & ~m); }

	// Assignment
	inline Address& operator=(const Address& address) { pg = address.pg; off = address.off; return *this; }
	inline Address& operator=(offset_t offset) { pg = 0; off = offset; return *this; }
	inline Address& operator+=(int offset) { ASSERT(!offset || !isNull()); off += offset; return *this; }
	inline Address& operator+=(t::uint32 offset) { ASSERT(!offset || !isNull()); off += offset; return *this; }
	inline Address& operator-=(int offset) { ASSERT(!offset || !isNull()); off -= offset; return *this; }
	inline Address& operator-=(t::uint32 offset) { ASSERT(!offset || !isNull()); off -= offset; return *this; }

	// Operations
	inline Address operator+(t::int32 offset) const { ASSERT(!offset || !isNull()); return Address(pg, off + offset); }
	inline Address operator+(t::uint32 offset) const { ASSERT(!offset || !isNull()); return Address(pg, off + offset); }
	inline Address operator-(t::int32 offset) const { ASSERT(!offset || !isNull()); return Address(pg, off - offset); }
	inline Address operator-(t::uint32 offset) const { ASSERT(!offset || !isNull()); return Address(pg, off - offset); }
	inline offset_t operator-(const Address& address) const
		{ ASSERT(!isNull() && !address.isNull()); ASSERT(pg == address.pg); return off - address.off; }
	inline Address operator&(t::int32 offset) const { ASSERT(!offset || !isNull()); return Address(pg, off & offset); }
	inline Address operator&(t::uint32 offset) const { ASSERT(!offset || !isNull()); return Address(pg, off & offset); }

	// Comparisons
	inline bool equals(const Address& address) const { return pg == address.pg && off == address.off; }
	inline int compare(const Address& address) const
		{ ASSERT(pg == address.pg); if(off < address.off) return -1; else if(off > address.off) return 1; else return 0; }
	inline bool operator==(const Address& addr) const { return equals(addr); }
	inline bool operator!=(const Address& addr) const { return !equals(addr); }
	inline bool operator<(const Address& addr) const { ASSERT(pg == addr.pg); return off < addr.off; }
	inline bool operator<=(const Address& addr) const { ASSERT(pg == addr.pg); return off <= addr.off; }
	inline bool operator>(const Address& addr) const { ASSERT(pg == addr.pg); return off > addr.off; }
	inline bool operator>=(const Address& addr) const { ASSERT(pg == addr.pg); return off >= addr.off; }

private:
	page_t pg;
	offset_t off;
};

typedef Address address_t;
namespace ot { elm::io::IntFormat address(Address addr); }
elm::io::Output& operator<<(elm::io::Output& out, Address addr);


// MemArea class
class MemArea {
public:
	static MemArea null;

	inline MemArea(void): _size(0) { }
	inline MemArea(const MemArea& a): _base(a._base), _size(a._size) { }
	inline MemArea(const Address& base, ot::size size): _base(base), _size(size) { }
	inline MemArea(const Address& base, const Address& top): _base(base), _size(top - base) { }

	inline Address address(void) const { return _base; }
	inline ot::size size(void) const { return _size; }
	inline Address topAddress(void) const { return _base + _size; }
	inline Address lastAddress(void) const { return _base + (_size - 1); }

	inline bool isNull(void) const { return _base.isNull(); }
	inline bool isEmpty(void) const { return !_size; }
	inline bool contains(const Address& addr) const { return _base <= addr && addr <= lastAddress(); }

	inline bool equals(const MemArea& a) const { return _base == a._base && _size == a._size; }
	bool includes(const MemArea& a) const;
	bool meets(const MemArea& a) const;
	MemArea meet(const MemArea& a) const;
	MemArea join(const MemArea& a) const;

	inline void set(const MemArea& a) { _base = a._base; _size = a._size; }
	inline void move(const Address& base) { _base = base; }
	inline void moveTop(const Address& top) { _base = top - _size; }
	inline void resize(const ot::size size) { _size = size; }

	inline operator bool(void) const { return !isEmpty(); }
	inline bool operator==(const MemArea& a) const { return equals(a); }
	inline bool operator!=(const MemArea& a) const { return !equals(a); }
	inline bool operator>=(const MemArea& a) const { return includes(a); }
	inline bool operator>(const MemArea& a) const { return includes(a) && !equals(a); }
	inline bool operator<(const MemArea& a) const { return a.includes(*this); }
	inline bool operator<=(const MemArea& a) const { return a.includes(*this) && !equals(a); }

	inline MemArea operator&(const MemArea& a) const { return meet(a); }
	inline MemArea operator+(const MemArea& a) const { return join(a); }
	inline MemArea operator=(const MemArea& a) { set(a); return *this; }
	inline void operator&=(const MemArea& a) { set(meet(a)); }
	inline void operator+=(const MemArea& a) { set(join(a)); }

private:
	Address _base;
	ot::size _size;
};
io::Output& operator<<(io::Output& out, const MemArea& a);


// Exception class
class Exception: public elm::MessageException {
public:
	Exception(void);
	Exception(string message);
	Exception(elm::Exception& exn);
	Exception(string message, elm::Exception& exn);
};

} // otawa

// Useful ELM predefinitions
namespace elm {
	template <>
	class HashKey<otawa::Address> {
	public:
		inline unsigned long computeHash (const otawa::Address &key) const { return key.page() + key.offset(); }
		inline bool isEqual(const otawa::Address &key1, const otawa::Address &key2) const { return key1 == key2; }
	};

} // elm

#endif	// OTAWA_BASE_H
