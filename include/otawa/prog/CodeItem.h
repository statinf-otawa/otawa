/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/prog/CodeItem.h -- CodeItem class interface.
 */
#ifndef OTAWA_PROG_CODEITEM_H
#define OTAWA_PROG_CODEITEM_H

#include <otawa/prog/ProgItem.h>

namespace otawa {

// External classes
class Inst;

// Code class
class CodeItem: public ProgItem {
public:
	virtual Inst *first(void) const = 0;
	virtual Inst *last(void) const = 0;
	virtual CodeItem *toCode(void) { return this; };
	virtual IteratorInst<Inst *> *insts(void) = 0;
};

} // otawa

#endif // OTAWA_PROG_CODEITEM_H
