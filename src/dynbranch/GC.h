#ifndef OTAWA_DYNBRANCH_GC_H
#define OTAWA_DYNBRANCH_GC_H
#include <elm/alloc/GroupedGC.h>
#include <otawa/dfa/hai/WideningListener.h>
#include <elm/data/Vector.h>

extern unsigned long processedSemInstCount;
extern unsigned int gcIndex;


namespace otawa { namespace dynbranch {
//#define NO_GC

class MyGC: public elm::GroupedGC {
public:
	dfa::hai::WideningListener<GlobalAnalysisProblem> *listener;
	dfa::hai::WideningFixPoint<dfa::hai::WideningListener<dynbranch::GlobalAnalysisProblem> > *fixPoint;
	dfa::hai::HalfAbsInt<dfa::hai::WideningFixPoint<dfa::hai::WideningListener<dynbranch::GlobalAnalysisProblem> > > *ai;

	MyGC(WorkSpace* _ws): elm::GroupedGC(50*1024*1024), listener(0), fixPoint(0), ai(0), total(0), _tempRegs(0), fs(0), ws(_ws) { } // 50MB per chuck
	//MyGC(WorkSpace* _ws): elm::GroupedGC(2*1024), listener(0), fixPoint(0), ai(0), total(0), ws(_ws), fs(0), _tempRegs(0) { }

	void add(const FastStateWrapper* d) {
		vd.add(d);
	}

	void addPV(const PotentialValue* pv) {
		_pvs.add(pv);
	}

	void clearPV() {
		_pvs.clear();
	}

	void setListener(dfa::hai::WideningListener<dynbranch::GlobalAnalysisProblem>& l) { listener = &l; }
	void setFixPoint(dfa::hai::WideningFixPoint<dfa::hai::WideningListener<dynbranch::GlobalAnalysisProblem> >& f) { fixPoint = &f; }
	void setAbsInt(dfa::hai::HalfAbsInt<dfa::hai::WideningFixPoint<dfa::hai::WideningListener<dynbranch::GlobalAnalysisProblem> > >& a) { ai = &a; }
	void setFastState(dfa::FastState<PotentialValue, MyGC>* _fs) { fs = _fs; }
	void setTempRegs(Vector<PotentialValue>* vdp) { _tempRegs = vdp; }

	virtual void *allocate(t::size size) {
		total = total + size;
#ifdef EVALUATION
		static int i = 0;
		i++;
		if((i%1024) == 0)
			elm::cout << __SOURCE_INFO__ << "MyGC allocates " << total << " Bytes" << io::endl;
#endif
		return GroupedGC::allocate(size);
	}

	// template access
	template <class T>
	inline T *allocate(int n = 1) {
		unsigned long size = n * sizeof(T);
		total = total + size;
#ifdef EVALUATION
		static int i = 0;
		i++;
		if((i%1024) == 0)
			elm::cout << __SOURCE_INFO__ << "MyGC allocates " << total << " Bytes" << io::endl;
#endif
		return static_cast<T *>(GroupedGC::allocate(size));
	}


	unsigned long total;

protected:
#ifdef NO_GC
	virtual void beginGC(void) {}
	virtual void endGC(void) {}
#endif
	virtual void collect(void) {

#ifdef NO_GC
		return;
#endif
		// collect the fundamental domains: top, bot, ent
		for(int ii = 0; ii < vd.length(); ii++) {
			vd[ii]->collect(this);
		}

		for(int ii = 0; ii < _tempRegs->length(); ii++) {
			_tempRegs->item(ii).collect(this);
		}

		for(int ii = 0; ii < _pvs.length(); ii++) {
			_pvs.item(ii)->collect(this);
		}


		if(fs)
			fs->collect(); // to collect the current using state in the function of the FastState


		if(listener)
			listener->collect(this);


		int fpe = 0;
		const CFGCollection *col = INVOLVED_CFGS(ws);
		for (int ci = 0; ci < col->count();  ci++) {
			CFG *cfg = col->get(ci);
			for (int j = 0; j < cfg->count(); j++) {
				fpe = fpe + fixPoint->collect(cfg->at(j), this);
			//	fpe2 = fpe2 + fixPoint->collect2(cfg->at(j), this);
			} // for each BB
		} // for each CFG


		// collecting input and output state in AI
		if(ai)
			ai->collect(this);


	}
private:
	Vector<const FastStateWrapper*> vd;
	Vector<PotentialValue> *_tempRegs;
	Vector<const PotentialValue*> _pvs;
	dfa::FastState<PotentialValue, MyGC>* fs;
	WorkSpace* ws;

}; // end of MyGC

}}

#endif
