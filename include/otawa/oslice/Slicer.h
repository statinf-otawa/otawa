/*
 *	Slice class interface
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
#ifndef __OTAWA_OSLICE_SLICER_H__
#define __OTAWA_OSLICE_SLICER_H__

#include <elm/data/FragTable.h>
#include <elm/data/HashMap.h>
#include <elm/data/Vector.h>
#include <elm/util/BitVector.h>
#include <otawa/oslice/oslice.h>
#include <otawa/oslice/DotDisplayer.h>
#include <otawa/prog/Inst.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/oslice/LivenessChecker.h>

namespace otawa { namespace oslice {

class Slicer: public otawa::Processor {
public:

	Slicer(AbstractRegistration& _reg = reg);
	static p::declare reg;

	void configure(const PropList &props) override;
	void *interfaceFor(const AbstractFeature& f) override;

protected:
	void processWorkSpace(WorkSpace *fw) override;
	void commit(WorkSpace *ws) override;
	void destroy(WorkSpace *ws) override;

	virtual void slicing(void);

 	void processWorkingList(elm::Vector<WorkingElement*>& workingList);
 	void initIdentifiersForEachBB(const CFGCollection& coll);


    // used to create new CFG collection
    void make(CFG *cfg, CFGMaker& maker);
    void makeCFG(CFG *cfg);
    CFGMaker& makerOf(CFG *cfg);
    CFGMaker& newMaker(Inst *first);
    // the very first CFG
    HashMap<CFG *, CFGMaker *> map;
    FragTable<CFGMaker *> makers;

    bool interestingAddrs(otawa::dfa::MemorySet::t const & a, otawa::dfa::MemorySet::t const & b);
	bool interestingRegs(elm::BitVector const & a, elm::BitVector const & b);

    // used for debugging
    String _slicingCFGOutputPath;
    String _slicedCFGOutputPath;
    CFGCollection *sliced_coll;
    t::uint32 _debugLevel;
    bool _outputCFG;
    bool _lightSlicing;
};


class LightSlicer: public Slicer {
public:
	LightSlicer(AbstractRegistration& _reg = reg): Slicer(_reg) { _lightSlicing = true; }
	static p::declare reg;
};


} } // otawa::oslice

#endif
