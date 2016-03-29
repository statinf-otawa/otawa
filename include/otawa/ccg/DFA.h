/*
 *	CCGDFA class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006, IRIT UPS.
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
#ifndef OTAWA_CCG_CCGDFA_H
#define OTAWA_CCG_CCGDFA_H

#include <otawa/dfa/BitSet.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ilp.h>
#include <otawa/prop/Identifier.h>
#include <otawa/hard/Cache.h>
#include <otawa/cfg.h>

namespace otawa {

// Extern classes	
class BasicBlock;
class LBlockSet;

namespace ccg {

class Domain: public dfa::BitSet {
public:
	inline Domain(int size) : dfa::BitSet(size) { }
	inline void reset(void) 		{ empty(); }
	inline void join(Domain *d) 	{ add(*d); }
	inline void meet(Domain *d) 	{ mask(*d); }
	inline bool equals(Domain *d)	{ return(dfa::BitSet::equals(*d)); }
};

// Problem Class
class Problem {
public:
	typedef Domain domain_t;

	inline Problem (LBlockSet *_ccggraph, int _size , const hard::Cache *_cach, WorkSpace *_fw)
		{ ccggraph = _ccggraph; cach = _cach; size = _size; fw = _fw; }

	Domain *empty(void) {
		Domain *tmp = new Domain(size);
		tmp->reset();
		return(tmp);
	}
	
	Domain *gen(CFG *cfg, Block *bb);
	Domain *preserve(CFG *cfg, Block *bb);

	inline void free(Domain *d) { delete d; }

private:
    LBlockSet *ccggraph;
    const hard::Cache *cach;
    static int vars;
    int size;
    WorkSpace *fw;
};

} }	// otawa::ccg

#endif	// OTAWA_CCG_CCGDFA_H
