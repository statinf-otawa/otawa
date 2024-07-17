/*
 *	$Id$
 *	ipet::FlowFactLoader class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-08, IRIT UPS.
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
#include <otawa/cfg.h>
#include <otawa/cfg/Dominance.h>
#include <otawa/cfg/Loop.h>
#include <otawa/flowfact/features.h>
#include <otawa/flowfact/conflict.h>  
#include <otawa/ipet/FlowFactLoader.h>
#include <otawa/ipet/IPET.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/prog/Inst.h>
#include <otawa/flowfact/FlowFactLoader.h>
#include <otawa/ipet/FlowFactLoader.h>

namespace otawa { namespace ipet {

/**
 * @class FlowFactLoader
 * This processor allows using extern flow facts in an IPET system.
 *
 * @par Configuration
 * @li @ref FLOW_FACTS_PATH
 *
 * @par Required Features
 * @li @ref ipet::LOOP_HEADERS_FEATURE
 * @li @ref otawa::FLOW_fACTS_FEATURE
 *
 * @par Provided Features
 * @li @ref ipet::FLOW_FACTS_FEATURE
 */

p::declare FlowFactLoader::reg = p::init("otawa::ipet::FlowFactLoader", Version(2, 0, 0))
	.base(ContextualProcessor::reg)
	.maker<FlowFactLoader>()
	.require(EXTENDED_LOOP_FEATURE)
	.require(otawa::FLOW_FACTS_FEATURE)
	.provide(otawa::ipet::FLOW_FACTS_FEATURE);


/**
 * Build a new flow fact loader.
 */
FlowFactLoader::FlowFactLoader(p::declare& r)
:	ContextualProcessor(r),
 	lines_available(false),
 	total_loop(0),
 	found_loop(0),
 	line_loop(0),
 	max(0),
 	total(0),
 	min(0),
 	isIntoConstraint(false),
	dom(nullptr)
{
}


/**
 * One edge may be present on many conflict and be anotated for several contextual path (partial 
 * @param 	path current path
 * @param 	ListOfPathOfBenginingOfConstraint
 * @return	list of compatibles path	
 */
 
Vector  < Pair<ContextualPath,ContextualPath >  > getListOfCompatiblePath(const ContextualPath& path , /*genstruct::*/Vector  < Pair<ContextualPath,ContextualPath > >&ListOfPathOfBenginingOfConstraint){
	 Vector < Pair<ContextualPath,ContextualPath >  > res;
	int lg = ListOfPathOfBenginingOfConstraint.length();
	for(int i=0;i<lg;i++){
		Pair<ContextualPath,ContextualPath >  current = ListOfPathOfBenginingOfConstraint.get(i);
		 
		bool isCompa = true;
		for (int j=0;isCompa && j<current.fst.count();j++){
			if (path[j] != current.fst[j]) isCompa = false;
		}
		if (isCompa) res.push(current);
	}
	return res;
}


/**
 * One edge may be present on many conflict and be anotated for several contextual path (partial
 * @param current one of compatible path	 
 * @param 	path currnt complet  path
 * @param 	currentIncomplet created partial path from the precedents one 		
 */
void consCurrentContextualPath(const Pair<ContextualPath,ContextualPath >  &current,const ContextualPath& path,	ContextualPath &currentIncomplet ){
	int lgffx = current.fst.count() ;
	int cp =0;
	
	for(  cp = 0 ;cp <lgffx && current.fst[cp] !=  current.snd[0];  cp ++ );//find beggining of ffx
	for( cp++ ;cp <path.count(); currentIncomplet.push(path[cp]), cp ++ );//cp++ bug
 	
}

/**
 * Transfer conflict flow information from the given source instruction to the given BB.
 * @param source	Source instruction.
 * @param bb		Target BB.
 * @param path	    Current path
 * @param intoLoop     
 */
