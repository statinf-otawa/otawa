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
		public:

			inline Domain(const int _size, const int _A)
			: A (_A), size(_size)
			{

			}

			inline ~Domain() {
				for (int i = 0; i < data.length(); i++)
					delete data[i];
			}
			
			inline Domain(const Domain &source) : A(source.A), size(source.size) {
				for (int i = 0; i < source.data.length(); i++)
					data.add(new Item(*source.data[i]));
			} 
			
			inline Domain& operator=(const Domain &src) {
				empty(); /* XXX !!gruik!! */
				for (int i = 0; i < src.data.length(); i++)
					data.add(new Item(*src.data[i]));
			}	

			inline void lub(const Domain &dom) {
				
				/* X lub bottom == X */
				if (dom.data.length() == 0)
					return;
				if (data.length() == 0) {
					for (int i = 0; i < dom.data.length(); i++)
						data.add(new Item(*dom.data[i]));
					return;
				}
				
				ASSERT(dom.data.length() == data.length());
				/* otherwise, LUB all the items */
				for (int i = 0; i < dom.data.length(); i++)
					data[i]->lub(*dom.data[i]);
			}
			
			inline bool equals(const Domain &dom) const {				
				for (int i = 0; i < dom.data.length(); i++) {
					if (!data[i]->equals(*dom.data[i]))
						return false;
				}
				return true;
			}
			
			inline void empty() {
				for (int i = 0; i < data.length(); i++)
					delete data[i];
				data.clear();				
			}
			
			inline bool contains(const int id, const int index) {
				return(data[index]->contains(id));
			}
			
			
			inline void inject(MUSTProblem::Domain *must, const int id) {
				for (int i = 0; i < data.length(); i++)
					data[i]->inject(must, id);			
			}
			
			inline bool isWiped(const int id, const int index) {
				return(data[index]->isWiped(id));
				
			}
			
			inline int getAge(const int id, const int index) const {
				return(data[index]->getAge(id));
			}
			
			inline bool isPersistent(const int id, const int index) {
				return(data[index]->isPersistent(id));
		
			}
			inline void addDamage(const int id, const int index, int damage) {
				data[index]->addDamage(id, damage);
			}
			
			inline void addDamage(const int id, int damage) {
				for (int n = 0; n < data.length(); n++)
					data[n]->addDamage(id, damage);
			}
			
			inline void refresh(const int id, const int index, int newage) {
				data[index]->refresh(id, newage);
			}
			
			inline void refresh(const int id, int newage) {
				for (int n = 0; n < data.length(); n++)
					data[n]->refresh(id, newage);
			}
			
			inline void print(elm::io::Output &output) const {
				bool first = true;
				output << "(";
				for (int i = 0; i < data.length(); i++) {
					if (!first)
						output << "|";
					data[i]->print(output);
					first = false;
				}
				output << ")";
			}
			
			inline void enterContext() {
				Item item(size, A);
				item.empty();
				data.push(new Item(item));
				
			}
			
			inline void leaveContext() {
				Item *ptr = data.pop();
				delete ptr;
			}
			inline int length() {
				return data.length();
			}
			
			inline Item& getItem(const int idx) const {
				return(*data.get(idx));
			}
			
					
		private:
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
