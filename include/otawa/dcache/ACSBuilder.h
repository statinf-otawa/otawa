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
#include <otawa/util/HalfAbsInt.h>

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

		/*inline void glb(const Domain &dom) {
			ASSERT((A == dom.A) && (size == dom.size));
			for (int i = 0; i < size; i++)
				if (((age[i] > dom.age[i]) && (dom.age[i] != -1)) || (age[i] == -1))
					age[i] = dom.age[i];
		}*/

		/*inline void lub(const Domain &dom) {
			ASSERT((A == dom.A) && (size == dom.size));
			for (int i = 0; i < size; i++)
				if (((age[i] < dom.age[i]) && (age[i] != -1))|| (dom.age[i] == -1))
					age[i] = dom.age[i];
		}*/

		inline void addDamage(const int id, const int damage) {
			ASSERT((id >= 0) && (id < size));
			if (age[id] == -1)
				return;
			age[id] += damage;
			if (age[id] >= A)
				age[id] = -1;
		}

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

	};

	Domain callstate;

	MUSTProblem(int _size, int _set, WorkSpace *_fw, const hard::Cache *_cache, int _A);

	const Domain& bottom(void) const;
	const Domain& entry(void) const;
	inline void lub(Domain &a, const Domain &b) const {
		for (int i = 0; i < size; i++)
			if (((a[i] < b[i]) && (a[i] != -1))|| (b[i] == -1))
				a[i] = b[i];
	}
	inline void assign(Domain &a, const Domain &b) const { a = b; }
	inline void assign(Domain &a, const ACS &b) const { a = b; }
	inline bool equals(const Domain &a, const Domain &b) const { return (a.equals(b)); }
	void update(Domain& out, const Domain& in, BasicBlock* bb);
	void update(Domain& s, const BlockAccess& access);
	void purge(Domain& out, const BlockAccess& acc);
	inline void enterContext(Domain &dom, BasicBlock *header, util::hai_context_t ctx) { }
	inline void leaveContext(Domain &dom, BasicBlock *header, util::hai_context_t ctx) { }

private:
	WorkSpace *fw;
	int set;
	const hard::Cache *cache;
	Domain bot;
	Domain ent;
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
	genstruct::Vector<ACS *> *must_entry;
};

} }	// otawa::dcache

#endif /* OTAWA_DCACHE_ACSBUILDER_H_ */
