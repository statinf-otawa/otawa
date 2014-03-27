/*
 *	$Id $
 *	Inst class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-08, IRIT UPS.
 *
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <otawa/instruction.h>
#include <otawa/prog/Process.h>

namespace otawa {

/**
 * A table containing no sets.
 */
const elm::genstruct::Table<hard::Register *> Inst::no_regs;


/**
 * @class Inst
 * This class represents assembly instruction of a piece of code.
 * As they must represents a large set of machine language that may contain
 * unusual instruction, it provides a very generic way to access information
 * about the instruction. When the instruction is usual or simple enough, it may
 * be derived in more accurate and specialized representation like MemInst or
 * ControlInst.
 * @ingroup prog
 */


/**
 * @var Inst::IS_COND
 * Mask of a conditional instruction of an instruction kind. Note that
 * conditional property is not bound to branch but may also be found in
 * guarded instructions.
 */

/**
 * @var Inst::IS_CONTROL
 * Mask of a control instruction.
 */

/**
 * @var Inst::IS_CALL
 * Mask of a call instruction.
 */

/**
 * @var Inst::IS_RETURN
 * Mask of a return instruction.
 */

/**
 * @var Inst::IS_MEM
 * Mask of an instruction accessing the memory.
 */

/**
 * @var Inst::IS_LOAD
 * Mask of an instruction performing a memory load.
 */

/**
 * @var Instr::IS_STORE
 * Mask of an instruction performing a memory store.
 */

/**
 * @var Inst::IS_INT
 * Mask of an instruction processing integer.
 * @note Conversion instruction must have both masks IS_INT and IS_FLOAT.
 */

/**
 * @var Inst::IS_FLOAT
 * Mask of an instruction processing floats.
 * @note Conversion instruction must have both masks IS_INT and IS_FLOAT.
 */

/**
 * @var Inst::IS_ALU
 * Mask of an instruction performing a computation.
 * @note Memory accesses with a computed address must have this bit set.
 */

/**
 * @var Inst::IS_MUL
 * Mask of a multiplication instruction.
 */

/**
 * @var Inst::IS_DIV
 * Mask of a division instruction.
 */

/**
 * @var Inst::IS_SHIFT
 * Mask of an instruction performing a shift (this includes logicial shifts,
 * arithmetic shifts and rotations).
 */

/**
 * @var Inst::IS_TRAP
 * Mask of a trap instruction. It may be a programmed interruption, a system
 * call, a debugging break or any control instruction whose control target
 * is computed by the system.
 */

/**
 * @var Inst::IS_INTERN
 * Mask of an instruction performing setup internal to the processor.
 */

/**
 * @var Inst::IS_MULTI
 * This mask denotes an instructions that perform multi-value store or load.
 * For example, the "ldmfd" or "stmfd" in the ARM ISA.
 */

/**
 * @var Inst::IS_SPECIAL
 * This mask denotes an instruction that is processed in a special way in the pipeline.
 * This concerns very complex instruction usually found in old CISC processors.
 */

/**
 * @var Inst::IS_UNKNOWN
 * This mask denotes an unknown instruction: its opcode does not match any known
 * instruction in the loader. This may denotes either a limitation of the loader,
 * or an execution path error in the program decoding.
 */

/**
 * @type Instr::kind_t;
 * The kind of an instruction is a bit array where each bit represents an
 * instruction property. The following masks gives access to the property bits:
 * @ref IS_COND, @ref IS_CONTROL, @ref IS_CALL, @ref IS_RETURN,
 * @ref IS_MEM, @ref IS_LOAD, @ref IS_STORE, @ref IS_INT, @ref IS_FLOAT, @ref IS_ALU,
 * @ref IS_MUL, @ref IS_DIV, @ref IS_SHIFT, @ref IS_TRAP, @ref IS_INTERN,
 * @ref IS_MULTI, @ref IS_SPECIAL, @ref IS_UNKNOWN.
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
 * @fn  bool Inst::isAtomic(void);
 * Test if the instruction is an atomic synchronization instruction, that is, performing
 * an atomic read-write memory.
 * @return True if it is an synchronization function.
 */


