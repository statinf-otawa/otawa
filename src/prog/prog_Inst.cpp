/*
 *	Inst class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-17, IRIT UPS.
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
//const Array<hard::Register *> Inst::no_regs;


/**
 * @class Inst
 * This class represents assembly instructions of a piece of code.
 * As they must represents a large set of machine language that may contain
 * unusual instruction, they provide a very generic way to access information
 * about the instruction.
 *
 * @ref Inst is basically an abstract class that does not provide any facilities (except
 * linkage with other instruction). To be actually used, it has to be subclassed by the
 * loader to an actual instruction of the loader instruction set.
 *
 * Depending on the features provided by the loader, only several or all of its functions
 * are implemented. If an unimplemented function is called, an exception
 * @ref UnsupportedFeatureException is raised. The supported feature of the loader
 * are set on the @ref Process and automatically transfered to the @ref Workspace using
 * the @ref Process. Therefore, they can be tested using the function @ref WorkSpace::isProvided().
 *
 * The loader features and the corresponding @ref Inst functions are listed below:
 * * @ref CONTROL_DECODING_FEATURE -- @ref Inst::target()
 * * @ref REGISTER_USAGE_FEATURE -- @ref Inst::readRegSet(), @ref Inst::writeRegSet().
 * * @ref SEMANTICS_INFO -- @ref Inst::semInsts().
 * * @ref DELAYED_FEATURE -- @ref Inst::delayType(), @ref Inst::delaySlots.
 * * @ref CONDITIONAL_INSTRUCTIONS_FEATURE -- @ref Inst::condition(), @ref Inst::updateCondition(),
 * * @ref VLIW_FEATURE -- @ref Inst::bundle()/
 *
 * Notice that the availability of a feature does not always mean that the corresponding
 * loader is incomplete. If @ref DELAYED_FEATURE and @ref CONDITIONAL_INSTRUCTIONS_FEATURE
 * are not provided, the instruction does not provide these facilties.
 *
 * @section proc_vliw VLIW Support
 * 
 * Very-Long Instruction Word (VLIW) architectures encodes Instruction Level
 * Parallelism (ILP) in the instruction themselves: in fact, the scheduling of
 * instructions is done by the compiler. Instructions are grouped by bundles
 * and the bundle traverses the processor pipeline as a unitary instruction.
 * 
 * To support the bundle, the new version of OTAWA considers the bundle as a
 * single instruction and the program loader in charge of the VLIW architecture
 * has to translate the effects of the instructions composing the bundle as
 * a single instruction. Yet, the individual instruction are still reachable
 * with the function @ref Inst::bundle(). This function may be useful for
 * the display of the instructions but considering the instruction bundle as
 * a single instructions prevent support of VLIW for other OTAWA analyses.
 * 
 * To detect if the underlying architecture of a @ref otawa::Process supports
 * VLIW instructions, the feature @ref otawa::VLIW_FEATURE is provided.
 * 
 * Notice that the previous API using the flag Inst::IS_BUNDLE to mark the end of
 * bundle is now deprecated. The overall result is a significant simplification
 * of OTAWA API and of the implementation of analyses.
 * 
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
 * @var Inst::IS_REPEAT
 * This task denotes instructions that implements repetition on the next instruction.
 */

/**
 * @var Inst::IS_BUNDLE
 * Applied on a VLIW architecture, marks instructions part of a bundle but
 * not at end of the bundle.
 */

/**
 * @type Instr::kind_t;
 * The kind of an instruction is a bit array where each bit represents an
 * instruction property. The following masks gives access to the property bits:
 * @ref IS_COND, @ref IS_CONTROL, @ref IS_CALL, @ref IS_RETURN,
 * @ref IS_MEM, @ref IS_LOAD, @ref IS_STORE, @ref IS_INT, @ref IS_FLOAT, @ref IS_ALU,
 * @ref IS_MUL, @ref IS_DIV, @ref IS_SHIFT, @ref IS_TRAP, @ref IS_INTERN,
 * @ref IS_MULTI, @ref IS_SPECIAL, @ref IS_UNKNOWN, @ref IS_REPEAT.
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
 * @fn bool Inst::isRepeat();
 * Test if an instruction implements a repetition of the next instruction.
 * @return	True if it is a repetition, false else.
 * @see Inst::IS_REPEAT
 */


/**
 * @fn bool Inst::isBundle(void);
 * On VLIW architecture, mark an instruction that is part of a bundle but not last instruction.
 * @return	True if it is a not bundle-ending instruction, false else.
 * @see Inst::IS_BUNDLE, Inst::isBundleEnd()
 */


/**
 * @fn bool Inst::isBundleEnd(void);
 * On VLIW architecture, mark an instruction that is the last instruction of a bundle.
 * @return	True if it is a bundle-ending instruction, false else.
 * @see Inst::IS_BUNDLE, Inst::isBundle.
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
 * Accumulate the list of registers read by the instruction in the given register set.
 * @param set	Set filled with platform numbers of read registers.
 * @warning	This method is only implemented when the owner loader
 * asserts the @ref REGISTER_USAGE_FEATURE.
 */
