#ifndef __OSLICE_LIVENESS_CHECKER_H__
#define __OSLICE_LIVENESS_CHECKER_H__

#include "oslice.h"
#include <otawa/prog/WorkSpace.h>
#include <otawa/cfg.h>
#include <otawa/cfg/features.h>
#include <otawa/prog/Inst.h>
#include <otawa/cfg/BasicBlock.h>
#include <elm/util/BitVector.h>
#include <otawa/data/clp/features.h> // to use the CLP features

using namespace otawa;

namespace otawa { namespace oslice {
class LivenessChecker: public otawa::Processor {
public:

	LivenessChecker(AbstractRegistration& _reg = reg);
	static p::declare reg;
	virtual void configure(const PropList &props);
	virtual void processWorkSpace(WorkSpace *fw);
	inline static elm::BitVector getRegisters() { return _defaultRegisters; }
 	static void provideRegisters(Inst* inst, elm::BitVector& regsToModify, int readOrWrite);
 	//static void displayAddrs(io::Output& out, clp_value_set_t & v);
 	template<class T>
 	inline static void displayAddrs(io::Output& out, T & v) {
// 		if(v.count() == 0) {
// 			out << "None";
// 			return;
// 		}
// 		for(class T::Iterator p(v); p; ++p)
// 			out << *p << " ";
 		out << v;
 	}
 	inline static void displayAddrs(io::Output& out, clp_bag_t & v) {
 		for(int i = 0; i < v.count(); ++i)
 			out << v[i] << " ";
 		if(v.count() == 0)
 			out << "None ";
 	}


 	//static void getMems(BasicBlock* bb, Inst* inst, int & currentIndex, clp_value_set_t & clpSet, int readOrWrite);
 	static void getMems(BasicBlock* bb, Inst* inst, int & currentIndex, otawa::dfa::MemorySet::t & clpSet, int readOrWrite);
 	//static bool containsAllAddrs(clp_value_set_t & a, clp_value_set_t & b);
 	static bool containsAllAddrs(otawa::dfa::MemorySet::t & a, otawa::dfa::MemorySet::t & b);
 	//static void updateAddrsFromInstruction(clp_value_set_t & workingMem, clp_value_set_t & readMem, clp_value_set_t & writeMem, t::uint32 debugLevel);
 	static void updateAddrsFromInstruction(otawa::dfa::MemorySet::t & workingMem, otawa::dfa::MemorySet::t & readMem, otawa::dfa::MemorySet::t & writeMem, t::uint32 debugLevel);

protected:

	//virtual bool interestingAddrs(clp_value_set_t const & a, clp_value_set_t const & b);
 	virtual bool interestingAddrs(dfa::MemorySet::t const & a, dfa::MemorySet::t const & b);
	virtual bool interestingRegs(elm::BitVector const & a, elm::BitVector const & b);

private:
 	void buildReverseSynthLink(const otawa::CFGCollection& coll);
 	void processWorkingList(elm::genstruct::Vector<WorkingElement*>& workingList);
 	void initIdentifiersForEachBB(const CFGCollection& coll);
 	void identifyAddrs(BasicBlock* bb);
 	void clearAddrs(BasicBlock* bb);

 	clp::Manager *clpManager;
 	static elm::BitVector _defaultRegisters;
 	static t::uint32 _debugLevel;

 	//void intersect()

}; // class LivenessChecker

}} // otawa::oslice
#endif
