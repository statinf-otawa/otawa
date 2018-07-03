/*
 *	otawa::Segment -- program segment class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-7, IRIT UPS.
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
#ifndef OTAWA_PROG_SEGMENT_H
#define OTAWA_PROG_SEGMENT_H

#include <elm/types.h>
#include <elm/PreIterator.h>
#include <elm/inhstruct/DLList.h>
#include <otawa/prog/ProgItem.h>
#include <otawa/prog/Symbol.h>

namespace otawa {

using namespace elm;

// Defined classes
class ProgItem;
class CodeItem;
class Data;

// Segment class
class Segment: public PropList {
public:
	static rtti::Type& __type;

	typedef t::uint32 flags_t;
	const static flags_t EXECUTABLE = 0x01;
	const static flags_t WRITABLE = 0x02;
	const static flags_t INITIALIZED = 0x04;

	// Constructor
	Segment(CString name, address_t address, ot::size size, flags_t flags);

	// Accessors
	inline CString name(void) const { return _name; }
	inline flags_t flags(void) const { return _flags; }
	inline bool isExecutable(void) const { return _flags & EXECUTABLE; }
	inline bool isWritable(void) const { return _flags & WRITABLE; }
	inline bool isInitialized(void) const { return _flags & INITIALIZED; }
	inline address_t address(void) const { return _address; }
	inline ot::size size(void) const { return _size; }
	inline address_t topAddress(void) const { return _address + t::uint32(_size); }
	ProgItem *findItemAt(const Address& addr);
	Inst *findInstAt(const Address& addr);
	inline bool contains(const Address& addr) const { return address() <= addr && addr < topAddress(); }

	// ItemIter class	
	class ItemIter: public PreIterator<ItemIter, ProgItem *> {
	public:
		inline ItemIter(void): cur(nullptr) { }
		inline ItemIter(const Segment *seg): cur((ProgItem *)seg->_items.first())
			{ if(seg->_items.isEmpty()) cur = 0; }
		inline ProgItem *item(void) const { return cur; }
		inline bool ended(void) const { return cur == nullptr; }
		inline void next(void) { cur = cur->next(); }
		inline bool equals(const ItemIter& i) { return cur == i.cur; }
	private:
		ProgItem *cur;
	};

	// ItemRange class
	class ItemRange {
	public:
		inline ItemRange(const Segment *seg): _seg(seg) { }
		inline ItemIter begin(void) const { return ItemIter(_seg); }
		inline ItemIter end(void) const { return ItemIter(); }
	private:
		const Segment *_seg;
	};
	inline ItemRange items(void) const { return ItemRange(this); }

protected:
	friend class File;
	virtual Inst *decode(address_t address);
	virtual ~Segment(void);
	void insert(ProgItem *item);

private:
	flags_t _flags;
	CString _name;
	Address _address;
	ot::size _size;
	inhstruct::DLList _items;
	ProgItem **map;
};

};	// namespace otawa

#endif		// OTAWA_PROG_SEGMENT_H
