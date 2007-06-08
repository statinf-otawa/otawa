#ifndef UTIL_DEFAULTLISTENER_H_
#define UTIL_DEFAULTLISTENER_H_

#include <otawa/util/DefaultFixPoint.h>

namespace otawa {

template <class P>
class DefaultListener {

  public:
	typedef P Problem;

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

	void blockInterpreted(const DefaultFixPoint< DefaultListener >  *fp, BasicBlock* bb, const typename Problem::Domain& in, const typename Problem::Domain& out, CFG *cur_cfg) const;
	
	void fixPointReached(const DefaultFixPoint<DefaultListener > *fp, BasicBlock*bb );
	
	inline Problem& getProb() {
		return(prob);
	}
			
  private:	
	WorkSpace *fw;
	Problem& prob;	
	
 
		
	
	

	
};

template <class Problem >
void DefaultListener<Problem>::blockInterpreted(const DefaultFixPoint<DefaultListener>  *fp, BasicBlock* bb, const typename Problem::Domain& in, const typename Problem::Domain& out, CFG *cur_cfg) const {

		int bbnumber = bb->number() ;
		int cfgnumber = cur_cfg->number();
	
		prob.lub(*results[cfgnumber][bbnumber], in);
#ifdef DEBUG
		cout << "[TRACE] Block " << bbnumber << ": IN=" << in << " OUT=" << out << "\n";
#endif		
}

template <class Problem >
void DefaultListener<Problem>::fixPointReached(const DefaultFixPoint<DefaultListener> *fp, BasicBlock*bb ) {
}
	
}

#endif 
