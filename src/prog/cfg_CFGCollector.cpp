/*
 *	$Id$
 *	CFGCollector class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006-08, IRIT UPS.
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

#include <elm/assert.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/cfg/features.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/prog/Manager.h>
#include <otawa/prog/TextDecoder.h>
#include <otawa/util/FlowFactLoader.h>

namespace otawa {

static Identifier<int> CFG_INDEX("", -1);
static Identifier<BasicBlock *> BB("", 0);


/**
 * @class CFGCollection <otawa/cfg/features.h>
 * Contains a collection of CFGs (used with @ref INVOLVED_CFGS property).
 * @see otawa::CFGCollector, otawa::INVOLVED_CFGS
 *
 * @ingroup cfg
 */

/**
 * @fn int CFGCollection::count(void) const;
 * Get the count of CFG in the collection.
 * @return CFG count.
 */

/**
 * @fn CFG *CFGCollection::get(int index) const;
 * Get a CFG from the collection using its index.
 * @param index	Index of the got CFG.
 * @return		CFG matching the index.
 */

/**
 * @fn CFG *CFGCollection::operator[](int index) const;
 * Shortcut to @ref get().
 */

/**
 * @class CFGCollection::Iterator
 * Iterator on the CFG contained in a @ref CFGCollection.
 */

/**
 * @fn CFGCollection::Iterator::Iterator(const CFGCollection *cfgs);
 * Build an iterator on the given CFG collection.
 * @param cfgs	CFG collection to iterate on.
 */

/**
 * @fn CFGCollection::Iterator(const CFGCollection& cfgs);
 * Build an iterator on the given CFG collection.
 * @param cfgs	CFG collection to iterate on.
 */

/**
 * Add a CFG to the collection.
 * @param cfg	Added CFG.
 */
void CFGCollection::add(CFG *cfg) {
	cfg->idx = cfgs.count();
	cfgs.add(cfg);
}


/**
 * @class CFGCollector
 * This processor is used to collect all CFG implied in a computation.
 * It uses the @ref otawa::ENTRY_CFG or @ref otawa::TASK_ENTRY properties to find
 * the base CFG and explore in depth the CFG along subprograms calls.
 *
 * @par Configuration
 * @li @ref ENTRY_CFG -- CFG of the entry of the current task,
 * @li @ref TASK_ENTRY -- name if the entry function of the current task,
 * @li @ref CFGCollector::ADDED_CFG -- CFG to add to the collection.
 * @li @ref CFGCollector::ADDED_FUNCTION -- function name to add to the collection.
 *
 * @note @ref otawa::ENTRY_CFG is first looked on the @ref WorkSpace and then in the configuration properties.
 *
 * @par Provided Features
 * @ref COLLECTED_CFG_FEATURE
 *
 * @par Required Features
 * @li @ref CFG_INFO_FEATURE
 *
 * @ingroup cfg
 */


/**
 */
void CFGCollector::processWorkSpace(WorkSpace *ws) {
	genstruct::Vector<Inst *> todo;

	// find address of label
	for(int i = 0; i < added_funs.count(); i++) {
		Address addr = ws->process()->program()->findLabel(added_funs[i]);
		if(!addr)
			throw ProcessorException(*this, _ << "cannot find label \"" << added_funs[i] << "\"");
		Inst *inst = ws->findInstAt(addr);
		if(!inst)
			throw ProcessorException(*this, _ << "symbol \"" << added_funs[i] << "\" is not mapped in memory");
		maker(inst);
	}

	// add instructions of CFGs to process
	for(int i = 0; i < added_cfgs.count(); i++) {
		Inst *inst = ws->findInstAt(added_cfgs[i]);
		if(!inst)
			throw ProcessorException(*this, _ << "address " << added_cfgs[i] << " is out of code segments.");
		maker(inst);
	}

	// process CFGs until end
	for(int i = 0; i < makers.count(); i++)
		processCFG(makers[i].fst);
}

/**
 * Scan the CFG to find all BBs.
 * @param e		Entry instruction.
 * @param bbs	To store found basic blocks.
 */
