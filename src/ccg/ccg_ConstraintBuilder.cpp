/*
 *	CCGConstraintBuilder class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006, IRIT UPS.
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
#include <elm/io.h>
#include <otawa/ccg/ConstraintBuilder.h>
#include <otawa/cfg.h>
#include <otawa/instruction.h>
#include <otawa/ccg/Node.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/ccg/DFA.h>
#include <otawa/ilp.h>
#include <otawa/ipet/IPET.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/util/Dominance.h>
#include <otawa/util/ContextTree.h>
#include <otawa/cfg.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Platform.h>
#include <otawa/ccg/Builder.h>
#include <otawa/ipet.h>
#include <otawa/util/LBlockBuilder.h>
#include <otawa/ipet/TrivialInstCacheManager.h>

using namespace otawa::ilp;
using namespace otawa;
using namespace elm::genstruct;
using namespace otawa::ipet;

namespace otawa { namespace ccg {

/* Properties */
static Identifier<ilp::Var *> BB_VAR("otawa::ccg::BB_VAR", 0);
static Identifier<ilp::Var *> HIT_VAR("otawa::ccg::HIT_VAR", 0);


/**
 * Gives the variable counting the number of misses.
 *
 * @par Features
 * @li @ref CONSTRAINT_FEATURE
 *
 * @par Hooks
 * @li @ref LBlock
 *
 * @ingroup ccg
 */
Identifier<ilp::Var *> MISS_VAR("otawa::ccg::MISS_VAR", 0);


/**
 * This feature ensures that the CCG has been translated in the form of ILP constraints.
 *
 * @par Properties
 * @li @ref MISS_VAR
 *
 * @par Default Processor
 * @li @ref ConstraintBuilder
 *
 * @par Plugin
 * @li @ref ccg
 *
 * @ingroup ccg
 */
p::feature CONSTRAINT_FEATURE("otawa::ccg::CONSTRAINT_FEATURE", new Maker<ConstraintBuilder>());


/**
 * @class ConstraintBuilder
 * This processor allows handling timing effects of the instruction cache in
 * the IPET approach. Based on the Cache Conflict Graph of the task,
 * it generates constraints and improve the objective function of the ILP system
 * representing the timing behaviour of the task.
 *
 * @par Provided Features
 * @li @ref ICACHE_SUPPORT_FEATURE
 *
 * @par Required Features
 * @li @ref FEATURE
 * @li @ref COLLECTED_LBLOCKS_FEATURE
 * @li @ref ASSIGNED_VARS_FEATURE
 * @li @ref CONTEXT_TREE_FEATURE
 * @li @ref ILP_SYSTEM_FEATURE
 *
 * @ingroup ccg
 */

p::declare ConstraintBuilder::reg = p::init("otawa::ccg::ConstrainterBuilder", Version(1, 1, 0))
	.maker<ConstraintBuilder>()
	.require(FEATURE)
	.require(ASSIGNED_VARS_FEATURE)
	.require(COLLECTED_LBLOCKS_FEATURE)
	.require(CONTEXT_TREE_FEATURE)
	.require(ILP_SYSTEM_FEATURE)
	.provide(INST_CACHE_SUPPORT_FEATURE);


/**
 * Constructor.
 */
ConstraintBuilder::ConstraintBuilder(p::declare& r): Processor(r), _explicit(false) {
}


/**
 */
void ConstraintBuilder::configure(const PropList& props) {
	Processor::configure(props);
	_explicit = EXPLICIT(props);
}


/**
 */
