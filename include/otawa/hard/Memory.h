/*
 *	$Id$
 *	Platform class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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
#ifndef OTAWA_HARD_MEMORY_H
#define OTAWA_HARD_MEMORY_H

#include <elm/assert.h>
#include <elm/data/HashMap.h>
#include <elm/data/Array.h>
#include <elm/string.h>
#include <elm/serial2/collections.h>
#include <elm/serial2/macros.h>
#include <elm/sys/Path.h>

#include <otawa/hard/features.h>
#include <otawa/prog/Manager.h>

namespace elm { namespace xom { class Element; } }

namespace otawa { namespace hard {

using namespace elm;

// ModeTransition class
class Mode;
class ModeTransition {
	SERIALIZABLE(ModeTransition,
		field("latency", _latency) & field("power", _power) & field("dest", _dest));
public:
	inline ModeTransition(void): _latency(1), _power(0), _dest(0) { }
	virtual ~ModeTransition(void);
	inline int latency(void) const { return _latency; }
	inline int power(void) const { return _power; }
	inline const Mode *dest(void) const { return _dest; }

private:
	int _latency;
	int _power;
	const Mode *_dest;
};


// Mode class
class Mode {
	SERIALIZABLE(Mode,
		field("name", _name)
		& field("latency", _latency)
		& field("static_power", _static_power)
		& field("dynamic_power", _dynamic_power)
		& field("transitions", _transitions));
public:
	inline Mode(void): _name("no name"), _latency(1), _static_power(0), _dynamic_power(0) { }
	virtual ~Mode(void);
	inline const string& name(void) const { return _name; }
	inline int latency(void) const { return _latency; }
	inline int staticPower(void) const { return _static_power; }
	inline int dynamicPower(void) const { return _dynamic_power; }
	inline const Array<ModeTransition>& transitions(void) const { return _transitions; }

private:
	string _name;
	int _latency;
	int _static_power, _dynamic_power;
	AllocArray<ModeTransition> _transitions;
};

// Bank class
class Bus;
class Bank {
public:
	typedef enum type_t {
		NONE = 0,
		DRAM = 1,
		SPM = 2,
		ROM = 3,
		IO = 4
	} type_t;
private:
	SERIALIZABLE(Bank,
		field("name", _name) &
		field("address", _address) &
		field("size", _size) &
		field("type", _type) &
		field("latency", _latency) &
		field("power", _power) &
		field("block_bits", _block_bits) &
		field("modes", _modes) &
		field("cached", _cached) &
		field("on_chip", _on_chip) &
		field("writable", _writable) &
		field("port_num", _port_num) &
		field("bus", _bus) &
		field("write_latency", _write_latency));
public:
	static Bank full;
	Bank(void);
	Bank(cstring name, Address address, size_t size);
	virtual ~Bank(void);

	inline const string& name() const { return _name; }
	inline const Address& address(void) const { return _address; }
	inline const int size() const { return _size; }
	inline MemArea area() const { return MemArea(address(), size()); }
	inline type_t type() const { return _type; }
	inline ot::time readLatency() const { return _latency; }
	inline ot::time writeLatency() const { if(!_write_latency) return _latency; else return _write_latency; }
	inline int power() const { return _power; }
	inline int blockBits() const { return _block_bits; }
	inline int blockSize() const { return 1 << _block_bits; }
	inline const Array<const Mode *>& modes(void) const { return _modes; }
	inline bool isCached() const { return _cached; }
	inline bool isOnChip() const { return _on_chip; }
	inline bool isWritable() const { return _writable; }
	inline int portNum() const { return _port_num; }
	inline const Bus *bus(void) const { return _bus; }

	inline Address topAddress(void) const { return address() + size(); }
	inline bool contains(Address addr) const
		{ return addr.page() == address().page() && addr >= address() && addr <= (topAddress() - 1); }

	// deprecated
	inline ot::time latency(void) const { return _latency; }

private:
	string _name;
	Address _address;
	int _size;
	type_t _type;
	ot::time _latency, _power, _write_latency;
	int _block_bits;
	AllocArray<const Mode *> _modes;
	bool _cached;
	bool _on_chip;
	bool _writable;
	int _port_num;
	const Bus *_bus;
};


// Bus class
class Bus {
public:
	typedef enum type_t {
		LOCAL = 0,
		SHARED = 1
	} type_t;
private:
	SERIALIZABLE(Bus, field("name", _name) & field("type", _type));
public:
	inline Bus(void): _name("no name"), _type(LOCAL) { }
	virtual ~Bus(void);
	inline const string& name(void) const { return _name; }
	inline type_t type(void) const { return _type; }

private:
	string _name;
	type_t _type;
};


// Memory class
class Memory {
private:
	SERIALIZABLE(Memory, field("banks", _banks) & field("buses", _buses));
public:
	static const Memory null, full;
	Memory(bool full = false);
	virtual ~Memory(void);

	inline const AllocArray<const Bank *>& banks(void) const { return _banks; }
	inline const AllocArray<const Bus *>& buses(void) const  { return _buses; }
	static Memory *load(const elm::sys::Path& path);
	static Memory *load(xom::Element *element);
	const Bank *get(Address address) const;

	ot::time worstReadTime(void) const;
	ot::time worstWriteTime(void) const;
	ot::time worstAccessTime(void) const;

	ot::time bestReadTime(void) const;
	ot::time bestWriteTime(void) const;
	ot::time bestAccessTime(void) const;

	ot::time readTime(Address a) const;
	ot::time writeTime(Address a) const;
	ot::time accessTime(Address a) const;

	// deprecated
	inline ot::time worstAccess(void) const { return worstAccessTime(); }
	inline ot::time worstReadAccess(void) const { return worstReadTime(); }
	inline ot::time worstWriteAccess(void) const { return worstWriteTime(); }

private:
	AllocArray<const Bank *> _banks;
	AllocArray<const Bus *> _buses;
	mutable ot::time _waccess, _wread, _wwrite;
	mutable ot::time _baccess, _bread, _bwrite;
};

// features
//extern p::feature MEMORY_FEATURE;
//extern Identifier<const Memory *> MEMORY;

} // hard

io::Output &operator <<(io::Output &o, const hard::Bank::type_t &t);

} // otawa

ENUM(otawa::hard::Bus::type_t);
ENUM(otawa::hard::Bank::type_t);
namespace elm { namespace serial2 {
	void __serialize(elm::serial2::Serializer &serializer, const otawa::Address& address);
	void __unserialize(elm::serial2::Unserializer &serializer, otawa::Address& address);
}}

#endif // OTAWA_HARD_MEMORY_H_
