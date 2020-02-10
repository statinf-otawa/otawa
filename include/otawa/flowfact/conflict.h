/*
 *	$Id$
 *	Contextual conflict class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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
#ifndef OTAWA_FLOWFACT_CONFLICT_H_
#define OTAWA_FLOWFACT_CONFLICT_H_

#include <elm/xom/Element.h>
#include <otawa/prop/ContextualProperty.h>
#include <otawa/proc/AbstractFeature.h>
#include <otawa/prog/Inst.h>
//#include <elm/genstruct/Vector.h>//MDM ADD
#include <elm/util/LockPtr.h>

namespace otawa {
class Conflict {
 public :
    Conflict() : idConflict(-1){}
	Conflict(int id) : idConflict(id){}
	inline int getConflictID(){return  idConflict;}
	static inline bool equals(const Conflict & p1, const Conflict & p2) { return p1.idConflict == p2.idConflict; }

 private :
	int idConflict;
};
static inline bool  operator==  (const Conflict & p1 , const Conflict & p2) { return Conflict::equals(p1,p2); }

class ConflictOfPath :public Lock{
public :
	ConflictOfPath(ContextualPath p , Conflict id) : path(p){ conflictList.push(id);}
	ContextualPath getIPath(){return path;}	
	inline void setPath(ContextualPath np){path=np;}
	inline bool isMenConflict(const Conflict &id) {return conflictList.contains(id);}   
	inline void addConflictToPath(const Conflict & id) {conflictList.push(id);}
	inline  Vector <Conflict > & getListOfConflicts(){return conflictList;}
 private :	
	ContextualPath path;
	 Vector <Conflict > conflictList;

};
 
class   LoopOfConflict  {
	public:
	typedef enum {ALL_IT = 2, FIRST_IT=1, LAST_IT =0} conflictLoopQualifier;
	
	inline int getIdConflict(){ return constraintId;}
	inline conflictLoopQualifier  getQualifier(){ return itInfo;}

	LoopOfConflict (int a, conflictLoopQualifier b):	constraintId(a),	itInfo(b){}
	LoopOfConflict ( ):	constraintId(0),	itInfo(ALL_IT){}
	private :
		int constraintId;
		conflictLoopQualifier itInfo; 

	
} ;


class  EdgeInfoOfConflict :public Lock {
	public : 

		inline Address getSource(){return begin;}
		inline Address getTarget(){return end;}
		inline  Vector <Pair<int, int> > & getInfoOfConflicts(){return idConflic;}		
		inline void addInfoOfConflicts(Pair<int, int> ni){ idConflic.push(ni);}
		inline void setSource(Address s) { begin = s;}	 
		inline void setTarget(Address s) { end = s;}	 
		
		inline int isIntoLevel(int id )const{
			for(int i=0; i< idConflic.length(); i++){
				 Pair<int, int> elt = idConflic.get(i);
				 if  (elt.fst ==  id  ) return elt.snd;
			}
			return -1;
		}

		inline bool isInto(int id   )const{
			return isIntoLevel(  id  )>-1;	
		}



	private :
		 Address begin;
		 Address end;
		  Vector <Pair<int, int> > idConflic; //num of conflict and edge number	
}   ;
 
class ListOfLoopConflict : public  Vector <LoopOfConflict >, public Lock {
};
class ListOfEndConflict : public  Vector <int >, public Lock {
};
class ListOfEdgeConflict : public  Vector< LockPtr<EdgeInfoOfConflict > >, public Lock {
};

extern Identifier< LockPtr<ConflictOfPath>> INFEASABLE_PATH; //MDM add id Of constraint
extern Identifier< LockPtr<ListOfEndConflict> > INFEASABLE_PATH_END; //MDM add id Of constraint
extern Identifier< LockPtr <ListOfLoopConflict > > LOOP_OF_INFEASABLE_PATH_I  ;//MDM add id Of constraint
extern Identifier< LockPtr<ListOfEdgeConflict >  > EDGE_OF_INFEASABLE_PATH_I  ;//MDM add id Of constraint
} // otawa

#endif /* OTAWA_FLOWFACT_CONFLICT_H_ */