void ConstraintBuilder::processLBlockSet(WorkSpace *fw, LBlockSet *lbset) {

	// Initialization
	//CFG *entry_cfg = ENTRY_CFG(fw);
	System *system = SYSTEM(fw);
	ASSERT (system);
	const hard::Cache *cach = hard::CACHE_CONFIGURATION(fw)->instCache();
	int dec = cach->blockBits();

	// Initialization
	for(LBlockSet::Iterator lblock(*lbset); lblock; lblock++) {

		// Build variables
		if(lblock->bb()) {

			// Link BB variable
			ilp::Var *bbvar = VAR(lblock->bb());
			ASSERT(bbvar);
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
		for(Graph::OutIterator edge(Graph::NODE(lblock)); edge; edge++) {
			Node *succ = edge->target();
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
			VAR(edge) = system->newVar(name);
		}
	}

	// Building all the constraints of each lblock
	for (LBlockSet::Iterator lbloc(*lbset); lbloc; lbloc++) {


		/* P(x,y) == eccg_x_y */
		/*
		 * (entry node) Rule 18:
		 * sum P(entry,*) = 1
		 */
		if (lbloc->id() == 0) {
			// !!CONS!!
			Constraint *cons18 = system->newConstraint(Constraint::EQ,1);
			for(Graph::OutIterator edge(Graph::NODE(lbloc)); edge; edge++)
				cons18->add(1, VAR(edge));
		}

		// Non-entry, non-exit node
		if(lbloc->id() != 0 && lbloc->id() != lbset->count() - 1) {
			/*int identif =*/ lbloc->id();
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
			//ilp::Var *xhit;
			Constraint *cons2;
			// !!CONS!!
			Constraint *cons17 = system->newConstraint(Constraint::EQ);
			cons17->addLeft(1, BB_VAR(lbloc));

			for(GenGraph<Node, Edge>::OutIterator edge(Graph::NODE(lbloc));
			edge; edge++) {
				cons17->addRight(1, VAR(edge));
				Node *target = edge->target();
				if (target->lblock()->id() == lbset->count() - 1)
					findend = true;


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
			for(GenGraph<Node, Edge>::InIterator inedge(Graph::NODE(lbloc));
			inedge; inedge++) {

				cons->addRight(1, VAR(inedge));
				Node *source = inedge->source();
				if (source->lblock()->id() == 0){
					 finds = true;
					 psi = VAR(inedge);
				}
				if (source->lblock()->cacheblock() == lbloc->cacheblock())
					findlooplb = true;
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
		// cout << "pre-examine block (addr = " <<  lbloc->address() <<   ") " << lbloc->id() << " findlooplb = " << findlooplb << "\n";


		if (findlooplb) {
		  	if (finds && findend) {
		  	        // constraint 19
		 		cons = system->newConstraint(Constraint::LE);
		 		cons2 = system->newConstraint(Constraint::LE);
				cons->addLeft(1, HIT_VAR(lbloc));

				unsigned long taglbloc = ((unsigned long)lbloc->address()) >> dec;
				for(Graph::InIterator inedge(Graph::NODE(lbloc)); inedge; inedge++)
				{
					unsigned long taginedge = ((unsigned long)inedge->source()->lblock()->address()) >> dec;
					if(inedge->source()->lblock()->id() != 0
					&& inedge->source()->lblock()->id() != lbset->count() - 1){
						if (taglbloc == taginedge) {
								cons->addRight(1, VAR(inedge));
								cons2->addLeft(1, VAR(inedge));
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
	//	 		unsigned long taglbloc = ((unsigned long)lbloc->address()) >> dec;
		 		for(GenGraph<Node, Edge>::InIterator inedge(Graph::NODE(lbloc));
		 		inedge; inedge++) {
		 			// cout << "examine block (addr = " <<  lbloc->address() <<   ") " << lbloc->id() << " avec predecesseur : " << inedge->lblock()->id() << "\n";
//		 			unsigned long taginedge = ((unsigned long)inedge->lblock()->address()) >> dec;
					if(inedge->source()->lblock()->id() != 0
					&& inedge->source()->lblock()->id() != lbset->count() - 1){
						if (inedge->source()->lblock()->cacheblock() == lbloc->cacheblock())
							cons2->addRight(1, VAR(inedge));
					}
		 		}
		 	}
		 }

		// building the (16)
//		if(lbloc->getNonConflictState() && !findlooplb){
                // xihit = sum p(uv, ij) / cache_block(uv) = cache_block(ij)
		else if(Builder::NON_CONFLICT(lbloc)) {
			cons = system->newConstraint(Constraint::EQ);
			cons->addLeft(1, HIT_VAR(lbloc));
			unsigned long taglbloc = ((unsigned long)lbloc->address()) >> dec;
			for(Graph::InIterator inedge(Graph::NODE(lbloc)); inedge; inedge++) {
				unsigned long taginedge = ((unsigned long)inedge->source()->lblock()->address()) >> 3;
				if(inedge->source()->lblock()->id() != 0
					&& inedge->source()->lblock()->id() != lbset->count() - 1) {
						if(taglbloc == taginedge)
							cons->addRight(1,  VAR(inedge));
					}
				}
			}

			//builduig the constraint (32)
			ContextTree *cont = CONTEXT_TREE(fw);
			addConstraintHeader(system, lbset, cont, lbloc);
		}
	}

  	// Fix the object function
	for(LBlockSet::Iterator lbloc(*lbset); lbloc; lbloc++) {
		if(lbloc->id() != 0 && lbloc->id() != lbset->count()- 1)
  			system->addObjectFunction( cach->missPenalty(), MISS_VAR(lbloc));
	}
}


/**
 */
void ConstraintBuilder::processWorkSpace(WorkSpace *fw) {
	ASSERT(fw);
	LBlockSet **lbsets = LBLOCKS(fw);
	const hard::Cache *cache = hard::CACHE_CONFIGURATION(fw)->instCache();
	if(!cache)
		return;
	for(int i = 0; i < cache->rowCount(); i++)
		processLBlockSet(fw, lbsets[i]);
}

/**
 */
void ConstraintBuilder::addConstraintHeader(
	ilp::System *system,
	LBlockSet *graph,
	ContextTree *cont,
	LBlock *boc
) {
	int size = graph->count();
	bool dominate = false;
	for(ContextTree::ChildrenIterator son(cont); son; son++)
		addConstraintHeader(system, graph, son, boc);
	BasicBlock *b = boc->bb();
	if(cont->kind() == ContextTree::LOOP){
		for(ContextTree::BBIterator bs(cont); bs; bs++) {
			if (b == bs){
			    // p(uv, ij) / header not dom bb(uv) <= sum xi
			    // / (xi, header) in E and header not dom xi
				BasicBlock *header = cont->bb();
				bool used = false;
				Constraint *cons32 = system->newConstraint(Constraint::LE);
				for(GenGraph<Node, Edge>::InIterator inedge(Graph::NODE(boc));
				inedge; inedge++) {
					Node *source = inedge->source();
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
						cons32->addLeft(1, VAR(inedge));
						dominate = false;
						used = true;
					}

				}
				if(used) {
				        bool set = false;
					for(BasicBlock::InIterator inedg(header); inedg ; inedg++) {
						BasicBlock *preheader = inedg->source();
						if(!Dominance::dominates(header, inedg->source()))
						cons32->addRight(1, VAR(preheader));
						set = true;
					}
					ASSERT(set);
				}
				else
					delete cons32;
				break;
			}
		}
	}
}

} } //otawa::ccg
