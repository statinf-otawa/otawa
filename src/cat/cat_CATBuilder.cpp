/*
 *	CATBuilder implementation
 *	Copyright (c) 2005-12, IRIT UPS.
 *
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
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */
#include <elm/io.h>
#include <otawa/cat/CATBuilder.h>
#include <otawa/cache/LBlock.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/cfg.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/util/LBlockBuilder.h>
#include <otawa/hard/Platform.h>
#include <otawa/util/ContextTree.h>
#include <otawa/dfa/XCFGVisitor.h>
#include <otawa/cat/CATDFA.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/prop/DeletableProperty.h>
#include <otawa/cache/categories.h>

using namespace otawa;
using namespace otawa::ilp;
using namespace otawa::ipet;
using namespace otawa::dfa;

namespace otawa { namespace cat {

using namespace otawa;
using namespace cache;

/**
 * Does anybody know anything about this?
 * @ingroup cat
 */
Identifier<bool> CATBuilder::NON_CONFLICT("otawa::cat::CATBuilder::NON_CONFLICT", false);


/**
 * Private property.
 */
static Identifier<BitSet *> IN("", 0);


/**
 * Private property.
 */
static Identifier<BitSet *> SET("", 0);


/**
 * This property is set for L-Block categories that has been lowered, that is,
 * they was classified as first-miss but lowered to always-miss due to effect
 * of a surrounding loop (whose header basic block is stored).
 *
 * @ingroup cat
 */
Identifier<BasicBlock *> LOWERED_CATEGORY("otawa::cat::LOWERED_CATEGORY", 0);


/**
 */
void CATBuilder::processLBlockSet(WorkSpace *fw, LBlockSet *lbset) {
	ASSERT(fw);
	ASSERT(lbset);

	// Compute Abstract Cache States
	const hard::Cache *cach = hard::CACHE_CONFIGURATION(fw)->instCache();
	CATProblem prob(lbset, lbset->count(), cach, fw);
	const CFGCollection *coll = INVOLVED_CFGS(fw);
	dfa::XCFGVisitor<CATProblem> visitor(*coll, prob);
	dfa::XIterativeDFA<dfa::XCFGVisitor<CATProblem> > engine(visitor);
	engine.process();

	// Assign ACS to BB
	for (CFGCollection::Iterator cfg(coll); cfg; cfg++) {
		for (CFG::BBIterator block(*cfg); block; block++) {
			dfa::XCFGVisitor<CATProblem>::key_t pair(*cfg, *block);
			BitSet *bitset = engine.in(pair);
			block->addProp(new DeletableProperty<BitSet *>(IN, new BitSet(*bitset)));
		}
	}

	// Build categories
	ContextTree *ct = CONTEXT_TREE(fw);
	ASSERT(ct);
	/*for(LBlockSet::Iterator lblock(*lbset); lblock; lblock++)
		CATBuilder::NODE(lblock) += new CATNode(lblock);*/
	/*BitSet *virtuel =*/ buildLBLOCKSET(lbset, ct);
	setCATEGORISATION(lbset, ct, cach->blockBits());

	// Clean up
	for (CFGCollection::Iterator cfg(coll); cfg; cfg++)
		for (CFG::BBIterator block(*cfg); block; block++)
			IN(block).remove();
}


/**
 */
void CATBuilder::processWorkSpace(WorkSpace *fw) {
	ASSERT(fw);

	// Check the cache
	const hard::Cache *cache = hard::CACHE_CONFIGURATION(fw)->instCache();
	if(!cache)
		return;

	// Get the l-block sets
	LBlockSet **lbsets = LBLOCKS(fw);
	if(!lbsets) {
		LBlockBuilder builder;
		builder.process(fw);
		lbsets = LBLOCKS(fw);
	}

	// Process the l-block sets
	for(int i = 0; i < cache->rowCount(); i++)
		processLBlockSet(fw, lbsets[i]);
}


/**
 * @class CATBuilder
 * Create a new CATBuilder processor.
 *
 * @par Provided Feature
 * @li @ref ICACHE_CATEGORY_FEATURE
 *
 * @par Required Features
 * @li @ref CONTEXT_TREE_FEATURE
 * @li @ref COLLECTED_LBLOCKS_FEATURE
 *
 * @ingroup cat
 */

p::declare CATBuilder::reg = p::init("otawa::cat::CATBuilder", Version(1, 0, 0))
	.require(CONTEXT_TREE_FEATURE)
	.require(COLLECTED_LBLOCKS_FEATURE)
	.provide(ICACHE_CATEGORY_FEATURE);

CATBuilder::CATBuilder(p::declare& r): Processor(r) {
}


/*
 * This function categorize the l-blocks in the ContextTree S (and its children)
 *
 * @param lineset The l-block list
 * @param S The root context tree
 * @param dec In an address, the number of bits representing the offset in a cache block.
 */
void CATBuilder::setCATEGORISATION(LBlockSet *lineset ,ContextTree *S ,int dec){
	int size = lineset->count();
	int ident;
	BitSet *u = new BitSet(size);
	LBlock *cachelin;

	/*
	 * Categorize first all the l-blocks in the children ContextTree
	 */
	for(ContextTree::ChildrenIterator fils(S); fils; fils++){
		setCATEGORISATION(lineset,fils,dec);
	}

	/* Now categorize the l-blocks in this ContextTree */
	if(S->kind() == ContextTree::LOOP){
		/*
		 * Call worst() on each l-block of the loop.
		 */
		u = SET(S);
		for (int a = 0; a < size; a++){
			if (u->contains(a)){
				cachelin = lineset->lblock(a);
				worst(cachelin ,S,lineset,dec);
			}
		}
	}
	else {
		/*
		 * Call worst() on each l-block of this ContextTree.
		 */
		for(ContextTree::BBIterator bk(S); bk; bk++){
			for(BasicBlock::InstIter inst(bk); inst; inst++) {
				address_t adlbloc = inst->address();
				for (LBlockSet::Iterator lbloc(*lineset); lbloc; lbloc++){
					if ((adlbloc == (lbloc->address()))&&(bk == lbloc->bb())){
						ident = lbloc->id();
						cachelin = lineset->lblock(ident);
						worst(cachelin ,S , lineset,dec);
					}
				}
			}
		}

	}
}

/*
 * ??? This function really categorizes the lblock "line"... ???
 * Annotates the "line" with ALWAYSMISS/ALWAYSHIT/FIRSTMISS/FIRSTHIT
 * In the case of FIRSTMISS, also annotate with the loop-header of the most inner loop.
 */
void CATBuilder::worst(LBlock *line , ContextTree *node , LBlockSet *idset, int dec){
	int number = idset->count();
	BasicBlock *bb = line->bb();
	LBlock *cacheline;
	BitSet *in = new BitSet(number);


	in = IN(bb);

	//int count = 0;
	bool nonconflitdetected = false;
	bool continu = false;
	unsigned long tagcachline,tagline;

	//test if it's the lbloc which find in the same memory block

	/*
	 * If the IN(line) = {LB} and cacheblock(line)==cacheblock(LB), then
	 * nonconflict (Always Hit)
	 */
	if (in->count() == 1){
		for (int i=0;i < number;i++){
		if (in->contains(i)){
			cacheline = idset->lblock(i);
			tagcachline = ((unsigned long)cacheline->address()) >> dec;
			unsigned long tagline = ((unsigned long)line->address()) >> dec;
				if (tagcachline == tagline )
					nonconflitdetected = true;
			}
		}
	}



	//test the virtual non-conflit state
	bool nonconflict = false;
	for (int i=0;i < number;i++) {
			if (in->contains(i)) {
				cacheline = idset->lblock(i);
				tagcachline = ((unsigned long)cacheline->address()) >> dec;
			    tagline = ((unsigned long)line->address()) >> dec;
				if(cacheline->address() == line->address()
				&& line->bb() != cacheline->bb())
					nonconflict = true;
			}
	}


	bool exist = false;
	// test if the first lblock in the header is Always miss
	if (in->count() == 2){
		bool test = false;
		for (int i=0;i < number;i++){
			if (in->contains(i)) {
				cacheline = idset->lblock(i);
				tagcachline = ((unsigned long)cacheline->address()) >> dec;
				unsigned long tagline = ((unsigned long)line->address()) >> dec;
				if(tagcachline == tagline && line->bb() != cacheline->bb())
					test = true;
				if(tagcachline != tagline )
					exist = true;
				if(test && line->address() == cacheline->address())
					continu = true;
			}
		}
	}

	//the last categorisation of the l-block
	cache::category_t lastcateg = cache::CATEGORY(line);
	if(node->kind()!=ContextTree::LOOP){
		//if ((line->returnSTATENONCONF())&&( nonconflitdetected)){
		if ( nonconflitdetected || continu){
			cache::CATEGORY(line) = cache::ALWAYS_HIT;
		}
		else
			cache::CATEGORY(line) = cache::ALWAYS_MISS;
	}
	bool dfm = false;
	if(node->kind()== ContextTree::LOOP){
		BitSet *w = new BitSet(number);
		w = SET(node);
		if (w->count()== 2){
			int U[2];
			int m = 0;
			for (BitSet::Iterator iter(*w); iter; iter++) {
				U[m] = *iter;
				m = m + 1;
			}
			//cacheline = lineset->returnLBLOCK(i);
			tagcachline = ((unsigned long)idset->lblock(U[0])->address()) >> dec;
		    tagline = ((unsigned long)idset->lblock(U[1])->address()) >> dec;
			if(tagcachline == tagline)
					dfm = true;
		}

		BitSet inter = *w;
		BitSet dif = *in;

		// intersection
		inter.mask(*in);

		//difference (IN - U)
		dif.remove(*w);

		int identif = line->id();

		//basic bock of the header
		BasicBlock * blockheader = node->bb();
		//if ((line->returnSTATENONCONF())&&(nonconflitdetected || continu)){
		if ((nonconflitdetected )|| (continu && !exist)){
			cache::CATEGORY(line) = cache::ALWAYS_HIT;
		}
		else
			if(lastcateg == cache::FIRST_HIT
//			|| ((line->getNonConflictState() || nonconflict)
			|| ((CATBuilder::NON_CONFLICT(line) || nonconflict)
				&& inter.count() > 0
				&& dif.count()== 1
				&& blockheader == bb))
				cache::CATEGORY(line) = cache::FIRST_HIT;
			else if ((((lastcateg == cache::FIRST_MISS) || (lastcateg == cache::INVALID_CATEGORY))
				&&(in->contains(identif)&&(inter.count()== 1)&&(inter.contains(identif))&& (dif.count() >= 0)&&(w->count()== 1)))
				||((inter.count() == 2)&&(dfm)&&(inter.contains(identif))))
				cache::CATEGORY(line) = cache::FIRST_MISS;
				else {
					cache::CATEGORY(line) = cache::ALWAYS_MISS;
					 if (lastcateg == cache::FIRST_MISS){
					 	LOWERED_CATEGORY(line) = node->bb();
					 	//CATBuilder::NODE(line)->setHEADEREVOLUTION(node->bb(),true);
					 }

				}
		}

}


/**
 * Annotate all the loop headers with the set of the l-blocks contained in the loop.
 * Sets the categorisation to "INVALID" for all l-blocks processed by this function.
 * Annotate all processed l-blocks with the the header of the most inner loop.
 * (or with the root of the CT. if there isn't any loop containing the l-block)
 *
 * @param lcache The lblock set.
 * @param root The root ContextTree
 * @return The set of all lblocks contained in the "root" ContextTree and its children (that is, the set of all processed l-blocks)
 *
 */
BitSet *CATBuilder::buildLBLOCKSET(LBlockSet *lcache, ContextTree *root){
		int lcount = lcache->count();
		BitSet *set = new BitSet(lcount);
		BitSet *v = new BitSet(lcount);
		bool inloop = false;

		if (root->kind()== ContextTree::LOOP )
				inloop = true;
		int ident;

		/*
		 * Call recursively buildLBLOCKSET for each ContextTree children
		 * Merge result with current set
		 */
		for(ContextTree::ChildrenIterator son(root); son; son++){
			 v = buildLBLOCKSET(lcache, son);
			 set->add(*v);
		}

		/*
		 * For each lblock that is part of any non-(entry|exit) BB of the current ContextTree:
		 *   - Set the lblock's categorization to INVALID
		 *   - Add this lblock to the current set.
		 */
		for(ContextTree::BBIterator bb(root); bb; bb++){
			if ((!bb->isEntry())&&(!bb->isExit())){ /* XXX */
			for(BasicBlock::InstIter inst(bb); inst; inst++) {
				address_t adlbloc = inst->address();
				for (LBlockSet::Iterator lbloc(*lcache); lbloc; lbloc++){
					if ((adlbloc == (lbloc->address()))&&(bb == lbloc->bb())){
						ident = lbloc->id();
						cache::CATEGORY(lbloc).add(cache::INVALID_CATEGORY);
						//CATBuilder::NODE(lbloc)->setHEADERLBLOCK(root->bb(),inloop);
						set->BitSet::add(ident);

					}
				}
			}
		}
	}

	/*
	 * For loops, annotate the loop-header with the set of all l-blocks in the loop
	 */
	if(root->kind()== ContextTree::LOOP){
		SET(root) = set;
	}
	return set;
}

} } // otawa::cat




