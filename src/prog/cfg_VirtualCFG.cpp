/*
 * $Id$
 * Copyright (c) 2005 IRIT-UPS
 * 
 * src/prog/cfg_VirtualCFG.cpp -- implementation of VirtualCFG class.
 */

#include <elm/genstruct/HashTable.h>
#include <otawa/cfg.h>
#include <otawa/cfg/VirtualCFG.h>
#include <otawa/cfg/VirtualBasicBlock.h>

// HashKey for address_t
namespace elm {
class AddressHashKey: public elm::HashKey<otawa::address_t> {
public:
	virtual unsigned long hash (otawa::address_t key)
		{ return (unsigned long)key; };
	virtual bool equals (otawa::address_t key1, otawa::address_t key2)
		{ return key1 == key2; };
};
static AddressHashKey def;
template <> HashKey<otawa::address_t>& HashKey<otawa::address_t>::def
	= AddressHashKey::def;
} // elm

namespace otawa {

/* Used for resolving recursive calls as loops */
typedef struct call_t {
	struct call_t *back;
	CFG *cfg;
	BasicBlock *entry;
} call_t;


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
	
	// Prepare data
	elm::genstruct::HashTable<address_t, BasicBlock *> map;
	call_t call = { stack, cfg, 0 };
	
	// Translate BB
	for(Iterator<BasicBlock *> bb(cfg->bbs()); bb; bb++)
		if(!bb->isEntry() && !bb->isExit()) {
			BasicBlock *new_bb = new VirtualBasicBlock(bb);
			map.put(bb->address(), new_bb);
			_bbs.add(new_bb);
		}
	
	// Find local entry
	for(Iterator<Edge *> edge(cfg->entry()->outEdges()); edge; edge++) {
		assert(!call.entry);
		call.entry = map.get(edge->target()->address(), 0);
		assert(call.entry);
		new Edge(entry, call.entry, Edge::VIRTUAL_CALL);
	}
	
	// Translat edges
	for(Iterator<BasicBlock *> bb(cfg->bbs()); bb; bb++)
		if(!bb->isEntry() && !bb->isExit()) {
			CFG *called = 0;
			BasicBlock *called_exit = 0;
			
			// Resolve source
			BasicBlock *src = map.get(bb->address(), 0);
			assert(src);
			
			// Look edges
			for(Iterator<Edge *> edge(bb->outEdges()); edge; edge++)
				if(edge->target()->isExit()) {
					new Edge(src, exit, Edge::VIRTUAL_RETURN);
					called_exit = exit;
				}
				else if(edge->kind() == Edge::CALL && isInlined())
					called = edge->calledCFG();
				else if(edge->target()) {
					BasicBlock *tgt = map.get(edge->target()->address(), 0);
					assert(tgt);
					new Edge(src, tgt, edge->kind());
					called_exit = tgt;
				}
			
			// Process the call
			if(called && isInlined()) {
				for(call_t *cur = &call; cur; cur = cur->back)
					if(cur->cfg == called) {
						new Edge(bb, cur->entry, Edge::VIRTUAL_CALL);
						called = 0;
						break;
					}
				if(called) {
					assert(called_exit);
					virtualize(&call, called, bb, called_exit);
				}
			}
		}
}


/**
 */
void VirtualCFG::scan(void) {
	
	// Build the virtual CFG
	_bbs.add(entry());
	virtualize(0, _cfg, entry(), exit());
	_bbs.add(exit());
	
	// Give a number to each BB
	for(int i = 0; i < _bbs.length(); i++)
		_bbs[i]->add<int>(ID_Index, i);
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
