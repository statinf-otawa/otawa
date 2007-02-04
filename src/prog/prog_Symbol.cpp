/*
 *	$Id$
 *	Copyright (c) 2003-06, IRIT UPS.
 *
 *	prog/prog_Symbol.h -- Symbol class implementation.
 */

#include <otawa/prog/Symbol.h>

namespace otawa {

/**
 * @class Symbol
 * A symbol is a name of a location in a program. Currently, only function
 * and label symbols are supported.
 * 
 * To fit with the OTAWA low-level abstraction architecture, this class is only
 * an empty interface implemented by the program loader.
 */


/**
 * @enum Symbol::kind_t
 * This type describes the kind of existing symbol.
 */


/**
 * @var Symbol::kind_t Symbol::NONE
 * Unknown symbol.
 */


/**
 * @var Symbol::kind_t Symbol::FUNCTION
 * Denotes a function symbol.
 */


/**
 * @var Symbol::kind_t Symbol::LABEL
 * Denotes a label symbol.
 */


/**
 */
Symbol::~Symbol(void) {
}


/**
 * @fn kind_t Symbol::kind(void);
 * Get the kind of the symbol.
 * @return	Symbol kind.
 */


/**
 * @fn elm::String Symbol::name(void);
 * Get the name of the symbol.
 * @return	Symbol name.
 */


/**
 * @fn address_t Symbol::address(void);
 * Get the address of the location referenced by this symbol.
 * @return	Symbol address.
 */


/**
 * @fn size_t Symbol::size(void);
 * Get the size of the item referenced by this symbol.
 * @return	Symbol size.
 */


/**
 * @fn Inst *Symbol::findInst(void);
 * If the symbol points to code memory, return the matching instruction.
 * @return	Pointed instruction.
 */


// GenericIdentifier<Symbol_t *>::print Specialization
template <>
void Identifier<Symbol *>::print(elm::io::Output& out, const Property& prop) const {
	out << "symbol(" << get(prop)->name() << ")";
}


/**
 * This property is used to attach a symbol to an instruction.
 * 
 * @par Hooks
 * @li @ref Inst
 */
Identifier<Symbol *> Symbol::ID("Symbol::ID", 0, otawa::NS);

} // otawa
