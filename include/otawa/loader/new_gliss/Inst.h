/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS <casse@irit.fr>
 *
 *	loader::Inst class interface
 */
#ifndef OTAWA_LOADER_NEW_GLISS_INST_H
#define OTAWA_LOADER_NEW_GLISS_INST_H

#include <otawa/prog/Inst.h>
#include <otawa/loader/new_gliss/Process.h>

namespace otawa { namespace loader { namespace new_gliss {

// Inst class
class Inst: public otawa::Inst {

public:
	Inst(Process& process, kind_t kind, address_t addr);
	inline Process& process(void) const { return proc; }
	
	virtual void dump(io::Output& out);
	virtual kind_t kind(void);
	virtual const elm::genstruct::Table<hard::Register *>& readRegs(void);
	virtual const elm::genstruct::Table<hard::Register *>& writtenRegs(void);
	virtual address_t address(void) const;

protected:
	kind_t _kind;
	static const unsigned long REGS_DONE = 0x80000000;
	static const unsigned long TARGET_DONE = 0x40000000;
	static const unsigned long MASK = REGS_DONE | TARGET_DONE;
	elm::genstruct::AllocatedTable<hard::Register *> in_regs;
	elm::genstruct::AllocatedTable<hard::Register *> out_regs;
	
	virtual void decodeRegs(void);

private:
	address_t addr;
	Process& proc;
};

} } } // otawa::loader::new_gliss

#endif // OTAWA_LOADER_NEW_GLISS_INST_H
