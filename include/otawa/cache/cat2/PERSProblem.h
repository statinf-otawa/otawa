#ifndef CACHE_PERSPROBLEM_H_
#define CACHE_PERSPROBLEM_H_

#include <otawa/dfa/BitSet.h>



namespace otawa {

class PERSProblem {
	
	// Types
	public:
	class Item {
			int A, size;
			
			public:			
			inline Item(const int _size, const int _A)
			: A (_A), size(_size)
			{
				age = new int [size];
				for (int i = 0; i < size; i++)
					age[i] = -1;
			}
			
			inline ~Item() {
				delete [] age;
			}
			
			inline Item(const Item &source) : A(source.A), size(source.size) {
				age = new int [size];
				for (int i = 0; i < size; i++)
					age[i] = source.age[i];

			} 
			
			inline Item& operator=(const Item &src) {
				assert((A == src.A) && (size == src.size));
				for (int i = 0; i < size ; i++)
					age[i] = src.age[i];
				
			}
			
			inline void refresh(int id, int newage) {
				ASSERT((id >= 0) && (id < size));
				
				if ((newage != -1) && ((age[id] > newage) || (age[id] == -1)))
					age[id] = newage;
			}
			
			inline void lub(const Item &dom) {
				assert((A == dom.A) && (size == dom.size));

				for (int i = 0; i < size; i++) {					
					if (((age[i] < dom.age[i]) && (dom.age[i] != -1)) || (age[i] == -1))
						age[i] = dom.age[i];
				}
			}
			
