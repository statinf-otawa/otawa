/*
 *	$Id$
 *	ipet::FlowFactConflictLoader class implementation
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
#include <otawa/cfg/CFGCollector.h>
#include <otawa/cfg/Dominance.h>
#include <otawa/dfa/BitSet.h>
#include <otawa/flowfact/features.h>
#include <otawa/flowfact/conflict.h>


#include <otawa/ilp.h>
#include <otawa/ipet/FlowFactConflictConstraintBuilder.h>
#include <otawa/ipet/FlowFactLoader.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ipet/VarAssignment.h>
#include <otawa/flowfact/FlowFactLoader.h>

namespace otawa { namespace ipet {

/**
 * @class FlowFactConflictConstraintBuilder
 * This processor allows using extern flow facts in an IPET system.
 * Uses the LOOP_COUNT properties provided by FlowFactLoader to build constraints
 *
 * @par Configuration
 * @li @ref FLOW_FACTS_PATH
 *
 * @par Required Features
 * @li @ref ipet::ILP_SYSTEM_FEATURE
 * @li @ref ipet::COLLECTED_CFG_FEATURE
 * @li @ref ipet::FLOW_FACTS_FEATURE
 * @li @ref LOOP_HEADERS_FEATURE
 *  @li @ref WEIGHT_FEATURE
 * @par Provided Features
 * @li @ref ipet::FLOW_FACTS_CONFLICT_CONSTRAINTS_FEATURE
 */


/**
 */
p::declare FlowFactConflictConstraintBuilder::reg = p::init("otawa::ipet::FlowFactConflictConstraintBuilder", Version(1, 1, 0))
	.require(COLLECTED_CFG_FEATURE)
	.require(LOOP_HEADERS_FEATURE)
	.require(ASSIGNED_VARS_FEATURE)
	.require(FLOW_FACTS_FEATURE)
	.require (LOOP_INFO_FEATURE)  
	.require(WEIGHT_FEATURE)  
	.provide(FLOW_FACTS_CONFLICT_CONSTRAINTS_FEATURE)  
	.make<FlowFactConflictConstraintBuilder>()
	.extend<BBProcessor>();


/**
 * Build a new flow fact loader.
 */
FlowFactConflictConstraintBuilder::FlowFactConflictConstraintBuilder(p::declare& r)
: BBProcessor(r), system(0), _explicit(false),nbOfConstraint(0)
{}


/**
 */
void FlowFactConflictConstraintBuilder::setup(WorkSpace *ws) {
	system = SYSTEM(ws);
	ASSERT(system);
}


void FlowFactConflictConstraintBuilder::processWorkSpace(WorkSpace *fw) {
	CFGProcessor::processWorkSpace(fw) ; // Get the CFG collection
	processBBConflict(fw);
}

 /**
 * get the loop header of a block if it exist or NULL
 * @param source	first bb.
 * @return		Matching loop header or null   if not found.
 */ 
Block * getLoopInnerHeader ( Block *source){
    Block *  lhs = (LOOP_HEADER(source) ?  source : ( ENCLOSING_LOOP_HEADER(source) ?  ENCLOSING_LOOP_HEADER(source) :(otawa::Block * )NULL));
	if (lhs != NULL ) return lhs;
	int nbCaller =0;
	for(auto call:  source->cfg()->callers()){
	//for(CFG::CallerIter call = source->cfg()->callers(); call; call++, nbCaller++) {
		if (nbCaller>0) cout <<"getLoopInnerHeader nbCaller error " << nbCaller+1<<endl;
		lhs = getLoopInnerHeader(call);	 
		nbCaller++;
	}
	return lhs;
}

 
/**
 * Called to built a list ordered of all loops containing Block that contains an edge of the conflict
 * @param myLoop	first bb.
 */ 
void EdgeInfoConflict::getLoopHeaderList( Block *myLoop ){
	
	if (myLoop == NULL) return;
	if (LOOP_HEADER(myLoop)) {
		if (ENCLOSING_LOOP_HEADER(myLoop) ) getLoopHeaderList(ENCLOSING_LOOP_HEADER(myLoop));
		else{
			int nbCaller =0;
			for(auto call:  myLoop->cfg()->callers()){
			//for(CFG::CallerIter call = myLoop->cfg()->callers(); call; call++, nbCaller++) {
				 if (nbCaller>0) cout <<"getLoopInnerHeader nbCaller error " << nbCaller+1<<endl;
				 Block * next = getLoopInnerHeader(call);
				 if (next) getLoopHeaderList(next);	 
				 nbCaller++;
			}
		}
		addContainingLoop (myLoop);
	}
	else { 
			Block * next = getLoopInnerHeader ( myLoop);
			if (next) getLoopHeaderList(next);
	}
}	 
 	 
 
/**
 * @param bbnext	a bb.
 * @param idConflict	an id of conflict
 * @return	true if bb is men into the conflict else false
 */ 