void CFGCollector::scanCFG(Inst *e, genstruct::FragTable<Inst *>& bbs) {
	if(logFor(Processor::LOG_FUN))
		log << "\tscanning CFG at " << e->address() << io::endl;

	// traverse all BBs until end
	genstruct::Vector<Inst *> todo;
	todo.add(e);
	while(todo) {

		// next block
		Inst *i = todo.pop();
		if(BB(i).exists())
			continue;
		BB(i) = 0;
		bbs.add(i);

		// iterate until basic block start or branch
		Inst *next = i->nextInst();
		while(next && !BB(next).exists()) {
			i = next;
			if(i->isControl())
				break;
			next = i->nextInst();
		}

		// push sequence if required
		if(i->isConditional() || i->isCall())
			if(i->nextInst())
				todo.push(i->nextInst());

		// push target(s) if any
		if(i->isReturn() || i->isCall())
			continue;
		if(i->target())
			todo.push(i->target());
		else {
			for(Identifier<Address>::Getter t(i, otawa::BRANCH_TARGET); t; t++) {
				Inst *i = workspace()->findInstAt(addr);
				if(i)
					todo.push(i);
			}
		}
	}
}

/**
 * Build the required basic blocks.
 * @param bbs	Basic block entries.
 * @param maker	Current CFG maker.
 */
void CFGCollector::buildBBs(CFGMaker& maker, const genstruct::FragTable<Inst *>& bbs) {
	for(genstruct::FragTable<Inst *>::Iterator e(bbs); e; e++) {

		// build list of instructions
		genstruct::Vector<Inst *> insts;
		insts.add(e);
		for(Inst *i = e->nextInst(); i && !BB(i).exists(); i = i->nextInst()) {
			insts.add(i);
			if(i->isControl())
				break;
		}

		// create the basic block
		BasicBlock *v = new BasicBlock(insts.detach());
		maker.add(v);
		BB(e) = v;
		if(logFor(LOG_BLOCK))
			log << "\t\tmaking BB at " << e->address() << ":" << v->topAddress() << io::endl;
	}
}

/**
 * Create a sequential edge.
 * @param m		Maker.
 * @param b		Basic block containing code.
 * @param src	Source block.
 */
void CFGCollector::seq(CFGMaker& m, BasicBlock *b, Block *src) {
	Inst *ni = b->last()->nextInst();
	if(ni)
		m.seq(src, BB(ni), new Edge());
}

/**
 * Build edges between basic blocks.
 * @param m	Current CFG maker.
 */
void CFGCollector::buildEdges(CFGMaker& m) {
	bool first = true;
	for(CFG::BlockIter v(m.blocks()); v; v++)
		if(v->isEntry())
			continue;
		else if(v->isBasic()) {

			// first block: do not forget edge with entry!
			if(first) {
				first = false;
				m.add(m.entry(), v, new Edge());
			}

			// process basic block
			BasicBlock *bb = **v;
			if(bb) {
				Inst *i = bb->control();

				// conditional or call case -> sequential edge
				if(!i || i->isConditional())
					seq(m, bb, bb);

				// branch cases
				if(i) {

					// return case
					if(i->isReturn())
						m.add(bb, m.exit(), new Edge());

					// not a call
					else if(!i->isCall()) {

						// compute list of targets
						genstruct::Vector<Inst *> ts;
						if(i->target())
							ts.add(i->target());
						else
							for(Identifier<Address>::Getter a(i, BRANCH_TARGET); a; a++) {
								Inst *t = workspace()->findInstAt(a);
								if(t)
									ts.add(t);
							}

						// buildes edges
						if(!ts)
							m.add(bb, m.unknown(), new Edge());
						else
							for(genstruct::Vector<Inst *>::Iterator t(ts); t; t++)
								m.add(bb, BB(t), new Edge());
					}

					// a call
					else {

						// compute list of targets
						genstruct::Vector<Inst *> cs;
						if(i->target())
							cs.add(i->target());
						else
							for(Identifier<Address>::Getter a(i, BRANCH_TARGET); a; a++) {
								Inst *t = workspace()->findInstAt(a);
								if(t)
									cs.add(t);
							}

						// build edges
						if(!cs) {
							Block *b = new SynthBlock();
							m.add(b);
							m.add(bb, b, new Edge());
							seq(m, bb, b);
						}
						else
							for(genstruct::Vector<Inst *>::Iterator c(cs); c; c++) {
								SynthBlock *cb = new SynthBlock();
								CFGMaker& cm = maker(c);
								m.add(cb, cm);
								m.add(bb, cb, new Edge());
								seq(m, bb, cb);
							}
					}

				}
			}
		}
}

/**
 * Remove markers on instruction header of basic blocks.
 * @param bbs	Heads of basic block.
 */
void CFGCollector::cleanBBs(const genstruct::FragTable<Inst *>& bbs) {
	for(int i = 0; i < bbs.count(); i++)
		BB(bbs[i]).remove();
}


