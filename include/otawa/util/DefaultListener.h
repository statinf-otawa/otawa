/*
 *	$Id$
 *	Listener associated with DefaultFixPoint
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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

#ifndef UTIL_DEFAULTLISTENER_H_
#define UTIL_DEFAULTLISTENER_H_

#include <otawa/util/DefaultFixPoint.h>
#include <otawa/cfg/CFG.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/cfg/Edge.h>
#include <otawa/prop/PropList.h>
#include <elm/genstruct/Vector.h>

namespace otawa {



template <class P>
class DefaultListener {

  public:
  

  	
	typedef P Problem;
	
	static Identifier<typename Problem::Domain*> BB_OUT_STATE; 

	typename Problem::Domain ***results;
	
	DefaultListener(WorkSpace *_fw, Problem& _prob) : fw(_fw), prob(_prob) {
		CFGCollection *col = INVOLVED_CFGS(fw);
		results = new typename Problem::Domain**[col->count()];
		
		for (int i = 0; i < col->count();  i++) {
			CFG *cfg = col->get(i); 
			results[i] = new typename Problem::Domain*[cfg->countBB()];
			for (int j = 0; j < cfg->countBB(); j++)
				results[i][j] = new typename Problem::Domain(prob.bottom());
		} 
	}
	
	~DefaultListener() {
		CFGCollection *col = INVOLVED_CFGS(fw);
		for (int i = 0; i < col->count();  i++) {
			CFG *cfg = col->get(i); 
			for (int j = 0; j < cfg->countBB(); j++)
				delete results[i][j];	
			delete [] results[i];
		} 
		delete [] results;			
	}

	void blockInterpreted(const DefaultFixPoint< DefaultListener >  *fp, BasicBlock* bb, const typename Problem::Domain& in, const typename Problem::Domain& out, CFG *cur_cfg, elm::genstruct::Vector<Edge*> *callStack) const;
	
	void fixPointReached(const DefaultFixPoint<DefaultListener > *fp, BasicBlock*bb );
	
	inline Problem& getProb() {
		return(prob);
	}
			
  private:	
	WorkSpace *fw;
	Problem& prob;	
	
 
		
	
	

	
};

/**
 * PRIVATE - DO NOT USE
 */ 
template <class Problem >
Identifier<typename Problem::Domain*> DefaultListener<Problem>::BB_OUT_STATE("", NULL);

template <class Problem >
void DefaultListener<Problem>::blockInterpreted(const DefaultFixPoint<DefaultListener>  *fp, BasicBlock* bb, const typename Problem::Domain& in, const typename Problem::Domain& out, CFG *cur_cfg, elm::genstruct::Vector<Edge*> *callStack) const {

		int bbnumber = bb->number() ;
		int cfgnumber = cur_cfg->number();
	
		prob.lub(*results[cfgnumber][bbnumber], in);
		
		if (BB_OUT_STATE(bb) != NULL)
			prob.lub(*BB_OUT_STATE(bb), out);
#ifdef DEBUG
		cout << "[TRACE] Block " << bbnumber << ": IN=" << in << " OUT=" << out << "\n";
#endif		
}

template <class Problem >
void DefaultListener<Problem>::fixPointReached(const DefaultFixPoint<DefaultListener> *fp, BasicBlock*bb ) {
}
	
}

#endif 
