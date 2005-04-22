/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/util_DFA.cpp -- DFA class implementation.
 */

#include <otawa/util/DFA.h>
#include <otawa/cfg.h>

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
 *xcvxcvxcv
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
			info->buf->reset();
			for(Iterator<Edge *> edge(bb->inEdges()); edge; edge++) {
				dfa_info_t *info = edge->source()->use<dfa_info_t *>(info_id);
				assert(info);
				info->buf->add(info->cur);
			}
			bb->add<DFASet *>(in_id, info->buf);
		}
	
		// Store OUT information
		if(out_id)
			bb->add<DFASet *>(out_id, info->cur);
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
	
	// Prolog
	bool fix_point = false;
	startup(cfg);
	
	// Fix point look up
	while(!fix_point) {
		fix_point = true;
		
		// Look in BB
		for(Iterator<BasicBlock *> bb(cfg->bbs()); bb; bb++) {
			dfa_info_t *info = bb->use<dfa_info_t *>(info_id);
			assert(info);
			
			// Build new set
			info->buf->reset();
			for(Iterator<Edge *> edge(bb->outEdges()); edge; edge++) {
				dfa_info_t *in_info = edge->source()->use<dfa_info_t *>(info_id);
				assert(in_info);
				info->buf->add(in_info->cur);
			}
			info->buf->remove(info->kill);
			info->buf->add(info->gen);
			
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

} // otawa
