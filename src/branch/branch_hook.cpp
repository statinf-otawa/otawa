/*
 *	branch plugin hook
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2014, IRIT UPS.
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

#include <otawa/branch/features.h>
#include <otawa/etime/features.h>
#include <otawa/hard/BHT.h>
#include <otawa/ilp.h>
#include <otawa/ipet/features.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/proc/ProcessorPlugin.h>

namespace otawa { namespace branch {

/**
 * @defgroup branch branch Plugin
 *
 * This plugins implements the analysis for branch predictor proposed in paper below:
 *
 * A. Colin, I. Puaut.
 * Worst case execution time analysis for a processor with branch prediction
 * (BTB with 2-bit counter and saturation).
 * Real-Time Systems, Special issue on worst-case execution time analysis, 2000.
 *
 * @par Category Building
 *
 * The feature CATEGORY_FEATURE assigns a category to each branch that can be used to evaluate
 * a bound on the number of miss-predictions. The supported categories are:
 * @li @ref ALWAYS_D -- never present in the BTB and default policy applies (taken),
 * @li @ref ALWAYS_H -- always present in the BTB,
 * @li @ref FIRST_UNKNOW -- always present in the BTB except for the first iteration,
 * @li @ref NOT_CLASSIFIED -- none of the previous categories applies.
 *
 * @par Constraint Building
 *
 * The feature CONSTRAINTS_FEATURE generates ILP constraints to bound
 * the number of mispredictions. Let:
 * @li x_i -- the number of execution of basic block i on WCET path,
 * @li x^mp_i -- the number of misprediction for basic block i,
 * @li e^nt_i -- number of execution of not-taken edge from i,
 * @li e^t_i -- number of execution of taken edge from i,
 * @li x_h -- number of execution of header associated with FIRST_UNKNOWN.
 * The following constraints are generated:
 * @li 0 <= x^mp_i <= x_i
 * @li x^mp_i = x^nt_i				if category is ALWAYS_D
 * @li x^mp_i <= 2 e^t_i + 2		if category is ALWAYS_H
 * @li x^mp_i <= 2 e^nt_i + 2		if category is ALWAYS_H
 * @li x^mp_i <= 2 e^t_i + 2 x_h	if category is FIRST_UNKOWN
 * @li x^mp_i <= 2 e^nt_i + 2 x_h	if category is FIRST_UNKOWN
 *
 * @par WCET Contribution
 *
 * The feature SUPPORT_FEATURE increase the WCET objective function
 * with the rough evaluation of misprediction cost in cycles dependeing on
 * the branch type: direct conditional, indirect inconditional and conditional.
 * The term below is added to the objective function:
 * @li x^mp_i * t_mp with t_mp the cost of misprediction (from BHT configuration).
 *
 * @par Events
 *
 * Finally, EVENT_FEATURE ensures that the misprediction has been translated
 * as event object as used by the @ref etime plugin and stored on basic block
 * to be used for event-based WCET computation.
 *
 * Several events are created and put on the edges according two
 * policies, taken (T) and not-taken(T).
 *
 * In the simpler case, ALWAYS_D, the occurrence is:
 * @li NEVER on the default prediction direction,
 * @li ALWAYS in the reverse case (bounded the edge occurrences).
 *
 * For others categories, one variable for each edge needs to be created
 * that will be abstracted by x_T (taken) and x_NT (not-taken) and, respectively,
 * x^mp_T and x^mp_NT for misprediction number. The following constraints are added:
 * @li x^mp_T + x^mp_NT = x^mp_i
 * @li x^mp_T <= x_T	(no more mispredictions than edge taken),
 * @li x^mp_NT <= x_NT
 *
 *
 * @par Hardware Domain
 * It applies only on branch predictors made of a BTB with a 2-bit counter with saturation.
 *
 * @par Configuration
 * Misprediction costs are taken from @ref otawa::hard::BHT.
 *
 * @par Plugin Information
 * @li name: otawa/branch
 * @li header: <otawa/branch/features.h>
 *
 */

class Manager;
static Identifier<Manager *> MANAGER("", 0);
static Identifier<ilp::Constraint *> GLOBAL_CONSTRAINT("", 0);

#if 0
/**
 * Manager for events for the whole workspace.
 */
class Manager: public BBCleaner {
public:
	Manager(WorkSpace *ws, bool _exp)
		: BBCleaner(ws), sys(ipet::SYSTEM(ws)), exp(_exp), bht(hard::BHT_CONFIG(ws))
	{
		ASSERT(sys);
		ASSERT(bht);
	}

	~Manager(void) { BBCleaner::clean(); }

	/**
	 * Check if the default prediction policy matches the given edge.
	 * @param e		Considered loop.
	 * @return		True if the edge is well predicted, false else.
	 */
	bool checkDefaultPred(Edge *e) {
		int pred = bht->getDefaultPrediction();
		if(pred == hard::PREDICT_DIRECT) {
			if(e->sink()->address() <= e->source()->address())
				pred = hard::PREDICT_TAKEN;		// backward branch
			else
				pred = hard::PREDICT_NOT_TAKEN;	// forward branch
		}
		return	(pred == hard::PREDICT_TAKEN && e->isTaken())
			||	(pred == hard::PREDICT_NOT_TAKEN && e->isNotTaken());
	}

