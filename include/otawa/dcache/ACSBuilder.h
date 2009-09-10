/*
 * DataACSBuilder.h
 *
 *  Created on: 12 juil. 2009
 *      Author: casse
 */

#ifndef DATAACSBUILDER_H_
#define DATAACSBUILDER_H_

#include <otawa/proc/Feature.h>
#include <otawa/util/HalfAbsInt.h>

namespace otawa {

using namespace elm;
class BlockCollection;

namespace dcache {

// Cache analysis
class MUSTProblem {
public:
	class Domain {
		int A, size;
		int *age;
	public:
		inline Domain(const int _size, const int _A) : A (_A), size(_size) {
			age = new int [size];
			for (int i = 0; i < size; i++)
				age[i] = 0;
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

		inline void glb(const Domain &dom) {
			ASSERT((A == dom.A) && (size == dom.size));
			for (int i = 0; i < size; i++)
				if (((age[i] > dom.age[i]) && (dom.age[i] != -1)) || (age[i] == -1))
					age[i] = dom.age[i];
		}

		inline void lub(const Domain &dom) {
			ASSERT((A == dom.A) && (size == dom.size));
			for (int i = 0; i < size; i++)
				if (((age[i] < dom.age[i]) && (age[i] != -1))|| (dom.age[i] == -1))
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
				for (int i = 0; i < size; i++) {
					if ((age[i] < age[id]) && (age[i] != -1))
						age[i]++;
				}
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
					// output << i << ":" << age[i];
					output << i;
					output << ":";
					output << age[i];

					first = false;
				}
				output << "]";
			}
		}

		inline int getAge(int id) const { ASSERT(id < size); return(age[id]); }
		inline void setAge(const int id, const int _age) {
			ASSERT(id < size);
			ASSERT((_age < A) || (_age == -1));
			age[id] = _age;
		}

		inline void set(const Domain& dom) {
			ASSERT(A == dom.A);
			ASSERT(size == dom.size);
			for(int i = 0; i < size; i++)
				age[i] = dom.age[i];
		}
	};

	Domain callstate;

	MUSTProblem(int _size, int _set, WorkSpace *_fw, const hard::Cache *_cache, int _A);

	const Domain& bottom(void) const;
	const Domain& entry(void) const;
	inline void lub(Domain &a, const Domain &b) const { a.lub(b); }
	inline void assign(Domain &a, const Domain &b) const { a = b; }
	inline bool equals(const Domain &a, const Domain &b) const { return (a.equals(b)); }
	void update(Domain& out, const Domain& in, BasicBlock* bb);
	inline void enterContext(Domain &dom, BasicBlock *header, util::hai_context_t ctx) { }
	inline void leaveContext(Domain &dom, BasicBlock *header, util::hai_context_t ctx) { }

private:
	WorkSpace *fw;
	int set;
	const hard::Cache *cache;
	Domain bot;
	Domain ent;

};

elm::io::Output& operator<<(elm::io::Output& output, const MUSTProblem::Domain& dom);


// type of unrolling
typedef enum data_fmlevel_t {
		DFML_INNER = 0,
		DFML_OUTER = 1,
		DFML_MULTI = 2,
		DFML_NONE
} data_fmlevel_t;


// ACSBuilder processor
class ACSBuilder : public otawa::Processor {
public:
	ACSBuilder(void);
	virtual void processWorkSpace(otawa::WorkSpace*);
	virtual void configure(const PropList &props);

private:
	void processLBlockSet(otawa::WorkSpace*, const BlockCollection& coll, const hard::Cache *);
	data_fmlevel_t level;
	bool unrolling;
	genstruct::Vector<MUSTProblem::Domain *> *must_entry;
};

extern Feature<ACSBuilder> DCACHE_ACS_FEATURE;
extern Identifier<genstruct::Vector<MUSTProblem::Domain *> *> DCACHE_ACS_MUST;
//extern Identifier<genstruct::Vector<DataPERSProblem::Domain *> *> DCACHE_ACS_PERS;
extern Identifier<data_fmlevel_t> DATA_FIRSTMISS_LEVEL;
extern Identifier<bool> DATA_PSEUDO_UNROLLING;
extern Identifier<genstruct::Vector<MUSTProblem::Domain *> *> DCACHE_ACS_MUST_ENTRY;

} }	// otawa::dcache

#endif /* DATAACSBUILDER_H_ */
