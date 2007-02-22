/*
 *	$Id$
 *	Copyright (c) 2005-07, IRIT UPS.
 *
 *	CodeItem class interface
 */
#ifndef OTAWA_PROG_CODEITEM_H
#define OTAWA_PROG_CODEITEM_H

#include <elm/Iterator.h>
#include <elm/inhstruct/DLList.h>
#include <otawa/prog/ProgItem.h>
#include <otawa/prog/Instruction.h>

namespace otawa {

// External classes
class Symbol;

// Code class
class CodeItem: public ProgItem {
public:
	CodeItem(address_t address, size_t size,
		inhstruct::DLList& instructions);
	inline Inst *first(void) const { return (Inst *)insts.first(); }
	virtual Inst *last(void) const { return (Inst *)insts.last(); }
	Inst *findByAddress(address_t addr);
	Symbol *closerSymbol(Inst *inst);

	// InstIter class
	class InstIter: public PreIterator<InstIter, Inst *> {
		Inst *cur;
	public:
		inline InstIter(CodeItem *item): cur(item->first()) { }
		inline InstIter(const InstIter& iter): cur(iter.cur) { }
		inline Inst *item(void) const { return cur; }
		inline bool ended(void) const { return !cur->atEnd(); }
		inline void next(void) { cur = cur->next(); }
	};

protected:
	virtual ~CodeItem(void);

private:
	inhstruct::DLList insts; 
	Inst **map;
};

} // otawa

#endif // OTAWA_PROG_CODEITEM_H
