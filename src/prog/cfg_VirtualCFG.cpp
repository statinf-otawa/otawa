/*
 * $Id$
 * Copyright (c) 2005 IRIT-UPS
 * 
 * src/prog/cfg_VirtualCFG.cpp -- implementation of VirtualCFG class.
 */

#include <elm/io.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/cfg.h>
#include <otawa/cfg/VirtualCFG.h>
#include <otawa/cfg/VirtualBasicBlock.h>

namespace otawa {

/* Used for resolving recursive calls as loops */
typedef struct call_t {
	struct call_t *back;
	CFG *cfg;
	BasicBlock *entry;
} call_t;


/**
 * A property with this identifier is hooked at the edge performing a virtual
 * call when inling is used. The associated value is the CFG of the called
 * function.
 */
GenericIdentifier<CFG *> CALLED_CFG("called_cfg", 0, OTAWA_NS);


/**
 * A property with this identifier is hooked to edge performing a recursive
 * call when inlining is used.
 */
GenericIdentifier<bool> RECURSIVE_LOOP("recursive_loop", false, OTAWA_NS);


/**
 * @class VirtualCFG
 * A virtual CFG is a CFG not-mapped to real code, that is, it may contains
 * virtual nodes for inlining functions calls or for separating nodes according
 * some context information.
 */


/**
 * Build the virtual CFG.
 * @param stack		Stack to previous calls.
 * @param cfg		CFG to develop.
 * @param ret		Basic block for returning.
 */
void VirtualCFG::virtualize(struct call_t *stack, CFG *cfg, BasicBlock *entry,
BasicBlock *exit) {
	assert(cfg);
	assert(entry);
	assert(exit);
	//cout << "Virtualizing " << cfg->label() << "(" << cfg->address() << ")\n";
	
	// Prepare data
	elm::genstruct::HashTable<void *, BasicBlock *> map;
	call_t call = { stack, cfg, 0 };
	
	// Translate BB
	for(Iterator<BasicBlock *> bb(cfg->bbs()); bb; bb++)
		if(!bb->isEntry() && !bb->isExit()) {
			BasicBlock *new_bb = new VirtualBasicBlock(bb);
			map.put(bb, new_bb);
			_bbs.add(new_bb);
		}
	
	// Find local entry
	for(BasicBlock::OutIterator edge(cfg->entry()); edge; edge++) {
		assert(!call.entry);
		call.entry = map.get(edge->target(), 0);
		assert(call.entry);
		Edge *edge = new Edge(entry, call.entry, Edge::VIRTUAL_CALL);
		CALLED_CFG(edge) = cfg;
	}
	
	// Translate edges
	for(Iterator<BasicBlock *> bb(cfg->bbs()); bb; bb++)
		if(!bb->isEntry() && !bb->isExit()) {
			assert(!bb->isVirtual());
			
			// Resolve source
			BasicBlock *src = map.get(bb, 0);
			assert(src);

			// Is there a call ?			
			CFG *called = 0;
			BasicBlock *called_exit = 0;
			if(isInlined())
				for(BasicBlock::OutIterator edge(bb); edge; edge++)
					if(edge->kind() == Edge::CALL)
						called = edge->calledCFG();

			// Look edges
			for(BasicBlock::OutIterator edge(bb); edge; edge++)
				if(edge->kind() == Edge::CALL) {
					if(!isInlined())
						new Edge(src, edge->target(), Edge::CALL);
				}
				else if(edge->target()) { 
					if(edge->target()->isExit()) {
						Edge *edge = new Edge(src, exit, Edge::VIRTUAL_RETURN);
						CALLED_CFG(edge) = cfg;
						called_exit = exit;
					}
					else {
						BasicBlock *tgt = map.get(edge->target(), 0);
						assert(tgt);
						if(called && edge->kind() == Edge::NOT_TAKEN)
							called_exit = tgt;
						else
							new Edge(src, tgt, edge->kind());
					}
				}
			
			// Process the call
			if(called) {
				for(call_t *cur = &call; cur; cur = cur->back)
					if(cur->cfg == called) {
						Edge *edge = new Edge(bb, cur->entry, Edge::VIRTUAL_CALL);
						CALLED_CFG(edge) = cur->cfg;
						RECURSIVE_LOOP(edge) = true;
						called = 0;
						break;
					}
				if(called) {
					assert(called_exit);
					//cout << "CALL " << bb->address() << " -> " << called_exit->address() << "\n";
					virtualize(&call, called, src, called_exit);
				}
			}
		}
}


/**
 */
void VirtualCFG::scan(void) {
	
	// Build the virtual CFG
	_bbs.add(&_entry);
	virtualize(0, _cfg, &_entry, &_exit);
	_bbs.add(&_exit);
	
	// Give a number to each BB
	for(int i = 0; i < _bbs.length(); i++) {
		INDEX(_bbs[i]) = i;
		_bbs[i]->_cfg = this;
	}
	
	// Set the tag
	flags |= FLAG_Scanned;
}


/**
 * Build a new virtual CFG from the given CFG.
 * @param cfg		CFG to buid from.
 * @param inlined	If true, performs inlining.
 */
VirtualCFG::VirtualCFG(CFG *cfg, bool inlined): _cfg(cfg) {
	assert(cfg);
	flags |= FLAG_Virtual;
	if(inlined)
		flags |= FLAG_Inlined;
}


/**
 * @fn CFG *VirtualCFG::cfg(void) const;
 * Get the base CFG of the current virtual CFG.
 * @return	Base CFG.
 */

} // otawa
