/*
 *	VirtualInst class implementation
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

#include <otawa/cfg/features.h>
#include <otawa/prog/VirtualInst.h>
#include <otawa/prog/WorkSpace.h>

namespace otawa {

/**
 */
static p::id<VirtualInst *> TAG("", 0);
static p::id<bool> USED("", false);


/**
 * @class VirtualInst
 * VirtualInst is a shield class in front of an instruction that allows to specialize
 * an existing instruction to something lightly different. It may be used in a lot
 * of different situations but common usages includes:
 * @li insertion of a NOP instruction instead of an existing instruction,
 * @li light modification of instruction semantics.
 *
 * VirtualInst are managed by @ref VirtualInstManager that is responsible of their
 * deallocation as soon they are no more used in the program representation.
 * To get a VirtualInstManager, one has to require @ref VIRTUAL_INST_FEATURE.
 *
 * As a default, all method calls on the VirtualInst are redirected to the matching
 * method of the wrapped instruction.
 */

/**
 * Build a new virtual instruction.
 * @param ws	Workspace to live in.
 * @param inst	Instruction to wrap around.
 */
VirtualInst::VirtualInst(WorkSpace *ws, Inst *inst): _inst(inst), next(0) {
	VirtualInstManager *man = VIRTUAL_INST_MANAGER(ws);
	ASSERTP(man, "no VirtualInstManager is available");
	man->record(this);
}


/**
 * Build a virtual instruction.
 * @param man	Owner manager.
 * @param inst	Instruction to shield.
 */
VirtualInst::VirtualInst(VirtualInstManager& man, Inst *inst): _inst(inst), next(0) {
	man.record(this);
}


/**
 */
VirtualInst::~VirtualInst(void) {
}


/**
 */
Address VirtualInst::address(void) const { return _inst->address(); }


/**
 */
t::uint32 VirtualInst::size(void) const { return _inst->size(); }


/**
 * @fn Inst *VirtualInst::inst(void) const;
 * Get the wrapped instruction.
 * @return	Wrapped instruction.
 */


/**
 */
void VirtualInst::dump(io::Output& out) { _inst->dump(out); }

/**
 */
Inst::kind_t VirtualInst::kind(void) { return _inst->kind(); }

/**
 */
Inst *VirtualInst::target(void) { return _inst->target(); }

/**
 */
Type *VirtualInst::type(void) { return _inst->type(); }

/**
 */
void VirtualInst::semInsts(sem::Block& block) { _inst->semInsts(block); }

/**
 */
int VirtualInst::semInsts(sem::Block& block, int temp) { return _inst->semInsts(block, temp); }

/**
 */
void VirtualInst::semKernel(sem::Block& block) { return _inst->semKernel(block); }

///
int VirtualInst::semKernel(sem::Block& b, int t) { return _inst->semKernel(b, t); }

/**
 */
int VirtualInst::semWriteBack(sem::Block& block, int temp) { return _inst->semWriteBack(block, temp); }

/**
 */
delayed_t VirtualInst::delayType(void) { return _inst->delayType(); }

/**
 */
int VirtualInst::delaySlots(void) { return _inst->delaySlots(); }

/**
 */
void VirtualInst::readRegSet(RegSet& set) { _inst->readRegSet(set); }

/**
 */
void VirtualInst::writeRegSet(RegSet& set) { _inst->writeRegSet(set); }

/**
 */
Inst *VirtualInst::toInst(void) { return this; }

/**
 */
const Array<hard::Register *>& VirtualInst::readRegs(void) { return _inst->readRegs(); }

/**
 */
const Array<hard::Register *>& VirtualInst::writtenRegs(void) { return _inst->writtenRegs(); }

/**
 */
int VirtualInst::multiCount(void) { return _inst->multiCount(); }

/**
 */
Condition VirtualInst::condition(void) { return _inst->condition(); }


/**
 * @class NOP
 * Virtual instruction that represents an instruction that does nothing
 * (but it occupies the same place as the masked instruction).
 */

/**
 */
NOP::NOP(WorkSpace *ws, Inst *inst): VirtualInst(ws, inst) { }

/**
 */
NOP::NOP(VirtualInstManager& man, Inst *inst): VirtualInst(man, inst) { }

/**
 */
void NOP::dump(io::Output& out) {
	out << "NOP";
}

/**
 */
Inst::kind_t NOP::kind(void) {
	return 0;
}

/**
 */
Inst *NOP::target(void) {
	return 0;
}

/**
 */
Type *NOP::type(void) {
	return (Type *)&Type::no_type;
}

/**
 */
void NOP::semInsts(sem::Block& block) {
}

