/*
 *	$Id$
 *	Copyright (c) 2005-07, IRIT UPS.
 *
 *	CodeItem class interface
 */
#ifndef OTAWA_PROG_CODEITEM_H
#define OTAWA_PROG_CODEITEM_H

#include <otawa/prog/ProgItem.h>

namespace otawa {

// External classes
class Inst;
class Symbol;

// Code class
class CodeItem: public ProgItem {
public:
	virtual Inst *first(void) const = 0;
	virtual Inst *last(void) const = 0;
	virtual CodeItem *toCode(void) { return this; };
	virtual IteratorInst<Inst *> *insts(void) = 0;
	Symbol *closerSymbol(Inst *inst);
};

} // otawa

#endif // OTAWA_PROG_CODEITEM_H
