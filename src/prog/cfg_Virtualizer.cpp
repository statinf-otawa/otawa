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

using namespace otawa;
using namespace elm;



namespace otawa {


/* Used for resolving recursive calls as loops */
typedef struct call_t {
        struct call_t *back;
        CFG *cfg;
        BasicBlock *entry;
} call_t;


static SilentFeature::Maker<Virtualizer> VIRTUALIZED_CFG_MAKER;
/**
 * This features only show that the CFG has been virtualized. This may implies
 * a lot of transformation like function call inlining or loop unrolling.
 *
 * @par Properties
 * @li @ref CALLED_CFG
 * @li @ref RECURSIVE_LOOP
 * @li @ref VIRTUAL_RETURN_BLOCK
 *
 */
SilentFeature VIRTUALIZED_CFG_FEATURE("otawa::VIRTUALIZED_CFG_FEATURE", VIRTUALIZED_CFG_MAKER);


/**
 * This property is put on a BB performing a function call that has been virtualized (inlined
 * in the current CFG). It gives the BB after the return of the inlined CFG. It is useful
 * to jump over inlinved CFG.
 *
 * @par Hooks
 * @li @ref BasicBlock
 */
Identifier<BasicBlock*> VIRTUAL_RETURN_BLOCK("otawa::VIRTUAL_RETURN_BLOCK", NULL);


/**
 * A property with this identifier is hooked at the edge performing a virtual
 * call when inling is used. The associated value is the CFG of the called
 * function.
 *
 * @par Hooks
 * @li @ref Edge
 */
Identifier<CFG *> CALLED_CFG("otawa::CALLED_CFG", 0,
	make(DEF_BY, (const AbstractFeature *)&VIRTUALIZED_CFG_FEATURE),
	0);


/**
 * A property with this identifier is hooked to edge performing a recursive
 * call when inlining is used.
 *
 * @par Hooks
 * @li @ref Edge
 */
Identifier<bool> RECURSIVE_LOOP("otawa::RECURSIVE_LOOP", false);


/**
 * @class Virtualizer
 *
 * This processor inlines the function calls.
 *
 * @par Configuration
 * @li @ref VIRTUAL_INLINING
 * @li @ref DONT_INLINE
 *
 * @par Required features
 * @li @ref FLOW_FACTS_FEATURE
 *
 * @par Invalidated features
 * @li @ref COLLECTED_CFG_FEATURE
 *
 * @par Provided features
 * @li @ref VIRTUALIZED_CFG_FEATURE
 *
 * @par Statistics
 * none
 */


/**
 * Configuration property of @ref Virtualizer: it activates the inlining of function call
 * during virtualization (default to true).
 *
 * @par Hooks
 * @li Configuration of @ref Virtualizer code processor.
 */
Identifier<bool> VIRTUAL_INLINING("otawa::VIRTUAL_INLINING", true);


/**
 * This property tells the VirtualCFG to not inline a call to a function.
 *
 * @par Hooks
 * @li @ref CFG
 */
Identifier<bool> DONT_INLINE("otawa::DONT_INLINE", false);

/**
 */
Virtualizer::Virtualizer(void): Processor(reg) {
}

// Registration
Registration<Virtualizer> Virtualizer::reg(
	"otawa::Virtualizer",
	Version(1, 1, 0),
	p::use, &COLLECTED_CFG_FEATURE,
	p::invalidate, &COLLECTED_CFG_FEATURE,
	p::provide, &VIRTUALIZED_CFG_FEATURE,
	p::end
);


void Virtualizer::processWorkSpace(otawa::WorkSpace *fw) {

	const CFGCollection *coll = INVOLVED_CFGS(fw);
	VirtualCFG *vcfg = new VirtualCFG(false);
       /* if (!entry)
        	entry = ENTRY_CFG(fw);
        if(!entry) {
                CFGInfo *info = fw->getCFGInfo();
                CString name = TASK_ENTRY(fw);
                entry = info->findCFG(name);
        }
        if(!entry)
                throw ProcessorException(*this, "cannot find task entry point.");*/
	entry = coll->get(0);

	vcfg->addProps(*entry);
	vcfg->entry()->addProps(*entry->entry());
	vcfg->exit()->addProps(*entry->exit());

	virtual_inlining = VIRTUAL_INLINING(fw);
	vcfg->addBB(vcfg->entry());
	virtualize(0, entry, vcfg, vcfg->entry(), vcfg->exit());
	vcfg->addBB(vcfg->exit());
	vcfg->numberBBs();
	if(isVerbose())
		log << "\tINFO: " << vcfg->countBB() << " basic blocks." << io::endl;

	track(VIRTUALIZED_CFG_FEATURE, ENTRY_CFG(fw) = vcfg);
}

void Virtualizer::configure(const PropList &props) {
	entry = ENTRY_CFG(props);
	Processor::configure(props);
}

bool Virtualizer::isInlined() {
	return(virtual_inlining);
}


/**
 * Build the virtual CFG.
 * @param stack		Stack to previous calls.
 * @param cfg		CFG to develop.
 * @param ret		Basic block for returning.
 */
void Virtualizer::virtualize(struct call_t *stack, CFG *cfg, VirtualCFG *vcfg, BasicBlock *entry,
BasicBlock *exit) {
	assert(cfg);
	assert(entry);
	assert(exit);
	//cout << "Virtualizing " << cfg->label() << "(" << cfg->address() << ")\n";

	// Prepare data
	elm::genstruct::HashTable<void *, BasicBlock *> map;
	call_t call = { stack, cfg, 0 };
	Vector<CFG *> called_cfgs;
	if(isVerbose())
		log << "\tbegin inlining " << cfg->label() << io::endl;

	// Translate BB
	for(CFG::BBIterator bb(cfg); bb; bb++)
		if(!bb->isEntry() && !bb->isExit()) {
			BasicBlock *new_bb = new VirtualBasicBlock(bb);
			map.put(bb, new_bb);
			vcfg->addBB(new_bb);
		}

	// Find local entry
	for(BasicBlock::OutIterator edge(cfg->entry()); edge; edge++) {
		ASSERT(edge->kind() == Edge::VIRTUAL);
		ASSERT(!call.entry);
		call.entry = map.get(edge->target(), 0);
		ASSERT(call.entry);
		Edge *edge2 = new Edge(entry, call.entry, Edge::VIRTUAL_CALL);
		CALLED_CFG(edge2) = cfg;
	}

	// Translate edges
	for(CFG::BBIterator bb(cfg); bb; bb++)
		if(!bb->isEntry() && !bb->isExit()) {
			if(isVerbose())
				cerr << "\t\tprocessing " << *bb << io::endl;

			// Resolve source
			BasicBlock *src = map.get(bb, 0);
			assert(src);

			BasicBlock *called_exit = 0;
			if(isInlined())
				for(BasicBlock::OutIterator edge(bb); edge; edge++)
					if(edge->kind() == Edge::CALL && edge->calledCFG()) {
						if (DONT_INLINE(edge->calledCFG()))  {
							if ((cfgMap.get(edge->calledCFG(), 0) == 0)) {
								VirtualCFG *vcalled = new VirtualCFG(false);
								cfgMap.put(edge->calledCFG(), vcalled);
								vcalled->addProps(*edge->calledCFG());
								ENTRY(vcalled->entry()) = vcalled;
								vcalled->entry()->addProps(*edge->calledCFG()->entry());
								vcalled->exit()->addProps(*edge->calledCFG()->exit());
								vcalled->addBB(vcalled->entry());
								virtualize(&call, edge->calledCFG(), vcalled, vcalled->entry(), vcalled->exit());
								vcalled->addBB(vcalled->exit());
								vcalled->numberBB();
							}
						}
						else
							called_cfgs.add(edge->calledCFG());
					}

			// Look edges
			bool fix_exit = false;
			for(BasicBlock::OutIterator edge(bb); edge; edge++)
				if(edge->kind() == Edge::CALL) {
					if(!isInlined()) {
						new Edge(src, edge->target(), Edge::CALL);
					}
					if (isInlined() && edge->calledCFG() && DONT_INLINE(edge->calledCFG())) {
						VirtualCFG *vcalled = cfgMap.get(edge->calledCFG(), 0);
						ASSERT(vcalled != 0);
						new Edge(src, vcalled->entry(), Edge::CALL);
					}
				}
				else if(edge->target()) {
					if(edge->target()->isExit()) {
						called_exit = exit;
						if(!called_cfgs) {
							Edge *edge = new Edge(src, exit, Edge::VIRTUAL_RETURN);
							CALLED_CFG(edge) = cfg;
						}
						else // shared return edge will be added afterwards
							fix_exit = true;
					}
					else {
						BasicBlock *tgt = map.get(edge->target(), 0);
						ASSERT(tgt);
						if(called_cfgs)
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
					bool recursive = false;
					for(call_t *cur = &call; cur; cur = cur->back)
						if(cur->cfg == called) {
							recursive = true;
							Edge *edge = new Edge(map.get(bb), cur->entry, Edge::VIRTUAL_CALL);
							CALLED_CFG(edge) = cur->cfg;
							RECURSIVE_LOOP(edge) = true;
							VIRTUAL_RETURN_BLOCK(src) = called_exit;
							if(isVerbose())
								out << "INFO: recursivity found at " << bb->address()
									<< " to " << called->label() << io::endl;
							break;
						}

					// Virtualize the called CFG
					if(!recursive) {
						ASSERT(called_exit);
						VIRTUAL_RETURN_BLOCK(src) = called_exit;
						virtualize(&call, called, vcfg, src, called_exit);
						if(fix_exit)
							for(BasicBlock::InIterator vin(called_exit); vin; vin++) {
								for(Identifier<CFG *>::Getter found(vin, CALLED_CFG); found; found++)
									if(*called == *found)
										CALLED_CFG(vin).add(cfg);
							}
					}
				}

				// Reset the called list
				called_cfgs.clear();
			}
		}

	if(isVerbose())
		log << "\tend inlining " << cfg->label() << io::endl;
}


} /* end namespace */
