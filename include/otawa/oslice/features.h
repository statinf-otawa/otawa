#ifndef __OTAWA_OSLICE_FEATURES_H__
#define __OTAWA_OSLICE_FEATURES_H__

#include <otawa/proc/AbstractFeature.h>
#include <elm/util/BitVector.h>
#include <otawa/util/Bag.h>
#include <otawa/data/clp/ClpValue.h>
#include <otawa/cfg/CFG.h>
#include <otawa/dfa/MemorySet.h>

namespace otawa { namespace oslice {
extern p::feature DUMMY_SLICER_FEATURE;
extern p::feature SLICER_FEATURE;
extern p::feature COND_BRANCH_COLLECTOR_FEATURE;
extern p::feature UNKNOWN_TARGET_COLLECTOR_FEATURE;
extern p::feature LIVENESS_FEATURE;

extern Identifier<String> SLICED_CFG_OUTPUT_PATH;
extern Identifier<String> SLICING_CFG_OUTPUT_PATH;
extern Identifier<int> LIVENESS_DEBUG_LEVEL;
extern Identifier<int> SLICE_DEBUG_LEVEL;
extern Identifier<bool> CFG_OUTPUT;

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
