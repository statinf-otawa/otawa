#ifndef OTAWA_IPET_AGGREGATIONGRAPH_H
#define OTAWA_IPET_AGGREGATIONGRAPH_H

#include <otawa/ipet.h>
#include <otawa/cfg.h>
#include <elm/util/Pair.h>
#include <elm/genstruct/Vector.h>

namespace otawa { namespace ipet {

class AggregationGraph: public CFG {
protected:
	CFG *cfg;
	elm::genstruct::Vector<BBPath*> elts;
	elm::genstruct::Vector<elm::Pair<BBPath*, BBPath*>*> edges;
	elm::Option<int> max_length;
	elm::Option<int> max_insts;
	elm::Option<int> max_joins;
	elm::Option<int> max_splits;
	int length_of_longer_path;
	double mean_length;
	
	void link(BBPath *src, BBPath *dst);
	virtual void configure(PropList &props);
	virtual void makeGraph();
	virtual void scan();
	virtual bool boundary(BBPath *cur_bbpath, BBPath *next_bbpath);
	virtual bool header(BBPath *cur_bbpath, BBPath *next_bbpath);
	static String pathName(BBPath *path);
	static String bbName(BasicBlock *bb);

public:
	AggregationGraph(CFG *cfg, PropList &props = PropList::EMPTY);
	virtual void toDot(elm::io::Output &out);
	virtual void printStats(elm::io::Output &out);
	virtual void printEquivalents(elm::io::Output &out);
	
	static GenericIdentifier<int> ID_Max_Length;
	static GenericIdentifier<int> ID_Max_Insts;
	static GenericIdentifier<int> ID_Max_Joins;
	static GenericIdentifier<int> ID_Cur_Joins;
	static GenericIdentifier<int> ID_Max_Splits;
	static GenericIdentifier<int> ID_Cur_Splits;
};

} } // otawa::ipet

#endif /*OTAWA_IPET_AGGREGATIONGRAPH_H*/