bool ListOfEdgeInfoOfBB::isFound ( Block * bbnext, int idConflict)const{
	for(int i=0; i< length(); i++){
		 EdgeInfoOfBB elt = get(i); 
		 if  (elt.getBlock() ==  bbnext  && elt.getEdges() &&   elt.getEdges()->isInto(idConflict))
		    return true;  
	}
	return false;	
}	
/**
 * @param bbnext	a bb.
 * @param ed	an edge on conflict of conflict
 * @return	true if bb containts ed false
 */ 
bool ListOfEdgeInfoOfBB::isFoundEdge ( Block * bbnext,LockPtr<EdgeInfoOfConflict> ed )const{
	for(int i=0; i< length(); i++){
		 EdgeInfoOfBB elt = get(i); 
		 if  (elt.getBlock() ==  bbnext && elt.getEdges() && ed->getSource() == elt.getEdges() ->getSource() && ed->getTarget() == elt.getEdges() ->getTarget()) 
		    return true;	  
	}
	return false;	
}
/**
 * @param bbnext	a bb.
 * @param idConflict	an id of conflict
 * @param indexList	list of index of edge of idConflict out of bbnext
 */ 

void ListOfEdgeInfoOfBB::listedgeInfoOfBB ( Block * bbnext, int idConflict, /*genstruct::*/Vector< int > &indexList )const {
	for(int i=0; i< length(); i++){
		 EdgeInfoOfBB elt = get(i); 
		 if  (elt.getBlock() ==  bbnext  && elt.getEdges() &&    elt.getEdges()->isInto(idConflict ))
		    indexList.push( i);  
	}
}

/**
 * @param level	whith generic edge of conflict are looking for
 * @param idConflict	an id of conflict
 * @return	nb of edge of level into the conflict
 */ 

 int ListOfEdgeInfoOfBB::nbContainingConflictEdges(int idConflict,   int level  )const{
	 int res = 0;
	for(int i=0; i< length(); i++){
		 EdgeInfoOfBB elt = get(i); 
		 int mylevel = ( elt.getEdges()  ?  elt.getEdges()->isIntoLevel(  idConflict )  :-1) ;
		 if  (mylevel == level) res++;
	}
	return res;
}

/**
 * @param bbnext	a bb.
 * @return	index of bb into conflict
 */ 
int  ConflictType::getedgeInfoOfConstraint ( otawa::Block * bbnext)const{
	for(int i=0; i< length(); i++){
		 EdgeInfoConflict elt = get(i);
		 if  (elt.nbContainingLoop()>0 && elt.getContainingLoop(0) ==  bbnext) return i;
		 else if  ( elt.nbContainingLoop()==0 && NULL ==  bbnext) return i;
	}
	return -1;
}

/*GOTO REAL Return */ 
/**
 * @param initnext	a bb.yyyyyyyy
 * @return	true if its caller is synth
 */ 
bool isSynthCalleCaller(Block * initnext){
	bool res = false;
	CFG * cfg;
	 if (initnext &&initnext->cfg()){
		cfg =initnext->cfg();
		int nbCaller =0;
		for(auto call:  cfg->callers()){
		//for(CFG::CallerIter call = cfg->callers(); call; call++ ) {
			if (nbCaller>0) cout <<"CALL lgSubPath nbCaller error " << nbCaller+1<<endl; 
				if (call && call->isSynth()&&(call->toSynth()->callee()))  return true;	
			}
	}
	return res;
}
/**
 * @param initnext	a bb.
 * @return	return real return bb
 */ 

Block * getSynthCalleCaller(Block * initnext){
	Block * bb = initnext;
	Block * pred = initnext;
	CFG * cfg;
	String initName = initnext->toSynth()->callee()->name();
	while (bb && isSynthCalleCaller(bb) ){
		 
		cfg =bb->cfg();
		int nbCaller =0;
		for(auto call:  cfg->callers()){
		//for(CFG::CallerIter call = cfg->callers(); call; call++ ) {
			if (nbCaller>0) cout <<"CALL lgSubPath nbCaller error " << nbCaller+1<<endl; 
			if (initName.compare(bb->toSynth()->callee()->name())!=0) return pred;
			pred=bb;
			bb=call;
		}
	} 
	return pred;
}


/**
 * @param bb to visit
 * @param listOfNextEdge list of next edge of the current conflict (to maj)
 * @param level index of the generic edge of the conflict that we are looking for
 * @param seen bb already visited
 * @return	true if an edge is found used by nextBB
 */ 
bool FlowFactConflictConstraintBuilder::getNext(	Block * bb, int toPush, /*genstruct::*/Vector< int> & listOfNextEdge, int level,  BBList &seen){
	bool res=false;
	/*genstruct::*/Vector< int > lindex;
	EdgeListOfConstraint.listedgeInfoOfBB (  bb ,   idConflict, lindex);
	int lg = lindex.length();
	int mylevel =0;
 
	for(int iii =0; iii <lg; iii++){
		int index =  lindex.get(iii);  
		EdgeInfoOfBB e =EdgeListOfConstraint.get(index);
		mylevel =  e.getEdges()->isIntoLevel(  idConflict)  ; 
		if (mylevel == level ) { 
			res= true;
			if (!listOfNextEdge.contains(index)) { listOfNextEdge.push(index);nbBBOfLevel++; } 
		} 
	} 
	if(toPush) seen.push(bb);
	return res;	
}			
/**
 * @param bb from list path to visit
 * @param listOfNextEdge list of next edge of the current conflict (to maj)
 * @param level index of the generic edge of the conflict that we are looking for
 * @param seen bb already visited
 * @return	true if an edge is found used in getInfoOfConflict
 */ 
