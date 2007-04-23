/*
 *	$Id$
 *	Copyright (c) 2006-07, IRIT UPS.
 *
 *	Segment class interface
 */
#ifndef OTAWA_PROG_SEGMENT_H
#define OTAWA_PROG_SEGMENT_H

#include <elm/PreIterator.h>
#include <elm/inhstruct/DLList.h>
#include <otawa/prog/ProgItem.h>
#include <otawa/prog/Symbol.h>

namespace otawa {

// Defined classes
class ProgItem;
class CodeItem;
class Data;

// Segment class
class Segment: public PropList {
public:
	const static int EXECUTABLE = 0x01;	/**< Segment is executable. */
	const static int WRITABLE = 0x02;	/**< Segment is writable. */

	// Constructor
	Segment(CString name, address_t address, size_t size, unsigned long flags);

	// Accessors
	inline CString name(void) const { return _name; }
	inline unsigned long flags(void) const { return _flags; }
	inline bool isExecutable(void) const { return _flags & EXECUTABLE; }
	inline bool isWritable(void) const { return _flags & WRITABLE; }
	inline address_t address(void) const { return _address; }
	inline size_t size(void) const { return _size; }
	inline address_t topAddress(void) const { return _address + _size; }
	ProgItem *findItemAt(address_t addr);
	Inst *findInstAt(address_t addr);

	// ItemIter class	
	class ItemIter: public PreIterator<ItemIter, ProgItem *> {
		ProgItem *cur;
	public:
		inline ItemIter(Segment *seg): cur((ProgItem *)seg->items.first())
			{ if(seg->items.isEmpty()) cur = 0; }
		inline ItemIter(const ItemIter& iter): cur(iter.cur) { }
		inline ProgItem *item(void) const { return cur; }
		inline bool ended(void) const { return cur == 0; }
		inline void next(void) { cur = cur->next(); }
	};

protected:
	virtual Inst *decode(address_t address);
	virtual ~Segment(void);
	void insert(ProgItem *item);

private:
	unsigned long _flags;
	CString _name;
	address_t _address;
	size_t _size;
	inhstruct::DLList items;
	ProgItem **map;
};

};	// namespace otawa

#endif		// OTAWA_PROG_SEGMENT_H
