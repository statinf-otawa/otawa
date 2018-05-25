/*
 *	MUSTPERS class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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

#ifndef CACHE_MUSTPERS_H_
#define CACHE_MUSTPERS_H_

#include <elm/io.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/hard/Cache.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/dfa/hai/HalfAbsInt.h>


namespace otawa {

class MUSTPERS {
public:

	class Domain {
		friend class MUSTPERS;
	public:
		inline Domain(const int _size, const int _A)
			: pers(PERSProblem::Domain(_size, _A)), must(MUSTProblem::Domain(_size, _A))
			{ }
			
			inline Domain(const Domain &source) : pers(source.pers), must(source.must) { }
			inline Domain(const MUSTProblem::Domain& m, const PERSProblem::Domain& p)
				: pers(p), must(m) { }
			
			inline Domain& operator=(const Domain &src)
				{ pers = src.pers; must = src.must; return *this; }
			inline const MUSTProblem::Domain& getMust(void) const { return must; }
			inline const PERSProblem::Domain& getPers(void) const { return pers; }
			inline void lub(const Domain &dom) { pers.lub(dom.pers); must.lub(dom.must); }
			inline int getSize(void) { return must.getSize(); }
			inline bool equals(const Domain &dom) const
				{ return (pers.equals(dom.pers) && must.equals(dom.must)); }
			inline void empty() { must.empty(); pers.empty(); }
			inline bool mustContains(const int id) { return(must.contains(id)); }
			inline bool persContains(const int id, const int index)
				{ return(pers.contains(id, index)); }
			inline bool isWiped(const int id, const int index)
				{ return(pers.isWiped(id, index)); }
			inline bool isPersistent(const int id, const int index)
				{ return(pers.isPersistent(id, index)); }
			inline void inject(const int id)
				{ pers.inject(&must, id); must.inject(id); }
			
			inline void print(elm::io::Output &output) const {
				output << "PERS=[ ";
				pers.print(output);
				output << "] MUST=[ ";
				must.print(output);
				output << "]";
			}

		private:
			PERSProblem::Domain pers;
			MUSTProblem::Domain must;
	};
	
	
public:
	
	// Constructors
	MUSTPERS(const int _size, LBlockSet *_lbset, WorkSpace *_fw, const hard::Cache *_cache, const int _A);

	// Problem methods
	const Domain& bottom(void) const;
	const Domain& top(void) const;
	const Domain& entry(void) const;
		
	inline void lub(Domain &a, const Domain &b) const { a.lub(b); }
	inline void assign(Domain &a, const Domain &b) const { a = b; }
	inline bool equals(const Domain &a, const Domain &b) const { return (a.equals(b)); }
	
	void update(Domain& out, const Domain& in, Block* bb);
	
	inline void enterContext(Domain &dom, Block *header, dfa::hai::hai_context_t ctx) {
		persProb.enterContext(dom.pers, header, ctx);
		mustProb.enterContext(dom.must, header, ctx);
		
	}

	inline void leaveContext(Domain &dom, Block *header, dfa::hai::hai_context_t ctx) {
		persProb.leaveContext(dom.pers, header, ctx);
		mustProb.leaveContext(dom.must, header, ctx);

	}		

private:
	MUSTProblem mustProb;
	PERSProblem persProb;
	Domain bot;
	Domain _top;
	Domain ent;
	unsigned int line;
};

elm::io::Output& operator<<(elm::io::Output& output, const MUSTPERS::Domain& dom);

}	// elm

#endif /*CACHE_MUSTPROBLEM_H_*/
