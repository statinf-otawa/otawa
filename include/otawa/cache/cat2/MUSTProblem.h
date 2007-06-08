#ifndef CACHE_MUSTPROBLEM_H_
#define CACHE_MUSTPROBLEM_H_

#include <otawa/dfa/BitSet.h>



namespace otawa {


class MUSTProblem {
	
	// Types
	public:

	class Domain {
			int A, size;
						
			public:		
					/**
			 * @class Result
			 *
			 * Class to hold the results (for each basic block) of the abstract interpretation.
			 */
			
		
			inline Domain(const int _size, const int _A)
			: A (_A), size(_size)
			{
				age = new int [size];
				for (int i = 0; i < size; i++)
					age[i] = 0;
			}
			
			inline ~Domain() {
				delete [] age;
			}
			
			inline Domain(const Domain &source) : A(source.A), size(source.size) {
				age = new int [size];
				for (int i = 0; i < size; i++)
					age[i] = source.age[i];

			} 
			
			inline Domain& operator=(const Domain &src) {
				assert((A == src.A) && (size == src.size));
				for (int i = 0; i < size ; i++)
					age[i] = src.age[i];
				return(*this);
				
			}
			 
			inline void lub(const Domain &dom) {
				assert((A == dom.A) && (size == dom.size));

				for (int i = 0; i < size; i++) {
					if (((age[i] < dom.age[i]) && (age[i] != -1))|| (dom.age[i] == -1))
						age[i] = dom.age[i];
				}
			}
			
			inline int getSize(void) {
				return size;
			}
			
			inline bool equals(const Domain &dom) const {
				assert((A == dom.A) && (size == dom.size));
				for (int i = 0; i < size; i++)
					if (age[i] != dom.age[i])
						return false;
				return true;
			}
			
			inline void empty() {
				for (int i = 0; i < size; i++)
					age[i] = -1;
				
			}
			
			inline bool contains(const int id) {
				assert((id < size) && (id >= 0));
				return(age[id] != -1);				
			}
			
			
			inline void inject(const int id) {
				if (contains(id)) {
					for (int i = 0; i < size; i++) {
						if ((age[i] < age[id]) && (age[i] != -1))
							age[i]++;						
					}
					age[id] = 0;
				} else {
					for (int i = 0; i < size; i++) {
						if (age[i] != -1) 
							age[i]++;
						if (age[i] == A)
							age[i] = -1;
					}
				}
				age[id] = 0;				
			}
			
			inline void print(elm::io::Output &output) const {
				bool first = true;
				output << "[";
				for (int i = 0; i < size; i++) {
					if (age[i] != -1) {
						if (!first) {
							output << ", ";
						}
						// output << i << ":" << age[i];
						output << i;
						output << ":";
						output << age[i];
						
						first = false;
					}
				}
				output << "]";
				
			}
		
			/*
			 * For each cache block belonging to the set: 
			 * age[block] represents its age, from 0 (newest) to A-1 (oldest).
			 * The value -1 means that the block is not in the set.
			 */  
			int *age;
	};
	class UnrolledResult {
			
		public:	
		/*
		 * For each loop containing the BasicBlock, firstIter has a corresponding element. 
		 * This element is the LUB of the IN sets of the BasicBlock, for each first iteration of the 
		 * corresponding loop.
		 */
		Vector <Domain*> firstIter;
		/*
		 * For each loop containing the BasicBlock, firstIter has a corresponding element. 
		 * This element is the LUB of the IN sets of the BasicBlock, for each other (not first) 
		 * iteration of the corresponding loop.
		 */
		Vector <Domain*> otherIter;
		
		inline UnrolledResult(const Domain& bottom, int numLoops) {
			int i;
			for (i = 0; i < numLoops; i++) {
				firstIter.add(new Domain(bottom));
				otherIter.add(new Domain(bottom));
			}
		}
		
		inline UnrolledResult(const UnrolledResult &source) {
			int i;
			for (i = 0; i < source.firstIter.length(); i++) {
				firstIter.add(new Domain(*source.firstIter[i]));
				otherIter.add(new Domain(*source.otherIter[i]));			
			}		
		}
		
		inline UnrolledResult& operator=(const UnrolledResult &source) {
			int i;
			for (i = 0; i < source.firstIter.length(); i++) {
				*firstIter[i] = *source.firstIter[i];
				*otherIter[i] = *source.otherIter[i];
			}	
			return(*this);			
		}
		
		inline ~UnrolledResult() {
			for (Vector<Domain*>::Iterator iter(firstIter); iter; iter++)
				delete (*iter);
			for (Vector<Domain*>::Iterator iter(otherIter); iter; iter++)
				delete (*iter);
		}
	};	
	
	private:
	
	// Fields
	LBlockSet *lbset;
	WorkSpace *fw;
	const int line;
	const hard::Cache *cache;
	Domain bot;
	Domain ent;
	bool unrolling;
	
	public:
	Domain callstate;

	// Public fields
	
	// Constructors
	MUSTProblem(const int _size, LBlockSet *_lbset, WorkSpace *_fw, const hard::Cache *_cache, const int _A, bool _unrolling = false);
	
	// Destructors
	~MUSTProblem();
	
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

	}

	inline void leaveContext(Domain &dom, BasicBlock *header) {

	}		

};

elm::io::Output& operator<<(elm::io::Output& output, const MUSTProblem::Domain& dom);

}

#endif /*CACHE_MUSTPROBLEM_H_*/
