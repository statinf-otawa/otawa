/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS <casse@irit.fr>
 *
 *  Program Structure Tree Builder
 *	This file is part of OTAWA
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
 *	along with Foobar; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <elm/genstruct/Vector.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/cfg.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/cfg/Edge.h>
#include <otawa/dfa/BitSet.h>
#include <otawa/cfg/PSTBuilder.h>

namespace otawa {

using namespace elm;

static SilentFeature::Maker<PSTBuilder> PST_MAKER;
/**
 * This feature represents the availability of the Program Structure Tree.
 * 
 * @par Properties
 * @li @ref otawa::PROGRAM_STRUCTURE_TREE
 * 
 */
SilentFeature PST_FEATURE("otawa::PST_FEATURE", PST_MAKER);

// Private Properties
Identifier<int> PSTBuilder::PST_DFSNUM("", -1 );
Identifier<int> PSTBuilder::PST_HI("", -1 );
Identifier<PSTBuilder::BracketSet*> PSTBuilder::PST_BSET("", 0 );
	
Identifier<CEClass*> PSTBuilder::PST_CLASS("", 0 );
Identifier<int> PSTBuilder::PST_RECENTSIZE("", 0 );
Identifier<CEClass*> PSTBuilder::PST_RECENTCLASS("", 0 );

Identifier<bool> PSTBuilder::PST_IS_CAPPING("", false );
Identifier<bool> PSTBuilder::DFS_IS_BACKEDGE("", true );

// Public Properties
/**
 * When we get the VirtualCFG associated with a SESE region, 
 * this property, hooked on a BB from the Virtual CFG, gives
 * the corresponding BB on the original (physical) CFG 
 * 
 * @par Hooks
 * @li @ref BasicBlock
 */
Identifier<BasicBlock*> FROM_BB("otawa::FROM_BB", 0 );

/**
 * This property is like @li @ref otawa::FROM_BB but for edges
 */
Identifier<Edge*> FROM_EDGE("otawa::FROM_EDGE", 0 );

/**
 * This property points to the Program Structure Tree of the CFG.
 * 
 * @par Hooks
 * @li @ref CFG
 */

Identifier<PSTree *> PROGRAM_STRUCTURE_TREE("otawa::PROGRAM_STRUCTURE_TREE", 0 );
/**
 * @class PSTBuilder
 *
 * This processor computes the Program Structure Tree of the CFG.
 * 
 * The used algorithm used in this processor is described in:
 *  
 * The Program Structure Tree: Computing Control Regions in Linear Time. R. Johnson, D. Pearson, K. Pingali,
 * SIGPLAN Conference on Programming Language Design and Implementation, pages 171-185, year 1994. 
 *
 * @par Configuration
 * none
 *
 * @par Required features
 * @li @ref COLLECTED_CFG_FEATURE
 *
 * @par Provided features
 * @li @ref PST_FEATURE 
 * 
 * @par Statistics
 * none
 */

PSTBuilder::PSTBuilder(void) : CFGProcessor("otawa::PSTBuilder", Version(1, 0, 0)), pst(0) {
	require(COLLECTED_CFG_FEATURE);
	provide(PST_FEATURE);
}



void PSTBuilder::processCFG(WorkSpace *fw, CFG *cfg) {
	/* Add the fake back-edge from end to start, to make sure that the CFG is strongly connected */
	fakeEdge = new Edge(cfg->exit(), cfg->entry(), Edge::TAKEN);
		
	node = new BasicBlock*[cfg->countBB()];
	cur_dfsnum = 0;
	
	depthFirstSearch(cfg->entry());	
	
	assignClasses(cfg);
	
	delete fakeEdge;	
	
	SESERegion *rootRegion = new SESERegion(cfg);
	pst = new PSTree(rootRegion);
	
	buildTree(cfg, cfg->entry(), pst);
	
	PROGRAM_STRUCTURE_TREE(cfg) = pst;
	
	delete [] node;
				
}

/**
 * Display the PST
 * 
 * @param node The PST
 * @param col The beginning display column (generally 0)
 * @param ending True if it is the last node of a list of siblings (generally false)
 */  
int PSTBuilder::displayTree(PSTree *node, int col, bool ending) {
    int total = 0;
    
	if (col == 0) {
		cout << "Root region.\n";
	} else {
			for (int i = 0; i < (col-1); i++)
				cout << "  ";
			cout << (ending ? " \\" : " |"); 
			cout << "-Region: " << node->data()->getEntry()->source()->number() << "->" << node->data()->getEntry()->target()->number();
			if (node->data()->getExit()) {
				cout << " to " << node->data()->getExit()->source()->number() << "->" << node->data()->getExit()->target()->number() ;
			} else cout << " to ???";		
			cout << " " << ((node->data()->isFirst()) ? "FIRST" : "");
			cout << "\n";
		
		}	
	for (PSTree *child = node->children(); child; child = child->sibling()) {
		total += displayTree(child, col+1, (!child->sibling()));
	}
	return(total);
}

/**
 * Using the Cycle-Equivalent informations on the edges, recursively build the PST
 * 
 * @param cfg The CFG to PST-ize
 * @param bb The current region-starting BB
 * @param subtree The currently processed PST node 
 */
void PSTBuilder::buildTree(CFG *cfg, BasicBlock *bb, PSTree *subtree) {
	PROGRAM_STRUCTURE_TREE(bb) = subtree;
	subtree->data()->addBB(bb);
	for (BasicBlock::OutIterator edge(bb); edge; edge++) {
		BasicBlock *newbb = edge->target();
		if (edge->kind() != Edge::CALL) {
			
			CEClass *cl = PST_CLASS(edge);
			edge->removeProp(PST_CLASS);
			edge->removeProp(PST_RECENTCLASS);
			edge->removeProp(PST_RECENTSIZE);
			ASSERT(PST_IS_CAPPING(*edge) == false);			
			edge->removeProp(DFS_IS_BACKEDGE);	
					
			// Cleanup the temporary properties
			
			PSTree *newSubtree = subtree;
			/* If the edge isn't cycle-equivalent to any other edge ((first && last) = true)), 
			 * then it doesn't define a region. */
			if (cl && (!cl->isFirst() || !cl->isLast())) {
				
				// We enter a sequence of region(s)
				if (cl->isFirst()) {
					/* cout << "Entering region w/ class " << cl << " and count=" << cl->getCount() <<  "\n"; */
					newSubtree = new PSTree(new SESERegion(cfg, *edge, subtree, true, cl));
					subtree->prependChild(newSubtree);
				} 
				
				// We move from a region to the next region in sequence
				else if (!cl->isLast()) {
					/* cout << "Move to adjacent region\n"; */
					subtree->data()->setExit(*edge);
					newSubtree = new PSTree(new SESERegion(cfg,*edge, subtree->data()->getParent(), false, cl));
					subtree->appendChild(subtree->data()->getParent());	
				} 
				
				// We exit the sequence of region(s)
				else {
					/* cout << "Exiting region.\n"; */
					subtree->data()->setExit(*edge);
					subtree->data()->setLast();
					newSubtree = subtree->data()->getParent();	
				}	
				cl->dec();
			}
			if (PST_DFSNUM(newbb) != -1) {
				newbb->removeProp(PST_DFSNUM);
				newbb->removeProp(PST_HI);
				newbb->removeProp(PST_BSET);	
				buildTree(cfg, newbb, newSubtree);
			}
		}
	}	
}

/**
 * Do a depth-first search on the undirected version of the CFG 
 * assign the DFS number on each basic block and detect back edges
 * (back edge here means the back edges from the paper on the PST, not the back edges for loops) 
 *
 * @param bb Beginning basic block 
 */ 
void PSTBuilder::depthFirstSearch(BasicBlock *bb) {
	
	PST_DFSNUM(bb) = cur_dfsnum;
	/* cout << "BB " << bb->number() << " has DFSNUM " << cur_dfsnum << "\n"; */
	node[cur_dfsnum] = bb;
	cur_dfsnum++;
	

	for (BasicBlock::OutIterator edge(bb); edge; edge++)
		if ((edge->kind() != Edge::CALL) && (PST_DFSNUM(edge->target()) == -1)) {
			DFS_IS_BACKEDGE(*edge) = false; 
			depthFirstSearch(edge->target());
		}
	
	for (BasicBlock::InIterator edge(bb); edge; edge++)
		if (PST_DFSNUM(edge->source()) == -1) {
			DFS_IS_BACKEDGE(*edge) = false;
			depthFirstSearch(edge->source());
		}
}

/**
 * Using the DFS number on each BB and the backedge identification,
 * assign Cycle-Equivalent Classes to the edges. 
 * 
 * @param cfg The CFG to regionnize
 */
void PSTBuilder::assignClasses(CFG *cfg) {
	
	for (int i = cfg->countBB() - 1; i >= 0; i--) {
		int hi0 = cfg->countBB(), hi1 = cfg->countBB(), hi2 = cfg->countBB();

		BasicBlock *n = node[i];
		/* cout << "Processing BB: " << n->number() << " (dfsnum == " << i << ")\n"; */		
		/* hi0 = min{t.dfsnum | (n,t) is a backedge} */
		for (BasicBlock::OutIterator outedge(n); outedge; outedge++)
			if ((outedge->kind() != Edge::CALL) && DFS_IS_BACKEDGE(*outedge) && (PST_DFSNUM(outedge->target()) < i) && (PST_DFSNUM(outedge->target()) < hi0))
				hi0 = PST_DFSNUM(outedge->target());
		
		/* cout << "hi0 is " << hi0 << "\n"; */		
		
		for (BasicBlock::InIterator inedge(n); inedge; inedge++)
			if (DFS_IS_BACKEDGE(*inedge) && (PST_DFSNUM(inedge->source()) < i) && (PST_DFSNUM(inedge->source()) < hi0))
				hi0 = PST_DFSNUM(inedge->source());			

		/* hi1 = min { c.hi | c is a child of n } */
		BasicBlock *hichild = 0;
		
		for (BasicBlock::OutIterator outedge(n); outedge; outedge++)
			if ((outedge->kind() != Edge::CALL) && !DFS_IS_BACKEDGE(*outedge) && (PST_HI(outedge->target()) < i) && (PST_HI(outedge->target()) < hi1) && (PST_DFSNUM(outedge->target()) > i)) {
				hi1 = PST_HI(outedge->target());
				hichild = outedge->target();
			}
				
		for (BasicBlock::InIterator inedge(n); inedge; inedge++)
			if (!DFS_IS_BACKEDGE(*inedge) && (PST_HI(inedge->source()) < i) && (PST_HI(inedge->source()) < hi1) && (PST_DFSNUM(inedge->source()) > i)) {
				hi1 = PST_HI(inedge->source());
				hichild = inedge->source();
			}					
			
		ASSERT(hichild || (hi1 == cfg->countBB()));
		/* cout << "hi1 is " << hi1 << "\n"; */
		
		PST_HI(n) = (hi0 > hi1) ? hi1 : hi0; 
		
		/* cout << "n.hi is " << PST_HI(n) << "\n"; */
		
		/* hi2 = min {c.hi | c is a child of n and c.hi != hi1 */
		if (hichild) {
			for (BasicBlock::OutIterator outedge(n); outedge; outedge++)
				if ((outedge->kind() != Edge::CALL) && (PST_HI(outedge->target()) < i) && !DFS_IS_BACKEDGE(*outedge) && (PST_HI(outedge->target()) < hi2) && 
					(PST_DFSNUM(outedge->target()) > i) && (outedge->target() != hichild)) {
						hi2 = PST_HI(outedge->target());
					}
					
			for (BasicBlock::InIterator inedge(n); inedge; inedge++)
				if (!DFS_IS_BACKEDGE(*inedge) && (PST_HI(inedge->source()) < i) && (PST_HI(inedge->source()) < hi2) && 
					(PST_DFSNUM(inedge->source()) > i) && (inedge->source() != hichild)) {
						hi2 = PST_HI(inedge->source());

					}	 
		}
		/* cout << "hi2 is " << hi2 << "\n";*/ 
					
		/* Compute the bracket-list for n */
		PST_BSET(n) = new BracketSet();
		
		for (BasicBlock::OutIterator outedge(n); outedge; outedge++)
			if ((outedge->kind() != Edge::CALL) && !DFS_IS_BACKEDGE(*outedge) && (PST_DFSNUM(outedge->target()) > i)) {
				// cout << "Merging bracket-list for child: " << outedge->target() << "\n";
				for (BracketSet::Iterator iter(**PST_BSET(outedge->target())); iter; iter++)
					PST_BSET(n)->addLast(*iter);
			}
				
				
		for (BasicBlock::InIterator inedge(n); inedge; inedge++)
			if (!DFS_IS_BACKEDGE(*inedge) && (PST_DFSNUM(inedge->source()) > i)) {
				//cout << "Merging bracket-list for child: " << inedge->source() << "\n";
				for (BracketSet::Iterator iter(**PST_BSET(inedge->source())); iter; iter++)
					PST_BSET(n)->addLast(*iter);
			}
		
		/* Remove from set the backedges (including capping backedges) going to current bb */
		genstruct::Vector<Edge*> todel;
		for (BracketSet::Iterator bracket(**PST_BSET(n)); bracket; bracket++) {
			//cout << "Considering edge: " << bracket->source() << " to " << bracket->target() << " for deletion\n";
			if (((bracket->source() == n) && (PST_DFSNUM(bracket->target()) > i)) ||
				((bracket->target() == n) && (PST_DFSNUM(bracket->source()) > i))) {
				//cout << " deleting\n";
				todel.add(*bracket);
			} else {
				//cout << "bracket source = " << bracket->source() << " and n= " << n << "\n";
				//cout << " not deleting, target DFS = " << PST_DFSNUM(bracket->target()) << " and i = " <<  i << " \n";
			}
		}	
		for (genstruct::Vector<Edge*>::Iterator iter(todel); iter; iter++) {
			Edge *edge = *iter;			
			PST_BSET(n)->remove(edge);
			if (PST_IS_CAPPING(*edge)) {
				//cout << " *deleting* capping edge: " << edge->source() << " to " << edge->target() << "\n";
				delete edge;
			} else {
				//cout << " not *deleting* non-capping edge: " << edge->source() << " to " << edge->target() << "\n";
			}
		}

		/* Add to set the backedges going from n */
		for (BasicBlock::OutIterator outedge(n); outedge; outedge++)
			if ((outedge->kind() != Edge::CALL) && DFS_IS_BACKEDGE(*outedge) && (PST_DFSNUM(outedge->target()) < i))
				PST_BSET(n)->addLast(*outedge);
				
		for (BasicBlock::InIterator inedge(n); inedge; inedge++)
			if (DFS_IS_BACKEDGE(*inedge) && (PST_DFSNUM(inedge->source()) < i))
				PST_BSET(n)->addLast(*inedge);	
				
		if (hi2 < hi0) {
			Edge *capping = new Edge(n, node[hi2], Edge::TAKEN);
			PST_IS_CAPPING(capping) = true;
			//cout << "Adding capping edge from " << n << " to " << node[hi2] << "\n";
			PST_BSET(n)->addLast(capping);
			
		}
		
		if (i != 0) {
			
			/* Detect parent edge, i.e the tree edge with dfsnum < i  */
			Edge *edge = 0;
			for (BasicBlock::OutIterator outedge(n); outedge; outedge++)
				if ((outedge->kind() != Edge::CALL) && !DFS_IS_BACKEDGE(*outedge) && (PST_DFSNUM(outedge->target()) < i)) {
					ASSERT(edge);
					edge = *outedge;
				} 
				
			for (BasicBlock::InIterator inedge(n); inedge; inedge++)
				if (!DFS_IS_BACKEDGE(*inedge) && (PST_DFSNUM(inedge->source()) < i)) {
					ASSERT(!edge);
					edge = *inedge;
				}	
			ASSERT(edge);
			
			Edge *topBracket = PST_BSET(n)->last();
			ASSERT(topBracket);
			int currentSize = PST_BSET(n)->count();
			if (PST_RECENTSIZE(topBracket) != currentSize) {
				PST_RECENTSIZE(topBracket) = currentSize;
				PST_RECENTCLASS(topBracket) = new CEClass();
			}	
			PST_CLASS(edge) = PST_RECENTCLASS(topBracket);
//			cout << "Put edge " << edge->source() << "->" << edge->target() << " in class " << PST_CLASS(edge) << "\n";
			PST_CLASS(edge)->inc();
			
			ASSERT(PST_RECENTSIZE(topBracket) == PST_BSET(n)->count());
			if (PST_RECENTSIZE(topBracket) == 1) {
				PST_CLASS(topBracket) = PST_CLASS(edge);
//				cout << "Put edge (topBracket) " << topBracket->source() << "->" << topBracket->target() << " in class " << PST_CLASS(edge) << "\n";
				if (topBracket != fakeEdge) {
					PST_CLASS(topBracket)->inc(topBracket);
				}
			}					
		}
				
	}
}

/**
 * Return a VCFG of the basic blocks represented by the region. Excludes subregions.
 * 
 * @param tree The PST-node
 * @param map (empty) hashtable to receive associations of (basic block) <==> (virtual basic block)
 */
 
VirtualCFG *PSTBuilder::getVCFG(PSTree *tree, HashTable<BasicBlock*, BasicBlock*> &map) {
	SESERegion *region = tree->data();
	CFG *cfg = region->getCFG();
	VirtualCFG *vcfg = new VirtualCFG(false);
	BasicBlock *entrySucc = 0, *exitPred = 0;
	dfa::BitSet doneList(cfg->countBB());
	dfa::BitSet thisregion(cfg->countBB());
	genstruct::Vector<BasicBlock*> workList,mybbs;

		
	doneList.empty();
	thisregion.empty();

	for (genstruct::Vector<BasicBlock*>::Iterator iter(region->getBBs()); iter; iter++) {

		thisregion.add(iter->number());
	}

	ASSERT(!doneList.contains(region->getEntry()->target()->number()));
	
	workList.clear();
	mybbs.clear();
	workList.push(region->getEntry()->target());
	doneList.add(region->getEntry()->target()->number());
	vcfg->addBB(vcfg->entry());
	FROM_BB(vcfg->entry()) = region->getEntry()->source();
	map.put(region->getEntry()->source(), vcfg->entry());
	
	/* Add the virtual BB to the Virtual CFG */
	while (!workList.isEmpty()) {
		BasicBlock *current = workList.pop();
		
		for (BasicBlock::OutIterator outedge(current); outedge; outedge++) {
			if ((outedge->kind() != Edge::CALL) && !doneList.contains(outedge->target()->number()) && (*outedge != region->getExit())) {
				workList.push(outedge->target());
				doneList.add(outedge->target()->number());
			}
		}
		if (thisregion.contains(current->number())) {

			VirtualBasicBlock *vbb = new VirtualBasicBlock(current);
			vbb->clearProps();
			vcfg->addBB(vbb);	
			mybbs.add(current);
			FROM_BB(vbb) = current;
			map.put(current, vbb);
		}
	}	
	entrySucc = map.get(region->getEntry()->target(), 0);
	exitPred = map.get(region->getExit()->source(), 0);

	
	/* Now, make the edges */
	if (entrySucc) {
		Edge *newedge = new Edge(vcfg->entry(), entrySucc, Edge::VIRTUAL_CALL);
		FROM_EDGE(newedge) = region->getEntry();
	}
	if (exitPred) {
		Edge *newedge = new Edge(exitPred, vcfg->exit(), Edge::VIRTUAL_RETURN);
		FROM_EDGE(newedge) = region->getExit();
	}
	for (genstruct::Vector<BasicBlock*>::Iterator bb(mybbs); bb; bb++) {
		if (thisregion.contains(bb->number())) {
			BasicBlock *vbb = map.get(bb, 0);
			ASSERT(vbb);	
			for (BasicBlock::OutIterator outedge(*bb); outedge && (*outedge != region->getExit()); outedge++) {
				if (outedge->kind() == Edge::CALL) {
					Edge *newedge = new Edge(vbb, outedge->target(), Edge::CALL);
					CALLED_CFG(newedge) = outedge->calledCFG();
					FROM_EDGE(newedge) = outedge;				
				} else {
					if (thisregion.contains(outedge->target()->number())) {
						BasicBlock *vtarget = map.get(outedge->target(), 0);
						ASSERT(vtarget);
						Edge *newedge = new Edge(vbb, vtarget, outedge->kind());
						FROM_EDGE(newedge) = outedge;
					}
				}
			}
		}
	}
	vcfg->addBB(vcfg->exit());
	FROM_BB(vcfg->exit()) = region->getExit()->target();
	map.put(region->getExit()->target(), vcfg->exit());
	vcfg->numberBB();
	return(vcfg);	
}
}

