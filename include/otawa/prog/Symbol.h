/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	otawa/Symbol.h -- Symbol class interface.
 */
#ifndef OTAWA_PROG_SYMBOL_H
#define OTAWA_PROG_SYMBOL_H

#include <elm/string.h>
#include <otawa/base.h>

namespace otawa {

// Symbol kind
typedef enum symbol_kind_t {
	SYMBOL_None,
	SYMBOL_Function,
	SYMBOL_Label
} symbol_kind_t;
	
// Symbol class
class Inst;
class Symbol {
public:
	virtual symbol_kind_t kind(void) = 0;
	virtual elm::String name(void) = 0;
	virtual address_t address(void) = 0;
	virtual size_t size(void) = 0;
	virtual Inst *findInst(void) = 0;
};

} // otawa

#endif	// OTAWA_SYMBOL_H
