/*
 * DataCatConstraintBuilder.h
 *
 *  Created on: 12 juil. 2009
 *      Author: casse
 */

#ifndef DATACATCONSTRAINTBUILDER_H_
#define DATACATCONSTRAINTBUILDER_H_

#include <otawa/proc/Processor.h>
#include <otawa/prop/Identifier.h>


namespace otawa {

namespace ilp { class Var; }

namespace dcache {

extern Identifier<ilp::Var *> HIT_VAR;
extern Identifier<ilp::Var *> MISS_VAR;

class CatConstraintBuilder : public otawa::Processor {
public:
	CatConstraintBuilder(void);
	virtual void processWorkSpace(otawa::WorkSpace *ws);
	virtual void configure(const PropList& props);
	virtual void setup(otawa::WorkSpace *ws);

private:
	bool _explicit;
};

} }		// otawa::dcache

#endif /* DATACATCONSTRAINTBUILDER_H_ */
