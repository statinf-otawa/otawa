#include <elm/io.h>
#include <otawa/cfg.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/cfg/Virtualizer.h>
#include <otawa/ipet/FlowFactLoader.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/prog/Manager.h>

using namespace otawa;
using namespace elm;



namespace otawa {


/* Used for resolving recursive calls as loops */
typedef struct call_t {
        struct call_t *back;
        CFG *cfg;
        BasicBlock *entry;
} call_t;

Feature<Virtualizer> VIRTUALIZED_CFG_FEATURE("otawa::virtualized_cfg_feature");
Identifier<bool> VIRTUAL_INLINING("otawa::virtual_inlining", true);


/**
 * @class Virtualizer
 *
 * This processor inlines the function calls. 
 *
 * @par Configuration
 * @li@ref DONT_UNROLL : The CFG with DONT_UNROLL(cfg) == true are not inlined
 *
 * @par Required features
 * @li @ref FLOW_FACTS_FEATURE
 *
 * @par Invalidated features
 * @li @ref COLLECTED_CFG_FEATURE
 * 
 * @par Provided features
 * @li @ref VIRTUALIZED_CFG_FEATURES
 * 
 * @par Statistics
 * none
 */

Virtualizer::Virtualizer(void) : Processor("otawa::Virtualizer", Version(1, 0, 0)) {
	require(ipet::FLOW_FACTS_FEATURE);
	invalidate(COLLECTED_CFG_FEATURE);
	provide(VIRTUALIZED_CFG_FEATURE);
	provide(ipet::FLOW_FACTS_FEATURE);
}

void Virtualizer::processWorkSpace(otawa::WorkSpace *fw) {

	CFGCollection *coll = INVOLVED_CFGS(fw);	
	VirtualCFG *vcfg = new VirtualCFG();
	
        if(!entry) {
                CFGInfo *info = fw->getCFGInfo();
                CString name = TASK_ENTRY(fw);
                entry = info->findCFG(name);
        }
        if(!entry)
                throw ProcessorException(*this, "cannot find task entry point.");

	
	virtual_inlining = VIRTUAL_INLINING(fw);
	virtualize(0, entry, vcfg, vcfg->entry(), vcfg->exit());
	vcfg->numberBBs();

	ENTRY_CFG(fw) = vcfg;
	if (coll != NULL)
		delete coll;
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
	
	// Translate BB
	for(Iterator<BasicBlock *> bb(cfg->bbs()); bb; bb++)
		if(!bb->isEntry() && !bb->isExit()) {
			BasicBlock *new_bb = new VirtualBasicBlock(bb);
			map.put(bb, new_bb);
			vcfg->addBB(new_bb);
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
//			assert(!bb->isVirtual());
			
			// Resolve source
			BasicBlock *src = map.get(bb, 0);
			assert(src);

			// Is there a call ?			
			CFG *called = 0;
			BasicBlock *called_exit = 0;
			if(isInlined())
				for(BasicBlock::OutIterator edge(bb); edge; edge++)
					if(edge->kind() == Edge::CALL) {
						called = edge->calledCFG();
						if (DONT_INLINE(called))
						        called = NULL;
                        }

			// Look edges
			for(BasicBlock::OutIterator edge(bb); edge; edge++)
				if(edge->kind() == Edge::CALL) {
					if(!isInlined() || DONT_INLINE(edge->calledCFG()))
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
					virtualize(&call, called, vcfg, src, called_exit);
				}
			}
		}
}


} /* end namespace */
