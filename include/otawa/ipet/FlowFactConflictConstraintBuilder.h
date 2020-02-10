/*
 *	ipet::FlowFactConstraintBuilder class interface
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
#ifndef OTAWA_IPET_IPET_FLOW_FACT_CONFLICT_CONSTRAINT_BUILDER_H
#define OTAWA_IPET_IPET_FLOW_FACT_CONFLICT_CONSTRAINT_BUILDER_H
#include "../ipet/features.h"
#include <otawa/proc/Feature.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/flowfact/features.h>  
#include "../flowfact/conflict.h"  
namespace otawa { 

using namespace elm;

// Externals
namespace ilp { class System; }

namespace ipet {
	 
		
typedef /*genstruct::*/Vector <Block *> BBList;		
typedef Pair<Block *, int> bbInfo;
typedef Pair<Block *, LockPtr<ListOfEndConflict> > endInfo;

class EdgeInfoOfBB {
	public :
		EdgeInfoOfBB(Block *  b , LockPtr<EdgeInfoOfConflict> edgeInfo):bb(b),edgeOfConflict(edgeInfo){}
		EdgeInfoOfBB():bb(NULL),edgeOfConflict(NULL){}
		inline Block * getBlock(){return bb;}
		inline LockPtr<EdgeInfoOfConflict>  getEdges(){return edgeOfConflict;}
		
	private :
		Block * bb;
		LockPtr<EdgeInfoOfConflict> edgeOfConflict;
};

class EdgeAnnotation {
	public :
		
		EdgeAnnotation(int m, int p, ilp::Var * v ): me(m), pe(p), var (v)  {}
		EdgeAnnotation(): me(1), pe(1), var (NULL)  {}
		inline int getMe() { return me;}
		inline void setMe(int m) {   me = m ;}
		inline void setPe(int p) {  pe = p;}
		inline int getPe() {return pe;}
		inline ilp::Var * getVar(){ return var;}

		
		static inline bool equals(const EdgeAnnotation & p1, const EdgeAnnotation & p2) { return p1.var == p2.var; }
		
	private :
		int me;
		int pe;
		ilp::Var *var ;
 
};
static inline bool  operator==  (const EdgeAnnotation & p1 , const EdgeAnnotation & p2) { return EdgeAnnotation::equals(p1,p2); }




class EdgeInfoConflict {
	public :
			EdgeInfoConflict():s(1){}
			inline int getS(){ return s;}
			inline void setS(int news){ s = news;}

			inline void addContainingLoop( Block * bb) { myLoop.push(bb);}
			inline int nbContainingLoop(){return myLoop.length();}
			inline int getCardE(){return E.length();}
			inline Block *  getContainingLoop(int i){return myLoop.get(i);}

			inline void addAnnotationAndVar( EdgeAnnotation an, ilp::Var * v) { listOfAnnotatedEdges.push(an);E.push(v);}

			inline int nbEdges(){return listOfAnnotatedEdges.length();}
			
			inline void addAnnotation( EdgeAnnotation an) { listOfAnnotatedEdges.push(an);}
			inline EdgeAnnotation popEdge(){return listOfAnnotatedEdges.pop();}

			inline bool isMemEdge (  const EdgeAnnotation &anseqed){
				for (int i=0;i<listOfAnnotatedEdges.length();i++){
					if (EdgeAnnotation::equals(anseqed,listOfAnnotatedEdges.get(i)))   return true; 
				}
				return false;
			}

			void getLoopHeaderList( Block *myLoop );

	 private :
		int s;
		BBList myLoop;
		 /*genstruct::*/Vector<ilp::Var *> E;
		 /*genstruct::*/Vector<EdgeAnnotation> listOfAnnotatedEdges;
	
};

