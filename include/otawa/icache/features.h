/*
 *	icache features
 *	Copyright (c) 2016, IRIT UPS.
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
#ifndef OTAWA_ICACHE_FEATURES_H_
#define OTAWA_ICACHE_FEATURES_H_

#include <otawa/base.h>
#include <otawa/prop/PropList.h>
#include <otawa/proc/Feature.h>

namespace otawa {

class Inst;

namespace icache {

typedef enum kind_t {
	NONE = 0,
	FETCH = 1,
	PREFETCH = 2
} kind_t;

class Access: public PropList {
public:
	inline Access(void): _kind(NONE), _inst(0) { }
	inline Access(kind_t kind, Inst *inst, Address addr = Address::null)
		: _kind(kind), _inst(inst), _addr(addr) { }

	inline kind_t kind(void) const { return _kind; }
	inline Inst *instruction(void) const { return _inst; }
	inline Address address(void) const { return _addr; }

private:
	kind_t _kind;
	Inst *_inst;
	Address _addr;
};

io::Output& operator<<(io::Output& out, const Access& acc);

extern p::feature ACCESSES_FEATURE;
extern p::feature BLOCK_ACCESSES_FEATURE;
extern p::feature EDGE_ACCESSES_FEATURE;
extern p::id<Bag<Access> > ACCESSES;

} } // otawa::icache

#endif /* INCLUDE_OTAWA_ICACHE_FEATURES_H_ */
