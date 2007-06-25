#ifndef UTIL_UNROLLINGLISTENER_H_
#define UTIL_UNROLLINGLISTENER_H_

#include <otawa/util/FirstUnrollingFixPoint.h>
#include <elm/genstruct/Vector.h>

namespace otawa {

template <class P>
class UnrollingListener {

  public:
	typedef P Problem;

	typename Problem::Domain ***results;
	
	UnrollingListener(WorkSpace *_fw, Problem& _prob) : fw(_fw), prob(_prob) {
		CFGCollection *col = INVOLVED_CFGS(fw);
		results = new typename Problem::Domain**[col->count()];
		
		for (int i = 0; i < col->count();  i++) {
			CFG *cfg = col->get(i); 
			results[i] = new typename Problem::Domain*[cfg->countBB()];
			for (int j = 0; j < cfg->countBB(); j++)
				results[i][j] = new typename Problem::Domain(prob.bottom());
		} 
	}
	
	~UnrollingListener() {
		CFGCollection *col = INVOLVED_CFGS(fw);
		for (int i = 0; i < col->count();  i++) {
			CFG *cfg = col->get(i); 
			for (int j = 0; j < cfg->countBB(); j++)
				delete results[i][j];	
			delete [] results[i];
		} 
		delete [] results;			
	}

	void blockInterpreted(const FirstUnrollingFixPoint< UnrollingListener >  *fp, BasicBlock* bb, const typename Problem::Domain& in, const typename Problem::Domain& out, CFG *cur_cfg, elm::genstruct::Vector<Edge*> *callStack) const;
	
	void fixPointReached(const FirstUnrollingFixPoint<UnrollingListener > *fp, BasicBlock*bb );
	
	inline Problem& getProb() {
		return(prob);
	}
			
  private:	
	WorkSpace *fw;
	Problem& prob;	
	
 
		
	
	

	
};

template <class Problem >
void UnrollingListener<Problem>::blockInterpreted(const FirstUnrollingFixPoint<UnrollingListener>  *fp, BasicBlock* bb, const typename Problem::Domain& in, const typename Problem::Domain& out, CFG *cur_cfg, elm::genstruct::Vector<Edge*> *callStack) const {

		int bbnumber = bb->number() ;
		int cfgnumber = cur_cfg->number();
	
		prob.lub(*results[cfgnumber][bbnumber], in);


#ifdef DEBUG
		cout << "[TRACE] CFG " << cur_cfg->label() << " BB " << bbnumber << ": IN=" << in << " OUT=" << out << "\n";
		cout << "[TRACE] result: " << *results[cfgnumber][bbnumber] << "\n";
#endif		
}

template <class Problem >
void UnrollingListener<Problem>::fixPointReached(const FirstUnrollingFixPoint<UnrollingListener> *fp, BasicBlock*bb ) {
}
	
}

#endif 