bool FlowFactConflictConstraintBuilder::nextBB(   Block * bb,  int lg1, /*genstruct::*/Vector< int> &listOfNextEdge, int level,  BBList &seen ){
	int lg = lg1;
	if (bb == NULL || (seen.contains(bb) ) || endBBForCurrentCte.contains(bb)) return  false;
	if (lg == 0) return   false;
	bool res = false;
	if (MaxnbBBOfLevel == nbBBOfLevel) return true;
 	if (LOOP_HEADER(bb)){// dans une boucle on peut passer 2 fois dans le loop header une fois pour touver un chemin interne une fois pour trouver la sortie
		for(Block::EdgeIter e = bb->outs();  e() ;e ++) { 
			Block * lhs = (LOOP_HEADER(e->sink()) ? e->source() : (ENCLOSING_LOOP_HEADER(e->sink()) ?  ENCLOSING_LOOP_HEADER(e->sink()) :(otawa::Block * )NULL)); 
			if (lhs!=bb) {//exit			
				if (   EdgeListOfConstraint.isFound (e->sink(), idConflict))  {									
					res = res||getNext(	 e->sink(), false, listOfNextEdge, level, seen);
					if (res) return true;						 
				} 
				//because of next loop even when ...
				res = res ||nextBB( e->sink(), lg, listOfNextEdge, level, seen);  
				seen.push(e->sink());
				if (res) return res;  
			} 
		}
	}
	else {
		if(bb->isExit())  { 	 	
		    int nbCaller =0;
		    for(auto call:  bb->cfg()->callers()){
			//for(CFG::CallerIter call = bb->cfg()->callers(); call; call++, nbCaller++) {
				if (nbCaller>0) cout <<"CALL lgSubPath nbCaller error " << nbCaller+1<<endl;  
				bool syntF = (call->isSynth()&&(call->toSynth()->callee()));
				Block *  lgcall=NULL;
				bool recend=false;
 				nbCaller++;
				if( syntF){    
					lgcall =getSynthCalleCaller(call);	 
					if (lgcall->toSynth()->callee()->exit() == NULL ||
					lgcall->toSynth()->callee()->exit()==bb) { recend= true;}
				}	 
				if (!(!syntF ||recend))	  { //Real return 
					if (seen.contains(lgcall->toSynth()->callee()->exit()))
					res= res||nextBB( lgcall->toSynth()->callee()->exit(), lg, listOfNextEdge, level, seen); 	
					if (res ) return res; 	
				} 
			} 
		}   
	} seen.push(bb);  
	
		for(Block::EdgeIter edge = bb->outs();  edge() ;edge ++) { //For each edges 
			if (MaxnbBBOfLevel == nbBBOfLevel) return true;		
			if ( edge->sink()->isExit() ) {
				int nbCaller =0; 
				seen.push(edge->sink());
				for(auto call:  edge->sink()->cfg()->callers()){	 
				//for(CFG::CallerIter call = edge->sink()->cfg()->callers(); call; call++, nbCaller++) {//to call function
					if (nbCaller>0) cout <<"nextBB nbCaller error " << nbCaller+1<<endl;
					Block *  lgcall; 
					bool loop = LOOP_HEADER(edge->sink());
					bool syntF = (call->isSynth()&&(call->toSynth()->callee()));
					bool recend=false;
					nbCaller++;
					if(!loop&&syntF){     
						lgcall =getSynthCalleCaller(call);	 
						if (  lgcall->toSynth()->callee()->exit()==edge->sink() )  
							recend= true;
					}	 
					if (!syntF||loop ||recend)	
					{
						for(Block::EdgeIter e = call->outs();     e() ;e ++) {  
							if(!seen.contains( e->target())) {	
								if (  EdgeListOfConstraint.isFound (e->sink(), idConflict))  //edge of the constraint after the searched one
									return getNext(	 e->sink(), true, listOfNextEdge, level, seen);						   
								else  res= res||nextBB(e->sink(), lg, listOfNextEdge, level, seen); 
								if (res) return res;
							} 
						}//for
					}	
					else{//Real exit 
						if (seen.contains(lgcall->toSynth()->callee()->exit()))		
							res= res||nextBB(lgcall->toSynth()->callee()->exit(), lg, listOfNextEdge, level, seen); 	
						if (res ) return res;
					}				 				
				}//for				 
			} 
			else {	 
				if( edge->target()->isSynth()&&(edge->target()->toSynth()->callee())){ 	 	  
					Block * pred =getSynthCalleCaller(edge->target());
					if(!edge->target()->toSynth()->outs()->sink()->isSynth()) 
					{ 
						//into call ne pas empiler le fonctions sink * calls possibles
						bool isFoundbb=false;
						for(CFG::BlockIter nbb = edge->target()->toSynth()->callee()->blocks(); !isFoundbb && nbb(); nbb++) {
							if  (nbb-> address() == edge->target()->address () ) {//a
								isFoundbb = true;   
								if ( !seen.contains(*nbb) )
								{	 
 									if (   EdgeListOfConstraint.isFound (*nbb, idConflict)) //edge of the constraint after the searched one	 
										return getNext(	*nbb, true, listOfNextEdge, level, seen);
									else {
										res = res ||nextBB(*nbb, lg, listOfNextEdge, level, seen);   
									    if (res )   return res;	    
									}
								}  
							}  
						}   
					}
					else{//real call 
						SynthBlock* sb = pred->toSynth()->outs()->sink()->toSynth();
						if ( !seen.contains(sb->callee()->entry()) ) {
							res = res ||nextBB(sb->callee()->entry(), lg, listOfNextEdge, level, seen);  
							if (res )   return res;						 
						 } 
					} 
				}
				else {   
					if ( !seen.contains(edge->sink())){	 		 
						if (   EdgeListOfConstraint.isFound (edge->sink(), idConflict))  {  
							res = res||getNext(	edge->sink(), true, listOfNextEdge, level, seen);
							return false; //edge of the constraint after the searched one	
						} 
						else {  // we call on next bloc of the bb but not on itself
							if (edge->target() != bb)  {  
							   res = res || nextBB(edge->sink(), lg, listOfNextEdge, level, seen);	
								seen.push(edge->sink());
							}
						}  		
					}
				} 
			}						 
		} 	
	 		
	return res;
}
/**
 * @param elt edge currenly visited head of the conflict initialy
 * @param info list of collected info of the conflict (to maj)
 * @param level index of the generic edge of the conflict that we are looking for
 * @param maxLevel nb of edge into generc conflict
 * @return	true if all info of the conflict have been correctely collected
 */ 
