/*
 *	$Id$
 *	Virtualize class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-15, IRIT UPS.
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

#include <elm/io.h>
#include <otawa/cfg.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/cfg/Virtualizer.h>
#include <otawa/ipet/FlowFactLoader.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/prog/Manager.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/prop/info.h>
#include "../../include/otawa/flowfact/FlowFactLoader.h"

using namespace otawa;
using namespace elm;

namespace otawa {


/* Used for resolving recursive calls as loops */
typedef struct call_t {
        struct call_t *back;
        CFG *cfg;
        CFGMaker *maker;
} call_t;


/**
 * Put on the first instruction of a function to indicate whether it should be
 * inlined or not during virtualization.
 * This overrides @ref VIRTUAL_INLINING default policy of @ref Virtualizer
 * and @ref INLINING_POLICY of the caller CFG.
 * @li @ref FLOW_FACTS_FEATURE
 * @par Hooks
 * @li @ref Inst
 * @ingroup cfg
 */
Identifier<bool> NO_INLINE("otawa::NO_INLINE");


/**
 * Put on the first instruction of a function to set default inlining behavior
 * during its virtualization.
 * This overrides @ref VIRTUAL_INLINING default policy of @ref Virtualizer.
 * @li @ref FLOW_FACTS_FEATURE
 * @par Hooks
 * @li @ref Inst
 * @ingroup cfg
 */
Identifier<bool> INLINING_POLICY("otawa::INLINING_POLICY");


/**
 * Configuration property of @ref Virtualizer: it set the default behavior for
 * inlining of function call during virtualization (default to true).
 * The default behavior can be overridden by @ref NO_INLINE
 *
 * @par Hooks
 * @li Configuration of @ref Virtualizer code processor.
 */
Identifier<bool> VIRTUAL_DEFAULT("otawa::VIRTUAL_DEFAULT", true);



/**
 * This features only show that the CFG has been virtualized. This may implies
 * a lot of transformation like function call inlining or loop unrolling.
 *
 * @par Header
 * <otawa/cfg/features.h>
 *
 * @par Configuration
 * @li @ref NO_INLINE
 * @li @ref INLINING_POLICY
 *
 * @par Properties
 * @li @ref RECURSIVE_LOOP
 *
 */
p::feature VIRTUALIZED_CFG_FEATURE("otawa::VIRTUALIZED_CFG_FEATURE", new Maker<Virtualizer>());


/**
 * A property with this identifier is hooked to edge performing a recursive
 * call when inlining is used.
 *
 * @par Hooks
 * @li @ref Edge
 *
 * @par Features
 * @li @ref VIRTUALIZED_CFG_FEATURE
 */
Identifier<bool> RECURSIVE_LOOP("otawa::RECURSIVE_LOOP", false);


/**
 * @class Virtualizer
 *
 * This processor inlines the function calls.
 *
 * @par Configuration
 * @li @ref VIRTUAL_DEFAULT
 *
 * @par Required features
 * @li @ref FLOW_FACTS_FEATURE
 *
 * @par Invalidated features
 * @li @ref COLLECTED_CFG_FEATURE
 *
 * @par Provided features
 * @li @ref VIRTUALIZED_CFG_FEATURE
 * @li @ref COLLECTED_CFG_FEATURE
 *
 * @par Statistics
 * none
 */


/**
 */
Virtualizer::Virtualizer(void)
	: CFGProvider(reg), virtualize(false), entry(nullptr)
	{ }

// Registration
p::declare Virtualizer::reg = p::init("otawa::Virtualizer", Version(2, 1, 0))
	.maker<Virtualizer>()
	.extend<CFGProvider>()
	.use(COLLECTED_CFG_FEATURE)
	.invalidate(COLLECTED_CFG_FEATURE)
	.provide(VIRTUALIZED_CFG_FEATURE);


/**
 */
void Virtualizer::processWorkSpace(otawa::WorkSpace *fw) {

	// get the entry CFG
	const CFGCollection *old_coll = INVOLVED_CFGS(fw);
	entry = old_coll->get(0);

	// virtualize the entry CFG
	todo.push(entry);
	while(todo)
		makeCFG(0, todo.pop(), none);

}


/**
 */
void Virtualizer::configure(const PropList &props) {
	entry = ENTRY_CFG(props);
	virtualize = VIRTUAL_DEFAULT(props);
	CFGProvider::configure(props);
}


/**
 * Test if inlining has been activated on @param target CFG within
 * @param source CFG.
 * @return	True if inlining is activated, false else.
 */
bool Virtualizer::isInlined(CFG *cfg, Option<int> local_inlining, ContextualPath &path) {
	Inst *inst = cfg->first();

	// First look if inlining is requested for the CFG
	if (path(NO_INLINE, inst).exists())
		return !path.get(NO_INLINE, inst);

	// Then check if local inlining policy is set
	if (local_inlining.isOne())
		return local_inlining.value();

	// Finally decide based on global inlining policy
	return virtualize;
}


/**
 * Build the virtual CFG.
 * @param stack		Stack to previous calls.
 * @param cfg		CFG to inline into.
 * @param entry		Basic block performing the call.
 * @param exit		Basic block after the return.
 */
