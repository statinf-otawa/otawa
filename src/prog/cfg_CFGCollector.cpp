/*
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
#include <otawa/prog/File.h>
#include <otawa/prog/Manager.h>
#include <otawa/prog/TextDecoder.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/util/FlowFactLoader.h>

namespace otawa {

class CollectorCleaner: public Cleaner {
public:

	CollectorCleaner(WorkSpace *ws, CFGCollection *coll): _ws(ws), _coll(coll) { }

	virtual void clean(void) {
		for(CFGCollection::Iter g(_coll); g; g++)
			delete g;
		delete _coll;
		INVOLVED_CFGS(_ws).remove();
	}

private:
	WorkSpace *_ws;
	CFGCollection *_coll;
};


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
 * @fn CFG *CFGCollection::entry(void) const;
 * Get the entry CFG of the collection.
 * @return	Entry CFG.
 */

/**
 * @class CFGCollection::Iter
 * Iterator on the CFG contained in a @ref CFGCollection.
 */

/**
 * @fn CFGCollection::Iter::Iter(const CFGCollection *cfgs);
 * Build an iterator on the given CFG collection.
 * @param cfgs	CFG collection to iterate on.
 */

/**
 * @fn CFGCollection::Iter::Iter(const CFGCollection& cfgs);
 * Build an iterator on the given CFG collection.
 * @param cfgs	CFG collection to iterate on.
 */

/**
 * @fn CFGCollection::Iter::Iter(WorkSpace *ws);
 * CFG iterator builder on the CFG collection of a workspace.
 * @warning The COLLECTED_CFG_FEATURE must be provided!
 * @param ws	Workspace containing the CFG collection.
 */

/**
 * @class CFGCollection::BlockIter
 * Iterator on the total list of blocks composing CFGs of a CFG collection.
 */

/**
 * @fn CFGCollection::BlockIter::BlockIter(WorkSpace *ws);
 * Block iterator builder on the CFG collection of a workspace.
 * @warning The COLLECTED_CFG_FEATURE must be provided!
 * @param ws	Workspace containing the CFG collection.
 */

/**
 * Get the CFG collection from a workspace.
 * @param ws	Workspace to look in.
 * @return		Found CFG collection.
 * @warning The COLLECTED_CFG_FEATURE must be provided!
 */
const CFGCollection *CFGCollection::get(WorkSpace *ws) {
	const CFGCollection *coll = INVOLVED_CFGS(ws);
	ASSERT(coll);
	return coll;
}

/**
 * Add a CFG to the collection.
 * @param cfg	Added CFG.
 */
void CFGCollection::add(CFG *cfg) {
	cfg->idx = cfgs.count();
	cfg->_offset = cfgs.isEmpty() ? 0 : cfgs[cfgs.length()-1]->offset() + cfgs[cfgs.length()-1]->count();
	cfg->_offset = cfgs.isEmpty() ? 0 : cfgs[cfgs.length()-1]->offset() + cfgs[cfgs.length()-1]->count();
	cfgs.add(cfg);
}

/**
 * Count the number of BB in the CFG collection
 * (sum of BB count of each CFG).
 * @return	Collection BB number.
 */
int CFGCollection::countBlocks(void) const {
	if(!cfgs)
		return 0;
	else
		return cfgs[count() - 1]->offset() + cfgs[count() - 1]->count();
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
 * @ingroup cfg
 */


/**
 */
CFGCollector::CFGCollector(p::declare& r): AbstractCFGBuilder(r) {
}

/**
 * CFGCollector registration.
 */
p::declare CFGCollector::reg = p::init("otawa::CFGCollector", Version(2, 0, 1))
	.require(FLOW_FACTS_FEATURE)
	.require(LABEL_FEATURE)
	.provide(COLLECTED_CFG_FEATURE)
	.maker<CFGCollector>();


/**
 */
void CFGCollector::setup(WorkSpace *ws) {
	AbstractCFGBuilder::setup(ws);

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

}


/**
 */
void CFGCollector::cleanup(WorkSpace *ws) {

	// build the CFG collection and clean markers
	CFGCollection *coll = new CFGCollection();
	for(Iter m(*this); m; m++) {
		CFG *g = m->build();
		coll->add(g);
	}

	// install the collection
	otawa::INVOLVED_CFGS(ws) = coll;
	ENTRY_CFG(ws) = (*coll)[0];
	addCleaner(COLLECTED_CFG_FEATURE, new CollectorCleaner(ws, coll));

	// cleanup all
	AbstractCFGBuilder::cleanup(ws);
}


/**
 */
void CFGCollector::configure(const PropList& props) {
	AbstractCFGBuilder::configure(props);

	// Misc configuration
	Address addr = TASK_ADDRESS(props);
	if(addr)
		added_cfgs.add(addr);
	else {
		string name = TASK_ENTRY(props);
		if(!name)
			name = "main";
		added_funs.add(name);
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
p::id<const CFGCollection *> INVOLVED_CFGS("otawa::INVOLVED_CFGS", 0);


/**
 * This property is used to get the entry CFG of the currently
 * processed task.
 *
 * @para Hooks
 * @li @ref WorkSpace
 *
 * @ingroup cfg
 */
p::id<CFG *> ENTRY_CFG("otawa::entry_cfg", 0);


/**
 * This properties are put on a CFG to get the list of edges calling it.
 *
 * @par Hooks
 * @li @ref CFG
 *
 * @ingroup cfg
 */
p::id<Edge *> CALLED_BY("otawa::CALLED_BY", 0);


/**
 * This feature asserts that all CFG involved in the current computation has
 * been collected and accessible thanks to @ref INVOLVED_CFGS property
 *
 * @par Configuration
 * @li @ref BB_BOUNDS
 * @li @ref ENTRY_CFG
 * @li @ref ADDED_CFG
 * @li @ref ADDED_FUNCTION
 *
 * @par Properties
 * @li @ref ENTRY_CFG (@ref WorkSpace).
 * @li @ref INVOLVED_CFGS (@ref WorkSpace).
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
p::id<Address> ADDED_CFG("otawa::ADDED_CFG", 0);


/**
 * This configuration property allows to add unlinked functions to the used CFG
 * collection.
 *
 * @ingroup cfg
 */
p::id<CString> ADDED_FUNCTION("otawa::ADDED_FUNCTION", 0);

} // otawa