int FlowFactConflictConstraintBuilder::getInfoOfConflict (Vector <ConflictType> * info, EdgeInfoOfBB elt , int level, int maxLevel){
	int lastLevel =level;
	
	LockPtr<EdgeInfoOfConflict> edgeInfo   ;
	Vector<ilp::Var *> CE;
	otawa::ilp::Var *var ;
	Vector < Pair < Vector < otawa::ilp::Var * >, Pair <bbInfo , bbInfo> > > edgeLIst; 
	Block * bbnext = elt.getBlock() ;
	edgeInfo = elt.getEdges() ; 
	
	int numedge =0;
	bool trouve=false;
	if (edgeInfo &&  edgeInfo->isInto(  idConflict ) ) {
		for(Block::EdgeIter edge = bbnext->outs();  !trouve && edge() ;edge ++) {	 	
			ASSERT(edge->source());	
			Block * lhs =  getLoopInnerHeader ( edge->source());
			
			if ( (edge->source()->address() == edgeInfo->getSource()  && edge->sink()->address() == edgeInfo->getTarget())
					||(edge->source()->address() == edgeInfo->getSource() && edge->sink()->isExit() ) ) { 
				numedge++;
				trouve=true;
 				var  = VAR(*edge);			
					
				EdgeAnnotation edgeInfor(1, 1, var);
				Block * myLoop = lhs;
					
				//because the output edges are unfolded		
				MaxnbBBOfLevel = EdgeListOfConstraint.nbContainingConflictEdges(  idConflict,   level +1 );
					 		
				Vector <ConflictType> NEXtinfo; 
				if (MaxnbBBOfLevel >0 && level < maxLevel) { //il faut trouver les autres suivants
					Vector< int> listOfNextEdge;
					BBList seen2;
					nbBBOfLevel = 0;
					nextBB( bbnext,  1, listOfNextEdge, level+1, seen2);
 						
					for (int k12=0; k12< listOfNextEdge.length();k12++) 
						lastLevel = getInfoOfConflict ( &NEXtinfo, EdgeListOfConstraint.get(listOfNextEdge.get(k12)), level+1, maxLevel);
				}
					
				if (NEXtinfo.isEmpty()){//add edge to new path
					ConflictType ct;
					EdgeInfoConflict e; 
					e.addAnnotationAndVar(edgeInfor,var);
					e.getLoopHeaderList( myLoop);
					ct.push(e);
					info->push(ct); 
				}
				else {
					EdgeInfoConflict e;
					e.getLoopHeaderList( myLoop);
						
					for(int k55=0;k55<NEXtinfo.length();k55++){		 
						ConflictType  & ct  = NEXtinfo.get(k55); //item(k55); 
						int index = (e.nbContainingLoop()>0 ? ct.getedgeInfoOfConstraint (e.getContainingLoop(0)) : ct.getedgeInfoOfConstraint (myLoop));
						if (index == -1) {
							EdgeInfoConflict e;
							e.addAnnotationAndVar(edgeInfor,var);
							e.getLoopHeaderList( myLoop);
							ct.push(e);
						}
						else { 
							EdgeInfoConflict &e = ct.get(index);//item(index);  
							e.addAnnotationAndVar(edgeInfor,var); 
						}
						info->push(ct);
					}
				}
			}			
		} 
	}	
	return lastLevel;
 }
 
