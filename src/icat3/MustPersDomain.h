/*
 *	MustPersDomain class interface
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
#ifndef OTAWA_ICAT3_MUSTPERSDOMAIN_H_
#define OTAWA_ICAT3_MUSTPERSDOMAIN_H_

#include "features.h"

namespace otawa { namespace icat3 {

class MustPersDomain {
public:
	typedef ACS t;

	MustPersDomain(const LBlockCollection& coll, int set);
	MustPersDomain(const LBlockCollection& coll, int set, const t& init);
	inline const ACS& bot(void) const { return _bot; }
	inline const ACS& top(void) const { return _top; }
	inline const ACS& init(void) const { return _init; }
	inline void copy(ACS& t, const ACS& s) { t.copy(s); }
	inline void print(const ACS& a, io::Output& out) { a.print(_set, _coll, out); }
	void join(ACS& t, const ACS& s);
	inline bool contains(const ACS& t, int i) { return(t[i] != BOT_AGE); }
	void fetch(ACS& t, int b);
	void update(const icache::Access& access, ACS& a);
	void update(const Bag<icache::Access>& accs, ACS& a);
	void update(Block *v, Edge *e, ACS& a);
	bool equals(const ACS& a, const ACS& b);

private:
	int n;
	ACS _bot, _top;
	hard::Cache::set_t _set;
	const LBlockCollection& _coll;
	int A;
	ACS tmp;
	const ACS& _init;
};

} }		// otawa::icat3

#endif /* OTAWA_ICAT3_MUSTPERSDOMAIN_H_ */
