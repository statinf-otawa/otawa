/*
 *	Listener associated with FirstUnrollingFixPoint
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

#ifndef OTAWA_DFA_HAI_UNROLLINGLISTENER_H_
#define OTAWA_DFA_HAI_UNROLLINGLISTENER_H_

#include "FirstUnrollingFixPoint.h"
#include <otawa/cfg/CFG.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/cfg/Edge.h>
#include <otawa/prop/PropList.h>
#include <otawa/prop/Identifier.h>
#include <elm/data/Vector.h>

namespace otawa { namespace dfa { namespace hai {


template <class P>
class UnrollingListener {

  public:
	typedef P Problem;
	 
	static Identifier<typename Problem::Domain*> BB_OUT_STATE;
	
	typename Problem::Domain ***results;
	
	UnrollingListener(WorkSpace *_fw, Problem& _prob) 
	: fw(_fw), prob(_prob) {
		const CFGCollection *col = INVOLVED_CFGS(fw);
		results = new typename Problem::Domain**[col->count()];
		
		for (int i = 0; i < col->count();  i++) {
			CFG *cfg = col->get(i); 
			results[i] = new typename Problem::Domain*[cfg->count()];
			for (int j = 0; j < cfg->count(); j++)
				results[i][j] = new typename Problem::Domain(prob.bottom());
		} 
	}
	
	~UnrollingListener() {
		const CFGCollection *col = INVOLVED_CFGS(fw);
		for (int i = 0; i < col->count();  i++) {
			CFG *cfg = col->get(i); 
			for (int j = 0; j < cfg->count(); j++)
				delete results[i][j];	
			delete [] results[i];
		} 
		delete [] results;			
	}

	void blockInterpreted(const FirstUnrollingFixPoint< UnrollingListener >  *fp, Block* bb, const typename Problem::Domain& in, const typename Problem::Domain& out, CFG *cur_cfg, Vector<Edge*> *callStack) const;
	
	void fixPointReached(const FirstUnrollingFixPoint<UnrollingListener > *fp, Block*bb );
	
	inline Problem& getProb() {
		return(prob);
	}
			
  private:	
	WorkSpace *fw;
	Problem& prob;	
	
 
		
	
	

	
};

template <class Problem > 
Identifier<typename Problem::Domain*> UnrollingListener<Problem>::BB_OUT_STATE("", 0);
 
template <class Problem >
void UnrollingListener<Problem>::blockInterpreted(const FirstUnrollingFixPoint<UnrollingListener>  *fp, Block* bb, const typename Problem::Domain& in, const typename Problem::Domain& out, CFG *cur_cfg, Vector<Edge*> *callStack) const {

		int bbnumber = bb->index() ;
		int cfgnumber = cur_cfg->index();
	
		prob.lub(*results[cfgnumber][bbnumber], in);

		if (BB_OUT_STATE(bb) != 0)
			prob.lub(**BB_OUT_STATE(bb), out);
			
#ifdef DEBUG
		cout << "[TRACE] CFG " << cur_cfg->label() << " BB " << bbnumber << ": IN=" << in << " OUT=" << out << "\n";
		cout << "[TRACE] result: " << *results[cfgnumber][bbnumber] << "\n";
#endif		
}

template <class Problem >
void UnrollingListener<Problem>::fixPointReached(const FirstUnrollingFixPoint<UnrollingListener> *fp, Block*bb ) {
}
	
} } }	// otawa::dfa::hai

#endif 	// OTAWA_DFA_HAI
