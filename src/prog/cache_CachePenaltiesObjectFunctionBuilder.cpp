/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ipet/BasicConstraintsBuilder.h -- BasicConstraintsBuilder class implementation.
 */


#include <otawa/ipet/IPET.h>
#include <otawa/cfg.h>
#include <otawa/ilp.h>
#include <otawa/cache/CachePenaltiesObjectFunctionBuilder.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/ipet/VarAssignment.h>
#include <otawa/ipet/TrivialBBTime.h>
#include <otawa/ipet/ILPSystemGetter.h>
#include <otawa/util/Dominance.h>

using namespace otawa::ilp;

namespace otawa { namespace ipet {

Registration<CachePenaltiesObjectFunctionBuilder> CachePenaltiesObjectFunctionBuilder::reg(
	"otawa::ipet::CachePenaltiesObjectFunctionBuilder",
	Version(1, 0, 1),
	p::require, &ASSIGNED_VARS_FEATURE,
	p::end
);

/**
 * @class CachePenaltiesObjectFunctionBuilder
 * This processor is used for building the basic object function function
 * in case of a L1 instruction cache.
 * @p Then, it set the object function as maximizing the following expression:
 * @p	t1 * n1 + t2 * n2 + ... + tm * nm
 * @p where
 * <dl>
 * 	<dt>tk</dt><dd>Time of execution of basic block k.</dd>
 *  <dt>nk</dt><dd>Count of execution if basic block k.</dd>
 * </dl>
 *
 * @par Required Features
 * @li @ref ipet::ILP_SYSTEM_FEATURE
 * @li @ref ipet::ASSIGNED_VARS_FEATURE
 * @li @ref ipet::BB_TIME_FEATURE
 *
 * @par Provided Features
 * @li @ref ipet::OBJECT_FUNCTION_FEATURE
 */


/**
 * Build a new basic object function builder.
 */
CachePenaltiesObjectFunctionBuilder::CachePenaltiesObjectFunctionBuilder(void)
: BBProcessor(reg) {
}


/**
 */
void CachePenaltiesObjectFunctionBuilder::configure(const PropList& props) {
	BBProcessor::configure(props);
	_explicit = EXPLICIT(props);
}


/**
 */
void CachePenaltiesObjectFunctionBuilder::processBB(
	WorkSpace *fw,
	CFG *cfg,
	BasicBlock *bb)
{
	
	// do not process entry and exit
	if(bb->isEntry() || bb->isExit())
		return;

	// get system
	System *system = SYSTEM(fw);
	
	// examine each input
	for (BasicBlock::InIterator edge(bb); edge; edge++) {
		CachePenalty * cache_penalty = ICACHE_PENALTY(edge);
		
		// process a cache penalty
		if (cache_penalty) {
			
			// only one header: add to objective function: t_miss * x_(h, j)
			if (!cache_penalty->header(1)){
				ASSERT(cache_penalty->header(0));
				for (BasicBlock::InIterator h_edge(cache_penalty->header(0)) ; h_edge ; h_edge++)
					if (!Dominance::isBackEdge(h_edge))
						system->addObjectFunction(cache_penalty->penalty(CachePenalty::MISS), VAR(h_edge));
			}
			
			// more than one
			else {
				ilp::Var *entry, *loop;
				
				// generate variables: x_entry (entrying edges count), x_loop(looping edges count)
				if(!_explicit){
					entry = system->newVar();
					loop = system->newVar();;
				}
				else {
					BasicBlock *h0 = cache_penalty->header(0);
					StringBuffer buf1, buf2;
					buf1 << "XENTRY_b" << h0->number();
					String name1 = buf1.toString();
					entry = system->newVar(name1);
					buf2 << "XLOOP_b" << h0->number();
					String name2 = buf2.toString();
					loop = system->newVar(name2);
				}
				
				// c1: x_entry = sum{e_h0i not in back(h0)} e_h0i
				Constraint *cons_entry = system->newConstraint("double FM header: sum of entering", Constraint::EQ, 0);
				cons_entry->addLeft(1, entry);
				
				// c2: x_loop = sum{e_h0i not in back(h0)} e_h0i - sum{e_h0i not in back(h0)} e_h0i
				Constraint *cons_loop = system->newConstraint("double FM header: sum of entering and backing", Constraint::EQ, 0);
				cons_loop->addLeft(1, loop);
				for (BasicBlock::InIterator edge(cache_penalty->header(0)) ; edge ; edge++){
					if (!Dominance::isBackEdge(edge)){
						cons_entry->addRight(1, VAR(edge));
						cons_loop->addRight(-1, VAR(edge));
					}
					else {
						cons_loop->addRight(1, VAR(edge));
					}
				}
				
				// add to objective function: + x_entry * t_miss + x_loop * t_hit
				system->addObjectFunction(cache_penalty->penalty(CachePenalty::MISS_MISS), entry);
				system->addObjectFunction(cache_penalty->penalty(CachePenalty::HIT_MISS), loop);

				BasicBlock *h1 = cache_penalty->header(1);
				if(!_explicit){
					entry = system->newVar();
					loop = system->newVar();;
				}
				else {
					StringBuffer buf1, buf2;
					buf1 << "XENTRY_b" << h1->number();
					String name1 = buf1.toString();
					entry = system->newVar(name1);
					buf2 << "XLOOP_b" << h1->number();
					String name2 = buf2.toString();
					loop = system->newVar(name2);
				}
				cons_loop = system->newConstraint("double FM header: sum of entering", Constraint::EQ,0);
				cons_loop->addLeft(1, loop);
				for (BasicBlock::InIterator edge(cache_penalty->header(1)) ; edge ; edge++){
					if (!Dominance::isBackEdge(edge)){
						cons_loop->addRight(-1, VAR(edge));
					}
					else {
						cons_loop->addRight(1, VAR(edge));
					}
				}
				system->addObjectFunction(cache_penalty->penalty(CachePenalty::x_HIT), loop);
			}
		}
	}
}


} } // otawa::ipet
