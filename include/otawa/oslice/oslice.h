/*
 *     Common header file for otawa::oslice
 *
 *     This file is part of OTAWA
 *     Copyright (c) 2015, IRIT UPS.
 *
 *     OTAWA is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 *
 *     OTAWA is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with OTAWA; if not, write to the Free Software
 *     Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *     02110-1301  USA
 */

#ifndef __OTAWA_OSLICE_H__
#define __OTAWA_OSLICE_H__

#include <elm/log/Log.h>
#include <elm/avl/Set.h>
#include <otawa/prog/Inst.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/data/clp/features.h> // to use the CLP features
#include <otawa/oslice/features.h>
#include <elm/compare.h>
#include <otawa/dfa/MemorySet.h>

#define __SOURCE_INFO__ elm::log::Debug::debugPrefix(__FILE__, __LINE__, __FUNCTION__)
#define __RESET__ elm::color::RCol
#define __GREEN__ elm::color::IGre
#define __RED__ elm::color::IRed
#define __BLUE__ elm::color::IBlu
#define __YELLOW__ elm::color::IYel
#define __CYAN__ elm::color::ICya
#define __PURPLE__ elm::color::IPur
#define __TAB__ "    "

namespace otawa { namespace oslice {
// used by the Slicer and the Instruction collectors
class InterestedInstruction;
class MemoryAccessInformation;
typedef elm::avl::Set<InterestedInstruction*, elm::Comparator<InterestedInstruction*> > interested_instructions_t;
// used by the Slicer and the DotDisplayer (for coloring the instruction)
typedef elm::avl::Set<Inst*, elm::Comparator<Inst*> > InstSet;
typedef elm::avl::Set<BasicBlock*, elm::Comparator<BasicBlock*> > BBSet; // used by buildReverseSynthLink, for CFG v2
typedef elm::avl::Set<clp::Value, elm::Comparator<clp::Value> > clp_value_set_t;

// for OSlicer and LivenessChecker
typedef otawa::Bag<MemoryAccessInformation> inst_clp_bag_t;
typedef Vector<MemoryAccessInformation> clp_vector_t;
typedef otawa::Bag<otawa::clp::Value> clp_bag_t;

// for OSlicer, and InstCollector
extern Identifier<interested_instructions_t*> INTERESTED_INSTRUCTIONS;
extern Identifier<InstSet*> SET_OF_REMAINED_INSTRUCTIONS;
extern Identifier<String> SLICED_CFG_OUTPUT_PATH;
extern Identifier<String> SLICING_CFG_OUTPUT_PATH;

extern Identifier<otawa::oslice::BBSet*> SetOfCallers;

static const t::uint32
	DISPLAY_BB_MEM_ACCESS = 0x01,
	DISPLAY_WORKING_SET = 0x02,
	DISPLAY_BB_STATE_INFO = 0x04,
	IDENTIFY_MEM_ACCESS = 0x08,
	DISPLAY_LIVENESS_STAGES = 0x10,
	DISPLAY_SLICING_STAGES = 0x20,
	DISPLAY_CFG_CREATION = 0x40,
	DISPLAY_WIP = 0x80,
	NOTHING = 0;

class InterestedInstruction {
	Inst* _inst;
	BasicBlock* _bb;
public:
	inline InterestedInstruction(Inst* i, BasicBlock* b) : _inst(i), _bb(b) {}
	inline Inst* getInst() { return _inst; }
	inline BasicBlock* getBB() { return _bb; }
};

class WorkingElement {
public:
	Block* _bb;
	elm::BitVector _workingRegs;
	Inst* _inst;
	otawa::dfa::MemorySet::t _workingMems;
	inline WorkingElement(Block* bb, Inst* inst, elm::BitVector bv, otawa::dfa::MemorySet::t & rd) :
			_bb(bb), _workingRegs(bv), _inst(inst), _workingMems(rd) {
	}
};

class MemoryAccessInformation {
public:
	inline MemoryAccessInformation(Inst* _i = 0, clp::Value _c = 0, int _s = 0): inst(_i), clpValue(_c), size(_s) { }
	Inst* inst;
	clp::Value clpValue;
	int size;
};

}} // end of otawa::oslice

namespace elm {
template<>
class Comparator<otawa::oslice::InterestedInstruction*> {
public:
	inline static int compare(otawa::oslice::InterestedInstruction* const & a, otawa::oslice::InterestedInstruction* const & b) {
		if((a->getInst() == b->getInst()) && (a->getBB() == b->getBB())) return 0;
		else if (a->getInst()->address() > b->getInst()->address()) return 1;
		else return -1;
	}
};

template<>
class Comparator<otawa::Inst*> {
public:
       inline static int compare(otawa::Inst* const & a, otawa::Inst* const & b) {
               if(a->address() == b->address()) return 0;
               else if (a->address() > b->address()) return 1;
               else return -1;
       }
};

template<>
class Comparator<otawa::clp::Value> {
public:
       inline static int compare(otawa::clp::Value const & a, otawa::clp::Value const & b) {
               if(a == b) return 0;
               else if (a >= b.lower()) return 1;
               else return -1;
       }
};

} // end of elm


#endif
