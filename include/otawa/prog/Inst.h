/*
 *	$Id $
 *	Inst class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-07, IRIT UPS.
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
#ifndef OTAWA_INST_H
#define OTAWA_INST_H

#include <elm/string.h>
#include <elm/io.h>
#include <elm/genstruct/Table.h>
#include <otawa/prog/ProgItem.h>
#include <otawa/properties.h>

namespace otawa {

// Declaration
class Inst;
class PseudoInst;
namespace hard {
	class Register;
} // hard

// Inst class
class Inst: public ProgItem {
	friend class CodeItem;
protected:
	static const elm::genstruct::Table<hard::Register *> no_regs;
	virtual ~Inst(void) { };
public:

	// Kind management
	static const unsigned long IS_COND		= 0x0001;
	static const unsigned long IS_CONTROL	= 0x0002;
	static const unsigned long IS_CALL		= 0x0004;
	static const unsigned long IS_RETURN	= 0x0008;
	static const unsigned long IS_MEM		= 0x0010;
	static const unsigned long IS_LOAD		= 0x0020;
	static const unsigned long IS_STORE		= 0x0040;
	static const unsigned long IS_INT		= 0x0080;
	static const unsigned long IS_FLOAT		= 0x0100;
	static const unsigned long IS_ALU		= 0x0200;
	static const unsigned long IS_MUL		= 0x0400;
	static const unsigned long IS_DIV		= 0x0800;
	static const unsigned long IS_SHIFT		= 0x1000;
	static const unsigned long IS_TRAP		= 0x2000;
	static const unsigned long IS_INTERN	= 0x4000;
	typedef unsigned long kind_t;

	// Accessors
	inline Inst *nextInst(void) const;
	inline Inst *prevInst(void) const;
	virtual void dump(io::Output& out);
	
	// Kind access
	virtual kind_t kind(void) = 0;
	inline bool isIntern(void);
	inline bool isMem(void);
	inline bool isControl(void);
	inline bool isLoad(void);
	inline bool isStore(void);
	inline bool isBranch(void);
	inline bool isCall(void);
	inline bool isReturn(void);
	inline bool isConditional(void);
	virtual bool isPseudo(void);
	
	// Low-level register access
	virtual const elm::genstruct::Table<hard::Register *>& readRegs(void);
	virtual const elm::genstruct::Table<hard::Register *>& writtenRegs(void);
	
	// Specialized information
	virtual PseudoInst *toPseudo(void);
	virtual Inst *target(void);
	virtual Type *type(void);

	// Stack information
	static const unsigned long READ = 0x01;
	static const unsigned long WRITE = 0x02;
	static const long UNKNOW_CHANGE = 0x80000000;
	virtual long stackChange(void);
	virtual unsigned long stackAccess(void);

	// ProgItem overload
	virtual Inst *toInst(void);
};


// PseudoInst class
class PseudoInst: public virtual Inst {
	const AbstractIdentifier *_id;
public:
	inline PseudoInst(const AbstractIdentifier *id): _id(id) { };
	inline const AbstractIdentifier *id(void) const { return _id; };
	virtual address_t address(void) const;
	virtual size_t size(void) const { return 0; };
	virtual void dump(io::Output& out);
	virtual bool isPseudo(void) { return true; };
	virtual PseudoInst *toPseudo(void) { return this; };
	virtual kind_t kind(void) { return 0; };
};


// Inst Inlines
inline Inst *Inst::nextInst(void) const {
	ProgItem *item = next();
	if(!item)
		return 0;
	else
		return item->toInst();
}

inline Inst *Inst::prevInst(void) const {
	ProgItem *item = previous();
	if(!item)
		return 0;
	else
		return item->toInst();
}

inline bool Inst::isIntern(void) {
	return kind() & IS_INTERN;
}

inline bool Inst::isMem(void) {
	return kind() & IS_MEM;
}

inline bool Inst::isControl(void) {
	return kind() & IS_CONTROL;
}

inline bool Inst::isLoad(void) {
	return kind() & IS_LOAD;
}

inline bool Inst::isStore(void) {
	return kind() & IS_STORE;
}

inline bool Inst::isBranch(void) {
	kind_t k = kind();
	return (k & IS_CONTROL) && !(k & (IS_RETURN | IS_CALL | IS_TRAP));
}
 
inline bool Inst::isCall(void) {
	return kind() & IS_CALL;
}

inline bool Inst::isReturn(void) {
	return kind() & IS_RETURN;
}

inline bool Inst::isConditional(void) {
	return kind() & IS_COND;
}

inline elm::io::Output& operator<<(elm::io::Output& out, Inst *inst) {
	inst->dump(out);
	return out;
}

} // namespace otawa

#endif // OTAWA_INST_H