void Inst::readRegSet(RegSet& set) {
	const Array<hard::Register *>&tab = readRegs();
	for(int i = 0; i < tab.count(); i++)
		set.add(tab[i]->platformNumber());
}


/**
 * Accumulate the list of registers written by the instruction in the given register set.
 * @param set	Set filled with platform numbers of written registers.
 * @warning	This method is only implemented when the owner loader
 * asserts the @ref REGISTER_USAGE_FEATURE.
 */
void Inst::writeRegSet(RegSet& set) {
	const Array<hard::Register *>&tab = writtenRegs();
	for(int i = 0; i < tab.count(); i++)
		set.add(tab[i]->platformNumber());
}


/**
 * Get the registers read by the instruction.
 * @return	Read register table.
 * @warning	This method is only implemented when the owner loader
 * asserts the @ref REGISTER_USAGE_FEATURE.
 * @deprecated	Use readRegSet() instead.
 */
const Array<hard::Register *>& Inst::readRegs() {
	// TODO (remove the null reference)
	throw UnsupportedFeatureException(REGISTER_USAGE_FEATURE);
}


/**
 * Get the registers written by the instruction.
 * @return	Read register table.
 * @warning	This method is only implemented when the owner loader
 * asserts the @ref REGISTER_USAGE_FEATURE.
 * @deprecated	Use writeRegSet() instead.
 */
