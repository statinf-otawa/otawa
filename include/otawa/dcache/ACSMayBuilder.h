/*
 *	dcache::ACSMayBuilder class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2010, IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef MAYACSBUILDER_H_
#define MAYACSBUILDER_H_

#include <elm/data/Vector.h>
#include <otawa/prop/Identifier.h>
#include <otawa/proc/Processor.h>
#include <otawa/dfa/hai/HalfAbsInt.h>
#include "features.h"

namespace otawa {

class BasicBlock;
namespace hard { class Cache; }

namespace dcache {

// MAYProblem class
class MAYProblem {
public:

	class Domain: public ACS {
	public:
		inline Domain(const int _size, const int _A, int init = -1, int _any_age = 0): ACS(_size, _A, init), any_age(_any_age) { }
		inline Domain(const Domain &source) : ACS(source), any_age(source.any_age) { }
		inline Domain(const ACS& source, int _any_age = 0): ACS(source), any_age(_any_age) { }

		inline int anyAge() const { return any_age; }
		inline bool containsAny() const { return any_age < A; }

		inline Domain& operator=(const Domain &src) { ACS::operator=(src); any_age = src.any_age; return *this; }
		inline Domain& operator=(const ACS& src)  { ACS::operator=(src); any_age = 0; return *this; }

		inline void glb(const Domain &dom) { ASSERT(false); }

		inline void join(const Domain &dom) { // MAY: tries to explore the possibility of the data still stays in the cache, hence take the minimal age of the two ACS.
			ASSERT((A == dom.A) && (size == dom.size));
			for (int i = 0; i < size; i++)
				if (((age[i] > dom.age[i]) && (dom.age[i] != -1)) || (age[i] == -1))
					age[i] = dom.age[i];
			any_age = min(any_age, dom.any_age);
		}

		inline void addDamage(const int id, const int damage) {
			ASSERT((id >= 0) && (id < size));
			if (age[id] == -1)
				return;
			age[id] += damage;
			if (age[id] >= A)
				age[id] = -1;
			any_age = min(A, any_age + damage);
		}

		inline void inject(const int id) {
			if (contains(id)) {
				for (int i = 0; i < size; i++)
					if ((age[i] <= age[id]) && (age[i] != -1))
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
			any_age = min(A, any_age + 1);
		}

		inline void injectWriteThrough(const int id) {
			if (contains(id)) { // if the block is already in the cache,
				for (int i = 0; i < size; i++) {
					if ((age[i] < age[id]) && (age[i] != -1)) // only increment the age for the one whose age is younger
						age[i]++;
				}
				age[id] = 0;
			}
		}

		inline void ageAll(void) {
			for (int i = 0; i < size; i++) {
				if (age[i] != -1)
					age[i]++;
				if (age[i] == A)
					age[i] = -1;
			}
			any_age = min(A, any_age + 1);
		}

		inline void refreshAll() {
			for (int i = 0; i < size; i++)
				age[i] = 0;
		}

		inline void refreshAllWriteThrough(const int id) {
			for (int i = 0; i < size; i++)
				if(age[i] != -1)
					age[i] = 0;
		}

	private:
		int any_age;
	};

public:
	Domain callstate;

	MAYProblem(const  BlockCollection& coll, WorkSpace *_fw, const hard::Cache *_cache);
	~MAYProblem(void);

	inline const Domain& top(void) const { return _top; }
	inline const Domain& bottom(void) const { return bot; }
#ifdef OLD_WRITE_THROUGH
	inline const Domain& entry(void) const { return top(); }
#else
	inline const Domain& entry(void) const { return _entry; }
#endif
	inline void lub(Domain &a, const Domain &b) const { a.join(b); }
	inline void assign(Domain &a, const Domain &b) const { a = b; }
	inline bool equals(const Domain &a, const Domain &b) const { return (a.equals(b)); }
	void update(Domain& out, const Domain& in, otawa::Block* bb);
	void update(Domain& s, const BlockAccess& access);
	inline void enterContext(Domain &dom, otawa::Block *header, dfa::hai::hai_context_t ctx) { }
	inline void leaveContext(Domain &dom, otawa::Block *header, dfa::hai::hai_context_t ctx) { }

private:
	const BlockCollection& coll;
	WorkSpace *fw;
	int set;
	const hard::Cache *cache;
	Domain _top;
	Domain bot;
	Domain _entry;
};

elm::io::Output& operator<<(elm::io::Output& output, const MAYProblem::Domain& dom);

typedef elm::Vector<MAYProblem::Domain*> may_acs_t;


// ACSMayBuilder class
class ACSMayBuilder: public otawa::Processor {
public:
	static p::declare reg;
	ACSMayBuilder(p::declare& r = reg);
	virtual void processWorkSpace(otawa::WorkSpace *ws);
	virtual void configure(const PropList &props);

private:
	void processLBlockSet(otawa::WorkSpace *ws, const BlockCollection& coll, const hard::Cache *cache);
	bool unrolling;
	Vector<ACS *> *may_entry;
};

} } // otawa::dcache

#endif /* MAYACSBUILDER_H_ */
