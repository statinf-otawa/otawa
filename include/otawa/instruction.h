/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	instruction.h -- instruction classes.
 */
#ifndef OTAWA_INSTRUCTION_H
#define OTAWA_INSTRUCTION_H

#include <elm/string.h>
#include <elm/datastruct/Collection.h>
#include <elm/inhstruct/DLList.h>
#include <elm/datastruct/Vector.h>
#include <elm/io.h>
#include <otawa/program.h>
#include <otawa/platform.h>
#include <otawa/properties.h>

namespace otawa {
using namespace elm;
using namespace elm::datastruct;

// Declaration
class Operand;
class RegOp;
class ImmOp;
class MemOp;
class Address;
class PtrAddr;
class AbsAddr;
class SymAddr;
class OffsetAddr;
class IndexAddr;
class Inst;
class MemInst;
class ControlInst;
class PseudoInst;

// Base types	
typedef enum op_kind_t {
	OP_Reg,
	OP_Imm,
	OP_Mem,
	OP_Ptr,
	OP_Abs,
	OP_Sym,
	OP_Offset,
	OP_Index
} op_kind_t;
typedef enum op_access_t {
	OP_Read,
	OP_Write
} op_access_t;


// Operand class
class Operand {
public:
	virtual ~Operand(void) { };
	virtual op_kind_t kind(void) const = 0;
	virtual op_access_t access(void) const = 0;
	virtual RegOp *toReg(void) { return 0; };
	virtual ImmOp *toImm(void) { return 0; };
	virtual MemOp *toMem(void) { return 0; };
	virtual Address *toAddress(void) { return 0; };
	virtual PtrAddr *toPtr(void) { return 0; };
	virtual AbsAddr *toAbs(void) { return 0; };
	virtual SymAddr *toSym(void) { return 0; };
	virtual OffsetAddr *toOffset(void) { return 0; };
	virtual IndexAddr *toIndex(void) { return 0; };
};

// RegOp class
class RegBank;
class RegOp: public Operand {
	Register _reg;
	op_access_t acc;
public:
	inline RegOp(Register reg, op_access_t access): _reg(reg), acc(access) { };
	inline Register reg(void) const { return _reg; };
	virtual op_kind_t kind(void) const { return OP_Reg; };
	virtual op_access_t access(void) const { return acc; };
	virtual RegOp *toReg(void) { return this; };
};

// ImmOp class
class ImmOp: public Operand {
	long val;
public:
	inline ImmOp(int value): val(value) { };
	inline int value(void) const { return val; };
	virtual op_kind_t kind(void) const { return OP_Imm; };
	virtual op_access_t access(void) const { return OP_Read; };	
	virtual ImmOp *toImm(void) { return this; };
};

// MemOp class
class MemOp: public Operand {
	Address *addr;
	Type *_type;
	op_access_t acc;
public:
	inline MemOp(Address *address, Type *type, op_access_t access)
		: addr(address), _type(type), acc(access) { };
	inline Address *address(void) const { return addr; };
	inline Type *type(void) const { return _type; };
	virtual op_kind_t kind(void) const { return OP_Mem; };
	virtual op_access_t access(void) const { acc; };
	virtual MemOp *toMem(void) { return this; };
};

// Address class
class Address: public Operand {
public:
	virtual op_access_t access(void) const { return OP_Read; };
	virtual Address *toAddress(void) { return this; };	
};

// RegAddr class
class PtrAddr: public Address {
	Register _reg;
public:
	inline PtrAddr(Register reg): _reg(reg) { };
	inline Register reg(void) const { return _reg; };
	virtual op_kind_t kind(void) const { return OP_Ptr; };
	virtual PtrAddr *toPtr(void) { return this; };
};

// AbsAddr class
class AbsAddr: public Address {
	address_t addr;
public:
	inline AbsAddr(address_t address): addr(address) { };
	inline address_t address(void) const { return addr; };
	virtual op_kind_t kind(void) const { return OP_Abs; };
	virtual AbsAddr *toAbs(void) { return this; };	
};

// SymAddr class
class SymAddr: public Address {
	CString _name;
public:
	inline SymAddr(CString name): _name(name) { };
	inline CString name(void) const { return _name; };
	virtual op_kind_t kind(void) const { return OP_Sym; };
	virtual SymAddr *toSym(void) { return this; };	
};

// OffsetAddr class
class OffsetAddr: public Address {
	Address *_base;
	int off;
public:
	inline OffsetAddr(Address *base, int offset)
		: _base(base), off(offset) { };
	inline Address *base(void) const { return _base; };
	inline int offset(void) const { return off; };
	virtual op_kind_t kind(void) const { return OP_Offset; };
	virtual OffsetAddr *toOffset(void) { return this; };
};


// IndexAddr class
class IndexAddr: public Address {
	Address *_base;
	PtrAddr *idx;
	int sca;
public:
	inline IndexAddr(Address *base, PtrAddr *index, int scale = 0)
		: _base(base), idx(index), sca(scale) { };
	inline Address *base(void) const { return _base; };
	inline PtrAddr *index(void) const { return idx; };
	inline int scale(void) const { return sca; };
	virtual op_kind_t kind(void) const { return OP_Index; };
	virtual IndexAddr *toIndex(void) { return this; };	
};

// Inst class
class Inst: public inhstruct::DLNode, public PropList {
	friend class Code;
protected:
	virtual ~Inst(void) { };
public:
	inline Inst *next(void) const { return (Inst *)inhstruct::DLNode::next(); };
	inline Inst *previous(void) const { return (Inst *)inhstruct::DLNode::previous(); };

	virtual address_t address(void) = 0;
	virtual size_t size(void) = 0;
	virtual void dump(io::Output& out) { };
	
	virtual Collection<Operand *> *getOps(void) = 0;
	virtual Collection<Operand *> *getReadOps(void) = 0;
	virtual Collection<Operand *> *getWrittenOps(void) = 0;
	
	virtual bool isIntern(void) { return false; };
	virtual bool isMem(void) { return false; };
	virtual bool isControl(void) { return false; };
	virtual bool isLoad(void) { return false; };
	virtual bool isStore(void) { return false; };
	virtual bool isBranch(void) { return false; };
	virtual bool isCall(void) { return false; };
	virtual bool isReturn(void) { return false; };
	virtual bool isPseudo(void) { return false; };
	
	// Pseudo access
	virtual PseudoInst *toPseudo(void) { return 0; };
	
	// For control instruction
	virtual bool isConditional(void) { return false; };
	virtual Inst *target(void) { return 0; };
	
	// For memory instructions
	virtual Type *type(void) { return 0; };
	virtual Address *mem(void) { return 0; };
};


// PseudoInst class
class PseudoInst: public virtual Inst {
	id_t _id;
	static datastruct::Vector<Operand *> null;
public:
	inline PseudoInst(id_t id): _id(id) { };
	inline id_t id(void) const { return _id; };
	virtual address_t address(void);
	virtual void dump(io::Output& out);
	virtual size_t size(void) { return 0; };
	virtual Collection<Operand *> *getOps(void) { return &null; };
	virtual Collection<Operand *> *getReadOps(void) { return &null; };
	virtual Collection<Operand *> *getWrittenOps(void) { return &null; };
	virtual bool isPseudo(void) { return true; };
	virtual PseudoInst *toPseudo(void) { return this; };
};


} // namespace otawa

#endif // OTAWA_INSTRUCTION_H
