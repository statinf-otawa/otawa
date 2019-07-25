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

#include <otawa/dfa/hai/WideningFixPoint.h>
#include <otawa/cfg/CFG.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/cfg/Edge.h>
#include <otawa/prop/PropList.h>
#include <elm/data/Vector.h>

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
		else
			results_out = NULL;
		for (int i = 0; i < col->count();  i++) {
			CFG *cfg = col->get(i); 
			results[i] = new typename Problem::Domain*[cfg->count()];
			if (store_out)
			  results_out[i] = new typename Problem::Domain*[cfg->count()];
			for (int j = 0; j < cfg->count(); j++){
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
			for (int j = 0; j < cfg->count(); j++){
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

	void blockInterpreted(const WideningFixPoint< WideningListener >  *fp, Block* bb, const typename Problem::Domain& in, const typename Problem::Domain& out, CFG *cur_cfg, Vector<Edge*> *callStack) const;
	
	void fixPointReached(const WideningFixPoint<WideningListener > *fp, Block*bb );
	
	template <class GC> inline void collect(const GC* gc) const;

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
void WideningListener<Problem>::blockInterpreted(const WideningFixPoint<WideningListener>  *fp, Block* bb, const typename Problem::Domain& in, const typename Problem::Domain& out, CFG *cur_cfg, Vector<Edge*> *callStack) const {

		int bbnumber = bb->index() ;
		int cfgnumber = bb->cfg()->index();
	
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
void WideningListener<Problem>::fixPointReached(const WideningFixPoint<WideningListener> *fp, Block*bb ) {
}
	

template <class Problem> template<class GC>
inline void WideningListener<Problem>::collect(const GC *gc) const {
	if(!fw)
		return;
	int totalState = 0;
	const CFGCollection *col = INVOLVED_CFGS(fw);
	for (int ci = 0; col && (ci < col->count());  ci++) {
		CFG *cfg = col->get(ci);
		for (int j = 0; j < cfg->count(); j++){
			totalState = results[ci][j]->collect(gc, totalState);
			if (results_out) {
				elm::cout << "collecting OUT_STATE for " << cfg << ", " << cfg->at(j) << io::endl;
				elm::cout << cfg << cfg->at(j) << io::endl;
			}
			// keep the state of each out-edge
		} // for each BB
	} // for each CFG
}

} } }	// otawa::dfa::hai

#endif // OTAWA_DFA_HAI_WIDENINGLISTENER_H_
