/*
 *	BBTimeSimulator class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006-07, IRIT UPS.
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
 *	along with Foobar; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef OTAWA_TSIM_DELTA_H
#define OTAWA_TSIM_DELTA_H
#include <otawa/tsim/BBPath.h>
#include <otawa/tsim/TreePath.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/proc/Feature.h>


namespace otawa {

namespace ilp {
	class System;
}

namespace tsim {

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

} } // otawa::tsim

#endif /*OTAWA_IPET_TSIM_H*/
