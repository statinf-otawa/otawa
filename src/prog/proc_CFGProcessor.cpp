/*
 *	$Id$
 *	CFGProcessor class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-08, IRIT UPS.
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

#include <otawa/proc/CFGProcessor.h>
#include <otawa/cfg/CFGInfo.h>
#include <otawa/cfg.h>
#include <otawa/otawa.h>
#include <otawa/cfg/CFGCollector.h>

namespace otawa {

/**
 * @class CFGProcessor
 * This is a specialization of the processor class dedicated to CFG
 * processing. The @ref Processor::processFrameWork() method just take each
 * CFG of the framework and apply the processor on.
 *
 * It accepts in configuration the following properties:
 * @li @ref ENTRY_CFG: the entry CFG of the task to work with,
 * @li @ref RECURSIVE: to go down recursively in the task CFG.
 *
 * If statistics are required, it provides:
 * @li @ref PROCESSED_CFG: records the count of processed CFG.
 *
 * Finally, it puts on the framework the following properties:
 * @li @ref INVOLVED_CFGS: collection of CFG used by the current task.
 *
 * @note This processor automatically call @ref CFGCollector.
 * @ingroup proc
 */


/**
 */
p::declare CFGProcessor::reg = p::init("otawa::CFGProcessor", Version(2, 1, 0))
	.require(COLLECTED_CFG_FEATURE)
	.require(LABEL_FEATURE);


/**
 * Build a new CFG processor.
 * @deprecated
 */
CFGProcessor::CFGProcessor(void): Processor(reg), _cfg(nullptr), _coll(nullptr) {
}


/**
 * Build a new named processor.
 * @param name		Processor name.
 * @param version	Processor version.
 * @deprecated
 */
CFGProcessor::CFGProcessor(cstring name, elm::Version version)
: Processor(name, version, reg), _cfg(nullptr), _coll(nullptr) {
}


/**
 * Build a new CFG processor with the given registration information.
 * @param reg	Registration information.
 */
CFGProcessor::CFGProcessor(AbstractRegistration& reg)
: Processor(reg), _cfg(nullptr), _coll(nullptr) {
}


/**
 * @deprecated
 */
CFGProcessor::CFGProcessor(cstring name, const Version& version, AbstractRegistration& reg)
: Processor(name, version, reg), _cfg(nullptr), _coll(nullptr) {
}



/**
 */
void CFGProcessor::processWorkSpace(WorkSpace *ws) {

	// Get the CFG collection
	_coll = INVOLVED_CFGS(ws);
	ASSERT(_coll);

	// Visit CFG
	processAll(ws);

	// Record stats
	if(recordsStats())
		PROCESSED_CFG(stats) = _coll->count();
}


/**
 * This function is called once the CFG collection has been obtained.
 * It can be overriden to apply analysis on the whole task (collection
 * of CFGs). As a default, it call processCFG() for each CFG of the
 * collection.
 *
 * In this function, the following functions are available:
 * * CFGProcessor::cfgs()
 * * CFGProcessor::entryCFG()
 * * CFGProcessor::entryBlock()
 *
 * @param ws	Current workspace.
 */
void CFGProcessor::processAll(WorkSpace *ws) {
	for(auto g: *_coll) {
		if(logFor(LOG_CFG))
			log << "\tprocess CFG " << g->label() << io::endl;
		_cfg = g;
		processCFG(ws, g);
	}
}


/**
 * This function may be overridden by a subclass to provide custom cleanup
 * for a CFG. It is called for each CFG of the task when @ref doCleanUp() is called.
 * As a default, do nothing.
 * @param ws	Current workspace.
 * @param cfg	Current CFG.
 */
void CFGProcessor::cleanupCFG(WorkSpace *ws, CFG *cfg) {
}


/**
 * Trigger associated with CFG. For each CFG, perform a call to
 * @ref cleanupCFG() that may be customized by a subclass.
 */
void CFGProcessor::doCleanUp(void) {
	const CFGCollection *coll = INVOLVED_CFGS(workspace());
	for(int i = 0; i < coll->count(); i++)
		cleanupCFG(workspace(), coll->get(i));
}


/**
 * Propagate the destroy phase to CFG resources.
 * Default implementation does nothing.
 */
