/*
 *	$Id$
 *	Copyright (c) 2006 IRIT - UPS.
 *
 *	prog/cfg_CollectCFG.h -- implementation of CFGCollector class.
 */

#include <otawa/cfg/CFGCollector.h>
#include <otawa/cfg.h>
#include <otawa/proc/CFGProcessor.h>

namespace otawa {

/**
 * @class CFGCollection <otawa/cfg.h>
 * Contains a collection of CFGs (used with @ref INVOLVED_CFGS property).
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
 * @class CFGCollector
 * This processor is used to collect all CFG implied in a computation.
 * It uses the @ref ENTTY_CFG or @ref TASK_ENTRY properties to find
 * the base CFG and explore in depth the CFG along subprograms calls.
 */


/**
 */
void CFGCollector::processFrameWork (FrameWork *fw) {
	static GenericIdentifier<bool> MARK("otawa.cfg_collector.mark", false);
	
	// Set first queue node
	CFG *entry = ENTRY_CFG(fw);
	if(!entry) {
		CFGInfo *info = fw->getCFGInfo();
		CString name = TASK_ENTRY(fw);
		entry = info->findCFG(name);
		if(!entry)
			throw ProcessorException(*this, "cannot find task entry point \"%d.\"", &name);
		ENTRY_CFG(fw) = entry;
	}
	
	// Build the involved collection
	CFGCollection *cfgs = new CFGCollection();
	cfgs->cfgs.add(entry);
	INVOLVED_CFGS(fw) = cfgs;
	
	// Build it recursively
	if(RECURSIVE(fw))
		for(int i = 0; i < cfgs->count(); i++)
			for(CFG::BBIterator bb(cfgs->get(i)); bb; bb++)
				for(BasicBlock::OutIterator edge(bb); edge; edge++)
					if(edge->kind() == Edge::CALL
					&& edge->calledCFG()
					&& !MARK(edge->calledCFG())) {
						cfgs->cfgs.add(edge->calledCFG());
						MARK(edge->calledCFG()) = true;
					} 
}


/**
 * Build the CFG collector.
 * @param props	Configuration properties.
 */
CFGCollector::CFGCollector(const PropList& props)
: Processor("CFGCollector", Version(1, 0, 0), props) {
}


/**
 * This property is used to link the current computation involved CFG
 * on the framework.
 */
GenericIdentifier<CFGCollection *> INVOLVED_CFGS("otawa.involved_cfgs", 0);
	
} // otawa
