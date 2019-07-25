/*
 *	VirtualInst class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2016, IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */
#ifndef OTAWA_PROG_VIRTUALINST_H_
#define OTAWA_PROG_VIRTUALINST_H_

#include <elm/data/HashMap.h>
#include <otawa/prog/Inst.h>
#include <otawa/proc/AbstractFeature.h>

namespace otawa {

class CFGCollection;
class VirtualInstManager;

class VirtualInst: public Inst {
	friend class VirtualInstManager;
public:
	VirtualInst(WorkSpace *ws, Inst *inst);
	VirtualInst(VirtualInstManager& man, Inst *inst);
	inline Inst *inst(void) const { return _inst; }

	Address address(void) const override;
	t::uint32 size(void) const override;
	void dump(io::Output& out) override;
	kind_t kind(void) override;
	Inst *target(void) override;
	Type *type(void) override;
	void semInsts(sem::Block& block) override;
	int semInsts(sem::Block& block, int temp) override;
	void semKernel(sem::Block& block) override;
	int semKernel(sem::Block& block, int t) override;
	int semWriteBack(sem::Block& block, int temp) override;
	delayed_t delayType(void) override;
	int delaySlots(void) override;
	void readRegSet(RegSet& set) override;
	void writeRegSet(RegSet& set) override;
	Inst *toInst(void) override;
	const Array<hard::Register *>& readRegs(void) override;
	const Array<hard::Register *>& writtenRegs(void) override;
	int multiCount(void) override;
	Condition condition(void) override;

protected:
	~VirtualInst(void);
private:
	Inst *_inst;
	VirtualInst *next;
};

class NOP: public VirtualInst {
public:
	NOP(WorkSpace *ws, Inst *inst);
	NOP(VirtualInstManager& man, Inst *inst);

	virtual void dump(io::Output& out);
	virtual kind_t kind(void);
	virtual Inst *target(void);
	virtual Type *type(void);
	virtual void semInsts(sem::Block& block);
	virtual int semInsts(sem::Block& block, int temp);
	virtual int semWriteBack(sem::Block& block, int temp);
	virtual delayed_t delayType(void);
	virtual int delaySlots(void);
	virtual void readRegSet(RegSet& set);
	virtual void writeRegSet(RegSet& set);
	virtual const Array<hard::Register *>& readRegs(void);
	virtual const Array<hard::Register *>& writtenRegs(void);
	virtual int multiCount(void);
	virtual Condition condition(void);
};

class VirtualInstManager {
	friend class VirtualInst;
	friend class VirtualInstBuilder;
public:
	VirtualInst *makeNOP(Inst *inst);

	void start(void);
	void mark(Inst *inst);
	void finish(void);

	void collect(const CFGCollection& coll);

private:
	VirtualInstManager(void);
	void record(VirtualInst *inst);
	VirtualInst *head;
};

extern p::feature VIRTUAL_INST_FEATURE;
extern p::id<VirtualInstManager *> VIRTUAL_INST_MANAGER;

}	// otawa

#endif /* OTAWA_PROG_VIRTUALINST_H_ */