void Virtualizer::make(struct call_t *stack, CFG *cfg, CFGMaker& maker, elm::Option<int> local_inlining, ContextualPath &path) {
	ASSERT(cfg);
	CONTEXT(maker) = path;

	// preparation
	HashMap<Block *, Block *> bmap;
	call_t call = { stack, cfg, &maker };
	if(logFor(LOG_CFG))
		log << "\tbegin inlining " << cfg->label() << io::endl;
	if(path(INLINING_POLICY, cfg->first()).exists())
		local_inlining = path.get(INLINING_POLICY, cfg->first());
	CONTEXT(maker) = path;

	// add initial blocks
	bmap.put(cfg->entry(), maker.entry());
	bmap.put(cfg->exit(), maker.exit());
	if(cfg->unknown())
		bmap.put(cfg->unknown(), maker.unknown());

	// add other blocks
	for(CFG::BlockIter v = cfg->blocks(); v(); v++)

		// process end block
		if(v->isVirtual()) {
			if(v->isUnknown())
				bmap.put(cfg->unknown(), maker.unknown());
			else if(v->isPhony()) {
				Block *nv = new PhonyBlock();
				maker.add(nv);
				bmap.add(*v, nv);
			}
			else
				continue;
		}

		// process basic block
		else if(v->isBasic()) {
			BasicBlock *bb = v->toBasic();
			Vector<Inst *> insts(bb->count());
			for(BasicBlock::InstIter i = bb->insts(); i(); i++)
				insts.add(*i);
			BasicBlock *nv = new BasicBlock(insts.detach());
			maker.add(nv);
			bmap.add(*v, nv);
		}

		// process synthetic block
		else {

			// build synth block
			SynthBlock *sb = v->toSynth();
			SynthBlock *nsb = new SynthBlock();
			bmap.put(sb, nsb);

			// link with callee
			if(!sb->callee())
				maker.add(nsb);
			else if(isInlined(cfg, local_inlining, path)) {

				// recursive call case
				bool rec = false;
				for(struct call_t *c = &call; c; c = c->back)
					if(c->cfg == sb->callee()) {
						maker.call(nsb, *c->maker);
						rec = true;
						break;
					}

				// non-recursive call
				if(!rec) {

					// prepare context
					Inst *calli = sb->callInst();
					if(cfg->type() == CFG::SUBPROG)
						path.push(ContextualStep(ContextualStep::FUNCTION, cfg->address()));
					if(calli)
						path.push(ContextualStep(ContextualStep::CALL, calli->address()));

					// build CFG
					CFGMaker& cmaker = newMaker(sb->callee()->first());
					make(&call, sb->callee(), cmaker, local_inlining, path);
					maker.call(nsb, cmaker);

					// pop context
					if(calli)
						path.pop();
					if(cfg->type() == CFG::SUBPROG)
						path.pop();
				}

			}
			else {
				todo.push(sb->callee());
				maker.call(nsb, makerOf(sb->callee()));
			}

		}

	// add edges
	for(CFG::BlockIter v = cfg->blocks(); v(); v++)
		for(BasicBlock::EdgeIter e = v->outs(); e(); e++) {
			auto flags = e->flags();
			Block *nsrc = bmap.get(e->source());
			Block *nsnk = bmap.get(e->sink());
			if((flags & (Edge::CALL | Edge::RETURN)) != 0) {
				if((flags & Edge::CALL) != 0 && !nsnk->isSynth())
					flags &= ~Edge::CALL;
				if((flags & Edge::RETURN) != 0 && !nsrc->isSynth())
					flags &= ~Edge::RETURN;
			}
			maker.add(nsrc, nsnk, new Edge(flags));
		}

	// leaving call
	if(logFor(LOG_CFG))
		log << "\tend inlining " << cfg->label() << io::endl;
}

/**
 * Virtualize a CFG and add it to the cfg map.
 * @param call	Call string.
 * @param cfg	CFG to virtualize.
 */
void Virtualizer::makeCFG(struct call_t *call, CFG *cfg, Option<int> local_inlining) {
	ContextualPath path;
	make(call, cfg, makerOf(cfg), local_inlining, path);
}


/**
 * Obtain the maker for a particular CFG.
 * @param cfg	CFG to look a maker for.
 * @return		Associated CFG maker.
 */
CFGMaker& Virtualizer::makerOf(CFG *cfg) {
	CFGMaker *r = map.get(cfg, 0);
	if(!r) {
		r = &newMaker(cfg->first());
		map.put(cfg, r);
	}
	return *r;
}


/**
 * Build a new maker for a CFG (an inlined CFG).
 * @param 	First instruction of CFG.
 * @return	Built maker.
 */
CFGMaker& Virtualizer::newMaker(Inst *first) {
	CFGMaker *m = new CFGMaker(first);
	makers.add(m);
	return *m;
}


/**
 */
void Virtualizer::cleanup(WorkSpace *ws) {
	auto coll = new CFGCollection();
	for(FragTable<CFGMaker *>::Iter m(makers); m(); m++) {
		coll->add(m->build());
		delete *m;
	}
	setCollection(coll);
}

} /* end namespace */
