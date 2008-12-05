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
#include <otawa/cfg.h>
#include <otawa/cfg/CFGBuilder.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/prog/Manager.h>

namespace otawa {

static Identifier<bool> MARK("", false);

/**
 * @class CFGCollection <otawa/cfg.h>
 * Contains a collection of CFGs (used with @ref INVOLVED_CFGS property).
 * @see otawa::CFGCollector, otawa::INVOLVED_CFGS
 */


// CFGCollection cleaner
class CFGCollectionCleaner: public Cleaner {
public:
	inline CFGCollectionCleaner(CFGCollection *_cfgs) { cfgs = _cfgs; }
	virtual ~CFGCollectionCleaner(void) {
		for(CFGCollection::Iterator cfg(cfgs); cfg; cfg++)
			delete *cfg;
		delete cfgs;
	}
private:
	CFGCollection *cfgs;	
};


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
 * It uses the @ref otawa::ENTRY_CFG or @ref otawa::TASK_ENTRY properties to find
 * the base CFG and explore in depth the CFG along subprograms calls.
 *
 * @par Configuration
 * @li @ref ENTRY_CFG -- CFG of the entry of the current task,
 * @li @ref TASK_ENTRY -- name if the entry function of the current task,
 * @li @ref RECURSIVE -- collect CFG recursively.
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
 */


/**
 */
void CFGCollector::processWorkSpace (WorkSpace *fw) {

        int index = 0;
	
	// Set first queue node
	CFG *ws_entry = ENTRY_CFG(fw);
    if(ws_entry)
          entry = ws_entry;
	else {
		if(!entry && name) {
			CFGInfo *info = fw->getCFGInfo();
			entry = info->findCFG(name);
		}
		if(!entry)
			throw ProcessorException(*this, _
				<< "cannot find task entry point \"" << name << "\"");
		ENTRY_CFG(fw) = entry;
	}
	
	// Build the involved collection
	CFGCollection *cfgs = new CFGCollection();
	INDEX(entry) = index;
	index++;
	
	// Entry CFG
	cfgs->cfgs.add(entry);
	if(isVerbose())
		log << "\tadding " << entry->label() << io::endl;
	
	// Added functions
	for(int i = 0; i < added_funs.length(); i++) {
		CFGInfo *info = fw->getCFGInfo();
		CFG *cfg = info->findCFG(added_funs[i]);
		if(cfg) {
			cfgs->cfgs.add(cfg);
			if(isVerbose())
				log << "\tadding " << cfg->label() << io::endl;
		}
		else
			warn(_ << "cannot find a function called \"" << added_funs[i] << "\".");
	}
	
	// Added CFG
	for(int i = 0; i < added_cfgs.length(); i++) {
		cfgs->cfgs.add(added_cfgs[i]);
		if(isVerbose())
			log << "\tadding " << added_cfgs[i]->label() << io::endl;
	}

	INVOLVED_CFGS(fw) = cfgs;
	addCleaner(COLLECTED_CFG_FEATURE, new CFGCollectionCleaner(cfgs));
	
	// Build it recursively
	if(rec) {
		if(isVerbose())
			log << "\tstarting recursive traversal\n";
		for(int i = 0; i < cfgs->count(); i++) {
			for(CFG::BBIterator bb(cfgs->get(i)); bb; bb++)
				for(BasicBlock::OutIterator edge(bb); edge; edge++) {
					if(edge->kind() == Edge::CALL
					&& edge->calledCFG()
					&& !MARK(edge->calledCFG())) {
					        INDEX(edge->calledCFG()) = index;
					        index++;
							cfgs->cfgs.add(edge->calledCFG());
							MARK(edge->calledCFG()) = true;
							if(isVerbose())
								log << "\t\tadding " << edge->calledCFG()->label() << io::endl;
					} 
				}
		}
		if(isVerbose())
			log << "\tending recursive traversal\n";
	}
}


/**
 * Build the CFG collector.
 * @param props	Configuration properties.
 */
CFGCollector::CFGCollector(void)
: Processor("CFGCollector", Version(1, 0, 0)), entry(0), rec(false) {
	require(CFG_INFO_FEATURE);
	provide(COLLECTED_CFG_FEATURE);
}

void CFGCollector::cleanup(WorkSpace *ws) {
	CFGCollection *coll = INVOLVED_CFGS(ws);	
	for (CFGCollection::Iterator iter(*coll); iter; iter++)
		MARK(iter) = false;
}


/**
 */
void CFGCollector::configure(const PropList& props) {
	Processor::configure(props);
	
	// Misc configuration
	entry = ENTRY_CFG(props);
	if(!entry)
		name = TASK_ENTRY(props);
	rec = otawa::RECURSIVE(props);

	// Collect added CFGs
	added_cfgs.clear();
	for(Identifier<CFG *>::Getter cfg(props, ADDED_CFG); cfg; cfg++)
		added_cfgs.add(cfg);
	added_funs.clear();
	for(Identifier<CString>::Getter fun(props, ADDED_FUNCTION); fun; fun++)
		added_funs.add(fun);
}


/**
 * This property is used to link the current computation involved CFG
 * on the framework.
 * 
 * @par Hooks
 * FrameWork
 */
Identifier<CFGCollection *> INVOLVED_CFGS("otawa::involved_cfgs", 0);


static SilentFeature::Maker<CFGCollector> COLLECTED_CFG_MAKER;
/**
 * This feature asserts that all CFG involved in the current computation has
 * been collected and accessible thanks to @ref INVOLVED_CFGS property
 * 
 * @par Properties
 * @ref ENTRY_CFG (FrameWork).
 * @ref INVOLVED_CFGS (FrameWork).
 */
SilentFeature COLLECTED_CFG_FEATURE("otawa::collected_cfg", COLLECTED_CFG_MAKER);


/**
 * This configuration property allows to add unlinked CFG to the used CFG
 * collection.
 */
Identifier<CFG *> CFGCollector::ADDED_CFG("otawa::CFGCollector::ADDED_CFG", 0);


/**
 * This configuration property allows to add unlinked functions to the used CFG
 * collection.
 */
Identifier<CString> CFGCollector::ADDED_FUNCTION("otawa::CFGCollector::ADDED_FUNCTION", 0);

} // otawa
