/*
 *	$Id$
 *	Copyright (c) 2003-07, IRIT UPS.
 *
 *	Symbol class interface
 */
#ifndef OTAWA_PROG_SYMBOL_H
#define OTAWA_PROG_SYMBOL_H

#include <elm/io.h>
#include <otawa/base.h>
#include "../prop.h"

namespace otawa {

// External classes
class File;
class Inst;

// Symbol class
class Symbol: public PropList {
public:

	typedef enum kind_t {
		NONE = 0,
		FUNCTION = 1,
		LABEL = 2,
		DATA = 3
	} kind_t;

	Symbol(File& file, String name, kind_t kind, address_t address, ot::size size = 0);
	inline File& file(void) const { return _file; }
	inline kind_t kind(void) const { return _kind; }
	inline const String& name(void) const { return _name; }
	inline address_t address(void) const { return _address; }
	inline ot::size size(void) const { return _size; }
	Inst *findInst(void) const;
	inline bool doesNotReturn(void) const { return no_return; }
	void setNoReturn(void);
	void print(io::Output& out) const;

private:
	File& _file;
	String _name;
	kind_t _kind;
	address_t _address;
	size_t _size;
	bool no_return;
};

// GenericIdentifier<Symbol_t *>::print Specialization
inline io::Output& operator<<(io::Output& out, Symbol *sym) { sym->print(out); return out; }
io::Output& operator<<(io::Output& out, Symbol::kind_t k);

} // otawa

#endif	// OTAWA_SYMBOL_H
