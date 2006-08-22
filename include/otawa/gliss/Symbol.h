/*
 *	$Id$
 *	Copyright (c) 2004, IRIT UPS.
 *
 *	otawa/gliss/Symbol.h -- GLISS Symbol class interface.
 */
#ifndef OTAWA_GLISS_SYMBOL_H
#define OTAWA_GLISS_SYMBOL_H

#include <otawa/prog/Symbol.h>
#include <otawa/gliss/File.h>

namespace otawa { namespace gliss {

// Symbol class
class Symbol: public otawa::Symbol {
	File& _file;
	String _name;
	kind_t _kind;
	address_t addr;	
public:
	Symbol(File& file, String name, kind_t kind, address_t address);	

	// otawa::Symbol overload
	virtual kind_t kind(void);
	virtual String name(void);
	virtual address_t address(void);
	virtual size_t size(void);
	virtual Inst *findInst(void);
};
	
} } // otawa::gliss

#endif // OTAWA_GLISS_SYMBOL_H
