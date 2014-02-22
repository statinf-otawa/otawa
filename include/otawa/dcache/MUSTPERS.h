/*
 *	dcache::MUSTPERS class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2013, IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */

#ifndef OTAWA_DCACHE_MUSTPERS_H_
#define OTAWA_DCACHE_MUSTPERS_H_

#include <elm/io.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/dcache/features.h>
#include <otawa/hard/Cache.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/util/HalfAbsInt.h>
#include <otawa/dcache/ACSBuilder.h>
#include <otawa/dcache/PERSProblem.h>

namespace otawa { namespace dcache {

class MUSTPERS {
public:
	class Domain {
		friend class MUSTPERS;
	public:
		inline Domain(const int _size, const int _A)
			: pers(PERSProblem::Domain(_size, _A)), must(MUSTProblem::Domain(_size, _A)) { }
		inline Domain(const Domain &source) : pers(source.pers), must(source.must) { }
		inline MUSTProblem::Domain& getMust(void) { return must; }
		inline PERSProblem::Domain& getPers(void) { return pers; }
	private:
		PERSProblem::Domain pers;
		MUSTProblem::Domain must;
	};

public:
	MUSTPERS(const BlockCollection *_lbset, WorkSpace *_fw, const hard::Cache *_cache);
	inline void setMust(Domain& d, const ACS& acs) const { mustProb.assign(d.must, acs); }
	inline void emptyPers(Domain& d) const { d.pers.empty(); }
	inline void setPers(Domain& d, const ACS& acs, const acs_stack_t& s) { d.pers.set(acs, s); }

	// abstract interpretation
	const Domain& bottom(void) const;
	const Domain& entry(void) const;
	inline void lub(Domain &a, const Domain &b) const
		{ mustProb.lub(a.must, b.must); persProb.lub(a.pers, b.pers); }
	inline void assign(Domain &a, const Domain &b) const //{ a = b; }
		{ persProb.assign(a.pers, b.pers); mustProb.assign(a.must, b.must); }
	inline bool equals(const Domain &a, const Domain &b) const //{ return (a.equals(b)); }
		{ return (persProb.equals(a.pers, b.pers) && mustProb.equals(a.must, b.must)); };
	void print(elm::io::Output &output, const Domain& d) const;

	void update(Domain& out, const Domain& in, BasicBlock* bb);
	void update(Domain& s, const BlockAccess& access);
	inline void ageAll(Domain& d) const { d.must.ageAll(); d.pers.ageAll();  }
	inline void inject(Domain& d, const int id) const { d.pers.inject(&d.must, id); d.must.inject(id); }
	
	inline void enterContext(Domain &dom, BasicBlock *header, util::hai_context_t ctx) {
		persProb.enterContext(dom.pers, header, ctx);
		mustProb.enterContext(dom.must, header, ctx);
	}

	inline void leaveContext(Domain &dom, BasicBlock *header, util::hai_context_t ctx) {
		persProb.leaveContext(dom.pers, header, ctx);
		mustProb.leaveContext(dom.must, header, ctx);
	}		

private:
	Domain bot;
	Domain ent;
	unsigned int set;
	MUSTProblem mustProb;
	PERSProblem persProb;
};

//elm::io::Output& operator<<(elm::io::Output& output, const Pair<const MUSTPERS&, const MUSTPERS::Domain>& dom);

} }	// otawa::dcache

#endif /* OTAWA_DCACHE_MUSTPROBLEM_H_*/
