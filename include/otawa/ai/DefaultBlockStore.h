/*
 *	DefaultBlockStore class
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2014, IRIT UPS.
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
#ifndef OTAWA_AI_DEFAULTBLOCKSTORE_H_
#define OTAWA_AI_DEFAULTBLOCKSTORE_H_

#include <otawa/cfg/features.h>

namespace otawa { namespace ai {

using namespace elm;

template <class D>
class DefaultBlockStore {
public:

	DefaultBlockStore(): size(0), tab(nullptr) { }
	~DefaultBlockStore() { if(tab != nullptr) delete [] tab; }

	void init(const CFGCollection& c, const D& x) {
		size = c.countBlocks();
		tab = new D [size];
		array::set(tab, size, x);
	}

	inline const D& get(Block *v) const { return tab[v->id()]; }
	inline void set(Block *v, const D& x) { tab[v->id()] = x; }

private:
	int size;
	D *tab;
};

} }		// otawa::ai

#endif /* OTAWA_AI_DEFAULTBLOCKSTORE_H_ */
