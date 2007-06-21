/*
 *	$Id$
 *	Copyright (c) 2005-07, IRIT UPS.
 *
 *	CCGConstraintsBuilder class implementation
 */
#include <elm/io.h>
#include <otawa/cache/ccg/CCGConstraintBuilder.h>
#include <otawa/cfg.h>
#include <otawa/instruction.h>
#include <otawa/cache/ccg/CCGNode.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/cache/ccg/CCGDFA.h>
#include <otawa/ilp.h>
#include <otawa/ipet/IPET.h>
#include <elm/Collection.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/util/Dominance.h>
//#include <otawa/util/DFABitSet.h>
#include <otawa/util/ContextTree.h>
#include <otawa/cfg.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Platform.h>
#include <otawa/cache/ccg/CCGBuilder.h>
#include <otawa/ipet.h>
#include <otawa/util/LBlockBuilder.h>
#include <otawa/ipet/TrivialInstCacheManager.h>

using namespace otawa::ilp;
using namespace otawa;
using namespace elm::genstruct;
using namespace otawa::ipet;

namespace otawa {

namespace ipet {

/* Properties */
static Identifier<ilp::Var *> BB_VAR("otawa::bb_var", 0);
static Identifier<ilp::Var *> HIT_VAR("otawa::hit_var", 0);
static Identifier<ilp::Var *> MISS_VAR("otawa::miss_var", 0);


/**
 * @class CCGConstraintBuilder
 * This processor allows handling timing effects of the instruction cache in
 * the IPET approach. Based on the Cache Conflict Graph of the task,
 * it generates constraints and improve the objective function of the ILP system
 * representing the timing behaviour of the task.
 * 
 * @par Provided Features
 * @li @ref ICACHE_SUPPORT_FEATURE
 * 
 * @par Required Features
 * @li @ref CCG_FEATURE
 * @li @ref COLLECTED_LBLOCKS_FEATURE
 * @li @ref ASSIGNED_VARS_FEATURE
 * @li @ref CONTEXT_TREE_FEATURE
 */

/**
 * Constructor.
 */
CCGConstraintBuilder::CCGConstraintBuilder(void):
	Processor("otawa::ipet::CCGConstrainterBuilder", Version(1, 0, 0)),
	_explicit(false)
{
	require(CCG_FEATURE);
	require(ASSIGNED_VARS_FEATURE);
	require(COLLECTED_LBLOCKS_FEATURE);
	require(CONTEXT_TREE_FEATURE);
	provide(INST_CACHE_SUPPORT_FEATURE);
}


/**
 */
void CCGConstraintBuilder::configure(const PropList& props) {
	Processor::configure(props);
	_explicit = EXPLICIT(props);
}


/**
 */
void CCGConstraintBuilder::processLBlockSet(WorkSpace *fw, LBlockSet *lbset) {
	
	// Initialization
	CFG *entry_cfg = ENTRY_CFG(fw);
	System *system = getSystem(fw, ENTRY_CFG(fw));
	assert (system);
	const hard::Cache *cach = fw->platform()->cache().instCache();
	int dec = cach->blockBits();
	
	// Initialization
	for(LBlockSet::Iterator lblock(*lbset); lblock; lblock++) {
		
		// Build variables
		if(lblock->bb()) {
			
			// Link BB variable
			ilp::Var *bbvar = VAR(lblock->bb());
			assert(bbvar);
			BB_VAR(lblock) = bbvar;
		
			// Create x_hit variable
			String namex;
			if(_explicit) {
				StringBuffer buf;
				buf << "xhit_" << lblock->address()
					<< '_' << lblock->bb()->number()
					<< '_' << lblock->bb()->cfg()->label();
				namex = buf.toString();
			}
			ilp::Var *vhit = system->newVar(namex);
			HIT_VAR(lblock) = vhit;
		
			// Create x_miss variable
			String name1;
			if(_explicit) {
				StringBuffer buf1;
				buf1 << "xmiss_" << lblock->address()
					 << '_' << lblock->bb()->number()
					 << '_' << lblock->bb()->cfg()->label();
			 	name1 = buf1.toString();
			}
			ilp::Var *miss = system->newVar(name1);
			MISS_VAR(lblock) = miss;
		}
		
		// Put variables on edges
		for(CCG::Successor succ(CCG::NODE(lblock)); succ; succ++) {
			String name;
			if(_explicit) {
				StringBuffer buf;
				buf << "eccg_";
				if(!lblock->address())
					buf << "ENTRY";
				else
					buf << lblock->address()
						<< '_' << lblock->bb()->number()
						<< '_' << lblock->bb()->cfg()->label();
				buf << '_';
				if(!succ->lblock()->address())
					buf << "EXIT";
				else
					buf << succ->lblock()->address()
						<< '_' << succ->lblock()->bb()->number()
						<< '_' << succ->lblock()->bb()->cfg()->label();
				name = buf.toString();
			}
			VAR(succ.edge()) = system->newVar(name); 
		}
	}
	
	// Building all the constraints of each lblock
	for (Iterator<LBlock *> lbloc(lbset->visit()); lbloc; lbloc++) {
		
		
	/* P(x,y) == eccg_x_y */	
	/* 
	 * (entry node) Rule 18:
	 * sum P(entry,*) = 1
	 */
	if (lbloc->id() == 0) {
		// !!CONS!!
		Constraint *cons18 = system->newConstraint(Constraint::EQ,1);
		for(CCG::Successor outedg(CCG::NODE(lbloc)); outedg; outedg++)
			cons18->add(1, VAR(outedg.edge()));
	}
		
		// Non-entry, non-exit node
		if(lbloc->id() != 0 && lbloc->id() != lbset->count() - 1) {
			int identif = lbloc->id();				
			address_t address = lbloc->address();
			
			/*
			 * Rule 13:
                         * xi = xhit_xxxxx_i + xmiss_xxxx_i 
			 */
			Constraint *cons = system->newConstraint(Constraint::EQ);
			cons->addLeft(1, BB_VAR(lbloc));
			cons->addRight(1, HIT_VAR(lbloc));
			cons->addRight(1, MISS_VAR(lbloc));
		
			//contraints of input/output (17)	
			/*
			 * Rule 17:
			 * xi = sum p(*, xij) = sum p(xij, *)
                         */
			bool used = false;
			bool findend = false;
			bool findlooplb = false;
			ilp::Var *xhit;
			ilp::Var *pii;
			Constraint *cons2;
			// !!CONS!!
			Constraint *cons17 = system->newConstraint(Constraint::EQ);
			cons17->addLeft(1, BB_VAR(lbloc));
			
			for(GenGraph<CCGNode,CCGEdge>::Successor outedg(CCG::NODE(lbloc));
			outedg; outedg++) {
				cons17->addRight(1, VAR(outedg.edge()));
				CCGNode *target = outedg;
				if (target->lblock()->id() == lbset->count() - 1)
					findend = true;
				if (target->lblock()->id() == lbloc->id()) {
					findlooplb = true;
					pii = VAR(outedg.edge());
				}
				used = true;
				// p(ij, uv)  <= xi
				/*cons2 = system->newConstraint(Constraint::LE);
				cons2->addLeft(1,outedg.edge()->varEDGE());
				cons2->addRight(1, lbloc->use<ilp::Var *>(CCGBuilder::ID_BBVar));*/
			}
			
			if(!used)
				delete cons17;
				
			// contraint of input(17)	
			cons = system->newConstraint(Constraint::EQ);
			cons->addLeft(1, BB_VAR(lbloc));
			used = false;
			bool finds = false;
			ilp::Var * psi;
			for(GenGraph<CCGNode,CCGEdge>::Predecessor inedge(CCG::NODE(lbloc));
			inedge; inedge++) {

				cons->addRight(1, VAR(inedge.edge()));
				CCGNode *source = inedge;
				if (source->lblock()->id() == 0){
					 finds = true;
					 psi = VAR(inedge.edge());
				}
				used = true;
				
				// building contraint (30)
				// p(uv, ij) <= xi
				/*if (lbloc->id() != inedge->lblock()->id()) {
					cons2 = system->newConstraint(Constraint::LE);
					cons2->addLeft(1,inedge.edge()->varEDGE());
					cons2->addRight(1, lbloc->use<ilp::Var *>(CCGBuilder::ID_BBVar));
				}*/
                        }
                if(!used)
                        delete cons;
				
		// building contraints (19) & (20)
		// cache_block(uv) = cach_block(ij)
		// (19) p(ij, ij) + p(uv, ij) <= xihit <= p(ij, ij) + p(uv, ij) + p(entry, ij) if p(entry, ij) and p(ij, exit)
		// (20) p(ij, ij) + p(uv, ij) = xihit else
		if (findlooplb) {
		  	if (finds && findend) {
		  	        // constraint 19
		 		cons = system->newConstraint(Constraint::LE);
		 		cons2 = system->newConstraint(Constraint::LE);
				cons->addLeft(1, HIT_VAR(lbloc));
				
				unsigned long taglbloc = ((unsigned long)lbloc->address()) >> dec;
				for(CCG::Predecessor inedge(CCG::NODE(lbloc)); inedge; inedge++)
				{
					unsigned long taginedge = ((unsigned long)inedge->lblock()->address()) >> dec;
					if(inedge->lblock()->id() != 0
					&& inedge->lblock()->id() != lbset->count() - 1){
						if (taglbloc == taginedge) {
								cons->addRight(1, VAR(inedge.edge()));
								cons2->addLeft(1, VAR(inedge.edge()));
						}
					}
		 		}
				cons->addRight(1, psi);
				cons2->addRight(1, HIT_VAR(lbloc));
		 	}
		 	
		 	else {
				// contraint 20
		 		cons2 = system->newConstraint(Constraint::EQ);
		 		cons2->addLeft(1, HIT_VAR(lbloc));
		 		unsigned long taglbloc = ((unsigned long)lbloc->address()) >> dec;
		 		for(GenGraph<CCGNode,CCGEdge>::Predecessor inedge(CCG::NODE(lbloc));
		 		inedge; inedge++) {
		 			unsigned long taginedge = ((unsigned long)inedge->lblock()->address()) >> dec;
					if(inedge->lblock()->id() != 0
					&& inedge->lblock()->id() != lbset->count() - 1){
						if (taglbloc == taginedge)
							cons2->addRight(1, VAR(inedge.edge()));
					}
		 		}
		 	} 
		 }
		
		// building the (16)
//		if(lbloc->getNonConflictState() && !findlooplb){
                // xihit = sum p(uv, ij) / cache_block(uv) = cache_block(ij)
		else if(CCGBuilder::NON_CONFLICT(lbloc)) {
			cons = system->newConstraint(Constraint::EQ);
			cons->addLeft(1, HIT_VAR(lbloc));
			unsigned long taglbloc = ((unsigned long)lbloc->address()) >> dec;
			for(CCG::Predecessor inedge(CCG::NODE(lbloc)); inedge; inedge++) {
				unsigned long taginedge = ((unsigned long)inedge->lblock()->address()) >> 3;
				if(inedge->lblock()->id() != 0
					&& inedge->lblock()->id() != lbset->count() - 1) {
						if(taglbloc == taginedge)
							cons->addRight(1,  VAR(inedge.edge()));
					}					
				}
			}
				
			//builduig the constraint (32)
			ContextTree *cont = CONTEXT_TREE(fw);
			addConstraintHeader(system, lbset, cont, lbloc);
		}
	}
  
  	// Fix the object function
	for(Iterator<LBlock *> lbloc(lbset->visit()); lbloc; lbloc++) {
		if(lbloc->id() != 0 && lbloc->id() != lbset->count()- 1)
  			system->addObjectFunction( cach->missPenalty(), MISS_VAR(lbloc));
	}		
}


/**
 */
void CCGConstraintBuilder::processWorkSpace(WorkSpace *fw) {
	assert(fw);
	LBlockSet **lbsets = LBLOCKS(fw);
	const hard::Cache *cache = fw->platform()->cache().instCache();
	if(!cache)
		return;
	for(int i = 0; i < cache->rowCount(); i++)
		processLBlockSet(fw, lbsets[i]);
}

/**
 */
void CCGConstraintBuilder::addConstraintHeader(
	ilp::System *system,
	LBlockSet *graph,
	ContextTree *cont,
	LBlock *boc
) {
	int size = graph->count();
	bool dominate = false;
	for(Iterator<ContextTree *> son(cont->children()); son; son++)
		addConstraintHeader(system, graph, son, boc);
	BasicBlock *b = boc->bb();
	if(cont->kind() == ContextTree::LOOP){
		for(Iterator<BasicBlock *> bs(cont->bbs()); bs; bs++) {
			if (b == bs){
			    // p(uv, ij) / header not dom bb(uv) <= sum xi
			    // / (xi, header) in E and header not dom xi
				BasicBlock *header = cont->bb();
				bool used = false;
				Constraint *cons32 = system->newConstraint(Constraint::LE);
				for(GenGraph<CCGNode,CCGEdge>::Predecessor inedge(CCG::NODE(boc));
				inedge; inedge++) {
					CCGNode *source = inedge;
					if(source->lblock()->id() != 0
					&& source->lblock()->id() !=  size-1) {
						BasicBlock *bblock = source->lblock()->bb();
						if(header->cfg() != bblock->cfg())
							throw ProcessorException(*this,
								"the function calls in this task must be "
								"virtualized to be safely handled by this "
								"processor !");
						else
							dominate = Dominance::dominates(header, bblock);
					}
					if(boc != source->lblock()
					&& (!dominate || source->lblock()->id() == 0)) {
						cons32->addLeft(1, VAR(inedge.edge()));
						dominate = false;
						used = true;
					}
					
				}
				if(used) {
				        bool set = false;
					for(Iterator<Edge *> inedg(header->inEdges()); inedg ; inedg++) {
						BasicBlock *preheader = inedg->source();
						if(!Dominance::dominates(header, inedg->source()))
						cons32->addRight(1, preheader->use<ilp::Var *>(VAR));
						set = true;
					}
					assert(set);
				}
				else
					delete cons32;
				break;
			}
		}
	}	
}

} } //otawa::ipet	



















