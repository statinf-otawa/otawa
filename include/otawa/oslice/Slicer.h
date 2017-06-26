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

	virtual void configure(const PropList &props);

protected:
	virtual void processWorkSpace(WorkSpace *fw);
	virtual void cleanup(WorkSpace *ws);
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