/**
 * @param bb an bb of the current conflict
 * @param index1  
 * used for Pascal Raymond algo to group edge into correct nessted loop
 */ 
void FlowFactConflictConstraintBuilder::setBasicBlockIntoList(  Block * bb, int index1){
	for(int i=0; i< indexListByLevel.length(); i++){// A conflict to several one => for each one an ILP constraint	
		Pair < Block *, Vector<   int> >    elt1 =indexListByLevel.get(i);//item(i);
		if (bb == elt1.fst) {
			elt1.snd.push( index1 );
			return;
		}
	}

	Pair < Block *, Vector   < int> >  elt;
	elt.fst = bb;
	elt.snd.push( index1 );
	indexListByLevel.push(elt );	
}
/**
 * @param bb an bb of the current conflict
 * @param indexList  (built here)
 * used for Pascal Raymond algo to group edge into correct nessted loop
 */ 
void FlowFactConflictConstraintBuilder ::getBasicBlockListOfIndex (  Block * bb, /*genstruct::*/Vector <int> & indexList){
	for(int i=0; i< indexListByLevel.length(); i++){ 
		Pair < Block *, Vector<   int> >    elt1 =indexListByLevel.get(i);
		if (bb == elt1.fst) {
			indexList = elt1.snd ;
			return;
		}
	}	
}
/**
 * @param currentLoop loop header
 * @return	nb iteration
 */ 
int FlowFactConflictConstraintBuilder :: getNbItOfUnqualifiedLoop (Block * currentLoop){
	for(int i=0; i< bbUnqualifiedLoopList.length(); i++){ 
		bbInfo   elt1 =bbUnqualifiedLoopList.get(i);
		if (currentLoop == elt1.fst)   return elt1.snd;
	}	
	return -1;
}

/**
 * @param currentLoop loop header
 * @return	LoopInfo ie nb iteration + qualibier ex ALL_IT from complet list of bbQualifiedLoopList
 */	
LoopInfo FlowFactConflictConstraintBuilder::getNbItOfqualifiedLoop (  Block * currentLoop){ //retunr -1 if not found else nbIt and qualifier
	for(int i=0; i< bbQualifiedLoopList.length(); i++){ 
		Pair < bbInfo, LockPtr <ListOfLoopConflict > >    elt1 =bbQualifiedLoopList.get(i);
		if (currentLoop == elt1.fst.fst){
			for(int j=0; elt1.snd&&j< elt1.snd->length(); j++){ 
				LoopOfConflict conflict = elt1.snd->get(j);
				if( conflict.getIdConflict() == idConflict)   return LoopInfo(elt1.fst.snd , conflict.getQualifier());
			}
		}
	}	
	return LoopInfo (-1 , LoopOfConflict::ALL_IT);
}
/**
 * @param currentLoop loop header
 * @return	LoopInfo ie nb iteration + qualibier ex ALL_IT from complet list of current conflictLoopInfo
 */	
LoopInfo FlowFactConflictConstraintBuilder::getLoopInfo ( Block * currentLoop){
	for(int i=0; i< conflictLoopInfo.length(); i++){ 
		Pair < Block *, LoopInfo  >   elt1 =conflictLoopInfo.get(i);
		if (currentLoop == elt1.fst)  return elt1.snd;
	}	
	return LoopInfo (-1 , LoopOfConflict::ALL_IT);
}


/**
 * Pascal Raymond algo implementation for eval seq, L(body1, b1,c1)   , L(body2, b2,c2) …, L(bodyl, bl,cl)
 * @param seq, the
 * @param subLoopConflict
 * @param conflictListEvalTohead partial result of eval
 * @return	succes/failled
 */
bool evalSequence(EdgeInfoConflict &seq,  EdgeInfoIntoLoopList & subLoopConflict,	EdgeInfoConflict &   conflictListEvalTohead ) {
	int sed =  subLoopConflict.length();	
	if (seq.nbEdges()>0) sed++;
	if (sed >0){			
		 // getProd
		 int prod =1;	
		 int prod2=1;
		 for (int i=0;i<  subLoopConflict.length();i++){ 
			EdgeInfoIntoLoop loopc = subLoopConflict.get(i);
 			int temp = (loopc.snd.snd == LoopOfConflict::ALL_IT ?loopc.snd.fst:1) ;
			prod *= temp;
			prod2 *= loopc.fst.getS();
		 }
		 for (int i = seq.nbEdges();i>0 ;i--) {
			EdgeAnnotation anseqed = seq.popEdge();
			int pe =prod*prod2;
			anseqed.setPe (pe); 
			if (!conflictListEvalTohead.isMemEdge (anseqed) ){
				 
				conflictListEvalTohead.addAnnotation(anseqed) ;
			}else {  return false; } 
		 }
		 
		 //for sequential loop
		 int s = prod;
		 
		 while(! subLoopConflict.isEmpty()){ 
			EdgeInfoIntoLoop loopc = subLoopConflict.pop();
			EdgeInfoConflict anseqed=loopc.fst;
			int temp = (loopc.snd.snd == LoopOfConflict::ALL_IT ?loopc.snd.fst:1) ;
			int b = loopc.snd.fst;
			
			s *= loopc.fst.getS() ;
			for (int i = loopc.fst.nbEdges();i>0 ;i--) { 
				EdgeAnnotation anseqed = loopc.fst.popEdge();
				if (temp == 0 || loopc.fst.getS() == 0) return false;
				int res = anseqed.getPe()*prod/temp*prod2  /loopc.fst.getS(); 
				anseqed.setPe ( res); 
				res = anseqed.getMe() * b; 
				anseqed.setMe (res); 
				conflictListEvalTohead.addAnnotation(anseqed);
			}	
		 }
		  conflictListEvalTohead.setS (s);
	}	 
   return true;
}

