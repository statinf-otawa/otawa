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

#include <otawa/cfg.h>
#include <otawa/icat3/features.h>

namespace otawa { namespace icat3 {

class MustDomain {
public:
	typedef ACS t;

	MustDomain(const LBlockCollection& coll, int set, const t *init);
	inline const t& bot(void) const { return _bot; }
	inline const t& top(void) const { return _top; }
	inline const t& init(void) const { return _init; }
	inline void copy(t& d, const t& s) { d.copy(s); }
	inline void print(const t& a, io::Output& out) const { a.print(_set, _coll, out); }
	void join(t& d, const t& s);
	inline bool contains(const t& a, int i) { return(a[i] != BOT_AGE); }
	void fetch(t& a, const LBlock *lb);
	void update(const icache::Access& access, t& a);
	void update(const Bag<icache::Access>& accs, t& a);
	void update(Edge *e, t& a);
	bool equals(const t& a, const t& b);

private:
	int n;
	t _bot, _top;
	hard::Cache::set_t _set;
	const LBlockCollection& _coll;
	int A;
	const t& _init;
	t tmp;
};

class PersDomain {
public:
	typedef ACSStack t;

	PersDomain(const LBlockCollection& coll, int set, const ACS *init);
	inline const t& bot(void) const { return _bot; }
	inline const t& top(void) const { return _top; }
	inline const t& init(void) const { return _init; }
	inline void copy(t& d, const t& s) { d.copy(s); }
	void print(const t& a, io::Output& out) const;
	void join(t& d, const t& s);
	void fetch(LBlock *b, ACS& a);
	void update(const icache::Access& access, t& a);
	void update(const Bag<icache::Access>& accs, t& a);
	void update(Edge *e, t& a);
	bool equals(const t& a, const t& b);
	void enter(t& a);
	void leave(t& a);

private:
	void join(ACS& d, const ACS& s);
	void update(const icache::Access& access, ACS& a);
	bool equals(const ACS& a1, const ACS& a2);

	int n;
	t _bot, _top;
	hard::Cache::set_t _set;
	const LBlockCollection& _coll;
	int A;
	t _init;
	ACS tmp;
};


class MustPersDomain {
public:
	typedef struct t {
		inline t(void) { }
		inline t(const t& a): must(a.must), pers(a.pers) { }
		inline t(int n): must(n), pers(n) { }
		inline t(const MustDomain::t& _must, const PersDomain::t& _pers): must(_must), pers(_pers) { }
		MustDomain::t must;
		PersDomain::t pers;
	} t;

	MustPersDomain(const LBlockCollection& coll, int set, const MustDomain::t *must_init = nullptr, const ACS *pers_init = nullptr);
	MustDomain& mustDomain(void) { return _must; }
	PersDomain& persDomain(void) { return _pers; }

	inline const t& bot(void) const { return _bot; }
	inline const t& top(void) const { return _top; }
	inline const t& init(void) const { return _init; }
	inline void copy(t& d, const t& s) { d.must.copy(s.must); d.pers.copy(s.pers); }
	void print(const t& a, io::Output& out) const;
	inline io::Printable<t, MustPersDomain> print(const t& a) const { return io::p(a, *this); }
	void join(t& d, const t& s);
	inline bool contains(const t& a, int i) { return(a.must[i] != BOT_AGE); }
	void update(const icache::Access& access, t& a);
	void update(const Bag<icache::Access>& accs, t& a);
	void update(Edge *e, t& a);
	bool equals(const t& a, const t& b);
	inline const MustDomain::t& must(const t& a) { return a.must; }
	inline const PersDomain::t& pers(const t& a) { return a.pers; }
	void doCall(t& a, Block *b);
	void doReturn(t& a, Block *b);
	void enterLoop(t& a);
	void leaveLoop(t& a);

private:
	static p::id<int> DEPTH;
	MustDomain _must;
	PersDomain _pers;
	int n;
	t _bot, _top;
	t _init;
};

} }		// otawa::icat3

#endif /* OTAWA_ICAT3_MUSTPERSDOMAIN_H_ */
