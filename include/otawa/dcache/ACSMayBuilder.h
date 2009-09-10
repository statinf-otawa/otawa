/*
 * MayACSBuilder.h
 *
 *  Created on: 14 juil. 2009
 *      Author: casse
 */

#ifndef MAYACSBUILDER_H_
#define MAYACSBUILDER_H_

#include <elm/genstruct/Vector.h>
#include <otawa/prop/Identifier.h>
#include <otawa/proc/Processor.h>
#include <otawa/util/HalfAbsInt.h>

namespace otawa {

class BasicBlock;
class BlockCollection;
namespace hard { class Cache; }

namespace dcache {

// MAYProblem class
class MAYProblem {
public:

	class Domain {
		int A, size;
	public:
		inline Domain(const int _size, const int _A): A (_A), size(_size) {
			age = new int [size];
			for (int i = 0; i < size; i++)
				age[i] = -1;
		}

		inline ~Domain() { delete [] age; }

		inline Domain(const Domain &source) : A(source.A), size(source.size) {
			age = new int [size];
			for (int i = 0; i < size; i++)
				age[i] = source.age[i];
		}

		inline Domain& operator=(const Domain &src) {
			ASSERT((A == src.A) && (size == src.size));
			for (int i = 0; i < size ; i++)
				age[i] = src.age[i];
			return(*this);
		}

		inline void glb(const Domain &dom) { ASSERT(false); }

		inline void lub(const Domain &dom) {
			ASSERT((A == dom.A) && (size == dom.size));
			for (int i = 0; i < size; i++)
				if (((age[i] > dom.age[i]) && (dom.age[i] != -1)) || (age[i] == -1))
					age[i] = dom.age[i];
		}

		inline int getSize(void) { return size; }

		inline void addDamage(const int id, const int damage) {
			ASSERT((id >= 0) && (id < size));
			if (age[id] == -1)
				return;
			age[id] += damage;
			if (age[id] >= A)
				age[id] = -1;
		}

		inline bool equals(const Domain &dom) const {
			ASSERT((A == dom.A) && (size == dom.size));
			for (int i = 0; i < size; i++)
				if (age[i] != dom.age[i])
					return false;
			return true;
		}

		inline void empty() { for (int i = 0; i < size; i++) age[i] = -1; }
		inline bool contains(const int id) { ASSERT((id < size) && (id >= 0)); return(age[id] != -1); }

		inline void inject(const int id) {
			if (contains(id))
				for (int i = 0; i < size; i++)
					if ((age[i] <= age[id]) && (age[i] != -1))
						age[i]++;
			else
				for (int i = 0; i < size; i++) {
					if (age[i] != -1)
						age[i]++;
					if (age[i] == A)
						age[i] = -1;
				}
			age[id] = 0;
		}

		inline void ageAll(void) {
			for (int i = 0; i < size; i++) {
				if (age[i] != -1)
					age[i]++;
				if (age[i] == A)
					age[i] = -1;
			}
		}

		inline void print(elm::io::Output &output) const {
			bool first = true;
			output << "[";
			for (int i = 0; i < size; i++) {
				if (age[i] != -1) {
					if (!first)
						output << ", ";
					output << i;
					output << ":";
					output << age[i];
					first = false;
				}
			}
			output << "]";
		}

		inline int getAge(int id) const { ASSERT(id < size); return(age[id]); }
		inline void setAge(const int id, const int _age) {
			ASSERT(id < size);
			ASSERT((_age < A) || (_age == -1));
			age[id] = _age;
		}
		int *age;
	};

private:
	const BlockCollection& coll;
	WorkSpace *fw;
	int line;
	const hard::Cache *cache;
	Domain bot;
	Domain ent;

public:
	Domain callstate;

	MAYProblem(const  BlockCollection& coll, WorkSpace *_fw, const hard::Cache *_cache);
	~MAYProblem(void);

	const Domain& bottom(void) const;
	const Domain& entry(void) const;
	inline void lub(Domain &a, const Domain &b) const { a.lub(b); }
	inline void assign(Domain &a, const Domain &b) const { a = b; }
	inline bool equals(const Domain &a, const Domain &b) const { return (a.equals(b)); }
	void update(Domain& out, const Domain& in, BasicBlock* bb);
	inline void enterContext(Domain &dom, BasicBlock *header, util::hai_context_t ctx) { }
	inline void leaveContext(Domain &dom, BasicBlock *header, util::hai_context_t ctx) { }
};

elm::io::Output& operator<<(elm::io::Output& output, const MAYProblem::Domain& dom);

typedef elm::genstruct::Vector<MAYProblem::Domain*> may_acs_t;


// ACSMayBuilder class
class ACSMayBuilder: public otawa::Processor {
public:
	ACSMayBuilder(void);
	virtual void processWorkSpace(otawa::WorkSpace *ws);
	virtual void configure(const PropList &props);

private:
	void processLBlockSet(otawa::WorkSpace *ws, const BlockCollection& coll, const hard::Cache *cache);
	bool unrolling;
	may_acs_t *may_entry;
};


// configuration
extern Identifier<may_acs_t *> ACS_MAY_ENTRY;

// ACS_MAY_FEATURE
extern Feature<ACSMayBuilder> ACS_MAY_FEATURE;
extern Identifier<may_acs_t *> ACS_MAY;

} } // otawa::dcache

#endif /* MAYACSBUILDER_H_ */
