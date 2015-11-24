/*
 *	$Id$
 *	Virtualize class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-09, IRIT UPS.
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
#include <otawa/prop/info.h>
#include <otawa/util/FlowFactLoader.h>

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
Virtualizer::Virtualizer(void): Processor(reg), virtualize(false), entry(0), coll(0) {
}

// Registration
p::declare Virtualizer::reg = p::init("otawa::Virtualizer", Version(2, 0, 0))
	.maker<Virtualizer>()
	.use(COLLECTED_CFG_FEATURE)
	.invalidate(COLLECTED_CFG_FEATURE)
	.provide(VIRTUALIZED_CFG_FEATURE)
	.provide(COLLECTED_CFG_FEATURE);


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
	Processor::configure(props);
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
void Virtualizer::make(struct call_t *stack, CFG *cfg, CFGMaker *maker, elm::Option<int> local_inlining, ContextualPath &path) {
	ASSERT(cfg);
	genstruct::HashTable<Block *, Block *> bmap;

	// add initial blocks
	bmap.put(cfg->entry(), maker->entry());
	bmap.put(cfg->exit(), maker->exit());
	if(cfg->unknown())
		bmap.put(cfg->unknown(), maker->unknown());

	// add other blocks
	for(CFG::BlockIter v = cfg->blocks(); v; v++)

		// process end block
		if(v->isEnd())
			continue;

		// process basic block
		else if(v->isBasic()) {
			BasicBlock *bb = v->toBasic();
			genstruct::Vector<Inst *> insts(bb->count());
			for(BasicBlock::InstIter i = bb->insts(); i; i++)
				insts.add(i);
			maker->add(new BasicBlock(insts.detach()));
		}

		// process synthetic block
		else {

		}

	// add edges
	for(CFG::BlockIter v = cfg->blocks(); v; v++)
		for(BasicBlock::EdgeIter e = v->outs(); e; e++)
			maker->add(bmap.get(e->source()), bmap.get(e->sink()), new Edge());
}

/*void Virtualizer::virtualize(struct call_t *stack, CFG *cfg, VirtualCFG *vcfg,
		BasicBlock *entry, BasicBlock *exit, Option<int> local_inlining, ContextualPath &path) {
	ASSERT(cfg);
	ASSERT(entry);
	ASSERT(exit);

	// Prepare data
	elm::genstruct::HashTable<void *, BasicBlock *> map;
	call_t call = { stack, cfg, 0 };
	Vector<CFG *> called_cfgs;
	if(logFor(LOG_CFG))
		log << "\tbegin inlining " << cfg->label() << io::endl;

	// Translate BB
	for(CFG::BBIterator bb(cfg); bb; bb++)
		if(!bb->isEntry() && !bb->isExit()) {
			BasicBlock *new_bb = new VirtualBasicBlock(bb);
			map.put(bb, new_bb);
			vcfg->addBB(new_bb);
		}

	// Find local entry
	BasicBlock *to = NULL;
	for(BasicBlock::OutIterator edge(cfg->entry()); edge; edge++) {
		call.entry = map.get(edge->target(), 0);
		Edge *vedge = new Edge(entry, call.entry, Edge::VIRTUAL_CALL);
		CALLED_CFG(vedge) = cfg;
		to = edge->target();
	}
	ASSERT(to);
	enteringCall(entry, to, path);

	// Translate edges
	for(CFG::BBIterator bb(cfg); bb; bb++)
		if(!bb->isEnd()) {
			if(logFor(LOG_BB))
				cerr << "\t\tprocessing " << *bb << io::endl;

			// Update local inlining policy
			if(path(INLINING_POLICY, cfg->firstInst()).exists())
				local_inlining = path.get(INLINING_POLICY, cfg->firstInst());

			// Resolve source
			BasicBlock *src = map.get(bb, 0);
			ASSERT(src);

			// process call edges
			for(BasicBlock::OutIterator edge(bb); edge; edge++) {
				if(edge->kind() == Edge::CALL && edge->calledCFG()) {
					if(isInlined(edge->calledCFG(), local_inlining, path))
						called_cfgs.add(edge->calledCFG());
					else if(!cfgMap.exists(edge->calledCFG()))
						virtualizeCFG(&call, edge->calledCFG(), local_inlining);
				}
			}

			// generate the edges
			BasicBlock *called_exit = 0;
			bool fix_exit = false;
			for(BasicBlock::OutIterator edge(bb); edge; edge++)
				if(edge->kind() == Edge::CALL) {
					if(edge->calledCFG() && !isInlined(edge->calledCFG(), local_inlining, path)) {
						VirtualCFG *vcalled = cfgMap.get(edge->calledCFG(), 0);
						ASSERT(vcalled);
						Edge *vedge = new Edge(src, vcalled->entry(), Edge::CALL);
						CALLED_BY(vedge->calledCFG()).add(vedge);
					}
				}
				else if(edge->target()) {
					if(edge->target()->isExit()) {
						called_exit = exit;
						if(!called_cfgs) {
							Edge *edge = new Edge(src, exit, Edge::VIRTUAL_RETURN);
							RETURN_OF(src) = call.entry;
							CALLED_CFG(edge) = cfg;
						}
						else // shared return edge will be added afterwards
							fix_exit = true;
					}
					else {
						BasicBlock *tgt = map.get(edge->target(), 0);
						ASSERT(tgt);
						if(edge->kind() == Edge::NOT_TAKEN && called_cfgs)
							called_exit = tgt;
						else
							new Edge(src, tgt, edge->kind());
					}
				}

			// Process the call
			if(called_cfgs) {

				// Process each call
				for(Vector<CFG *>::Iterator called(called_cfgs); called; called++) {

					// Check recursivity
					call_t *rec = 0;
					for(call_t *cur = &call; cur; cur = cur->back)
						if(cur->cfg == called) {
							rec = cur;
							break;
						}

					// handle recursivity
					if(rec) {
						Edge *edge = new Edge(map.get(bb), rec->entry, Edge::VIRTUAL_CALL);
						CALLED_CFG(edge) = rec->cfg;
						RECURSIVE_LOOP(edge) = true;
						VIRTUAL_RETURN_BLOCK(src) = called_exit;
						new Edge(src, called_exit, Edge::NOT_TAKEN);
						if(logFor(LOG_CFG))
							out << "INFO: recursivity found at " << bb->address()
								<< " to " << called->label() << io::endl;
						break;
					}

					// virtualize the called CFG
					else {
						ASSERT(called_exit);
						VIRTUAL_RETURN_BLOCK(src) = called_exit;
						virtualize(&call, called, vcfg, src, called_exit, local_inlining, path);
						if(fix_exit)
							for(BasicBlock::InIterator vin(called_exit); vin; vin++)
								for(Identifier<CFG *>::Getter found(vin, CALLED_CFG); found; found++)
									if(*called == *found)
										CALLED_CFG(vin).add(cfg);
					}
				}

				// Reset the called list
				called_cfgs.clear();
			}
		}

	if(logFor(LOG_CFG))
		log << "\tend inlining " << cfg->label() << io::endl;
	leavingCall(exit, path);
}*/


