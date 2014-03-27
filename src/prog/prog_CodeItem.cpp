/*
 *	$Id$
 *	Copyright (c) 2005-07, IRIT UPS.
 *
 *	CodeItem class implementation.
 */

#include <otawa/prog/CodeItem.h>
#include <otawa/prog/Symbol.h>
#include <otawa/prog/Inst.h>

namespace otawa {

// Configuration of the instruction map
#define MAP_BITS	6
#define MAP_MASK	((1 << MAP_BITS) - 1)
#define MAP_SIZE(s)	(((s) + MAP_MASK - 1) >> MAP_BITS)
#define MAP_INDEX(a) (((a) - address()) >> MAP_BITS)

	
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


/**
 * Build a code item.
 * @param name			Name of the code item.
 * @param _address		Base address of the memory block.
 * @param size			Size of the memory block.
 * @param instructions	List of instructions.
 */
CodeItem::CodeItem(
	address_t _address,
	size_t size,
	inhstruct::DLList& instructions)
:	ProgItem(code, _address, size),
	map(new Inst *[MAP_SIZE(size)]),
	insts(instructions)
{
	for(int i = 0; i < MAP_SIZE(size); i++)
		map[i] = 0;
	ASSERT(!insts.isEmpty());
	for(Inst *inst = first(); !inst->atEnd(); inst = inst->next()) {
		int index = MAP_INDEX((inst->address()));
		if(!map[index])
			map[index] = inst;
	}
}


/**
 * Find an instructions using to its address.
 * @param addr	Address of the instruction to find.
 * @return		Found instruction or null if not found.
 */
otawa::Inst *CodeItem::findByAddress(address_t addr) {

	// In the segment ?
	if(addr < address() || addr >= address() + size())
		return 0;
	
	// Look in the instruction
	Inst *inst = map[MAP_INDEX(addr)];
	if(inst)
		while(!inst->atEnd() && inst->address() <= addr) {
			if(inst->address() == addr)
				return inst;
			inst = inst->next();
		}
	return 0;
}


/**
 */
CodeItem::~CodeItem(void) {
	while(!insts.isEmpty()) {
		Inst *inst = (Inst *)insts.first();
		inst->remove();
		delete inst;
	}
}

} // otawa