/**
 * Get maker for the given instruction as function entry.
 * @param i		First instruction of CFG.
 * @return		Matching maker.
 */
CFGMaker &CFGCollector::maker(Inst *i) {
	int idx = CFG_INDEX(i);
	if(idx >= 0)
		return *makers[idx].snd;
	else {
		CFGMaker *maker = new CFGMaker(i);
		CFG_INDEX(i) = makers.count();
		makers.add(pair(i, maker));
		return *maker;
	}
}

/**
 * Process a new CFG starting at the given instruction.
 * @param i	Instruction starting the CFG.
 */
void CFGCollector::processCFG(Inst *i) {
	genstruct::FragTable<Inst *> entries;
	CFGMaker& m = maker(i);

	// traverse the BBs and mark them (ignore calls)
	scanCFG(i, entries);

	// build the basic blocks
	buildBBs(m, entries);

	// build the edges + call nodes
	buildEdges(m);

	// cleanup markers
	cleanBBs(entries);
}


/**
 */
CFGCollector::CFGCollector(p::declare& r)
: Processor(r) {
}

/**
 * CFGCollector registration.
 */
p::declare CFGCollector::reg = p::init("otawa::CFGCollector", Version(2, 0, 0))
	.require(DECODED_TEXT)
	.provide(COLLECTED_CFG_FEATURE)
	.maker<CFGCollector>();


/**
 */
void CFGCollector::cleanup(WorkSpace *ws) {

	// build the CFG collection and clean markers
	CFGCollection *coll = new CFGCollection();
	for(int i = 0; i < makers.count(); i++) {
		CFG *cfg = makers[i].snd->build();
		coll->add(cfg);
		CFG_INDEX(cfg->first()).remove();
	}

	// cleanup makers
	for(int i = 0; i < makers.count(); i++)
		delete makers[i].snd;
	makers.clear();

	// installing the collection
	otawa::INVOLVED_CFGS(ws) = coll;
	ENTRY_CFG(ws) = (*coll)[0];
}


/**
 */
void CFGCollector::configure(const PropList& props) {
	Processor::configure(props);

	// Misc configuration
	addr = TASK_ADDRESS(props);
	if(addr)
		added_cfgs.add(addr);
	else {
		name = TASK_ENTRY(props);
		if(!name) {
			name = "main";
			added_funs.add(name);
		}
	}

	// collect added CFGs
	for(Identifier<Address>::Getter cfg(props, ADDED_CFG); cfg; cfg++)
		added_cfgs.add(cfg);
	for(Identifier<CString>::Getter fun(props, ADDED_FUNCTION); fun; fun++)
		added_funs.add(*fun);
}


/**
 * This property is used to link the current computation involved CFG
 * on the framework.
 *
 * @par Hooks
 * @li @ref WorkSpace
 *
 * @ingroup cfg
 */
Identifier<const CFGCollection *> INVOLVED_CFGS("otawa::INVOLVED_CFGS", 0);


/**
 * This property is used to get the entry CFG of the currently
 * processed task.
 *
 * @para Hooks
 * @li @ref WorkSpace
 *
 * @ingroup cfg
 */
Identifier<CFG *> ENTRY_CFG("otawa::entry_cfg", 0);


/**
 * This properties are put on a CFG to get the list of edges calling it.
 *
 * @par Hooks
 * @li @ref CFG
 *
 * @ingroup cfg
 */
Identifier<Edge *> CALLED_BY("otawa::CALLED_BY", 0);


/**
 * This feature asserts that all CFG involved in the current computation has
 * been collected and accessible thanks to @ref INVOLVED_CFGS property
 *
 * @par Properties
 * @ref ENTRY_CFG (@ref WorkSpace).
 * @ref INVOLVED_CFGS (@ref WorkSpace).
 * @ref INDEX (@ref CFG)
 * @ref CALLED_BY (@ref CFG)
 *
 * @ingroup cfg
 */
p::feature COLLECTED_CFG_FEATURE("otawa::COLLECTED_CFG_FEATURE", new Maker<CFGCollector>());


/**
 * This configuration property allows to add unlinked CFG to the used CFG
 * collection.
 *
 * @ingroup cfg
 */
Identifier<Address> CFGCollector::ADDED_CFG("otawa::CFGCollector::ADDED_CFG", 0);


/**
 * This configuration property allows to add unlinked functions to the used CFG
 * collection.
 *
 * @ingroup cfg
 */
Identifier<CString> CFGCollector::ADDED_FUNCTION("otawa::CFGCollector::ADDED_FUNCTION", 0);

} // otawa