/**
 * Pascal Raymond algo implementation of a conflict (one path)
 * class each edge into a form seq, L(body1, b1,c1)   , L(body2, b2,c2) …, L(bodyl, bl,cl) where loop may have inner loops
 * @param head is the head of conflict (first edge bb)
 * @param pos current embrication level
 * @param indexList index on edegs of level
 * @param conflictListEvalTohead  result of eval
 * @return	succes/failled
 */	 
 bool FlowFactConflictConstraintBuilder::reduiceAux (	EdgeInfoConflict &  conflictListEvalTohead,   int pos, /*genstruct::*/Vector <int>  &indexList,
					EdgeInfoIntoLoopList &subLoopConflict) {
	EdgeInfoConflict seq ;
	//get sub loop
	BBList OuterLoop;
	int nbEdgeOfLevel = indexList.length();

	for(int i=0; i< nbEdgeOfLevel; i++){ //   list index of coflict from the current bb
		int index = indexList.get(i);
		EdgeInfoConflict e  =listOfEdges.get(index  );
		int nbLoop = e.nbContainingLoop();
		EdgeInfoIntoLoopList  subLoopConflictOfCurrentLoop; 
		if (0 == pos) { // no loop a sequence
		    seq = e; // outer loop contains only a sequence		 
		}
		else{
			Block * bb =  e.getContainingLoop(nbLoop - pos);//subLoops of 
			LoopInfo  info = getLoopInfo ( bb);	
			if (  pos == 1){ // last loop no inner loop 	
				if (nbLoop == 1)  { // inner and outer too
					if (evalSequence(e,  subLoopConflictOfCurrentLoop,  conflictListEvalTohead)== false) return false ; return true;
				}
				else {
					EdgeInfoConflict resaux ;
					if (evalSequence(e,  subLoopConflictOfCurrentLoop,  resaux)== false) return false ;
					subLoopConflict.push(EdgeInfoIntoLoop (resaux, info));	return true;
				}
			}
			else { // current contains sub loops :recursively from current to inner loop
				Vector <int>  indexListCurrent ;
				//get sub loop
				getBasicBlockListOfIndex(bb,  indexListCurrent); 
				EdgeInfoConflict e1;
				if ((nbLoop - pos)!=0){ // inner  loop
					bool res = reduiceAux (  e1, pos - 1, indexListCurrent,    subLoopConflict);
					if (res == false ){ return res;}
					EdgeInfoConflict resaux ;
					if (evalSequence(e1,  subLoopConflict,  resaux)== false) return false ;
					subLoopConflict.push(EdgeInfoIntoLoop(resaux, info));	
				}		
				else{ 
					bool res = reduiceAux (  e1, pos - 1, indexListCurrent,    subLoopConflict);
					if (res == false ){ return res;}
					EdgeInfoConflict resaux ;
					if (evalSequence(e1,  subLoopConflict,  conflictListEvalTohead)== false) return false ;
					return true;
				}	
			} 
		}
	}
	return true;
}
			
		

/**
 * Pascal Raymond algo implementation of a conflict (one path)
 * class each edge into a form seq, L(body1, b1,c1)   , L(body2, b2,c2) …, L(bodyl, bl,cl) where loop may have inner loops
 * imbrication level are built and the reduiceAux that eval by level is called
 * @param head is the head of conflict (first edge bb)
 * @param conflictListEvalTohead  result of eval
 * @return	succes/failled
 */					