void FlowFactLoader::transferConflict(Inst *source, otawa::Block *b, const ContextualPath& path, bool intoLoop){  
	if(!b->isBasic())
		return;
	BasicBlock * bb=b->toBasic();
	  
	LockPtr<ConflictOfPath > 	currentCteNum = path(INFEASABLE_PATH, source) ; 
	
	if (currentCteNum ){	 

	 	isIntoConstraint = true;
		INFEASABLE_PATH(bb)= currentCteNum;
		ListOfPathOfBenginingOfConstraint.push(Pair<ContextualPath,ContextualPath >(path,currentCteNum->getIPath()) );
	}
	
	
	if (!seenFunction.contains(b)) {  
		LockPtr<ListOfEndConflict> currentCteEndNum = path(INFEASABLE_PATH_END, source) ; 
		if ( currentCteEndNum  ){
			seenFunction.push(b);   
			INFEASABLE_PATH_END(bb)= currentCteEndNum;
		}
	}	
		 	 
 	if (isIntoConstraint) {
		//seach all begining of conflict path matching with current path
		Vector  < Pair<ContextualPath,ContextualPath > > pathList =  getListOfCompatiblePath( path ,  ListOfPathOfBenginingOfConstraint);
			
		int lg = pathList.length();
		LockPtr <ListOfEdgeConflict > edgeInfo = new ListOfEdgeConflict() ;	// generate o new vector to merge into;
		LockPtr <ListOfLoopConflict > loopInfo = (intoLoop ? new ListOfLoopConflict():NULL);	// generate o new vector to merge into;
        bool hasSomething = false;
        bool hasLoop = false;
 
 		for (int i = 0; i< 	lg;i++) { //for each matching sub path
			Pair<ContextualPath,ContextualPath >  current;
			ContextualPath currentIncomplet; 
			if (lg>1 ) {
				current = pathList.get(i); 
				if (lg>1 && current.fst.count() < path.count())  consCurrentContextualPath(current,path,	currentIncomplet );
				else currentIncomplet = path;
			}else currentIncomplet = path;	
				
			//transfert loop info into conflict   
			if(intoLoop){
				LockPtr <ListOfLoopConflict > loopInfoForConflict  = currentIncomplet.ref(LOOP_OF_INFEASABLE_PATH_I, source);
					
				if (loopInfoForConflict&&loopInfoForConflict->length()>0) {
					int lg2= loopInfoForConflict->length();
					hasLoop = true;
					for (int itoMerge=0; itoMerge<lg2;itoMerge++){//merge
						LoopOfConflict loop = loopInfoForConflict->get(itoMerge);
						loopInfo->push( loop); 
					}	 
				}	 
			}
			//transfert edge of loop info into conflict 
			LockPtr<ListOfEdgeConflict > edgeInfoCur;
			for(BasicBlock::InstIter inst = bb->insts(); inst(); inst++){ 
				edgeInfoCur = currentIncomplet(EDGE_OF_INFEASABLE_PATH_I, *inst)  ;
									
				if (edgeInfoCur&&edgeInfoCur->length()>0) {				 
					int lg2= edgeInfoCur->length();
					hasSomething= true;
					for (int itoMerge=0; itoMerge<lg2;itoMerge++){
						 LockPtr<EdgeInfoOfConflict> ed = edgeInfoCur->get(itoMerge);
						edgeInfo->push( ed);
					}	 
				}					
			} 
		}
			
		if (hasSomething)	  EDGE_OF_INFEASABLE_PATH_I(bb)= edgeInfo;
		if (intoLoop && hasLoop)	   LOOP_OF_INFEASABLE_PATH_I(bb)= loopInfo;
	}

	long force_wcet = path(FORCE_WCET, source);
	if(force_wcet >= 0) {
		FORCE_WCET(bb) = force_wcet;
		TIME(bb) = force_wcet;
		if(logFor(LOG_BB))
			log << "\t\t\tFORCE_WCET(" << path << ":" << bb << ") = " << force_wcet << io::endl;
	}
} 


/**
 * Transfer flow information from the given source instruction to the given BB.
 * @param source	Source instruction.
 * @param bb		Target BB.
 * @return			True if some loop bound information has been found, false else.
 */
bool FlowFactLoader::transfer(Inst *source, Block *bb, const ContextualPath& path) {
	bool all = true;
	transferConflict( source,  bb,   path, true); 

	// look for MAX_ITERATION
	if(max < 0) {
		max = path(MAX_ITERATION, source);
		if(max < 0)
			all = false;
		else {
			MAX_ITERATION(bb) = max;
			if(total < 0)
				found_loop++;
			if(logFor(LOG_BB))
				log << "\t\t\tMAX_ITERATION(" << path << ":" << bb << ") = " << max << io::endl;
		}
	}

	// look for MIN_ITERATION
	if(min < 0) {
		min = path(MIN_ITERATION, source);
		if(min < 0)
			all = false;
		else {
			MIN_ITERATION(bb) = min;
			if(logFor(LOG_BB))
				log << "\t\t\tMIN_ITERATION(" << path << ":" << bb << ") = " << min << io::endl;
		}
	}

	// look for TOTAL_ITERATION
	if(total < 0){
		total = path(TOTAL_ITERATION, source);
		if(total < 0)
			all = false;
		else {
			TOTAL_ITERATION(bb) = total;
			if(max < 0)
				found_loop++;
			if(logFor(LOG_BB))
				log << "\t\t\tTOTAL_ITERATION(" << path << ":" << bb << ") = " << total << io::endl;
		}
	}
	return all;
}


/**
 */
