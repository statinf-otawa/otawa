/*
 *	oslice module interface
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
#ifndef __OTAWA_OSLICE_FEATURES_H__
#define __OTAWA_OSLICE_FEATURES_H__

#include <elm/avl/Set.h>
#include <elm/util/BitVector.h>
#include <otawa/proc/AbstractFeature.h>
#include <otawa/dfa/MemorySet.h>

namespace otawa { namespace oslice {
typedef elm::avl::Set<Inst*, elm::Comparator<Inst*> > InstSet;

extern p::feature DUMMY_SLICER_FEATURE;
extern p::feature SLICER_FEATURE;
extern p::feature LIGHT_SLICER_FEATURE;
extern p::feature COND_BRANCH_COLLECTOR_FEATURE;
extern p::feature UNKNOWN_TARGET_COLLECTOR_FEATURE;
extern p::feature LIVENESS_FEATURE;
extern p::feature INSTRUCTION_COLLECTOR_FEATURE;

extern Identifier<String> SLICED_CFG_OUTPUT_PATH;
extern Identifier<String> SLICING_CFG_OUTPUT_PATH;
extern Identifier<int> LIVENESS_DEBUG_LEVEL;
extern Identifier<int> SLICE_DEBUG_LEVEL;
extern Identifier<bool> CFG_OUTPUT;
extern Identifier<InstSet*> SET_OF_REMAINED_INSTRUCTIONS;
extern Identifier<bool> ENABLE_LIGHT_SLICING;
extern Identifier<bool (*)(otawa::Inst*)> FPTR_FOR_COLLECTING;

extern Identifier<dfa::MemorySet::t* > MEM_BB_END_IN;
extern Identifier<BitVector> REG_BB_END_IN;
extern Identifier<dfa::MemorySet::t*> MEM_BB_BEGIN_OUT;
extern Identifier<BitVector> REG_BB_BEGIN_OUT;



class Manager {
public:
	typedef t::uint32 step_t;
	static const step_t
		NEW_INST = 0x01,
		HEAD = 0x02,
		ENDED = 0;

	Manager(WorkSpace *ws);
	step_t start(BasicBlock *bb);
	step_t next(void);
 	void displayState(io::Output & output); // display the state (register and memory) which are alive at the current step
	inline Inst *inst(void) { return _currInst; }
	elm::BitVector workingRegs(void);
	//otawa::Bag<otawa::clp::Value> workingMems(void);
	dfa::MemorySet::t* workingMems(void);
	bool isRegAlive(int regID);
	bool isMemAlive(otawa::Address memAddress);

private:
	t::uint32 _debugLevel;
	Inst* _currInst;
	BasicBlock* _currBB;
	int _currInstIndex;
};

}}
#endif
