/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	prog/prog_CodeItem.cpp -- CodeItem class implementation.
 */

#include <otawa/prog/CodeItem.h>
#include <otawa/prog/Symbol.h>
#include <otawa/prog/Instruction.h>

namespace otawa {
	
/**
 * @class CodeItem
 * This program item represents a piece of code. It is usually a function but
 * it may be something else according the processed language or the optimization
 * methods.
 */

/**
 * @fn Inst *CodeItem::first(void) const;
 * Get the first instruction in the code.
 * @return First instruction.
 */

/**
 * @fn Inst *CodeItem::last(void) const;
 * Get the last instruction in the code.
 * @return Last instruction.
 */

/**
 * @fn IteratorInst<Inst *> *CodeItem::insts(void);
 * Get an iterator on the instructions contained in the code item.
 * @return	Code item instruction iterator.
 */


/**
 * Get the closer symbol, below or on the current instruction.
 * @param inst	Instruction to start look for.
 *  @return		Found symbol or null.
 */
Symbol *CodeItem::closerSymbol(Inst *inst) {
	while(true) {
		Symbol *sym = Symbol::ID(inst);
		if(sym)
			return sym;
		if(inst->atBegin())
			break;
		inst = inst->previous();
	}
	return 0;
}

} // otawa