bool FlowFactConflictConstraintBuilder::reduice (  Block * head ,  EdgeInfoConflict &  conflictListEvalTohead){	
 		int nbEdges = listOfEdges.length();
		indexListByLevel.clear();
		EdgeInfoConflict seq , lastLoop;
		BBList OuterLoop;
		BBList AllLoops;
		conflictLoopInfo.clear();
		EdgeInfoIntoLoopList subLoopConflict;  
		Vector <int> lgLoop;
		for(int j=0; j< nbEdges; j++){// for each unfoldingConflict for each edge
			EdgeInfoConflict e =listOfEdges.get(j);
			int nbLoop = e.nbContainingLoop();  
			if (nbLoop == 0) { seq = e;}
			for (int k=0;k<nbLoop;k++) {
				Block * bb = e.getContainingLoop(k);
				setBasicBlockIntoList( bb , j);
				if (k==0&& !OuterLoop.contains(bb) ){  
					OuterLoop.push(bb);
					lgLoop.push(nbLoop);
				} 
				// get loop info for an unfoldingConflict  
				LoopInfo infoLoop = getNbItOfqualifiedLoop (  bb);
				if (infoLoop.fst == -1){
					infoLoop.snd = LoopOfConflict::ALL_IT;
					infoLoop.fst  = getNbItOfUnqualifiedLoop (  bb);
					if (infoLoop.fst == -1) return false;
				} 
				if  (! AllLoops.contains(bb )){
					AllLoops.push(bb);
					conflictLoopInfo.push(  Pair < Block *, LoopInfo >   (bb, infoLoop));
				}
			}
		} 
			
		for(int j=0; j< OuterLoop.length(); j++){
			EdgeInfoIntoLoopList subLoopConflictInner;  				
			Block * bb = OuterLoop.get(j);				 
			EdgeInfoConflict e ; 					
			Vector <int>  indexList ;
			int nbLoop = lgLoop.get(j);	
			//get sub loop
			getBasicBlockListOfIndex(bb, indexList);  
			bool res = reduiceAux (e, nbLoop, indexList, subLoopConflictInner);
			if (res == false ) {  return false;}  
			LoopInfo info = getLoopInfo ( bb);	
			subLoopConflict.push(EdgeInfoIntoLoop(e, info));
		}  
		
		int sed =  subLoopConflict.length();  
		if (seq.nbEdges()>0) sed++;
		if (seq.nbEdges()>0|| subLoopConflict.length()>=1) {
		if (evalSequence( seq,   subLoopConflict,  conflictListEvalTohead ) == false)return false; 
	}
	
	return true;
}


/**
 * for each generic conflict (ffx) try to generate ILP corresponding constraints 
 */	
void FlowFactConflictConstraintBuilder::processBBConflict  (WorkSpace *ws ){  
 	for(int i=0;  i<=nbOfConstraint; i++) { //for each conflict   	 
		EdgeListOfConstraint.clear(); 	// EdgeListOfConstraint list of edge of the current contraint	
		idConflict = i; 
		endBBForCurrentCte.clear();
		for(int j=0; j< constraintList.length(); j++){//extract bb of current conflict
			endInfo elt = constraintList.get(j);
			if  (  elt.snd->contains(idConflict) && !endBBForCurrentCte.contains(elt.fst) )    endBBForCurrentCte.push(elt.fst);
		}	
		
		 
		for(int j=0; j< EdgeList.length(); j++){//extract bb of current conflict
			EdgeInfoOfBB elt = EdgeList.get(j);	 
			if  (elt.getEdges()->isInto(idConflict))   EdgeListOfConstraint.push(elt);  
		}
		    
		ListOfEdgeInfoOfBB listOfbbOfConflict;
	    ListOfEdgeInfoOfBB others; 
		int nbMaxLevel=1; //nb of edges of the generic conflict
		for(int j=0; j< EdgeListOfConstraint.length(); j++){//extract head bb of current conflict
			EdgeInfoOfBB elt = EdgeListOfConstraint.get(j);	 
			int mylevel = elt.getEdges()->isIntoLevel(  idConflict)  ;
			if (elt.getBlock() != NULL && mylevel > 1 ){
				if (nbMaxLevel<mylevel ) nbMaxLevel=mylevel;
			}
			if (elt.getBlock() != NULL && mylevel == 1 )
				 listOfbbOfConflict.push(elt); //if not any head of conflict : the first edge is not transfert 
		}
		 Vector<ConflictType> conflictList;
		
	
		for(int j=0; j< listOfbbOfConflict.length(); j++){ // for reach unrolling conflict head  of the current constraint
			//OF EACH PATH CONTAINING THE CONFLICT FROM THE CURRENT HEAD GET INFO ilp VAR... TO MAKE THE ilp
			EdgeInfoOfBB elt = listOfbbOfConflict.get(j); // bb head elt.fst
			 Vector<ConflictType> conflictList;		
			 
			 int lastNumber = getInfoOfConflict ( &conflictList, elt, 1, nbMaxLevel);		
		 
			 bool Goodnb=true;
			//if  ( lastNumber !=nbMaxLevel all the edges of the constraint have not been found
			//GETTING INFO OK
 			while (Goodnb && lastNumber ==nbMaxLevel&& ! conflictList.isEmpty()){ // A conflict to several one => for each one an ILP constraint			
				 listOfEdges = conflictList.pop(); 
 				int carE=0;	 
				 
				for(int j12=0; j12< listOfEdges.length(); j12++){ // one conflict by unfolding conflict
					EdgeInfoConflict e =listOfEdges.get(j12);
					carE += e.getCardE();
				}	 
		 
				if (carE == lastNumber){//APPLY p. rAYMOND ALGO TO MAKE CONSTRAINT because all edge are colledted 
					EdgeInfoConflict  conflictListEvalTohead; 	
					bool ok= reduice ( elt.getBlock() ,   conflictListEvalTohead);  
					int s = conflictListEvalTohead.getS();
					string label;
					 
					if(_explicit) label = _ << "conflict from constraint  " <<idConflict;
					if (ok && conflictListEvalTohead.nbEdges()>0 &&  conflictListEvalTohead.nbEdges() == carE){
						otawa::ilp::Constraint *cons = system->newConstraint(label, otawa::ilp::Constraint::LE);		
						cons->addRight((carE-1)*s);
						cout << "conflict "<< idConflict <<" (carE-1)*s " <<(carE-1)*s << " s = "<< s << " cardE = "<< carE << " id conflict "<<idConflict<<  " AnnotatedEdges  " <<  conflictListEvalTohead.nbEdges() << endl;

						if(logFor(LOG_BB))
							log << "conflict "<< idConflict <<" (carE-1)*s " <<(carE-1)*s << " s = "<< s << " cardE = "<< carE << " id conflict "<<idConflict<<  " AnnotatedEdges  " <<  conflictListEvalTohead.nbEdges() << endl;
						
						for(int ie=0 ;ie < carE  ;ie++ ) {
							EdgeAnnotation anseqed = conflictListEvalTohead.popEdge();
							int l = anseqed.getPe() * anseqed.getMe() -s;
							cons->addLeft(anseqed.getPe(), anseqed.getVar());
							cons->addRight(l); 			 
						}  
					}    
				}  
				else Goodnb=false;
			}
		}  
	} 

}	

