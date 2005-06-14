/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/util_DFA.cpp -- DFA class implementation.
 */

#include <otawa/util/DFA.h>
#include <otawa/cfg.h>
#include <otawa/util/CFGNormalizer.h>

//#include <otawa/util/DFABitSet.h>

namespace otawa {

/**
 */
typedef struct dfa_info_t {
	DFASet *cur;	// Current output set
	DFASet *buf;	// Buffer set (they are exchanged)
	DFASet *gen;	// Generation set
	DFASet *kill;	// Kill set
} dfa_info_t;

// Storage identifier
static Identifier info_id("otawa.dfa.info");

/**
 * @class DFA
 * Implements the Data Flow Analysis method for peforming static analysis as
 * described in "Compilers - Principles, Techniques and Tools" by
 * Aho, Sethi and Ullman.
 *
 * For adapting this class for a specific static analysis, it must be
 * inherited and methods @ref initial(), @ref generate() and @ref kill() must be
 * defined accordingly.
 */


/**
 * Initialize the information in the CFG.
 * @param cfg	CFG to process.
 */
void DFA::startup(CFG *cfg) {
	assert(cfg);
	for(Iterator<BasicBlock *> bb(cfg->bbs()); bb; bb++) {
		dfa_info_t *info = new dfa_info_t;
		info->cur = initial();
		info->buf = initial();
		info->gen = generate(bb);
		info->kill = kill(bb);
		bb->addDeletable<dfa_info_t *>(info_id, info);
		//cout << "PREPARE " << bb->address() << "\n";
	}
}


/**
 * Cleanup the CFG from DFA information and store requested information.
 * @param cfg		CFG to process.
 * @param in_id		Identifier for storing IN (null if not used).
 * @param out_id	Identifier for storing OUT (null if not used).
 */
void DFA::cleanup(CFG *cfg, Identifier *in_id, Identifier *out_id) {
	assert(cfg);
	
	// Store information
	for(Iterator<BasicBlock *> bb(cfg->bbs()); bb; bb++) {
		dfa_info_t *info = bb->use<dfa_info_t *>(info_id);
		assert(info);
		
		// Store IN information
		if(in_id) {
			clear(info->buf);
			for(BasicBlock::InIterator edge(bb); edge; edge++) {
				dfa_info_t *in_info = edge->source()->get<dfa_info_t *>(info_id, 0);
				// !!TODO!! Look below.
				if(in_info);
					merge(info->buf, in_info->cur);
			}
			bb->add<DFASet *>(in_id, info->buf);
		}
	
		// Store OUT information
		if(out_id)
			bb->add<DFASet *>(out_id, info->cur);
		//elm::cout << bb->use<int>(CFG::ID_Index) << " " << *(DFABitSet *)info->cur << "\n";
	}
	
	// Cleanup
	for(Iterator<BasicBlock *> bb(cfg->bbs()); bb; bb++) {
		dfa_info_t *info = bb->use<dfa_info_t *>(info_id);
		assert(info);
		if(!in_id)
			delete info->buf;
		if(!out_id)
			delete info->cur;
		delete info->gen;
		delete info->kill;
		bb->removeProp(&info_id);
	}
}


/**
 * Resolve the current DFA for the given CFG. The result are stored in basic
 * blocks with the given optional identifiers.
 * @param cfg		CFG to process.
 * @param in_id		Identifier for storing IN (null if not used).
 * @param out_id	Identifier for storing OUT (null if not used).
 */
void DFA::resolve(CFG *cfg, Identifier *in_id, Identifier *out_id) {
	assert(cfg);
	//cout << "CFG: " << cfg->label() << "\n";
	
	// Prolog
	CFGNormalizer normalizer;
	PropList props;
	props.add<bool>(CFGNormalizer::ID_Verbose, true);
	normalizer.configure(props);
	normalizer.processCFG(0, cfg);
	bool fix_point = false;
	startup(cfg);
	
	// Fix point look up
	while(!fix_point) {
		fix_point = true;
		
		// Look in BB
		for(Iterator<BasicBlock *> bb(cfg->bbs()); bb; bb++) {
			//cout << "\tBB " << bb->address() << "\n";
			dfa_info_t *info = bb->use<dfa_info_t *>(info_id);
			assert(info);
			
			// Build new set
			clear(info->buf);
			for(BasicBlock::InIterator edge(bb); edge; edge++) {
				//cout << "\tEDGE from " << edge->source()->address() << "\n";
				dfa_info_t *in_info = edge->source()->get<dfa_info_t *>(info_id, 0);
				// !!TODO!! Quick fix. Need to output a message and to add
				// option in CFG computation to remove this kind of edge.
				if(in_info)
					merge(info->buf, in_info->cur);
			}
			info->buf->remove(info->kill);
			info->buf->add(info->gen);
			//elm::cout << bb->use<int>(CFG::ID_Index) << " " << *(DFABitSet *)info->buf << "\n";
			
			// Compare and fix it
			if(!info->cur->equals(info->buf)) {
				fix_point = false;
				DFASet *aux = info->cur;
				info->cur = info->buf;
				info->buf = aux;
			}
		}
	}
	
	// epilog
	cleanup(cfg, in_id, out_id);
}


/**
 * @fn DFASet *DFA::initial(void);
 * This pure virtual method is called for getting an initial set used in the
 * DFA resolution. According the increasing or the decreasing method, it must
 * return an empty or a full set.
 * @return	Empty set.
 */


/**
 * @fn DFASet *DFA::generate(BasicBlock *bb);
 * This pure virtual function must be ovveriden for getting the generation
 * set according the given basic block.
 * @return	Generation set.
 */


/**
 * @fn DFASet *DFA::kill(BasicBlock *bb);
 * This pure virtual function must be ovveriden for getting the killer
 * set according the given basic block.
 * @return	Killer set.
 */


/**
 * @fn void DFA::clear(DFASet *set);
 * Reset the given DFA set for performing merge thereafter.
 * @param set	Set to clear.
 */


/**
 * @fn void DFA::merge(DFASet *acc, DFASet *set);
 * This function is used for merging the OUT from predecessor nodes.
 * The acc represents the computed IN set of the current node and
 * the passed set is one of OUT sets of the predecessors.
 * @param acc	Computed IN set.
 * @param set	Predecessor OUT set.
 */


/**
 * @class DFASet
 * A set object usable by the @ref DFA class.
 */


/**
 * @fn void DFASet::empty(void);
 * Make the set empty.
 */


/**
 * @fn void DFASet::fill(void);
 * Make the set full.
 */


/**
 * @fn bool DFASet::equals(DFASet *set);
 * Test if the two sets are equal.
 * @param set	Set to compare with the current one.
 * @return		True if they are equal, false else.
 */


/**
 * @fn void DFASet::add(DFASet *set);
 * Add the given set to the current one.
 * @param set	Set to add.
 */


/**
 * @fn void DFASet::remove(DFASet *set);
 * Remove the given set from the current one.
 * @param set	Set to remove.
 */

} // otawa
