/*
 *	$Id$
 *	Copyright (c) 2004, IRIT UPS.
 *
 *	src/gliss/Symbol.cpp -- GLISS Symbol class implementation.
 */

#include <assert.h>
#include <otawa/gliss/Symbol.h>

namespace otawa { namespace gliss {


/**
 * @class Symbol
 * GLISS implementation of the Symbol class.
 */


/**
 * Build a new symbol.
 * @param file
 * @param name
 * @param kind
 * @param address
 */
Symbol::Symbol(File& file, String name, kind_t kind, address_t address)
: _file(file), _name(name), _kind(kind), addr(address) {
}

// otawa::Symbol overload
Symbol::kind_t Symbol::kind(void) {
	return _kind;
}

// otawa::Symbol overload
String Symbol::name(void) {
	return _name;
}

// otawa::Symbol overload
address_t Symbol::address(void) {
	return addr;
}

// otawa::Symbol overload
size_t Symbol::size(void) {
	switch(_kind) {
	case FUNCTION:
		return 4;
	case LABEL:
		return 4;
	default:
		assert(0);
	}
}

// otawa::Symbol overload
Inst *Symbol::findInst(void) {
	return _file.findByAddress(addr);
}

} } // otawa::gliss
