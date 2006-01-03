/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/test/LBlock.cpp -- implementation of LBlock class.
 */

#include <assert.h>
#include <otawa/cache/LBlock.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/cfg/BasicBlock.h>

namespace otawa {


/**
 * Build a new LBlock.
 * @param lbset		L-block set which owns this l-block.
 * @param address	Address of the l-block.
 * @param bb		Basic block containing this l-block.
 * @param size		Size of the l-block.
 */
LBlock::LBlock(LBlockSet *lbset, address_t address, BasicBlock *bb, size_t size)
: _size(size), addr(address), _bb(bb) {
	ident = lbset->LBlockSet::add(this);
}


/**
 * @fn size_t LBlock::size(void) const;
 * Get the size of the current l-block.
 * @return	L-block size.
 */


/**
 * Count the instructions in the l-block.
 * @return	L-block instruction count.
 */
int LBlock::countInsts(void) {
	int cnt = 0;
	
	if(_bb != 0){
		PseudoInst *pseudo;
		for(Iterator<Inst *> instr(_bb->visit()); instr; instr++) {
			pseudo = instr->toPseudo();
			if(!pseudo){
				if(instr->address() >= addr && instr->address() < addr + _size)
					cnt++;
			}
			else if(pseudo->id() == _bb->ID)
				break;	
		}	
	}
	
	return cnt;	
}


/**
 */
int LBlock::id(void){
	return ident;
}


/**
 * @fn address_t LBlock::address(void)
 * Get address of the l-block.
 * @return	L-block address.
 */


/**
 * @fn BasicBlock *LBlock::bb(void);
 * Get the BB containing the l-block.
 * @return	Container BB.
 */

} // otawa