/**
 */
int NOP::semInsts(sem::Block& block, int temp) {
	return temp;
}

/**
 */
int NOP::semWriteBack(sem::Block& block, int temp) {
	return temp;
}

/**
 */
delayed_t NOP::delayType(void) {
	return DELAYED_None;
}

/**
 */
int NOP::delaySlots(void) {
	return 0;
}

/**
 */
void NOP::readRegSet(RegSet& set) {
}

/**
 */
void NOP::writeRegSet(RegSet& set) {
}

/**
 */
const Array<hard::Register *>& NOP::readRegs(void) {
	return Array<hard::Register *>::null;
}

/**
 */
const Array<hard::Register *>& NOP::writtenRegs(void) {
	return Array<hard::Register *>::null;
}

/**
 */
int NOP::multiCount(void) {
	return 0;
}

/**
 */
Condition NOP::condition(void) {
	return Condition();
}


/**
 * @class VirtualInstManager
 *
 * The virtual manager is in charge of managing the allocation
 * and the release of @ref VirtualInst  classes. To be effective, each analysis
 * that creates virtual instructions should perform a garbage collection cycle
 * on the manager.
 *
 * A garbage collection cycle is made in three phases:
 * 1. first a call to start() is performed,
 * 2. then, for each instruction, a call to mark() is needed,
 * 3. finally, a call to finish() will release unused virtual instructions.
 *
 * Notice that the feature VIRTUAL_INST_FEATURE must be required before using
 * a VirtualInstManager.
 */


/**
 */
VirtualInstManager::VirtualInstManager(void): head(0) {
}


/**
 * Build a NOP instruction, that is, an instruction that does
 * nothing (doesn't use registers, does not branch, etc).
 * @return	Built instruction.
 */
VirtualInst *VirtualInstManager::makeNOP(Inst *inst) {
	return new NOP(*this, inst);
}


/**
 * Start the garbage collection.
 */
void VirtualInstManager::start(void) {
}


/**
 * Mark an instruction as used.
 * @param inst	Instruction to mark.
 */
void VirtualInstManager::mark(Inst *inst) {
	VirtualInst *vi = TAG(inst);
	while(vi) {
		USED(vi) = true;
		inst = vi->inst();
		vi = TAG(inst);
	}
}


/**
 * Finish the garbage collection.
 */
void VirtualInstManager::finish(void) {
	for(VirtualInst *i = head, **p = &head; i;)
		if(USED(i)) {
			p = &i->next;
			i = i->next;
		}
		else {
			*p = i->next;
			delete i;
			i = *p;
		}
}


/**
 * Perform a garbage collection cycle on the instruction
 * of the given CFG collection.
 */
void VirtualInstManager::collect(const CFGCollection& coll) {
	start();
	for(CFGCollection::BlockIter b(&coll); b(); b++)
		if(b->isBasic()) {
			BasicBlock *bb = b->toBasic();
			for(BasicBlock::InstIter i = bb->insts(); i(); i++)
				mark(*i);
		}
	finish();
}


/**
 * Record a new virtual instruction.
 * @param inst	Virtual instruction to record.
 */
void VirtualInstManager::record(VirtualInst *inst) {
	TAG(inst) = inst;
	inst->next = head;
	head = inst;
}


/**
 */
class VirtualInstBuilder: public Processor {
public:
	static p::declare reg;
	VirtualInstBuilder(void): Processor(reg) { }
protected:
	virtual void processWorkSpace(WorkSpace *ws) {
		if(!VIRTUAL_INST_MANAGER(ws))
			VIRTUAL_INST_MANAGER(ws) = new VirtualInstManager();
	}
};
p::declare VirtualInstBuilder::reg =
	p::init("otawa::VirtualInstBuilder", Version(1, 0, 0))
	.provide(VIRTUAL_INST_FEATURE)
	.make<VirtualInstBuilder>();


/**
 * Ensure that a virtual instruction manager is available to create virtual instructions
 * @see VirtualInst, VirtualInstManager
 *
 * @par Properties
 * @li @ref VIRTUAL_INST_MANAGER
 */
p::feature VIRTUAL_INST_FEATURE("otawa::VIRTUAL_INST_FEATURE", p::make<VirtualInstBuilder>());


/**
 * Get the current virtual instruction manager.
 *
 * Provided by:
 * @li @ref VIRTUAL_INST_FEATURE
 *
 * Hooked to:
 * @li @ref WorkSpace
 */
p::id<VirtualInstManager *> VIRTUAL_INST_MANAGER("otawa::VIRTUAL_INST_MANAGER", 0);

}	// otawa
