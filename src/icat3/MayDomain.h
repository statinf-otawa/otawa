/*
 *	MayDomain class interface
 *	Copyright (c) 2017, IRIT UPS.
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
#ifndef OTAWA_ICAT3_MAYDOMAIN_H_
#define OTAWA_ICAT3_MAYDOMAIN_H_

#include <otawa/icat3/features.h>

namespace otawa { namespace icat3 {

class MayDomain {
public:
	typedef ACS t;

	MayDomain(const LBlockCollection& coll, int set, const t *init);
	inline const t& bot(void) const { return _bot; }
	inline const t& top(void) const { return _top; }
	inline const t& init(void) const { return _init; }
	inline void print(const t& a, io::Output& out) const { a.print(_set, _coll, out); }
	inline io::Printable<t, MayDomain> print(const t& a) const { return io::p(a, *this); }
	inline bool contains(const t& a, int i) { return(a[i] != BOT_AGE); }
	inline void copy(t& d, const t& s) { d.copy(s); }
	inline bool equals(const t& a, const t& b)
		{ for(int i = 0; i < n; i++) if(a[i] != b[i]) return false; return true; }
	void join(t& d, const t& s);
	void fetch(t& a, const LBlock *lb);
	void update(const icache::Access& access, t& a);

private:
	int n;
	t _bot, _top;
	hard::Cache::set_t _set;
	const LBlockCollection& _coll;
	int A;
	const t& _init;
	t tmp;
};

} }

#endif /* OTAWA_ICAT3_MAYDOMAIN_H_ */
