/*
 *	$Id$
 *	Listener associated with WideningFixPoint
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

#ifndef OTAWA_DFA_HAI_WIDENINGLISTENER_H_
#define OTAWA_DFA_HAI_WIDENINGLISTENER_H_

#include <otawa/util/WideningFixPoint.h>
#include <otawa/cfg/CFG.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/cfg/Edge.h>
#include <otawa/prop/PropList.h>
#include <elm/genstruct/Vector.h>

namespace otawa { namespace dfa { namespace hai {



template <class P>
class WideningListener {

  public:
  

  	
	typedef P Problem;
	
	static Identifier<typename Problem::Domain*> BB_OUT_STATE; 

	typename Problem::Domain ***results;
	typename Problem::Domain ***results_out;
	
 WideningListener(WorkSpace *_fw, Problem& _prob, bool _store_out = false) : fw(_fw), prob(_prob), store_out(_store_out) {
		const CFGCollection *col = INVOLVED_CFGS(fw);
		results = new typename Problem::Domain**[col->count()];
		if (store_out)
		  	results_out = new typename Problem::Domain**[col->count()];
		for (int i = 0; i < col->count();  i++) {
			CFG *cfg = col->get(i); 
			results[i] = new typename Problem::Domain*[cfg->countBB()];
			if (store_out)
			  results_out[i] = new typename Problem::Domain*[cfg->countBB()];
			for (int j = 0; j < cfg->countBB(); j++){
				results[i][j] = new typename Problem::Domain(prob.bottom());
				if (store_out)
				  results_out[i][j] = new typename Problem::Domain(prob.bottom());
			}
		} 
	}
	
	~WideningListener() {
		const CFGCollection *col = INVOLVED_CFGS(fw);
		for (int i = 0; i < col->count();  i++) {
			CFG *cfg = col->get(i); 
			for (int j = 0; j < cfg->countBB(); j++){
				delete results[i][j];	
				if (store_out)
				  delete results_out[i][j];
			}
			delete [] results[i];
			if (store_out)
			  delete [] results_out[i];
		} 
		delete [] results;
		if (store_out)
		  delete [] results_out;
	}

	void blockInterpreted(const WideningFixPoint< WideningListener >  *fp, BasicBlock* bb, const typename Problem::Domain& in, const typename Problem::Domain& out, CFG *cur_cfg, elm::genstruct::Vector<Edge*> *callStack) const;
	
	void fixPointReached(const WideningFixPoint<WideningListener > *fp, BasicBlock*bb );
	
	inline Problem& getProb() {
		return(prob);
	}
			
  private:	
	WorkSpace *fw;
	Problem& prob;	
	bool store_out;
 
			
};

/**
 * PRIVATE - DO NOT USE
 */ 
template <class Problem >
Identifier<typename Problem::Domain*> WideningListener<Problem>::BB_OUT_STATE("", 0);

template <class Problem >
void WideningListener<Problem>::blockInterpreted(const WideningFixPoint<WideningListener>  *fp, BasicBlock* bb, const typename Problem::Domain& in, const typename Problem::Domain& out, CFG *cur_cfg, elm::genstruct::Vector<Edge*> *callStack) const {

		int bbnumber = bb->number() ;
		int cfgnumber = cur_cfg->number();
	
		prob.lub(*results[cfgnumber][bbnumber], in);
		
		if (BB_OUT_STATE(bb) != 0)
			prob.lub(**BB_OUT_STATE(bb), out);

		if (store_out)
		  prob.lub(*results_out[cfgnumber][bbnumber], out);
#ifdef DEBUG
		cout << "[TRACE] Block " << bbnumber << ": IN=" << in << " OUT=" << out << "\n";
#endif		
}

template <class Problem >
void WideningListener<Problem>::fixPointReached(const WideningFixPoint<WideningListener> *fp, BasicBlock*bb ) {
}
	
} } }	// otawa::dfa::hai

#endif // OTAWA_DFA_HAI_WIDENINGLISTENER_H_
