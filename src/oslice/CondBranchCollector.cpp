/*
 *	CondBranchCollector class implementation
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <otawa/oslice/CondBranchCollector.h>

namespace otawa { namespace oslice {

p::feature COND_BRANCH_COLLECTOR_FEATURE("otawa::oslice::COND_BRANCH_COLLECTOR_FEATURE", new Maker<CondBranchCollector>());

p::declare CondBranchCollector::reg = p::init("otawa::oslice::CondBranchCollector", Version(1, 0, 0))
	.make<CondBranchCollector>()
	.require(COLLECTED_CFG_FEATURE)
	.provide(COND_BRANCH_COLLECTOR_FEATURE)
	.provide(INSTRUCTION_COLLECTOR_FEATURE);

/**
 */
CondBranchCollector::CondBranchCollector(AbstractRegistration& _reg)
: 	InstCollector(_reg) { }

/**
 */
void CondBranchCollector::configure(const PropList &props) {
	InstCollector::configure(props);
}

bool CondBranchCollector::interested(Inst* i) {
	if(i->isConditional() && i->isControl())
		return true;
	else
		return false;
}




class UnknownTargetCollector: public InstCollector {
public:
	static p::declare reg;
	UnknownTargetCollector(AbstractRegistration& _reg = reg);

protected:
	virtual void configure(const PropList &props);
	virtual bool interested(Inst* i);
	virtual void cleanup(WorkSpace *ws);
};

/**
 */
UnknownTargetCollector::UnknownTargetCollector(AbstractRegistration& _reg)
: 	InstCollector(_reg) { }

/**
 */
void UnknownTargetCollector::configure(const PropList &props) {
	InstCollector::configure(props);
}

bool UnknownTargetCollector::interested(Inst* i) {
//	elm::cout << "inst " << i << " with kind " << hex(i->kind()) << io::endl;
	if(i->isControl() && !i->target() && !i->isReturn())
		return true;
	else
		return false;
}

void UnknownTargetCollector::cleanup(WorkSpace *ws) {
	addCleaner(UNKNOWN_TARGET_COLLECTOR_FEATURE, new InstCollectorCleaner(ws, interestedInstructionsLocal));
}


p::feature UNKNOWN_TARGET_COLLECTOR_FEATURE("otawa::oslice::UNKNOWN_TARGET_COLLECTOR_FEATURE", new Maker<UnknownTargetCollector>());

p::declare UnknownTargetCollector::reg = p::init("otawa::oslice::UnknownTargetCollector", Version(1, 0, 0))
	.make<UnknownTargetCollector>()
	.require(COLLECTED_CFG_FEATURE)
	.provide(UNKNOWN_TARGET_COLLECTOR_FEATURE)
	.provide(INSTRUCTION_COLLECTOR_FEATURE);

} }