/**
 * @fn bool Inst::isMulti(void);
 * Test if the instruction is multi-memory accesss load / store.
 * @return	True if it is multi-memory accesses, false else.
 * @see IS_MULTI
 */


/**
 * @fn bool Inst::isSpecial(void);
 * Test if the instruction is a complex special instruction.
 * @return	True if it is a complex special instruction, false else.
 * @see IS_SPECIAL
 */


/**
 * @fn bool Inst::isMul(void);
 * Test if the instruction is a multiplication.
 * @return	True if it is a multiplication, false else.
 * @see Inst::IS_MUL
 */


/**
 * @fn bool Inst::isDiv(void);
 * Test if the instruction is a division.
 * @return	True if it is a division, false else.
 * @see Inst::IS_DIV
 */


/**
 * @fn bool Inst::isUnknown(void);
 * Test if an instruction is unknown.
 * @return	True if it is unknown, false else.
 * @see Inst::IS_UNKNOWN
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
 * Output a displayable representation of the instruction.
 * The implementation of this method is not mandatory.
 * @param out	Output channel to use.
 */
void Inst::dump(io::Output& out) {
}


/**
 * @fn Type *Inst::type(void);
 * @deprecated
 * Get the type of the accessed object.
 * @return Accessed data type.
 */
Type *Inst::type(void) {
	return 0;
}


/**
 * @fn bool Inst::isConditional(void);
 * Test if this instruction is conditional.
 * @return True if the instruction is conditional, false else.
 */


/**
 * Get the target of the branch.
 * @return Target address of the branch.
 */
Inst *Inst::target(void) {
	return 0;
}


/**
 * Get the registers read by the instruction.
 * @return	Read register table.
 * @warning	This method is only implemented when the owner loader
 * asserts the @ref REGISTER_USAGE_FEATURE.
 */
const elm::genstruct::Table<hard::Register *>& Inst::readRegs(void) {
	throw UnsupportedFeatureException(0, REGISTER_USAGE_FEATURE);
}


/**
 * Get the registers written by the instruction.
 * @return	Read register table.
 * @warning	This method is only implemented when the owner loader
 * asserts the @ref REGISTER_USAGE_FEATURE.
 */
const elm::genstruct::Table<hard::Register *>& Inst::writtenRegs(void) {
	throw UnsupportedFeatureException(0, REGISTER_USAGE_FEATURE);
}


/**
 * @fn kind_t Inst::kind(void);
 * Get the kind of the current instruction. In fact, the kind is composed as
 * bit array representing an instruction property.
 * @return The kind of the instruction.

 */


/**
 * Return a list of semantics pseudo-instruction representing the effect
 * of the instruction.
 * @param block	Block to write semantics instruction to. An empty vector
 * 				means there is no operation.
 * @note		The passed block is not cleared by the method that
 * 				accumulates instructions allowing to wholly
 * 				translates parts of code.
 */
void Inst::semInsts(sem::Block& block) {
}


/**
 * For a branch instruction, returns the type of management for delay slots.
 * As a default, consider there is no delay slots.
 * @return	Type of management of delay slots.
 * @note 	Branches without delay slots does not need to overload this method.
 */
delayed_t Inst::delayType(void) {
	return DELAYED_None;
}


/**
 * For a branch instruction, returns the number of delayed instructions.
 * As a default, return 0.
 * @return	Number of delayed instructions.
 */
int Inst::delaySlots(void) {
	return 0;
}


/**
 */
Inst *Inst::toInst(void) {
	return this;
}


/**
 * This function is only defined for ISA supporting the @ref IS_MULTI attribute.
 * It returns the number of stored during the multi-memory access
 * (in term of memory accesses).
 * @return	Number of memory accesses.
 */
int Inst::multiCount(void) {
	return 0;
}


// NullInst class
class NullInst: public Inst {
public:
	virtual kind_t kind(void) { return 0; }
	virtual Address address(void) const { return Address::null; }
	virtual t::uint32 size(void) const { return 0; }
	virtual Inst *toInst(void) const { return (Inst *)this; }
} static_null;


/**
 * Null instruction with null address and null size (no kind).
 */
Inst& Inst::null = static_null;

}	// otawa