/**
 * Virtualize a CFG and add it to the cfg map.
 * @param call	Call string.
 * @param cfg	CFG to virtualize.
 */
void Virtualizer::makeCFG(struct call_t *call, CFG *cfg, Option<int> local_inlining) {
	ContextualPath path;
	make(call, cfg, maker(cfg), local_inlining, path);
}


void Virtualizer::enteringCall(BasicBlock *caller, BasicBlock *callee, ContextualPath &path) {
	if (!caller->isEntry()) {
		Inst *call = caller->last();
		if (!call->isCall()) {
			for (BasicBlock::InstIter inst(caller); inst; inst++)
				if (inst->isControl())
					call = inst;
			if (!call)
				call = caller->last();
			ASSERT(call);
		}
		path.push(ContextualStep::CALL, call->address());
	}
	path.push(ContextualStep::FUNCTION, callee->address());
}


void Virtualizer::leavingCall(BasicBlock *to, ContextualPath& path) {
	path.pop();
	if (!to->isExit())
		path.pop();
}


/**
 * Obtain the maker for a particular CFG.
 * @param cfg	CFG to look a maker for.
 * @return		Associated CFG maker.
 */
CFGMaker *Virtualizer::maker(CFG *cfg) {
	CFGMaker *r = map.get(cfg);
	if(!r) {
		r = new CFGMaker(cfg->first());
		map.put(cfg, r);
	}
	return r;
}


/**
 */
void Virtualizer::cleanup(WorkSpace *ws) {

	// allocate the collection
	addDeletor(COLLECTED_CFG_FEATURE, INVOLVED_CFGS(ws) = coll);

	// get entry CFG
	CFGMaker *entry_maker = map.get(entry, 0);
	CFG *entry_vcfg = entry_maker->build();
	addRemover(VIRTUALIZED_CFG_FEATURE, ENTRY_CFG(ws) = coll->get(0));

	// release makers
	coll->add(entry_vcfg);
	for(genstruct::HashTable<CFG *, CFGMaker *>::Iterator vcfg(map); vcfg; vcfg++)
		delete *vcfg;
}


} /* end namespace */
