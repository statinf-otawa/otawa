/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	instryction.cpp -- instruction classes implementation.
 */

#include <otawa/instruction.h>

namespace otawa {

/**
 * A table containing no sets.
 */
const elm::genstruct::Table<hard::Register *> Inst::no_regs;


/**
 * @class Inst <otawa/program.h>
 * This class represents assembly instruction of a piece of code.
 * As they must represents a large set of machine language that may contain
 * unusual instruction, it provides a very generic way to access information
 * about the instruction. When the instruction is usual or simple enough, it may
 * be derived in more accurate and specialized representation like MemInst or
 * ControlInst.
 */

/**
 * @fn address_t Inst::address(void);
 * Get the memory address of the instruction if available.
 * @return Memory address of the instruction or null.
 */

/**
 * @fn size_t Inst::size(void);
 * Get the size of the instruction in memory. For RISC architecture, this size
 * is usually a constant like 4 bytes.
 * @return Instruction size.
 */


/**
 * @fn bool Inst::isIntern(void);
 * Test if the instruction neither access memory, nor modify control flow.
 * @return True if the instruction is internal.
 */


/**
 * @fn bool Inst::isMem(void) ;
 * Test if the instruction access memory.
 * @return True if it perform memory access.
 */


/**
 * @fn bool Inst::isControl(void);
 * Test if the instruction changes the control flow.
 * @return True if control may be modified.
 */


/**
 * @fn  bool Inst::isLoad(void);
 * Test if the instruction is a load, that is, it performs only one simple memory read.
 * @return True if it is a load.
 */


/**
 * @fn bool Inst::isStore(void);
 * Test if the instruction is a store, that is, it performs only one simple memory write.
 * @return True if it is a store.
 */


/**
 * @fn  bool Inst::isBranch(void);
 * Test if the instruction is a branch, that is, it changes the control flow but
 * performs neither a memory access, nor a context storage.
 */


/**
 * @fn bool Inst::isCall(void);
 * Test if the instruction is a sub-program call, that is, it changes the control flow
 * but stores the current state for allowing performing a return.
 * @return True if it is a sub-program call.
 */


/**
 * @fn  bool Inst::isReturn(void);
 * Test if the instruction is a sub-program return, that is, it modifies the control flow
 * retrieving its context from a preceding call instruction.
 * @return True if it is a sub-program return.
 */


/**
 * @fn  bool Inst::isPseudo(void);
 * Test if the instruction is a pseudo-instruction.
 * @return True if it is a pseudo-instruction.
 */


/**
 * @fn PseudoInst *Inst::toPseudo(void);
 * Get the representation of this pseudo-instruction.
 * @return Pseudo-instruction representation or null.
 */


/**
 * @fn Inst *Inst::next(void) const;
 * Get the next instruction.
 * @return Next instruction.
 */


/**
 * @fn Inst *Inst::previous(void) const;
 * Get the previous instruction.
 * @return Previous instruction.
 */


/**
 * @fn void Inst::dump(io::Output& out);
 * Output a displayable representation of the instruction.
 * The implementation of this method is not mandatory.
 * @param out	Output channel to use.
 */


/**
 * @fn Type *Inst::type(void);
 * Get the type of the accessed object.
 * @return Accessed data type.
 */


/**
 * @fn bool Inst::isConditional(void);
 * Test if this instruction is conditional.
 * @return True if the instruction is conditional, false else.
 */


/**
 * @fn Address *Inst::target(void);
 * Get the target of the branch.
 * @return Target address of the branch.
 */
 

/**
 * Get the registers read by the instruction.
 * @return	Read register table.
 */
const elm::genstruct::Table<hard::Register *>& Inst::readRegs(void) {
	return no_regs;
}


/**
 * Get the registers written by the instruction.
 * @return	Read register table.
 */
const elm::genstruct::Table<hard::Register *>& Inst::writtenRegs(void) {
	return no_regs;
}

 
/**
 * @class PseudoInst
 * Pseudo-instruction does not represents realinstruction but markers that
 * may be inserted in the code by various code processors. It is used,
 * for example, for marking basic block limits.
 * @par
 * A pseudo instruction is associated with an identifier like the property
 * identifier allowing different code processor to have their own pseudo-instructions.
 */


/**
 * @fn PseudoInst::PseudoInst(const Identifier *id);
 * Builder of a pseudo-instruction.
 * @param id	Identifier of the pseudo-instruction.
 */


/**
 * @fn const Identifier *PseudoInst::id(void) const;
 * Get the identifier of this pseudo-instruction.
 * @return	Pseudo-instruction identifier.
 */


/**
 * Compute the address of this pseudo-instruction.
 */
address_t PseudoInst::address(void) {

	// Look forward
	for(Inst *inst = next(); !inst->atEnd(); inst = inst->next())
		if(!inst->isPseudo())
			return inst->address();
	
	// Look backward
	for(Inst *inst = previous(); !inst->atBegin(); inst = inst->previous())
		if(!inst->isPseudo())
			return inst->address() + inst->size();
	
	// None found
	return 0;
}


/**
 * Dump the pseudo-instruction.
 * @param out	Output to perform the dump on.
 */
void PseudoInst::dump(io::Output& out) {
	out << "pseudo <" << (void *)_id << '>';
}

}	// otawa
