/*
 *	$Id$
 *	Copyright (c) 2003-07, IRIT UPS.
 *
 *	Symbol class implementation
 */

#include <otawa/prog/Symbol.h>
#include <otawa/prog/File.h>
#include <otawa/util/FlowFactLoader.h>

namespace otawa {

/**
 * @class Symbol
 * A symbol is a name of a location in a program. Currently, only function
 * and label symbols are supported.
 *
 * To fit with the OTAWA low-level abstraction architecture, this class is only
 * an empty interface implemented by the program loader.
 *
 * @ingroup prog
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
 * Build a symbol.
 * @param file		Owner file.
 * @param name		Symbol name.
 * @param kind		Symbol kind.
 * @param address	Address in the memory space.
 * @param size		Object size (null for code).
 */
Symbol::Symbol(
	File& file,
	String name,
	kind_t kind,
	address_t address,
	ot::size size)
:	_file(file),
	_name(name),
	_kind(kind),
	_address(address),
	_size(size),
	no_return(false)
{
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
 * @fn File& Symbol::file(void);
 * Get the owner file of the given symbol.
 * @return	Owern file.
 */

/**
 * If the symbol points to code memory, return the matching instruction.
 * @return	Pointed instruction if any, null else.
 */
Inst *Symbol::findInst(void) const {
	return _file.findByAddress(_address);
}


// GenericIdentifier<Symbol_t *>::print Specialization
void Symbol::print(elm::io::Output& out) const {
	out << "symbol(" << name() << ")";
}


io::Output& operator<<(io::Output& out, Symbol::kind_t k) {
	static cstring labels[] = {
		"NONE",
		"FUNCTION",
		"LABEL",
		"DATA"
	};
	out << labels[k];
	return out;
}


/**
 * This property is used to attach a symbol to an instruction.
 *
 * @par Hooks
 * @li @ref Inst
 */
Identifier<Symbol *> Symbol::ID("otawa::Symbol::ID", 0);


/**
 * @fn bool Symbol::doesNotReturn(void) const;
 * Test if it is a no returning function.
 * @return	True if the function never returns, false else.
 */


/**
 * Set the no-return property to this symbol.
 */
void Symbol::setNoReturn(void) {
	ASSERTP(_kind == FUNCTION, "may only be called on function symbol");
	ASSERTP(!no_return, "already set");

	// Find the instruction
	Inst *inst = _file.findInstAt(_address);
	ASSERTP(inst, "bad function label \"" << _name << "\" !");

	// Set the property
	NO_RETURN(inst) = true;
}

} // otawa
