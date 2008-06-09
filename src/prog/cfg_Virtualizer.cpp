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


/**
 * This features only show that the CFG has been virtualized. This may implies
 * a lot of transformation like function call inlining or loop unrolling.
 * 
 * @par Properties
 * @li @ref CALLED_CFG (@ref Edge) -- put on edge resulting from a function
 * inlining to identify the original CFG.
 */
Feature<Virtualizer> VIRTUALIZED_CFG_FEATURE("otawa::VIRTUALIZED_CFG_FEATURE");



Identifier<bool> VIRTUAL_INLINING("otawa::virtual_inlining", true);
Identifier<BasicBlock*> VIRTUAL_RETURN_BLOCK("otawa::virtual_return_block", NULL);


/**
 * @class Virtualizer
 *
 * This processor inlines the function calls. 
 *
 * @par Configuration
 * @li @ref DONT_INLINE : The CFG with DONT_INLINE(cfg) == true are not inlined
 *
 * @par Required features
 * @li @ref FLOW_FACTS_FEATURE
 *
 * @par Invalidated features
 * @li @ref COLLECTED_CFG_FEATURE
 * 
 * @par Provided features
 * @li @ref VIRTUALIZED_CFG_FEATURE
 * 
 * @par Statistics
 * none
 */

Virtualizer::Virtualizer(void) : Processor("otawa::Virtualizer", Version(1, 0, 0)) {
	//require(ipet::FLOW_FACTS_FEATURE);
	require(COLLECTED_CFG_FEATURE);
	invalidate(COLLECTED_CFG_FEATURE);
	provide(VIRTUALIZED_CFG_FEATURE);
	//provide(ipet::FLOW_FACTS_FEATURE);
}

void Virtualizer::processWorkSpace(otawa::WorkSpace *fw) {

	CFGCollection *coll = INVOLVED_CFGS(fw);	
	VirtualCFG *vcfg = new VirtualCFG(false);
        if (!entry)
        	entry = ENTRY_CFG(fw);
        if(!entry) {
                CFGInfo *info = fw->getCFGInfo();
                CString name = TASK_ENTRY(fw);
                entry = info->findCFG(name);
        }
        if(!entry)
                throw ProcessorException(*this, "cannot find task entry point.");

	vcfg->addProps(*entry);
	vcfg->entry()->addProps(*entry->entry());
	vcfg->exit()->addProps(*entry->exit());
	
	virtual_inlining = VIRTUAL_INLINING(fw);
	vcfg->addBB(vcfg->entry());
	virtualize(0, entry, vcfg, vcfg->entry(), vcfg->exit());
	vcfg->addBB(vcfg->exit());
	vcfg->numberBBs();
	if(isVerbose())
		out << "INFO: " << vcfg->countBB() << " basic blocks." << io::endl;
		
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
	Vector<CFG *> called_cfgs;
	
	// Translate BB
	for(CFG::BBIterator bb(cfg); bb; bb++)
		if(!bb->isEntry() && !bb->isExit()) {
			BasicBlock *new_bb = new VirtualBasicBlock(bb);
			map.put(bb, new_bb);
			vcfg->addBB(new_bb);
			// !!DEBUG!!
			//cerr << (void *)bb << " -> " << (void *)new_bb << io::endl;
		}
	
	// Find local entry
	for(BasicBlock::OutIterator edge(cfg->entry()); edge; edge++) {
		ASSERT(edge->kind() == Edge::VIRTUAL);
		ASSERT(!call.entry);
		call.entry = map.get(edge->target(), 0);
		ASSERT(call.entry);
		Edge *edge = new Edge(entry, call.entry, Edge::VIRTUAL_CALL);
		CALLED_CFG(edge) = cfg;
	}
	
	// Translate edges
	for(CFG::BBIterator bb(cfg); bb; bb++)
		if(!bb->isEntry() && !bb->isExit()) {
//			assert(!bb->isVirtual());
			
			// Resolve source
			BasicBlock *src = map.get(bb, 0);
			assert(src);

			// Is there a call ?			
			// CFG *called = 0;
			
			BasicBlock *called_exit = 0;
			if(isInlined())
				for(BasicBlock::OutIterator edge(bb); edge; edge++)
					if(edge->kind() == Edge::CALL) {
						if (DONT_INLINE(edge->calledCFG()))  {
							if ((cfgMap.get(edge->calledCFG(), 0) == 0)) {
								VirtualCFG *vcalled = new VirtualCFG(false);
								cfgMap.put(edge->calledCFG(), vcalled);
								vcalled->addProps(*edge->calledCFG());
								ENTRY(vcalled->entry()) = vcalled;
								vcalled->entry()->addProps(*edge->calledCFG()->entry());
								vcalled->exit()->addProps(*edge->calledCFG()->exit());
								vcalled->addBB(vcalled->entry());
								virtualize(&call, edge->calledCFG(), vcalled, vcalled->entry(), vcalled->exit());
								vcalled->addBB(vcalled->exit());
								vcalled->numberBB();
							}  
						} else if(edge->calledCFG())
							called_cfgs.add(edge->calledCFG());
					}
						
			// Look edges
			for(BasicBlock::OutIterator edge(bb); edge; edge++)
				if(edge->kind() == Edge::CALL) {
					if(!isInlined()) {
						new Edge(src, edge->target(), Edge::CALL);
					}
					if (isInlined() && DONT_INLINE(edge->calledCFG())) {
						VirtualCFG *vcalled = cfgMap.get(edge->calledCFG(), 0);
						ASSERT(vcalled != 0);
						new Edge(src, vcalled->entry(), Edge::CALL);
					}
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
						if(called_cfgs)
							called_exit = tgt;
						else
							new Edge(src, tgt, edge->kind());
					}
				}
			
			// Process the call
			if(called_cfgs) {
				
				// Process each call
				for(Vector<CFG *>::Iterator called(called_cfgs); called; called++) {
					
					// Check recursivity
					bool recursive = false;
					for(call_t *cur = &call; cur; cur = cur->back)
						if(cur->cfg == called) {
							recursive = true;
							Edge *edge = new Edge(map.get(bb), cur->entry, Edge::VIRTUAL_CALL);
							CALLED_CFG(edge) = cur->cfg;
							RECURSIVE_LOOP(edge) = true;
							VIRTUAL_RETURN_BLOCK(src) = called_exit;
							if(isVerbose())
								out << "INFO: recursivity found at " << bb->address()
									<< " to " << called->label() << io::endl;
							break;
						}
					
					// Virtualize the called CFG
					if(!recursive) {
						ASSERT(called_exit);
						VIRTUAL_RETURN_BLOCK(src) = called_exit;
						virtualize(&call, called, vcfg, src, called_exit);

					}
				}
				
				// Reset the called list
				called_cfgs.setLength(0);
			}
		}
}


} /* end namespace */