void FlowFactLoader::setup(WorkSpace *ws) {
	lines_available = ws->isProvided(SOURCE_LINE_FEATURE);
	total_loop = 0;
	found_loop = 0;
	line_loop = 0;
	dom = DOMINANCE_FEATURE.get(ws);
}


/**
 */
void FlowFactLoader::cleanup(WorkSpace *ws) {
	if(logFor(LOG_DEPS)) {
		if(!total_loop)
			log << "\tno loop found\n";
		else {
			log << "\ttotal loop = " << total_loop << " (100%)\n";
			log << "\tfound loop = " << found_loop << " (" << ((float)found_loop * 100 / total_loop) << "%)\n";
			log << "\tline loop = " << line_loop << " (" << ((float)line_loop * 100 / total_loop) << "%)\n";
		}
	}
}


/**
 * Look for a bound for the given basic block according to a source/line
 * found on the instruction.
 * @param inst	Instruction to look in.
 * @param bb	BB to put the bound to.
 * @return		True if the bound has been found, false else.
 */
bool FlowFactLoader::lookLineAt(Inst *inst, Block *bb, const ContextualPath& path) {
	if(!lines_available)
		return false;

	// get the matching line
	Option<Pair<cstring, int> > res =
		workspace()->process()->getSourceLine(inst->address());
	if(!res)
		return false;

	// go back to the first statement of the line
	Vector<Pair<Address, Address> > addresses;
	workspace()->process()->getAddresses((*res).fst, (*res).snd, addresses);
	ASSERT(addresses);
	Inst *line_inst = workspace()->findInstAt(addresses[0].fst);
	ASSERT(line_inst);

	// perform transfer
	bool trans = transfer(line_inst, bb, path);
	if(trans)
		line_loop++;
	return trans;
}


/**
 */
void FlowFactLoader::processBB(WorkSpace *ws, CFG *cfg, Block *b, const ContextualPath& path) {

	 
	
	if(  !LOOP_HEADER(b)&& b->isBasic()) { 
		BasicBlock::InstIter source(b->toBasic());	 
		transferConflict( *source,  b,   path, false); 
	}
	
	
	// only for loop headers
	if(!LOOP_HEADER(b))
		return;
	total_loop++;

	// look for bounds
	max = -1;
	total = -1;
	min = -1;
	if(logFor(LOG_BB))
		log << "\t\tlooking bound for loop headed by " << b << io::endl;
	scan(b, b, path);

	// warning for missing loops
	// TODO
	if(max < 0 && total < 0) {
		warn(_ << "no limit for the loop at " << str(path.getEnclosingFunction(), Loop::of(b)->address()) << ".");
		warn(_ << " in the context " << path);
	}
}


/**
 * Scan for loop bound.
 * @param v		Block to scan.
 * @param t		Target of bound annotations.
 * @param path	Current context path.
 */
bool FlowFactLoader::scan(Block *v, Block *t, const ContextualPath& path) {
	if(logFor(LOG_BB))
		log << "\t\t\tlooking in " << v << io::endl;

	// is it phony?
	if(v->isPhony()) {
		for(auto e: v->outEdges())
			if(Loop::of(v) == Loop::of(e->sink())
			&& scan(e->sink(), t, path))
				return true;
	}

	// else it is a BB
	else {
		BasicBlock *bb = v->toBasic();

		// Look in the first instruction of the BB
		BasicBlock::InstIter iter(bb);
		ASSERT(iter);
		if(transfer(*iter, t, path))
			return true;

		// Attempt to look at the start of the matching source line
		if(lookLineAt(bb->first(), t, path))
			return true;

		// Look all instruction in the header
		// (in case of aggregation in front of the header)
		for(BasicBlock::InstIter inst(bb); inst(); inst++)
			if(lookLineAt(*inst, t, path))
				return true;

		// look in back edge in case of "while() ..." to "do ... while(...)" optimization
		for(BasicBlock::EdgeIter edge(bb->ins()); edge(); edge++)
			if(dom->isBackEdge(*edge) && edge->source()->isBasic()) {
				for(BasicBlock::InstIter inst(edge->source()->toBasic()); inst(); inst++)
					if(lookLineAt(*inst, t, path))
						return true;
			}

	}

	return false;
}


/**
 * This feature ensures that flow facts information (at less the loop bounds)
 * has been put on the CFG of the current task.
 *
 * @par Properties
 * @li @ref ipet::MAX_ITERATION
 * @li @ref ipet::MIN_ITERATION
 * @li @ref ipet::TOTAL_ITERATION
 */
p::feature FLOW_FACTS_FEATURE("otawa::ipet::FLOW_FACTS_FEATURE", new Maker<FlowFactLoader>());


} } // otawa::ipet
