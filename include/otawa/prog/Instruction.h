/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	otawa/prog/Instruction.h -- Instruction class interface.
 */
#ifndef OTAWA_INSTRUCTION_H
#define OTAWA_INSTRUCTION_H

#include <elm/string.h>
#include <elm/inhstruct/DLList.h>
#include <elm/io.h>
#include <elm/genstruct/Table.h>
#include <otawa/properties.h>

namespace otawa {

// Declaration
class Inst;
class PseudoInst;
namespace hard {
	class Register;
} // hard

// Inst class
class Inst: public elm::inhstruct::DLNode, public PropList {
	friend class CodeItem;
protected:
	static const elm::genstruct::Table<hard::Register *> no_regs;
	virtual ~Inst(void) { };
public:
	inline Inst *next(void) const { return (Inst *)inhstruct::DLNode::next(); };
	inline Inst *previous(void) const { return (Inst *)inhstruct::DLNode::previous(); };

	virtual address_t address(void) = 0;
	virtual size_t size(void) = 0;
	virtual void dump(io::Output& out) { };
	
	virtual bool isIntern(void) { return false; };
	virtual bool isMem(void) { return false; };
	virtual bool isControl(void) { return false; };
	virtual bool isLoad(void) { return false; };
	virtual bool isStore(void) { return false; };
	virtual bool isBranch(void) { return false; };
	virtual bool isCall(void) { return false; };
	virtual bool isReturn(void) { return false; };
	virtual bool isPseudo(void) { return false; };
	
	// Low-level register access
	virtual const elm::genstruct::Table<hard::Register *>& readRegs(void);
	virtual const elm::genstruct::Table<hard::Register *>& writtenRegs(void);
	
	// Pseudo access
	virtual PseudoInst *toPseudo(void) { return 0; };
	
	// For control instruction
	virtual bool isConditional(void) { return false; };
	virtual Inst *target(void) { return 0; };
	
	// For memory instructions
	virtual Type *type(void) { return 0; };
};


// PseudoInst class
class PseudoInst: public virtual Inst {
	id_t _id;
public:
	inline PseudoInst(id_t id): _id(id) { };
	inline id_t id(void) const { return _id; };
	virtual address_t address(void);
	virtual void dump(io::Output& out);
	virtual size_t size(void) { return 0; };
	virtual bool isPseudo(void) { return true; };
	virtual PseudoInst *toPseudo(void) { return this; };
};


// Inlines
inline elm::io::Output& operator<<(elm::io::Output& out, Inst *inst) {
	inst->dump(out);
	return out;
}

} // namespace otawa

#endif // OTAWA_INSTRUCTION_H
