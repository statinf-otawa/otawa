/*
 * StackAnalysis.h
 *
 *  Created on: 2 juil. 2009
 *      Author: casse
 */

#ifndef STACKANALYSIS_H_
#define STACKANALYSIS_H_

#include <otawa/proc/Processor.h>

namespace otawa {


class StackAnalysis: public Processor {
public:
	StackAnalysis(void);

protected:
	virtual void processWorkSpace(WorkSpace *ws);
};

extern Feature<StackAnalysis> STACK_ANALYSIS_FEATURE;

}	// otawa

#endif /* STACKANALYSIS_H_ */
