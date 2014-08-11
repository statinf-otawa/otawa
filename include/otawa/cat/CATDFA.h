/*
 *	CATDFA class interface
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	This file is part of OTAWA
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
#ifndef OTAWA_CAT_CATDFA_H_
#define OTAWA_CAT_CATDFA_H_

#include <otawa/dfa/BitSet.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ilp.h>
#include <otawa/prop/Identifier.h>
#include <otawa/hard/Cache.h>


namespace otawa {
	
class BasicBlock;
class LBlockSet;

namespace cat {

class CATDomain: public dfa::BitSet {

public:
	inline CATDomain(int size) : dfa::BitSet(size) {
	}
	void reset(void) {
		empty();
	}
	void join(CATDomain *d) {
		add(*d);
	}
	void meet(CATDomain *d) {
		mask(*d);
	}
	bool equals(CATDomain *d) {
		return(BitSet::equals(*d));
	}
};


class CATProblem {
    LBlockSet *lines;
    const hard::Cache *cach;
    WorkSpace *fw;
    int size;
    static int vars;
    
        public:

	typedef CATDomain domain_t;

	CATDomain *empty(void) {
		CATDomain *tmp = new CATDomain(size);
		tmp->reset();
		return(tmp);
	}
	
	CATDomain *gen(CFG *cfg, BasicBlock *bb);
	CATDomain *preserve(CFG *cfg, BasicBlock *bb);

	void free(CATDomain *d) {
		delete d;
	}
    
	CATProblem (LBlockSet *point, int _size, const hard::Cache *mem, WorkSpace *_fw){
		lines = point;
		size = _size;
		cach = mem;	
		fw = _fw;
	};
	
};

} }	// otawa::cat

#endif // OTAWA_CAT_CATDFA_H_
