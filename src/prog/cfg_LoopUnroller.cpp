

#include <stdio.h>
#include <elm/io.h>
#include <elm/genstruct/Vector.h>
#include <elm/genstruct/HashTable.h>
#include <elm/genstruct/VectorQueue.h>
#include <otawa/ipet/FlowFactLoader.h>
#include <otawa/ipet/IPET.h>
#include <otawa/util/Dominance.h>
#include <otawa/cfg.h>
#include <otawa/util/LoopInfoBuilder.h>
#include <otawa/cfg/CFGCollector.h>
#include <elm/util/Pair.h>

#include <otawa/cfg/LoopUnroller.h>
#include <otawa/cfg/Virtualizer.h>
using namespace otawa;
using namespace elm;



namespace otawa {
	
Feature<LoopUnroller> UNROLLED_LOOPS_FEATURE ("otawa::unrolled_loops_feature");


/**
 * @class LoopUnroller
 *
 * This processor unrolls the first iteration of each loop
 *
 * @par Configuration
 * none
 *
 * @par Required features
 * @li @ref DOMINANCE_FEATURE
 * @li @ref LOOP_HEADERS_FEATURE
 * @li @ref FLOW_FACT_FEATURE
 * @li @ref LOOP_INFO_FEATURE
 * @li @ref COLLECTED_CFGS_FEATURE

 *
 * @par Provided features
 * @li @ref UNROLLED_LOOPS_FEATURE
 * 
 * @par Statistics
 * none
 */

LoopUnroller::LoopUnroller(void) : Processor("otawa::LoopUnroller", Version(1, 0, 0)), coll(new CFGCollection()) {
	require(DOMINANCE_FEATURE);
	require(LOOP_HEADERS_FEATURE);
	require(LOOP_INFO_FEATURE);
	require(COLLECTED_CFG_FEATURE);
	require(ipet::FLOW_FACTS_FEATURE);

	invalidate(COLLECTED_CFG_FEATURE);
	
	provide(COLLECTED_CFG_FEATURE);
	provide(UNROLLED_LOOPS_FEATURE);
	provide(ipet::FLOW_FACTS_FEATURE);
	idx = 0;
}

void LoopUnroller::processWorkSpace(otawa::WorkSpace *fw) {
	
	int cfgidx = 0;	
	CFGCollection *orig_coll = INVOLVED_CFGS(fw);
	
	// Create the new VCFG collection first, so that it will be available when we do the loop unrolling
	for (CFGCollection::Iterator cfg(*orig_coll); cfg; cfg++, cfgidx++) {
		VirtualCFG *vcfg = new VirtualCFG();
		coll->cfgs.add(vcfg);
		INDEX(vcfg) = cfgidx;
		vcfg->addBB(vcfg->entry());
		
	}
	
	
	cfgidx = 0;
	for (CFGCollection::Iterator vcfg(*coll), cfg(*orig_coll); vcfg; vcfg++, cfg++) {
		ASSERT(INDEX(vcfg) == INDEX(cfg));
		LABEL(vcfg) = cfg->label();
		INDEX(vcfg->entry()) = 0;	
		
		
		idx = 1;
//		if (isVerbose()) {
			cout << "Processing CFG: " << cfg->label() << "\n";
		//}
		
		/* !!GRUIK!! Ca serait bien d'avoir une classe VCFGCollection */
		VirtualCFG *casted_vcfg = static_cast<otawa::VirtualCFG*>((otawa::CFG*)vcfg);
		
		unroll((otawa::CFG*) cfg, NULL, casted_vcfg);
		if (ENTRY_CFG(fw) == cfg) 
			ENTRY_CFG(fw) = vcfg;	
		
		casted_vcfg->addBB(vcfg->exit());		
		INDEX(vcfg->exit()) = idx;		
	}
	INVOLVED_CFGS(fw) = coll;
}



void LoopUnroller::unroll(otawa::CFG *cfg, BasicBlock *header, VirtualCFG *vcfg) {	
	VectorQueue<BasicBlock*> workList;
	VectorQueue<BasicBlock*> loopList;
	VectorQueue<BasicBlock*> virtualCallList;
	genstruct::Vector<BasicBlock*> doneList;
	typedef genstruct::Vector<Pair<VirtualBasicBlock*, Edge::kind_t> > BackEdgePairVector;
	BackEdgePairVector backEdges;
	bool dont_unroll = false;
	int start;
	 
	
	/* Avoid unrolling loops with LOOP_COUNT of 0, since it would create a LOOP_COUNT of -1 for the non-unrolled part of the loop*/
	if (header && (ipet::LOOP_COUNT(header) == 0)) {
		dont_unroll = true;
	}
	
	//if (header) dont_unroll = true;
	start = dont_unroll ? 1 : 0;
	
	for (int i = start; ((i < 2) && header) || (i < 1); i++) {		
		doneList.clear();
		assert(workList.isEmpty());
		assert(loopList.isEmpty());
		assert(doneList.isEmpty());
	
		workList.put(header ? header : cfg->entry());	
		doneList.add(header ? header : cfg->entry());

		genstruct::Vector<BasicBlock*> bbs;
		
		while (!workList.isEmpty()) {
		
			BasicBlock *current = workList.get();
			
			if (Dominance::isLoopHeader(current) && (current != header)) {
				/* we enter another loop */
				loopList.put(current); 
				
				/* add exit edges destinations to the worklist */
				for (genstruct::Vector<Edge*>::Iterator exitedge(*EXIT_LIST(current)); exitedge; exitedge++) {
					if (!doneList.contains(exitedge->target())) {
						workList.put(exitedge->target());
						doneList.add(exitedge->target());
					}
				} 	
			} else {
				VirtualBasicBlock *new_bb = NULL;
				if ((!current->isEntry()) && (!current->isExit())) {
					/* Duplicate the current basic block */
				
					new_bb = new VirtualBasicBlock(current);
					new_bb->removeAllProp(&ENCLOSING_LOOP_HEADER);
					new_bb->removeAllProp(&EXIT_LIST);
					new_bb->removeAllProp(&REVERSE_DOM);
					new_bb->removeAllProp(&LOOP_EXIT_EDGE);
					new_bb->removeAllProp(&LOOP_HEADER);
					new_bb->removeAllProp(&ENTRY); 
					
					/* Remember the call block so we can correct its destination when we have processed it */
					if (VIRTUAL_RETURN_BLOCK(new_bb))
						virtualCallList.put(new_bb);
					
					if (ipet::LOOP_COUNT(new_bb) != -1) {
						if (i == 0) {
							new_bb->removeAllProp(&ipet::LOOP_COUNT);
						} else {
							int old_count = ipet::LOOP_COUNT(new_bb);
							new_bb->removeAllProp(&ipet::LOOP_COUNT);
							ipet::LOOP_COUNT(new_bb) = old_count - (1 - start);
							assert(ipet::LOOP_COUNT(new_bb) >= 0);
							
						}
					}
					
					INDEX(new_bb) = idx;
					idx++;
					vcfg->addBB(new_bb);
				
				
					bbs.add(current);
//					cout << "Adding virtual BB " << new_bb->number() << " for BB: " << current->number() << "of cfg " << current->cfg()->label() << " ( i == " << i << ") with count " << ipet::LOOP_COUNT(new_bb) << "\n"; 
					map.put(current, new_bb);
				}
						
				
				/* add successors which are in loop (including possible sub-loop headers) */ 
				for (BasicBlock::OutIterator outedge(current); outedge; outedge++) {
					if (outedge->target() == cfg->exit())
						continue;
					if (outedge->kind() == Edge::CALL)
						continue;
					if (ENCLOSING_LOOP_HEADER(outedge->target()) == header) {
						if (!doneList.contains(outedge->target())) {
							workList.put(outedge->target());
							doneList.add(outedge->target());
						}
					}
					if (LOOP_EXIT_EDGE(outedge)) {
						assert(new_bb);
						/* Connect exit edge */
						VirtualBasicBlock *vdst = map.get(outedge->target());
						new Edge(new_bb, vdst, outedge->kind());
					}
				} 
				
			}					
		}
		
		while (!virtualCallList.isEmpty()) {
			BasicBlock *vcall = virtualCallList.get();
			BasicBlock *vreturn = map.get(VIRTUAL_RETURN_BLOCK(vcall), NULL);
			ASSERT(vreturn != NULL);
			VIRTUAL_RETURN_BLOCK(vcall) = vreturn;
		}


		while (!loopList.isEmpty()) {
			BasicBlock *loop = loopList.get();
			unroll(cfg, loop, vcfg);
		}
		
		
		/* Connect the internal edges for the current loop */
		for (genstruct::Vector<BasicBlock*>::Iterator bb(bbs); bb; bb++) {
			for (BasicBlock::OutIterator outedge(bb); outedge; outedge++) {
				if (LOOP_EXIT_EDGE(outedge))
					continue;
				if (Dominance::isLoopHeader(outedge->target()) && (outedge->target() != header))
					continue;
				if (outedge->target() == cfg->exit())
					continue;
					
				VirtualBasicBlock *vsrc = map.get(*bb, NULL);
				VirtualBasicBlock *vdst = map.get(outedge->target(), NULL);
				if (outedge->kind() == Edge::CALL) {
					CFG *called_cfg = outedge->calledCFG();
					int called_idx = INDEX(called_cfg);
					CFG *called_vcfg = coll->get(called_idx);
					new Edge(vsrc, called_vcfg->entry(), Edge::CALL);
					ENTRY(called_vcfg->entry()) = called_vcfg;
					CALLED_CFG(outedge) = called_vcfg;
				
				} else if ((outedge->target() != header) || ((i == 1) /* XXX && !dont_unroll */ )) {
					new Edge(vsrc, vdst, outedge->kind());
				} else {
					backEdges.add(pair(vsrc, outedge->kind()));
				}
			}
		}
		
		if (i == start) {
			/* Connect virtual entry edges */
			if (header) {
				for (BasicBlock::InIterator inedge(header); inedge; inedge++) {
					if (Dominance::dominates(header, inedge->source()))
						continue; /* skip back edges */
					if (inedge->source() == cfg->entry())
						continue;
					VirtualBasicBlock *vsrc = map.get(inedge->source());
					VirtualBasicBlock *vdst = map.get(header);
					new Edge(vsrc, vdst, inedge->kind());									
				}
			}
			
		} else { 			
			/* Connect virtual backedges from the first to the other iterations */	
			for (BackEdgePairVector::Iterator iter(backEdges); iter; iter++) {		
				VirtualBasicBlock *vdst = map.get(header);
				new Edge((*iter).fst, vdst, (*iter).snd);  
			}
		}	
	}	
	if (!header) {
		/* add main entry edges */
		for (BasicBlock::OutIterator outedge(cfg->entry()); outedge; outedge++) {
			VirtualBasicBlock *vdst = map.get(outedge->target());
			new Edge(vcfg->entry(), vdst, Edge::VIRTUAL_CALL);
		}
		/* add main exit edges */
		for (BasicBlock::InIterator inedge(cfg->exit()); inedge; inedge++) {
			VirtualBasicBlock *vsrc = map.get(inedge->source());
			new Edge(vsrc, vcfg->exit(), Edge::VIRTUAL_RETURN);
		}
	}
}

} /* end namespace */
