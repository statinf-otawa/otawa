/*
 *	LivenessChecker class interface
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
#ifndef __OSLICE_LIVENESS_CHECKER_H__
#define __OSLICE_LIVENESS_CHECKER_H__

#include <otawa/oslice/oslice.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/cfg.h>
#include <otawa/cfg/features.h>
#include <otawa/prog/Inst.h>
#include <otawa/cfg/BasicBlock.h>
#include <elm/util/BitVector.h>
#include <otawa/data/clp/features.h> // to use the CLP features

using namespace otawa;

namespace otawa { namespace oslice {
template<class T>
inline void displayAddrs(io::Output& out, T & v) {
	out << v;
}
inline void displayAddrs(io::Output& out, clp_bag_t & v) {
	for(int i = 0; i < v.count(); ++i)
		out << v[i] << " ";
	if(v.count() == 0)
		out << "None ";
}

class LivenessChecker: public otawa::Processor {
public:

	LivenessChecker(AbstractRegistration& _reg = reg);
	static p::declare reg;
	virtual void configure(const PropList &props);
	virtual void processWorkSpace(WorkSpace *fw);
	inline static elm::BitVector getRegisters() { return _defaultRegisters; }
 	static void provideRegisters(Inst* inst, elm::BitVector& regsToModify, int readOrWrite);
 	static void getMems(BasicBlock* bb, Inst* inst, int & currentIndex, otawa::dfa::MemorySet::t & clpSet, int readOrWrite);
 	static bool containsAllAddrs(otawa::dfa::MemorySet::t & a, otawa::dfa::MemorySet::t & b);
 	static void updateAddrsFromInstruction(otawa::dfa::MemorySet::t & workingMem, otawa::dfa::MemorySet::t & readMem, otawa::dfa::MemorySet::t & writeMem, t::uint32 debugLevel);

 	static void identifyAddrs(BasicBlock* bb);
 	static clp::Manager *clpManager;

 	static void buildReverseSynthLink(const otawa::CFGCollection& coll);
 	inline static void setDebugLevel(t::uint32 _dl) { _debugLevel = _dl; }

protected:
 	virtual bool interestingAddrs(dfa::MemorySet::t const & a, dfa::MemorySet::t const & b);
	virtual bool interestingRegs(elm::BitVector const & a, elm::BitVector const & b);

private:

 	void processWorkingList(Vector<WorkingElement*>& workingList);
 	void initIdentifiersForEachBB(const CFGCollection& coll);

 	void clearAddrs(BasicBlock* bb);


 	static elm::BitVector _defaultRegisters;
 	static t::uint32 _debugLevel;
}; // class LivenessChecker

}} // otawa::oslice
#endif