			inline bool equals(const Item &dom) const {
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
			
			inline void inject(MUSTProblem::Domain *must, const int id) {
				if (must->contains(id)) {
					for (int i = 0; i < size; i++) {
						if ((age[i] < age[id]) && (age[i] != -1) && (age[i] != A))
							age[i]++;						
					}
					age[id] = 0;
				} else {
					for (int i = 0; i < size; i++) {
						if ((age[i] != -1) && (age[i] != A)) 
							age[i]++;
					}
				}
				age[id] = 0;				
			}
			
			inline bool isWiped(const int id) {
				return(age[id] == A);				
			}
			
			inline bool isPersistent(const int id) {
				return(contains(id) && !isWiped(id));			
			}
			inline int getAge(const int id) const {
				ASSERT((id >= 0) && (id < size));
				return age[id];
			}
			
			inline void addDamage(const int id, int damage) {
				ASSERT((id >= 0) && (id < size));
				if (age[id] == -1)
					return;
				age[id] += damage;
				if (age[id] > A)
					age[id] = A;
			}
			
			inline void print(elm::io::Output &output) const {
				bool first = true;
				output << "[";
				for (int i = 0; i < size; i++) {
					if (age[i] != -1) {
						if (!first) {
							output << ", ";
						}
						output << i << ":" << age[i];
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
	
	class Domain {
			int A, size;
			bool isBottom;
		public:

			inline Domain(const int _size, const int _A)
			: A (_A), size(_size), whole(_size, _A)
			{
				isBottom = true;
				whole.empty();
			}

			inline ~Domain() {
				for (int i = 0; i < data.length(); i++)
					delete data[i];
			}
			
			inline Domain(const Domain &source) : A(source.A), size(source.size), whole(source.whole), isBottom(source.isBottom) {
				
				for (int i = 0; i < source.data.length(); i++)
					data.add(new Item(*source.data[i]));
			} 
			
			inline Domain& operator=(const Domain &src) {
				empty(); /* XXX !!gruik!! */
				whole = src.whole;
				isBottom = src.isBottom;
				for (int i = 0; i < src.data.length(); i++)
					data.add(new Item(*src.data[i]));
			}	
			
			inline void lub(const Domain &dom) {
				/*
				 * Rules for the LUB:
				 * 1. lub(anything,bottom) == anything
				 * 2. lub(dom1,dom2) where dom1 and dom2 has the same size, do the LUB of each corresponding item. 
				 * 3. lub(dom1,dom2) where dom1 has less items than dom2: we discard  items of dom2 (starting from outer-most loop)
				 * until it has the same size as dom1, then apply rule 2.
				 */
				
				if (isBottom && !dom.isBottom) {
					for (int i = 0; i < dom.data.length(); i++)
						data.add(new Item(*dom.data[i]));
					whole = dom.whole;
					isBottom = false;
				}
				
				
				if (dom.isBottom)
					return;
				
				int length = (data.length() < dom.data.length()) ? data.length() : dom.data.length();
				
				for (int i = data.length() - 1, j = dom.data.length() - 1, k = 0; k < length; i--, j--, k++) {
					data[i]->lub(*dom.data[j]);
				}
				
			
				for (int i = 0; i < data.length() - length; i++) {
					
					data.remove(0);
				}
				
				whole.lub(dom.whole);
				
			}


			inline void lub(const Item &item) {
				/* Special LUB: do the lub of each Item in current domain with
				 * item passed as parameter. Used for the partial analysis
				 */
				 for (int i = 0; i < data.length(); i++)
				 	data[i]->lub(item);
				 
				 whole.lub(item);
				 
			}

			
			inline bool equals(const Domain &dom) const {
				ASSERT(!isBottom && !dom.isBottom);			
				for (int i = 0; i < dom.data.length(); i++) {
					if (!data[i]->equals(*dom.data[i]))
						return false;
				}
				return (whole.equals(dom.whole) && (isBottom == dom.isBottom));
			}
			
			inline void empty() {
				for (int i = 0; i < data.length(); i++)
					delete data[i];
				data.clear();
				whole.empty();
				isBottom = false;		
			}
			
			inline void setToBottom() {
				for (int i = 0; i < data.length(); i++)
					delete data[i];
				data.clear();
				whole.empty();
				isBottom = true;		
			}
			
			inline Item &getWhole() {
				return whole;
			}
			
			inline bool contains(const int id, const int index) {
				ASSERT(!isBottom);
				return(data[index]->contains(id));
			}
			
			
			inline void inject(MUSTProblem::Domain *must, const int id) {
				ASSERT(!isBottom);
				for (int i = 0; i < data.length(); i++)
					data[i]->inject(must, id);		
				whole.inject(must, id);	
			}
			
			inline bool isWiped(const int id, const int index) {
				ASSERT(!isBottom);
				return(data[index]->isWiped(id));
				
			}
			
			inline int getAge(const int id, const int index) const {
				ASSERT(!isBottom);
				return(data[index]->getAge(id));
			}
			
			inline bool isPersistent(const int id, const int index) {
				ASSERT(!isBottom);
				return(data[index]->isPersistent(id));
		
			}
			inline void addDamage(const int id, const int index, int damage) {
				ASSERT(!isBottom);
				data[index]->addDamage(id, damage);
			}
			
			inline void addDamage(const int id, int damage) {
				ASSERT(!isBottom);
				for (int n = 0; n < data.length(); n++)
					data[n]->addDamage(id, damage);
				whole.addDamage(id, damage);
			}
			
			inline void refresh(const int id, const int index, int newage) {
				ASSERT(!isBottom);
				data[index]->refresh(id, newage);
			}
			
			inline void refresh(const int id, int newage) {
				ASSERT(!isBottom);
				for (int n = 0; n < data.length(); n++)
					data[n]->refresh(id, newage);
				whole.refresh(id, newage);
			}
			
			inline void print(elm::io::Output &output) const {
				bool first = true;
				if (isBottom) {
					output << "BOTTOM";
					return;
				}
				output << "(W=";
				whole.print(output); 
				output << ", ";
				for (int i = 0; i < data.length(); i++) {
					if (!first)
						output << "|";
					data[i]->print(output);
					first = false;
				}
				output << ")";
			}
			
			inline void enterContext() {
				ASSERT(!isBottom);
				Item item(size, A);
				item.empty();
				data.push(new Item(item));
				
			}
			
			inline void leaveContext() {
				ASSERT(!isBottom);
				Item *ptr = data.pop();
				delete ptr;
			}
			
			inline int length() {
				ASSERT(!isBottom);
				return data.length();
			}
			
			inline Item& getItem(const int idx) const {
				ASSERT(!isBottom);
				return(*data.get(idx));
			}
			
					
		private:
		
			Item whole;
			genstruct::Vector<Item*> data;

	};

	
	Domain callstate;
	
	private:
	
	// Fields
	LBlockSet *lbset;
	CFG *cfg;
	WorkSpace *fw;
	const hard::Cache *cache;
	Domain bot;
	Domain ent;
	const int line;

	
	public:
	// Public fields
	
	// Constructors
	PERSProblem(const int _size, LBlockSet *_lbset, WorkSpace *_fw, const hard::Cache *_cache, const int _A);
	
	// Destructors
	~PERSProblem();
	
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
	
	inline void enterContext(Domain& dom, BasicBlock *header, util::hai_context_t ctx) {
#ifndef PERFTEST
		if (ctx == util::CTX_LOOP)
			dom.enterContext(); 
#endif
	}

	inline void leaveContext(Domain& dom, BasicBlock *header, util::hai_context_t ctx) {
#ifndef PERFTEST
		if (ctx == util::CTX_LOOP)
			dom.leaveContext();
#endif		
	}	
};

elm::io::Output& operator<<(elm::io::Output& output, const PERSProblem::Domain& dom);

}

#endif /* CACHE_PERSPROBLEM_H_*/