const Array<hard::Register *>& Inst::writtenRegs() {
	throw UnsupportedFeatureException(REGISTER_USAGE_FEATURE);
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
 * Same as Inst::semInsts(sem::Block& block) to transform a machine instruction
 * into machine instructions but temp is used as a base to encode temporaries.
 * This is used for VLIW where instructions are executed in parallel and register
 * write-back only occurs at end of the semantic instructions.
 * @param block		Block to translate instruction in.
 * @param temp		Base number for temporaries used for write-back register saving.
 * @return			Number of used temporaries for write-back.
 * @deprecated		Use the new @ref proc_vliw support.
 * @see	@ref proc_vliw
 */
int Inst::semInsts(sem::Block& block, int temp) {
	semInsts(block);
	return 0;
}


/**
 * For a conditional instruction, fill the block with the semantic instructions
 * without the conditional evaluation part. For a non-conditional instruction,
 * do the same as semInsts().
 *
 * @param block	Block to fill with semantic instructions.
 */
void Inst::semKernel(sem::Block& block) {
	semInsts(block);
}


/**
 * For a conditional instruction, fill the block with the semantic instructions
 * without the conditional evaluation part. For a non-conditional instruction,
 * do the same as semInsts().
 *
 * Additional parameter is used for temporary renaming in case of VLIW instruction set.
 * For VLIW also, it must return of consumed temporaries.
 *
 * @param block	Block to fill with semantic instructions.
 * @param temp	Next temporary number to use.
 * @return		Number of used temporaries.
 * @deprecated	Use the new @ref proc_vliw support.
 */
int Inst::semKernel(sem::Block& block, int temp) {
	semKernel(block);
	return 0;
}


/**
 * VLIW instructions of a bundle perform read and write-back of registers
 * in parallel. This is mimicked in OTAWA by concatenating semantic instructions
 * of each machine instruction and only writing into temporaries.
 * by this method.
 * @param block		Block to fill with write-back semantic instructions.
 * @param temp		Base number for temporaries used for write-back register saving.
 * @return			Number of temporaries used by this instruction.
 * @see	@ref prog_vliw
 */
int Inst::semWriteBack(sem::Block& block, int temp) {
	return 0;
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
 * Return the condition associated with the current instruction.
 * In its default implementation, return a no-condition for
 * all instructions.
 * @return	Condition of the instruction
 * 			(no-condition if there is no condition,
 * 			any-condition if the condition is not handled by the condition system).
 *
 */
Condition Inst::condition(void) {
	return Condition();
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


/**
 * Can only called for a VLIW archietcture.
 * Provides the sub-instruction composing a bundle instruction.
 * @param insts		Vector to be filled with sub-instructions.
 * 					Default implementation does nothing.
 * @see @ref proc_vliw support.
 */
void Inst::bundle(Vector<Inst *>& insts) {
}


/**
 * @fn Inst::Kind Inst::getKind(void);
 * Get the kind of an instruction as an object. The Inst::Kind class is now the preferred way.
 * @return	Kind of the instruction.
 */

/**
 * @class Inst::Kind
 * This class is now the preferred way to manage and display kinds. It provides exactly
 * the same facilites as @ref Inst::kind_t and contains functions to test the different
 * aspects of the kind. As most of its functions are inlined, it is not heavier
 * than the @ref Inst function counterparts.
 *
 * @ingroup prog
 */


/**
 * @fn bool Inst::Kind::isIntern(void);
 * Test if the instruction neither access memory, nor modify control flow.
 * @return True if the instruction is internal.
 */


/**
 * @fn bool Inst::Kind::isMem(void) ;
 * Test if the instruction access memory.
 * @return True if it perform memory access.
 */


/**
 * @fn bool Inst::Kind::isControl(void);
 * Test if the instruction changes the control flow.
 * @return True if control may be modified.
 */


/**
 * @fn  bool Inst::Kind::isLoad(void);
 * Test if the instruction is a load, that is, it performs only one simple memory read.
 * @return True if it is a load.
 */


/**
 * @fn bool Inst::Kind::isStore(void);
 * Test if the instruction is a store, that is, it performs only one simple memory write.
 * @return True if it is a store.
 */


/**
 * @fn  bool Inst::Kind::isBranch(void);
 * Test if the instruction is a branch, that is, it changes the control flow but
 * performs neither a memory access, nor a context storage.
 */


/**
 * @fn bool Inst::Kind::isCall(void);
 * Test if the instruction is a sub-program call, that is, it changes the control flow
 * but stores the current state for allowing performing a return.
 * @return True if it is a sub-program call.
 */


/**
 * @fn  bool Inst::Kind::isReturn(void);
 * Test if the instruction is a sub-program return, that is, it modifies the control flow
 * retrieving its context from a preceding call instruction.
 * @return True if it is a sub-program return.
 */


/**
 * @fn  bool Inst::Kind::isAtomic(void);
 * Test if the instruction is an atomic synchronization instruction, that is, performing
 * an atomic read-write memory.
 * @return True if it is an synchronization function.
 */


/**
 * @fn bool Inst::Kind::isMulti(void);
 * Test if the instruction is multi-memory accesss load / store.
 * @return	True if it is multi-memory accesses, false else.
 * @see IS_MULTI
 */


/**
 * @fn bool Inst::Kind::isSpecial(void);
 * Test if the instruction is a complex special instruction.
 * @return	True if it is a complex special instruction, false else.
 * @see IS_SPECIAL
 */


/**
 * @fn bool Inst::Kind::isMul(void);
 * Test if the instruction is a multiplication.
 * @return	True if it is a multiplication, false else.
 * @see Inst::IS_MUL
 */


/**
 * @fn bool Inst::Kind::isDiv(void);
 * Test if the instruction is a division.
 * @return	True if it is a division, false else.
 * @see Inst::IS_DIV
 */


/**
 * @fn bool Inst::Kind::isUnknown(void);
 * Test if an instruction is unknown.
 * @return	True if it is unknown, false else.
 * @see Inst::IS_UNKNOWN
 */


/**
 * @fn bool Inst::Kind::isBundle(void);
 * On VLIW architecture, mark an instruction that is part of a bundle but not last instruction.
 * @return	True if it is a not bundle-ending instruction, false else.
 * @see Inst::IS_BUNDLE, Inst::isBundleEnd()
 */


/**
 * @fn bool Inst::isKind::BundleEnd(void);
 * On VLIW architecture, mark an instruction that is the last instruction of a bundle.
 * @return	True if it is a bundle-ending instruction, false else.
 * @see Inst::IS_BUNDLE, Inst::isBundle.
 */


/**
 * This feature is put by the loader for architecture implementing VLIW
 * instruction sets. This feature allows to call @ref Inst::bundle().
 * 
 * @ingroup prog
 */
p::feature VLIW_FEATURE("otawa::VLIW_FEATURE");



// NullInst class
class NullInst: public Inst {
public:
	virtual kind_t kind(void) { return 0; }
	virtual Address address(void) const { return Address::null; }
	virtual t::uint32 size(void) const { return 0; }
} static_null;


/**
 * Null instruction with null address and null size (no kind).
 */
Inst& Inst::null = static_null;


/**
 */
io::Output& operator<<(io::Output& out, Inst *inst) {
	inst->dump(out);
	return out;
}

/**
 */
io::Output& operator<<(io::Output& out, Inst::Kind kind) {
	static cstring is[] = {
		"IS_COND",
		"IS_CONTROL",
		"IS_CALL",
		"IS_RETURN",
		"IS_MEM",
		"IS_LOAD",
		"IS_STORE",
		"IS_INT",
		"IS_FLOAT",
		"IS_ALU",
		"IS_MUL",
		"IS_DIV",
		"IS_SHIFT",
		"IS_TRAP",
		"IS_INTERN",
		"IS_MULTI",
		"IS_SPECIAL",
		"IS_INDIRECT",
		"IS_UNKNOWN",
		"IS_ATOMIC",
		"IS_BUNDLE",
		"IS_REPEAT",
		""
	};
	bool fst = true;
	for(int i = 0; is[i]; i++)
		if(kind & (1 << i)) {
			if(fst)
				fst = false;
			else
				out << '|';
			out << is[i];
		}
	if(fst)
		out << "none";
	return out;
}

}	// otawa

