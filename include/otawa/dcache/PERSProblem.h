/*
 *	dcache::PERSProblem class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007-13, IRIT UPS.
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

#ifndef OTAWA_DCACHE_PERSPROBLEM_H_
#define OTAWA_DCACHE_PERSPROBLEM_H_

#include <elm/io.h>
#include <elm/assert.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/dcache/features.h>
#include <otawa/hard/Cache.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/dfa/hai/HalfAbsInt.h>


namespace otawa { namespace dcache {

class PERSProblem {
public:

	class Item: public ACS {
	public:
		inline Item(const int _size, const int _A): ACS(_size, _A, -1) { }
		inline Item(const Item &source): ACS(source) { }
		inline Item(const ACS &source): ACS(source) { }
		inline Item& operator=(const Item &src) { ACS::operator=(src); return *this; }
		inline Item& operator=(const ACS &src) { ACS::operator=(src); return *this; }
		inline void refresh(int id, int newage)
			{ ASSERT((id >= 0) && (id < size)); if ((newage != -1) && ((age[id] > newage) || (age[id] == -1))) age[id] = newage; }
		void lub(const Item &dom);
		bool equals(const Item &dom) const;
		void inject(MUSTProblem::Domain *must, const int id);
		void injectWriteThrough(MUSTProblem::Domain *must, const int id);
		inline bool isWiped(const int id) { return(age[id] == A); }
		inline bool isPersistent(const int id) { return(contains(id) && !isWiped(id));			 }
		void addDamage(const int id, int damage);
		void ageAll(void);
	};

	class Domain {
	public:
		inline Domain(const int _size, const int _A): whole(_size, _A), isBottom(true) { }
		inline Domain(const Domain &source): whole(source.whole), isBottom(source.isBottom)
			{ for (int i = 0; i < source.data.length(); i++) data.add(new Item(*source.data[i])); }
		inline ~Domain(void) { for (int i = 0; i < data.length(); i++) delete data[i]; }

		Domain& operator=(const Domain &src);
		void lub(const Domain &dom);
		void lub(const Item &item);
		bool equals(const Domain &dom) const;
		void empty(void);
		void setToBottom(void);
		inline Item &getWhole(void) { return whole; }
		inline bool contains(const int id, const int index) { ASSERT(!isBottom); return(data[index]->contains(id)); }
		void inject(MUSTProblem::Domain *must, const int id);
		void injectWriteThrough(MUSTProblem::Domain *must, const int id);
		inline bool isWiped(const int id, const int index) { ASSERT(!isBottom); return(data[index]->isWiped(id));	 }
		inline int getAge(const int id, const int index) const { ASSERT(!isBottom); return(data[index]->getAge(id)); }
		inline bool isPersistent(const int id, const int index) { ASSERT(!isBottom); return(data[index]->isPersistent(id)); }
		inline void addDamage(const int id, const int index, int damage) { ASSERT(!isBottom); data[index]->addDamage(id, damage); }
		void addDamage(const int id, int damage);
		inline void refresh(const int id, const int index, int newage) { ASSERT(!isBottom); data[index]->refresh(id, newage); }
		void refresh(const int id, int newage);
		void print(elm::io::Output &output) const;
		void enterContext(void);
		void leaveContext(void);
		inline int length(void) const { ASSERT(!isBottom); return data.length(); }
		inline Item& getItem(const int idx) const { ASSERT(!isBottom); return(*data.get(idx)); }
		void ageAll(void);
		inline void setNotBottom(void) { isBottom = false; }
		inline void set(const ACS& w, const Array<ACS *>& d) {
			isBottom = false;
			whole = w;
			int m = min(data.count(), d.count());
			for(int i = m; i < data.count(); i++)
				delete data[i];
			data.setLength(d.count());
			for(int i = 0; i < m; i++)
				*data[i] = *d[i];
			for(int i = m; i < d.count(); i++)
				data[i] = new Item(*d[i]);
		}

	private:
		Item whole;
		Vector<Item*> data;
		bool isBottom;
	};


	Domain callstate;

public:
	PERSProblem(const int _size, const BlockCollection *_lbset, WorkSpace *_fw, const hard::Cache *_cache, const int _A);
	~PERSProblem(void);
	const Domain& bottom(void) const;
	const Domain& top(void) const;
	inline const Domain& entry(void) const { return top(); }

	inline void lub(Domain &a, const Domain &b) const { a.lub(b); }
	inline void assign(Domain &a, const Domain &b) const { a = b; }
	inline bool equals(const Domain &a, const Domain &b) const { return (a.equals(b)); }
	void update(Domain& out, const Domain& in, otawa::Block* bb);
	void update(Domain& s, const BlockAccess& access);
	void purge(Item& item, const BlockAccess& acc);
	void purge(Domain& domain, const BlockAccess& acc);

	inline void enterContext(Domain& dom, otawa::Block *header, dfa::hai::hai_context_t ctx) {
#ifndef PERFTEST
		if (ctx == dfa::hai::CTX_LOOP)
			dom.enterContext();
#endif
	}

	inline void leaveContext(Domain& dom, otawa::Block *header, dfa::hai::hai_context_t ctx) {
#ifndef PERFTEST
		if (ctx == dfa::hai::CTX_LOOP)
			dom.leaveContext();
#endif
	}

private:
	const BlockCollection *lbset;
	CFG *cfg;
	WorkSpace *fw;
	const hard::Cache *cache;
	Domain bot;
	Domain _top;
	const int line;
};

elm::io::Output& operator<<(elm::io::Output& output, const PERSProblem::Domain& dom);

} }	// otawa::dcache

#endif /* OTAWA_DCACHE_PERSPROBLEM_H_*/
