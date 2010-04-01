/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	otawa/ipet/Delta.h -- Delta class interface.
 */

#ifndef OTAWA_IPET_DELTA_H
#define OTAWA_IPET_DELTA_H
#include <otawa/ipet/BBPath.h>
#include <otawa/ipet/TreePath.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/proc/Feature.h>


namespace otawa {

namespace ilp {
	class System;
}

namespace ipet {

class BBPath;
class Delta;

class Delta: public CFGProcessor {
	int levels;
	int completion;
	bool explicitNames;
	int max_length;
	int length_sum;
	int length_cnt;
	static int delta(BBPath &bbp, WorkSpace *fw);
	void processBBPath(WorkSpace *fw, ilp::System *system, BBPath *bbpath);

protected:
	virtual void setup(WorkSpace *fw);
	virtual void cleanup(WorkSpace *fw);
	virtual void processCFG(WorkSpace* fw, CFG* cfg);

public:
	Delta(void);
	static Registration<Delta> reg;

	virtual void configure(const PropList& props);

	static Identifier<int> LEVELS;
	static Identifier<int> DELTA;
	static Identifier<TreePath<BasicBlock*,BBPath*>*> TREE;
	static Identifier<int> SEQ_COMPLETION;
	static Identifier<double> MAX_LENGTH;
	static Identifier<double> MEAN_LENGTH;
};

// Features
extern Feature<Delta> DELTA_SEQUENCES_FEATURE;

} } // otawa::ipet

#endif /*OTAWA_IPET_DELTA_H*/