class ListOfEdgeInfoOfBB: public  /*genstruct::*/Vector <EdgeInfoOfBB>{
public :
	bool isFound ( Block * bbnext, int idConflict)const;
	bool isFoundEdge ( Block * bbnext,LockPtr<EdgeInfoOfConflict> ed )const;
	void listedgeInfoOfBB ( Block * bbnext, int idConflict,  /*genstruct::*/Vector< int > &)const;
	int nbContainingConflictEdges(int idConflict,  int level  )const;

};
	
							
class ConflictType: public  /*genstruct::*/Vector <EdgeInfoConflict> {
	public :
		int  getedgeInfoOfConstraint ( otawa::Block * bbnext)const;
 

};

	
	
// FlowFactConflictConstraintBuilder class


typedef Pair <int, LoopOfConflict::conflictLoopQualifier> LoopInfo; //nb it qualifier
typedef Pair<EdgeInfoConflict, LoopInfo > EdgeInfoIntoLoop;
typedef /*genstruct::*/Vector <EdgeInfoIntoLoop >  EdgeInfoIntoLoopList;
class FlowFactConflictConstraintBuilder: public BBProcessor {
public:
	static p::declare reg;
	FlowFactConflictConstraintBuilder(p::declare& r = reg);
	virtual void configure(const PropList& props = PropList::EMPTY);

protected:
	virtual void processBB(WorkSpace *ws, CFG *cfg, Block *bb);
	virtual void setup(WorkSpace *ws);
	virtual void processBBConflict(WorkSpace *ws ) ; 
	virtual void processWorkSpace(WorkSpace *fw); 
	

	
private:
	ilp::System *system;
	bool _explicit;
	bool reduice (  Block * head ,  EdgeInfoConflict &  conflictListEvalTohead ) ;
	int getNbItOfUnqualifiedLoop (Block * currentLoop);
	LoopInfo getNbItOfqualifiedLoop (   Block * currentLoop);
	bool reduiceAux (EdgeInfoConflict &  conflictListEvalTohead,  int pos,	/*genstruct::*/Vector <int>  &indexList, EdgeInfoIntoLoopList &subLoopConflict);
	int getInfoOfConflict (/*genstruct::*/Vector <ConflictType> * info, EdgeInfoOfBB elt ,   int level, int maxLevel);

	void setBasicBlockIntoList(   Block * bb, int index1);//indexListByLevel
	void getBasicBlockListOfIndex ( Block * bb, /*genstruct::*/Vector <int> & indexList);//get indexList from indexListByLevel
	LoopInfo getLoopInfo ( Block * currentLoop); //getloopInfo of current conflict
	bool nextBB(    Block * bb, int lg1, /*genstruct::*/Vector< int> &listOfNextEdge, int level,  BBList &seen );
	bool getNext(	Block * bb, int toPush,  /*genstruct::*/Vector< int> & listOfNextEdge, int level,  BBList &seen);
	
	BBList seenFunction;//endListContraint
	
	/*genstruct::*/Vector< Pair < Block *,LoopInfo > >  conflictLoopInfo;//loop info of current conflict
	/*genstruct::*/Vector<  endInfo> constraintList;//endListContraint
 	/*genstruct::*/Vector<  bbInfo> bbUnqualifiedLoopList; // basic bloc + it for unqualified constraint
	/*genstruct::*/Vector < Pair < bbInfo, LockPtr <ListOfLoopConflict > > > bbQualifiedLoopList; //for qualified constraint
	ListOfEdgeInfoOfBB EdgeList;//ALL
	ListOfEdgeInfoOfBB EdgeListOfConstraint; 	// EdgeListOfConstraint list of edge of the current contraint	
	ConflictType listOfEdges;//current eval list
	BBList endBBForCurrentCte;
	int nbOfConstraint;
	int idConflict; // id of current eval conflict
	/*genstruct::*/Vector< Pair < Block *, /*genstruct::*/Vector< int > > >  indexListByLevel; //edge are sort by level of loop into a conflict
	EdgeInfoConflict  conflictListEvalTohead; 	//current conflict eval
	int nbBBOfLevel ;
	int MaxnbBBOfLevel ; 	

	 
};

} } // otawa::ipet

#endif //OTAWA_IPET_IPET_FLOW_FACT_CONFLICT_CONSTRAINT_BUILDER_H
