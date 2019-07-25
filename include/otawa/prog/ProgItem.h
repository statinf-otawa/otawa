/*
 *	$Id$
 *	Copyright (c) 2005-07, IRIT UPS.
 *
 *	ProgItem class interface
 */
#ifndef OTAWA_PROG_PROG_ITEM_H
#define OTAWA_PROG_PROG_ITEM_H

#include <elm/assert.h>
#include <elm/string.h>
#include <elm/inhstruct/DLList.h>
#include <otawa/base.h>
#include "../prop.h"

namespace otawa {

// Extern classes
class Inst;
class Segment;

// ProgItem class
class ProgItem: public PropList, protected inhstruct::DLNode {
public:
	static rtti::Type& __type;

	ProgItem *next(void) const;
	ProgItem *previous(void) const;
	virtual address_t address(void) const = 0;
	virtual t::uint32 size(void) const = 0;
	inline address_t topAddress(void) const { return address() + size(); }
	virtual Inst *toInst(void);

	inline MemArea area(void) const { return MemArea(address(), size()); }
	inline bool contains(Address a) const { return area().contains(a); }
	inline bool overlap(const MemArea& a) const { return area().meet(a); }
	inline bool overlap(ProgItem *pi) const { return overlap(pi->area()); }

protected:
	friend class Segment;
	virtual ~ProgItem(void);
	inline void insertPseudo(ProgItem *pos)
		{ pos->insertBefore(this); }
};

} // otawa

#endif // OTAWA_PROG_PROG_ITEM_H
