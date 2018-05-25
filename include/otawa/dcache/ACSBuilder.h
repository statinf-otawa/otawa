/*
 *	dcache::ACSBuilder class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009, IRIT UPS.
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

#ifndef OTAWA_DCACHE_ACSBUILDER_H_
#define OTAWA_DCACHE_ACSBUILDER_H_

#include "features.h"
#include <otawa/dfa/hai/HalfAbsInt.h>

namespace otawa {

using namespace elm;

namespace dcache {

// Cache analysis
class MUSTProblem {
public:
	class Domain: public ACS {
	public:
		inline Domain(const int _size, const int _A): ACS(_size, _A, 0) { }
		inline Domain(const Domain &source): ACS(source) { }
		inline Domain(const ACS& source): ACS(source) { }
		inline Domain& operator=(const ACS& d) { ACS::operator=(d); return *this; }

		inline void addDamage(const int id, const int damage) {
			ASSERT((id >= 0) && (id < size));
			if (age[id] == -1)
				return;
			age[id] += damage;
			if (age[id] >= A)
				age[id] = -1;
		}

		// To obtain the age of a block a[x], given block b is associated with the writing address
		// if a[b] < A, then a[x] =
		// 0     , if x == b
		// a[x]++, if a[x] < a[b] // increment the age of other blocks which is younger than b, because b is put at the first place (age = 0)
		// a[x]  , otherwise
		inline void inject(const int id) {
			if (contains(id)) // if the block is already in the cache
				for (int i = 0; i < size; i++) {
					if ((age[i] < age[id]) && (age[i] != -1)) // only increment the age for the one whose age is younger
						age[i]++;
				}
			else // if the block is not in the cache yet, increment the age of all the other blocks
				for (int i = 0; i < size; i++) {
					if (age[i] != -1)
						age[i]++;
					if (age[i] == A)
						age[i] = -1;
				}
			age[id] = 0;
		}

		inline void injectWriteThrough(const int id) {
			if(id == -1) {
				for (int i = 0; i < size; i++) {
					if (age[i] != -1 && age[i] < (A-1)) // only increment the age for the one whose age is younger
						age[i]++;
				}
				ASSERT(0);
			}
			else if (contains(id)) { // if the block is already in the cache,
				for (int i = 0; i < size; i++) {
					if ((age[i] < age[id]) && (age[i] != -1)) // only increment the age for the one whose age is younger
						age[i]++;
				}
				age[id] = 0;
			}
		}

		inline void ageAll() {
			for (int i = 0; i < size; i++) {
				if (age[i] != -1)
					age[i]++;
				if (age[i] == A)
					age[i] = -1;
			}
		}

		inline void join(Domain& b) {
			ASSERT(size == b.size);
			for (int i = 0; i < size; i++)
			if (((age[i] < b[i]) && (age[i] != -1))|| (b.age[i] == -1))
				age[i] = b[i];
		}

		inline bool operator==(const Domain& b) {
			ASSERT(size == b.size);
			for (int i = 0; i < size; i++)
				if (age[i] != b[i])
					return false;
			return true;
		}

	};

	Domain callstate;

	MUSTProblem(int _size, int _set, WorkSpace *_fw, const hard::Cache *_cache, int _A);

	const Domain& bottom(void) const;
	const Domain& top(void) const;
	inline const Domain& entry(void) const { return _top; }
	inline void lub(Domain &a, const Domain &b) const {
		for (int i = 0; i < size; i++)
			if (((a[i] < b[i]) && (a[i] != -1))|| (b[i] == -1))
				a[i] = b[i];
	}
	inline void assign(Domain &a, const Domain &b) const { a = b; }
	inline void assign(Domain &a, const ACS &b) const { a = b; }
	inline bool equals(const Domain &a, const Domain &b) const { return (a.equals(b)); }
	void update(Domain& out, const Domain& in, otawa::Block* bb);
	void update(Domain& s, const BlockAccess& access);
	void purge(Domain& out, const BlockAccess& acc);
	inline void enterContext(Domain &dom, otawa::Block *header, dfa::hai::hai_context_t ctx) { }
	inline void leaveContext(Domain &dom, otawa::Block *header, dfa::hai::hai_context_t ctx) { }

private:
	WorkSpace *fw;
	int set;
	const hard::Cache *cache;
	Domain bot;
	Domain _top;
	int size;
};

elm::io::Output& operator<<(elm::io::Output& output, const MUSTProblem::Domain& dom);


// ACSBuilder processor
class ACSBuilder : public otawa::Processor {
public:
	static p::declare reg;
	ACSBuilder(p::declare& r = reg);
	virtual void processWorkSpace(otawa::WorkSpace*);
	virtual void configure(const PropList &props);

private:
	void processLBlockSet(otawa::WorkSpace*, const BlockCollection& coll, const hard::Cache *);
	data_fmlevel_t level;
	bool unrolling;
	Vector<ACS *> *must_entry;
};

} }	// otawa::dcache

#endif /* OTAWA_DCACHE_ACSBUILDER_H_ */