	/**
	 * Compute the occurrence type of the given edge.
	 * @return	Occurrence type.
	 */
	etime::occurrence_t occurrence(Edge *e) {
		switch(CATEGORY(e->source())) {
		case ALWAYS_D:
			if(checkDefaultPred(e))
				return etime::NEVER;
			else
				return etime::ALWAYS;
		default:
			return etime::SOMETIMES;
		}
	}

	/**
	 * Compute the cost of a misprediction for the given edge.
	 * @param e		Edge to process.
	 * @return		Misprediction cost in cycles.
	 */
	ot::time cost(Edge *e) {
		BasicBlock *bb = e->source()->toBasic();
		Inst *i = bb->control();
		ASSERT(i);
		if(i->target())
			return bht->getCondPenalty();
		else if(i->isConditional())
			return bht->getCondIndirectPenalty();
		else
			return bht->getIndirectPenalty();
	}

	/**
	 * Add new contribution for misprediction counting.
	 * @param edge	Edge of the event.
	 * @return		Matching variable.
	 */
	ilp::Var *add(Edge *edge) {

		// future constraint: (x^mp_T) + (x^mp_NT) <= x^mp_i
		ilp::Constraint *c = GLOBAL_CONSTRAINT(edge->source());
		if(!c) {
			static string label("branch: total misprediction bound");
			c = sys->newConstraint(label, ilp::Constraint::LE, 0.);
			c->addRight(1., MISSPRED_VAR(edge->source()));
		}

		// build the variable
		string name;
		if(exp)
			name = _ << "x^mp_" << (edge->isTaken() ? "T" : "NT")
					 << '_' << edge->source()->index() << '_' << edge->sink()->index()
					 << '_' << edge->source()->cfg()->label();
		ilp::Var *x_mp = sys->newVar(name);
		c->addLeft(1., x_mp);
		return x_mp;
	}

protected:
	virtual void clean(WorkSpace *ws, CFG *cfg, Block *bb) {
		bb->removeProp(GLOBAL_CONSTRAINT);
	}

private:
	ilp::System *sys;
	bool exp;
	hard::BHT *bht;
};


/**
 * Event to take into account branch mispredictions.
 * @ingroup branch
 */
class Event: public etime::Event {
public:
	Event(Manager& _m, Edge *_e, Inst *i): etime::Event(i), m(_m), e(_e), x_mp(0) { }
	virtual otawa::etime::kind_t kind(void) const { return otawa::etime::BRANCH; }
	virtual ot::time cost(void) const { return m.cost(e); }
	virtual etime::type_t type(void) const { return otawa::etime::EDGE; }
	virtual etime::occurrence_t occurrence(void) const { return m.occurrence(e); }
	virtual cstring name(void) const { return "branch misprediction"; }
	virtual string detail(void) const { return _ << "midsprediction of " << inst() << " along edge " << e; }
	virtual bool isEstimating(bool on) { return on; }
	virtual void estimate(ilp::Constraint *cons, bool on) {
		ASSERT(on);
		if(!x_mp)
			x_mp = m.add(e);
		cons->addRight(1., x_mp);
	}

private:
	Manager& m;
	Edge *e;
	ilp::Var *x_mp;
};


/**
 * Event builder for @ref branch plugin.
 *
 * @ingroup branch
 */
class EventBuilder: public BBProcessor {
public:
	static p::declare reg;
	EventBuilder(p::declare& r = reg): BBProcessor(r), man(0), _exp(false) { }

protected:
	virtual void setup(WorkSpace *ws) {
		man = new Manager(ws, _exp);
		this->track(EVENT_FEATURE, MANAGER(ws) = man);
	}

	virtual void processBB(WorkSpace *ws, CFG *cfd, Block *b) {
		if(CATEGORY(b) != UNDEF)
			for(Block::EdgeIter e = b->outs(); b; b++)
				etime::EVENT(b).add(new Event(*man, e, b->toBasic()->control()));
	}

private:
	Manager *man;
	bool _exp;
};

/**
 */
p::declare EventBuilder::reg = p::init("otawa::branch::EventBuilder", Version(1, 0, 0))
	.require(CONSTRAINTS_FEATURE)
	.require(hard::BHT_FEATURE)
	.require(ipet::ILP_SYSTEM_FEATURE)
	.provide(EVENT_FEATURE)
	.base(BBProcessor::reg)
	.maker<EventBuilder>();


/**
 * This feature ensures that the misprediction evaluation has been
 * translated as events as used by @ref etime plugin. Concretely, for each
 * basic block where a branch prediction arises, it add an EVENT property
 * describing the event.
 *
 * @par Properties
 * @li @ref otawa::etime::EVENT
 *
 * @ingroup branch
 */
p::feature EVENT_FEATURE("otawa::branch::EVENT_FEATURE", new Maker<EventBuilder>());
#endif

class Plugin: public ProcessorPlugin {
public:
	Plugin(void): ProcessorPlugin("otawa::branch", Version(2, 0, 0), OTAWA_PROC_VERSION) { }
};

} }		// otawa::cg

otawa::branch::Plugin otawa_branch;
ELM_PLUGIN(otawa_branch, OTAWA_PROC_HOOK);
