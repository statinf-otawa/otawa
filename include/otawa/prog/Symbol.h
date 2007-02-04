/*
 *	$Id$
 *	Copyright (c) 2003-06, IRIT UPS.
 *
 *	otawa/prog/Symbol.h -- Symbol class interface.
 */
#ifndef OTAWA_PROG_SYMBOL_H
#define OTAWA_PROG_SYMBOL_H

#include <elm/io.h>
#include <otawa/properties.h>

namespace otawa {

// Symbol kind
	
// Symbol class
class Inst;
class Symbol {
public:
	typedef enum kind_t {
		NONE,
		FUNCTION,
		LABEL
	} kind_t;
	static Identifier<Symbol *> ID;

	virtual ~Symbol(void);
	virtual kind_t kind(void) = 0;
	virtual elm::String name(void) = 0;
	virtual address_t address(void) = 0;
	virtual size_t size(void) = 0;
	virtual Inst *findInst(void) = 0;
};

// GenericIdentifier<Symbol_t *>::print Specialization
template <>
void Identifier<Symbol *>::print(elm::io::Output& output, const Property& prop) const;

} // otawa

#endif	// OTAWA_SYMBOL_H