/**
 * FlowFactConflictConstraintBuilder::processBB get info  : constraintList is the list of bb containing a begin of conflict		
 */
void FlowFactConflictConstraintBuilder::processBB(WorkSpace *ws, CFG *cfg, Block *bb) {
		 LockPtr<ConflictOfPath>  infeasablePathConstraint = INFEASABLE_PATH(bb) ; 

		if(infeasablePathConstraint){// get begin mark
			int lgi = infeasablePathConstraint->getListOfConflicts().length();
			if (lgi>0){ //is bb the beggining of a or more conflict
				for(int i=0; i<lgi; i++) { //for each conflict beggining on bb
					Conflict conf = infeasablePathConstraint->getListOfConflicts().get(i);
					int conflictId = conf.getConflictID(); 
					if (nbOfConstraint<conflictId) nbOfConstraint=conflictId;
				}
			}
		}
		LockPtr<ListOfEndConflict>currentCteEndNumList = INFEASABLE_PATH_END(bb) ; 
		if (currentCteEndNumList && !seenFunction.contains(bb)&& currentCteEndNumList->length() > 0 ){ // get end mark end of conflict
			seenFunction.push(bb); 
			constraintList.push (endInfo (bb ,   currentCteEndNumList));  
		}
		 
		LockPtr <ListOfEdgeConflict > listOfedgeInfo = EDGE_OF_INFEASABLE_PATH_I(bb)  ;		
		if(listOfedgeInfo){ // get edges marks
			int lg = listOfedgeInfo->length();			 
			for(int i=0; i<lg; i++) {
				LockPtr<EdgeInfoOfConflict>edgeInfo=listOfedgeInfo->get(i);
				if (!EdgeList.isFoundEdge (bb,edgeInfo))   EdgeList.push( EdgeInfoOfBB(bb , edgeInfo)); // list of edges of any conflict
			}	
		}

	if (LOOP_HEADER(bb)) {//infeasablePathconflict get info when loop
		LockPtr <ListOfLoopConflict > infeasablePathConstraintLoop = LOOP_OF_INFEASABLE_PATH_I(bb) ; //get loop qualifier
		int sum=0;  
		for(Block::EdgeIter edge = bb->ins();  edge() ;edge ++) {   //get it number
			if (BACK_EDGE(*edge) )sum += WEIGHT(edge->target());
		}		 
		if(infeasablePathConstraintLoop){
			if (infeasablePathConstraintLoop->length() > 0)  {//get Info of loop
				bbQualifiedLoopList.push(  Pair < bbInfo, LockPtr <ListOfLoopConflict >  >  ( bbInfo (bb , sum ), infeasablePathConstraintLoop));
			}
		}
		bbUnqualifiedLoopList.push(bbInfo(bb , sum ) );
 	}
}


/**
 */
void FlowFactConflictConstraintBuilder::configure(const PropList& props) {
	BBProcessor::configure(props);
	_explicit = EXPLICIT(props);
}


/**
 * This feature asserts that constraints tied to the flow fact information
 * has been added to the ILP system.
 */
p::feature FLOW_FACTS_CONFLICT_CONSTRAINTS_FEATURE("otawa::ipet::FLOW_FACTS_CONFLICT_CONSTRAINTS_FEATURE", p::make<FlowFactConflictConstraintBuilder>());

}

} // otawa::ipet