void CFGProcessor::destroyCFG(WorkSpace *ws, CFG *cfg) {
}


/**
 * @fn CFG *CFGProcessor::cfg(void) const;
 * Get the current CFG.
 * @return	Current CFG.
 * @warning	Can only be called from processCFG().
 */


/**
 * @fn Block *CFGProcessor::entry() const;
 * Get the entry block of the current CFG.
 * @return	Current CFG entry block.
 * @warning	Can only be called from processCFG().
 */


/**
 * @fn Block *CFGProcessor::exit() const;
 * Get the exit block of the current CFG.
 * @return	Current CFG exit block.
 * @warning	Can only be called from processCFG().
 */


/**
 * @fn const CFGCollection& CFGProcessor::cfgs(void) const;
 * Get a range on the CFG of the current task.
 * @return	Current CFGs range.
 * @warning	Can only be called from processAll() or processCFG().
 */


/**
 * @fn CFG *CFGProcessor::taskCFG() const;
 * Get the CFG entry point of the current task.
 * @return	Current task entry point CFG.
 * @warning	Can only be called from processAll() or processCFG().
 */


/**
 * @fn Block *CFGProcessor::taskEntry() const;
 * Get the entry block of the current task entry CFG.
 * @return	Entry block of the current entry CFG.
 * @warning	Can only be called from processAll() or processCFG().
 */


/**
 */
void CFGProcessor::destroy(WorkSpace *ws) {
	const CFGCollection *coll = INVOLVED_CFGS(workspace());
	for(int i = 0; i < coll->count(); i++)
		destroyCFG(ws, coll->get(i));
}

///
void CFGProcessor::dump(WorkSpace *ws, Output& out) {
	for(auto g: *_coll) {
		out << "CFG " << g << io::endl;
		dumpCFG(g, out);
	}
}

/**
 * The default implementation of CFGProcessor dump calls this function for
 * each CFG of the program. So it can be redefined to provide a custom dump.
 * The default implementation does nothing.
 * @param g		CFG to dump.
 * @param out	Stream to output to.
 */
void CFGProcessor::dumpCFG(CFG *g, Output& out) {

}


/**
 * This property is used to store statistics about the count of processed
 * CFG.
 */
Identifier<int> PROCESSED_CFG("otawa::processed_cfg", 0);


/**
 * Initialize the processor.
 * @param props	Configuration properties.
 */
void CFGProcessor::init(const PropList& props) {
}


/**
 * Configure the current processor.
 * @param props	Configuration properties.
 */
void CFGProcessor::configure(const PropList& props) {
	Processor::configure(props);
	init(props);
}


/**
 * Transform an address to a smart string, that is,
 * if a source line is available, transform it to "source_file:source_line",
 * if the CFG has a label, it gives "label + 0xoffset", else return the
 * address.
 * @param address	Address to display.
 * @return			Address transformed in string.
 */
string CFGProcessor::str(const Address& address) {
	return str(Address::null, address);
}


/**
 * Transform an address to a smart string, that is,
 * if a source line is available, transform it to "source_file:source_line",
 * if the CFG has a label, it gives "label + 0xoffset", else return the
 * address.
 * @param base		Base address of the function containing the give address.
 * @param address	Address to display.
 * @return			Address transformed in string.
 */
string CFGProcessor::str(const Address& base, const Address& address) {
	Inst *first;
	if(base.isNull())
		first = _cfg->first();
	else
		first = workspace()->findInstAt(base);
	String label;
	if(first)
		label = FUNCTION_LABEL(first);
	if(label) {
		int offset = address.offset() - first->address().offset();
		if(offset < 0)
			return _ << label << " - 0x" << io::hex(-offset) << " (" << address << ")";
		else
			return _ << label << " + 0x" << io::hex(offset) << " (" << address << ")";
	}
	else
		return _ << "0x" << address;
}


/**
 * @fn CFG *CFGProcessor::cfg(void) const { return cfg; }
 * Get the current CFG.
 * @return	Current CFG.
 */


/**
 * Activate the recucursive feature of BBProcessors : each time a basic block
 * contains a function call, the CFG of this function is recorded to be
 * processed later after the current CFG. Note that each function is only
 * processed once !
 */
p::id<bool> RECURSIVE("otawa::recursive", true);

} // otawa
