#ifndef CACHE_MUSTPERS_H_
#define CACHE_MUSTPERS_H_

#include <otawa/dfa/BitSet.h>



namespace otawa {

class MUSTPERS {

	MUSTProblem mustProb;
	PERSProblem persProb;
		
	// Types
	public:	
	class Domain {
			friend class MUSTPERS;

			PERSProblem::Domain pers;
			MUSTProblem::Domain must;  			
									
			public:
			inline Domain(const int _size, const int _A)
			: pers(PERSProblem::Domain(_size, _A)), must(MUSTProblem::Domain(_size, _A)) {	
	
			}
			
			
			inline Domain(const Domain &source) : pers(source.pers), must(source.must) {
			} 
			
			inline Domain& operator=(const Domain &src) {
				pers = src.pers;
				must = src.must;
			}

			inline MUSTProblem::Domain& getMust() {
				return must;
			}	
	

		        inline PERSProblem::Domain& getPers() {
				return pers;
			}	
			
			inline void lub(const Domain &dom) {
				pers.lub(dom.pers);
				must.lub(dom.must);
			}
			
			inline int getSize(void) {
				return must.getSize();
			}
			
			inline bool equals(const Domain &dom) const {
				return (pers.equals(dom.pers) && must.equals(dom.must));
			}
			
			inline void empty() {
				must.empty();
				pers.empty();
			}
			
			inline bool mustContains(const int id) {
				return(must.contains(id));			
			}
			
			inline bool persContains(const int id, const int index) {
				return(pers.contains(id, index));			
			}	
					
			inline bool isWiped(const int id, const int index) {
				return(pers.isWiped(id, index));				
			}
			
			inline bool isPersistent(const int id, const int index) {
				return(pers.isPersistent(id, index));			
			}			
			inline void inject(const int id) {
				pers.inject(&must, id);
				must.inject(id);
			}
			
			inline void print(elm::io::Output &output) const {
				output << "PERS=[ ";
				pers.print(output);
				output << "] MUST=[ ";
				must.print(output);
				output << "]";
			}

	};
	
	
	private:
	Domain bot;
	Domain ent;
	unsigned int line;

	public:
	
	// Constructors
	MUSTPERS(const int _size, LBlockSet *_lbset, WorkSpace *_fw, const hard::Cache *_cache, const int _A, bool _unrolling = false);

	// Problem methods
	const Domain& bottom(void) const;
	const Domain& entry(void) const;
		
	inline void lub(Domain &a, const Domain &b) const {
		a.lub(b);
	}
	inline void assign(Domain &a, const Domain &b) const {
		a = b;
	}
	inline bool equals(const Domain &a, const Domain &b) const {
		return (a.equals(b));
	}
	
	void update(Domain& out, const Domain& in, BasicBlock* bb);
	
	inline void enterContext(Domain &dom, BasicBlock *header) {
		persProb.enterContext(dom.pers, header);
		mustProb.enterContext(dom.must, header);
		
	}

	inline void leaveContext(Domain &dom, BasicBlock *header) {
		persProb.leaveContext(dom.pers, header);
		mustProb.leaveContext(dom.must, header);

	}		

};

elm::io::Output& operator<<(elm::io::Output& output, const MUSTPERS::Domain& dom);

}

#endif /*CACHE_MUSTPROBLEM_H_*/
