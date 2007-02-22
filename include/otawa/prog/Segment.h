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

	// Accessors
	inline CString name(void) const { return _name; }
	inline unsigned long flags(void) const { return _flags; }
	inline bool isExecutable(void) const { return _flags & EXECUTABLE; }
	inline bool isWritable(void) const { return _flags & WRITABLE; }
	inline address_t address(void) const { return _address; }
	inline size_t size(void) const { return _size; }
	inline address_t topAddress(void) const { return _address + _size; }
	Inst *findByAddress(address_t addr);
	ProgItem *findItemByAddress(address_t addr);
	
	// Item iterator
	class ItemIter: public PreIterator<ItemIter, ProgItem *> {
		ProgItem *cur;
	public:
		inline ItemIter(const Segment *seg):
			cur((ProgItem *)seg->items.first()) { }
		inline ItemIter(const ItemIter& iter): cur(iter.cur) { }
		inline ProgItem *item(void) const { return cur; }
		inline bool ended(void) const { return cur->atEnd(); }
		inline void next(void) { cur = (ProgItem *)cur->next(); }
	};

protected:	
	Segment(CString name, address_t address, size_t size, unsigned long flags);
	void splitItem(ProgItem *old_item, inhstruct::DLList& new_items);
	void putItem(ProgItem *new_item, ProgItem *old_item = 0);
	virtual ~Segment(void);

private:
	unsigned long _flags;
	CString _name;
	address_t _address;
	size_t _size;
	inhstruct::DLList items;
};

};	// namespace otawa

#endif		// OTAWA_PROG_SEGMENT_H
