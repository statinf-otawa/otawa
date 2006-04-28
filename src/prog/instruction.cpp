/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	instryction.cpp -- instruction classes implementation.
 */

#include <otawa/instruction.h>

namespace otawa {

/**
 * @class Operand
 * @par
 * This kind of object represents any argument handled by instructions.
 * This class is derived as register operands, immediate operands, address
 * computation operand and memory access operand.
 * @par
 * The operand provides information about the kind of access: read (OP_Read)
 * or written (OP_Written) or the sub-kind of this class.
 */

/**
 * @fn Operand::~Operand(void);
 * Virtual destructor for derived specific behaviour.
 */

/**
 * @fn op_kind_t Operand::kind(void) const;
 * Get the kind of this operand.
 * @return One of OP_Reg, OP_Imm, OP_Mem, OP_Sym, OP_Offset or  OP_Index.
 */

/**
 * @fn op_access_t Operand::access(void) const;
 * Get the kind of access to the operand.
 * @return One of OP_Read or OP_Written.
 */

/**
 * @fn RegOp *Operand::toReg(void);
 * Return to the pointer if this operand is a register operand.
 * @return Pointer to register operand or null.
 */

/**
 * @fn ImmOp *Operand::toImm(void);
 * Return to the pointer if this operand is a immediate operand.
 * @return Pointer to immediate operand or null.
 */

/**
 * @fn MemOp *Operand::toMem(void);
 * Return to the pointer if this operand is a memory operand.
 * @return Pointer to memory operand or null.
 */

/**
 * @fn Address *Operand::toAddress(void);
 * Return to the pointer if this operand is a address operand.
 * @return Pointer to address operand or null.
 */

/**
 * @fn AbsAddr *Operand::toAbs(void);
 * Return to the pointer if this operand is ab absolute address operand.
 * @return Pointer to absolute address operand or null.
 */

/**
 * @fn SymAddr *Operand::toSym(void);
 * Return to the pointer if this operand is a symbol address operand.
 * @return Pointer to symbol address operand or null.
 */

/**
 * @fn OffsetAddr *Operand::toOffset(void);
 * Return to the pointer if this operand is a offset address operand.
 * @return Pointer to offset address operand or null.
 */

/**
 * @fn IndexAddr *Operand::toIndex(void);
 * Return to the pointer if this operand is a indexed address operand.
 * @return Pointer to indexed address operand or null.
 */

/**
 * @fn PtrAddr *Operand::toPtr(void);
 * Return to the pointer if this operand is a pointer register address operand.
 * @return Pointer to pointer register address operand or null.
 */


/**
 * @class RegOp
 * This is a simple register operand. Its bank and oits number are stored
 * in this object.
 */

/**
 * @fn RegOp::RegOp(Register reg, op_access_t access);
 * Build a register operand.
 * @param reg			Register of the operand.
 * @param access		Kind of access to the register.
 */

/**
 * @fn Register RegOp::reg(void) const;
 * Get the register of the operand.
 * @return Operand register.
 */


/**
 * @class ImmOp
 * This operand stores an absolute signed/unsigned integer value with 
 * a maximal size of 32-bits.
 * @note An immediate value is only accessed by read.
 */

/**
 * @fn ImmOp::ImmOp(int value);
 * Build an immediate operand.
 * @param value	Immediate value to store.
 */

/**
 * @fn int ImmOp::value(void) const;
 * Get the immediate value stored in this operand.
 * @return	Stored immediate value.
 */


/**
 * @class MemOp
 * This kind of operand is obtained by accessing the memory. It takes
 * a sub-operand the address to read from or write to.
 */

/**
 * @fn MemOp::MemOp(Address *address, Type *type, op_access_t access);
 * Build a new memory operand.
 * @param address	Address to access in memory.
 * @param type		Type of data to get or put in memory.
 * @param access		Kind of access, read or written.
 */

/**
 * @fn Address *MemOp::address(void) const;
 * Get the accessed address.
 * @return Accessed address.
 */

/**
 * @fn Type *MemOp::type(void) const;
 * Get the type of accessed data.
 * @return Type of accessed data.
 */


/**
 * @class Address
 * Base type of operands used for computing an address.
 */


/**
 * @class AbsAddr
 * This address operand represents an absolute address.
 */

/**
 * @fn AbsAddr::AbsAddr(address_t address);
 * Build an absolute address operand.
 * @param address	Absolute address of the operand.
 */

/**
 * @fn address_t AbsAddr::address(void) const;
 * Get the absolute address of the operand.
 * @return	Absolute address value.
 */


/**
 * @class SymAddr
 * This operand contains an absolute address named by a symbol.
 */

/**
 * @fn SymAddr::SymAddr(CString name);
 * Build a symbolic address.
 * @param name	Name of the symbol.
 */

/**
 * @fn CString SymAddr::name(void) const;
 * Get the name of the symbol of the address.
 * @return Address symbol name.
 */


/**
 * @class OffsetAddr
 * This operand represents an address computed by adding an offset to
 * a base address.
 */

/**
 * @fn OffsetAddr::OffsetAddr(Address *base, int offset)
 * Build an offset address.
 * @param base	Base address.
 * @param offset	Offset to add.
 */

/**
 * @fn Address *OffsetAddr::base(void) const;
 * Get the base address of this address computation.
 * @return Get the base address of this computation.
 */

/**
 * @fn int OffsetAddr::offset(void) const { return off; };
 * Get the offset of this address computation.
 * @return Offset of this address.
 */


/**
 * @class IndexAddr
 * This operand computes an address by adding an index to a base address.
 * The index may be multiplied by a scale factor expressed as a power of two.
 */

/**
 * @fn IndexAddr::IndexAddr(Address *base, PtrAddr *index, int scale = 0);
 * Build a new indexed address.
 * @param base	Base address to add the index to.
 * @param index	Index to add.
 * @param scale	Scale factor as a power of two for multiplying the index.
 */

/**
 * @fn  Address *IndexAddr::base(void) const;
 * Get the base address of the address computation.
 * @return Base address.
 */

/**
 * @fn PtrAddr *IndexAddr::index(void) const;
 * Get the index of the address computation.
 * @return Used index.
 */

/**
 * @fn int IndexAddr::scale(void) const;
 * Get the scale factor of the index.
 * @return Scale factor.
 */


/**
 * @class PtrAddr
 * This class represents represents an address computed from a pointer register.
 */

/**
 * @fn PtrAddr::PtrAddr(Register reg)
 * Build a pointer address.
 * @param reg		Register of the pointer address.
 */

/**
 * @fn Register PtrAddr::reg(void) const;
 * Get the register of the pointer address.
 * @return Address register.
 */


/**
 * @class Inst
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
 * @fn elm::Collection<Operand *> Inst::getOps(void);
 * Get the operands of the instruction.
 * @return Instruction operands.
 */

/**
 * @fn elm::Collection<Operand *> Inst::getReadOps(void);
 * Get the read operands of the instruction.
 * @return Instruction read operands.
 */

/**
 * @fn elm::Collection<Operand *> Inst::getWrittenOps(void);
 * Get the written operands of the instruction.
 * @return Instruction written operands.
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
 * @class MemInst
 * This class represents a simple memory access instruction. It provides
 * information about the accessed address and the kind of accessed data.
 */

/**
 * @fn Type *Inst::type(void);
 * Get the type of the accessed object.
 * @return Accessed data type.
 */

/**
 * @fn Address *Inst::mem(void);
 * Get the accessed address in memory.
 * @return Accessed addres.
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
 * @class PseudoInst
 * Pseudo-instruction does not represents realinstruction but markers that
 * may be inserted in the code by various code processors. It is used,
 * for example, for marking basic block limits.
 * @par
 * A pseudo instruction is associated with an identifier like the property
 * identifier allowing different code processor to have their own pseudo-instructions.
 */

/**
 * @fn PseudoInst::PseudoInst(id_t id);
 * Builder of a pseudo-instruction.
 * @param id	Identifier of the pseudo-instruction.
 */

/**
 * @fn id_t PseudoInst::id(void) const;
 * Get the identifier of this pseudo-instruction.
 * @return	Pseudo-instruction identifier.
 */

/**
 * Internal: empty list of operands.
 */
//datastruct::Vector<Operand *> PseudoInst::null;

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
