/*
 *	$Id$
 *	BranchProblem class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2011, IRIT UPS.
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


#ifndef CACHE_BRANCHPROBLEM_H_
#define CACHE_BRANCHPROBLEM_H_

#include <otawa/prog/WorkSpace.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/hard/Cache.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/dfa/hai/HalfAbsInt.h>

using namespace otawa::cache;

namespace otawa {


class MUSTBranch {
public:

	class Domain {
	public:
			
		inline Domain(const int _size, const int _A, int init = 0): A(_A), size(_size)
			{ age = new int [size]; for(int i = 0; i < size; i++) age[i] = init; }
		inline Domain(const Domain &source) : A(source.A), size(source.size)
			{ age = new int [size]; for(int i = 0; i < size; i++) age[i] = source.age[i]; }
		inline ~Domain(void) { delete [] age; }
			
		inline Domain& operator=(const Domain &src) {
			ASSERT((A == src.A) && (size == src.size));
			for(int i = 0; i < size ; i++)
				age[i] = src.age[i];
			return(*this);
		}
			
		inline void lub(const Domain &dom) {
			ASSERT((A == dom.A) && (size == dom.size));
			for(int i = 0; i < size; i++)
				if(age[i] < dom.age[i])
					age[i] = dom.age[i];
		}
			
		inline int getSize(void) { return size; }

		inline bool equals(const Domain &dom) const {
			ASSERT((A == dom.A) && (size == dom.size));
			for(int i = 0; i < size; i++)
				if(age[i] != dom.age[i])
					return false;
			return true;
		}
			
		inline bool contains(const int id)
			{ ASSERT((id < size) && (id >= 0)); return(age[id] != A); }
			
		inline void inject(const int id) {
			if(contains(id)) {
				for(int i = 0; i < size; i++)
					if(age[i] < age[id])
						age[i]++;
			}
			else
				for (int i = 0; i < size; i++)
					if (age[i] != A)
						age[i]++;
			age[id] = 0;
		}
			
		void print(elm::io::Output &output) const;
		inline int getAge(int id) const { ASSERT(id < size); return(age[id]); }
		inline void setAge(const int id, const int _age)
			{ ASSERT(id < size); ASSERT((_age <= A) || (_age >= 0)); age[id] = _age; }
			
	private:
		int A, size;
		int *age;
	};

	MUSTBranch(int _size, int _A, int _row);
	~MUSTBranch(void);
	
	inline const Domain& bottom(void) const { return bot; }
	inline const Domain& top(void) const { return _top; }
	const Domain& entry(void) const { return _top; }
	inline void lub(Domain &a, const Domain &b) const { a.lub(b); }
	inline void assign(Domain &a, const Domain &b) const { a = b; }
	inline bool equals(const Domain &a, const Domain &b) const { return (a.equals(b)); }
	void update(Domain& out, const Domain& in, Block* bb);
	inline void enterContext(Domain &dom, Block *header, dfa::hai::hai_context_t ctx) { }
	inline void leaveContext(Domain &dom, Block *header, dfa::hai::hai_context_t ctx) { }

private:
	int row;
	Domain bot;
	Domain _top;
};



class MAYBranch {
public:

	class Domain {
	public:
		
		inline Domain(const int _size, const int _A, int init = -1) : A (_A), size(_size)
			{ if(init < 0) init = A; age = new int [size]; for (int i = 0; i < size; i++) age[i] = init; }
		inline Domain(const Domain &source) : A(source.A), size(source.size)
			{ age = new int [size]; for (int i = 0; i < size; i++) age[i] = source.age[i]; }
		inline ~Domain(void) { delete [] age; }

		inline Domain& operator=(const Domain &src) {
			ASSERT((A == src.A) && (size == src.size));
			for (int i = 0; i < size ; i++)
				age[i] = src.age[i];
			return(*this);
		}

		inline void lub(const Domain &dom) {
			ASSERT((A == dom.A) && (size == dom.size));
			for (int i = 0; i < size; i++)
				if (age[i] >= dom.age[i])
					age[i] = dom.age[i];
		}
			
		inline int getSize(void) { return size; }

		inline bool equals(const Domain &dom) const {
			ASSERT((A == dom.A) && (size == dom.size));
			for (int i = 0; i < size; i++)
				if (age[i] != dom.age[i])
					return false;
			return true;
		}
			
		inline void empty(void) { for (int i = 0; i < size; i++) age[i] = 0; }
		inline bool contains(const int id) { ASSERT((id < size) && (id >= 0)); return(age[id] != A); }
			
		inline void inject(const int id) {
			if(contains(id))
				for(int i = 0; i < size; i++) {
					if (age[i] <= age[id])
						age[i]++;
				}
			else
				for(int i = 0; i < size; i++)
					if(age[i] != A)
						age[i]++;
			age[id] = 0;
		}
			
		void print(elm::io::Output &output) const;
		inline int getAge(int id) const { ASSERT(id < size); return(age[id]); }
		inline void setAge(const int id, const int _age)
			{ ASSERT(id < size); ASSERT((_age <= A) || (_age >= 0)); age[id] = _age; }
			
	private:
		int A, size;
		int *age;
	};
	
	MAYBranch(int _size, int _A, int _row);
	~MAYBranch(void);

	const Domain& top(void) const { return _top; }
	const Domain& bottom(void) const { return bot; }
	const Domain& entry(void) const { return _top; }
		
	inline void lub(Domain &a, const Domain &b) const { a.lub(b); }
	inline void assign(Domain &a, const Domain &b) const { a = b; }
	inline bool equals(const Domain &a, const Domain &b) const { return (a.equals(b)); }
	
	void update(Domain& out, const Domain& in, BasicBlock* bb);
	inline void enterContext(Domain &dom, Block *header, dfa::hai::hai_context_t ctx) { }
	inline void leaveContext(Domain &dom, Block *header, dfa::hai::hai_context_t ctx) { }

private:
	int row;
	Domain bot;
	Domain _top;
};


class PERSBranch {
public:
	static const int BOT = -1;

	class Item {
	public:
		inline Item(int _size, int _A, int init = BOT): A (_A), size(_size)
			{ age = new int [size]; for (int i = 0; i < size; i++) age[i] = init; }
		inline Item(const Item &source) : A(source.A), size(source.size)
			{ age = new int [size]; for (int i = 0; i < size; i++) age[i] = source.age[i]; }
		inline ~Item(void) { delete [] age; }
			
		inline Item& operator=(const Item &src) {
			ASSERT((A == src.A) && (size == src.size));
			for(int i = 0; i < size ; i++)
				age[i] = src.age[i];
			return *this;
		}

		// TODO -- do we take into account the multi-path join problem?
		inline void lub(const Item &dom) {
			for(int i = 0; i < size; i++)
				if(age[i] == BOT || (age[i] < dom.age[i] && dom.age[i] != BOT))
					age[i] = dom.age[i];
		}
			
		inline bool equals(const Item &dom) const {
			ASSERT((A == dom.A) && (size == dom.size));
			for (int i = 0; i < size; i++)
				if (age[i] != dom.age[i])
					return false;
			return true;
		}
			
		inline bool contains(const int id)
			{ ASSERT((id < size) && (id >= 0)); return(age[id] != -1); }
			
		inline void inject(MUSTBranch::Domain *must, const int id) {
			if(must->contains(id)) {
				for(int i = 0; i < size; i++)
					if(age[i] < age[id] && age[i] != BOT && age[i] != A)
						age[i]++;
			}
			else
				for(int i = 0; i < size; i++)
					if(age[i] != BOT && age[i] != A)
						age[i]++;
			age[id] = 0;
		}
			
		inline bool isWiped(const int id) { return(age[id] == A); }
		inline bool isPersistent(const int id) { return(contains(id) && !isWiped(id)); }
		inline int getAge(const int id) const { ASSERT((id >= 0) && (id < size)); return age[id]; }
		void print(elm::io::Output &output) const;

	private:
		int A, size;
		int *age;
	};
	
	class Domain {
	public:
		inline Domain(int _size, int _A, int init = BOT): A (_A), size(_size), whole(_size, _A, init)
			{ isBottom = init == BOT; }
		inline Domain(const Domain &source) : A(source.A), size(source.size), isBottom(source.isBottom), whole(source.whole)
			{ for (int i = 0; i < source.data.length(); i++) data.add(new Item(*source.data[i])); }
		inline ~Domain(void)
			{ for (int i = 0; i < data.length(); i++) delete data[i]; }
			
		inline Domain& operator=(const Domain &src) {
			if (src.isBottom)
				setToBottom();
			else {
				whole = src.whole;
				isBottom = false;
				int sdl = src.data.length();
				int dl = data.length();
				int minl = (sdl > dl) ? dl : sdl;
				data.setLength((sdl > dl) ? dl : sdl);
				for (int i = 0; i < minl; i++)
					*data[i] = *src.data[i];
				for (int i = dl; i < sdl; i++)
					data.add(new Item(*src.data[i]));
			}	
			return *this;
		}
			
		inline void lub(const Domain &dom) {
			if(dom.isBottom)
				return;
			else if(isBottom) {
				for(int i = 0; i < dom.data.length(); i++)
					data.add(new Item(*dom.data[i]));
				whole = dom.whole;
				isBottom = false;
			}
			else {
				int dl = data.length();
				int ddl = dom.data.length();
				int length = (dl < ddl) ? dl : ddl;
				for (int i = dl - 1, j = ddl - 1, k = 0; k < length; i--, j--, k++)
					data[i]->lub(*dom.data[j]);
				for (int i = 0; i < dl - length; i++)
					data.remove(0);
				whole.lub(dom.whole);
			}
		}

		inline void lub(const Item &item) {
			for (int i = 0; i < data.length(); i++)
				data[i]->lub(item);
			 whole.lub(item);
		}

		inline bool equals(const Domain &dom) const {
			ASSERT(!isBottom && !dom.isBottom);
			for (int i = 0; i < dom.data.length(); i++)
				if (!data[i]->equals(*dom.data[i]))
					return false;
			return (whole.equals(dom.whole) && (isBottom == dom.isBottom));
		}
			
		inline void setToBottom(void) {
			for (int i = 0; i < data.length(); i++)
				delete data[i];
			data.clear();
			whole = Item(size, A, BOT);
			isBottom = true;
		}
			
		inline Item &getWhole(void) { return whole; }
		inline bool contains(int id, int index) { ASSERT(!isBottom); return(data[index]->contains(id)); }
			
		inline void inject(MUSTBranch::Domain *must, const int id) {
			ASSERT(!isBottom);
			for (int i = 0; i < data.length(); i++)
				data[i]->inject(must, id);
			whole.inject(must, id);
		}
			
		inline bool isWiped(const int id, const int index) { ASSERT(!isBottom); return (data[index]->isWiped(id)); }
		inline int getAge(const int id, const int index) const { ASSERT(!isBottom); return(data[index]->getAge(id)); }
		inline bool isPersistent(const int id, const int index) { ASSERT(!isBottom); return(data[index]->isPersistent(id)); }
		void print(elm::io::Output &output) const;
			
		inline void enterContext(void) { ASSERT(!isBottom); Item item(size, A, BOT); data.push(new Item(item)); }
		inline void leaveContext(void) { ASSERT(!isBottom); Item *ptr = data.pop(); delete ptr; }
		inline int length(void) { ASSERT(!isBottom); return data.length(); }
		inline Item& getItem(const int idx) const { ASSERT(!isBottom); return(*data.get(idx)); }
			
	private:
		int A, size;
		bool isBottom;
		Item whole;
		Vector<Item *> data;
	};
	
	PERSBranch(int _size, int _A, int _row);
	~PERSBranch(void);
	
	inline const Domain& bottom(void) const { return bot; }
	inline const Domain& entry(void) const { return _top; }
	inline const Domain& top(void) const { return _top; }
	inline void lub(Domain &a, const Domain &b) const { a.lub(b); }
	inline void assign(Domain &a, const Domain &b) const { a = b; }
	inline bool equals(const Domain &a, const Domain &b) const { return (a.equals(b)); }
	void update(Domain& out, const Domain& in, Block* bb);
	inline void enterContext(Domain& dom, Block *header, dfa::hai::hai_context_t ctx)
		{ if (ctx == dfa::hai::CTX_LOOP) dom.enterContext(); }
	inline void leaveContext(Domain& dom, Block *header, dfa::hai::hai_context_t ctx)
		{ if (ctx == dfa::hai::CTX_LOOP) dom.leaveContext(); }

private:
	int row;
	Domain bot;
	Domain _top;
};


class BranchProblem {
public:

	class Domain {
		friend class BranchProblem;
	public:
		inline Domain(const int _size, const int _A)
			: pers(PERSBranch::Domain(_size, _A)), must(MUSTBranch::Domain(_size, _A)), may(MAYBranch::Domain(_size, _A)) {	}
		inline Domain(const Domain &source) : pers(source.pers), must(source.must), may(source.may) { }
			
		inline Domain& operator=(const Domain &src) {
			pers = src.pers;
			must = src.must;
			may = src.may;
			return *this;
		}

		inline MAYBranch::Domain& getMay(void) { return may; }
		inline MUSTBranch::Domain& getMust(void) { return must; }
		inline PERSBranch::Domain& getPers(void) { return pers; }
			
		inline void lub(const Domain &dom) {
			pers.lub(dom.pers);
			must.lub(dom.must);
			may.lub(dom.may);
		}
			
		inline int getSize(void) { return must.getSize(); }
		inline bool equals(const Domain &dom) const
			{ return (pers.equals(dom.pers) && must.equals(dom.must) && may.equals(dom.may)); }

		inline void inject(const int id) {
			pers.inject(&must, id);
			must.inject(id);
			may.inject(id);
		}
			
		inline void print(elm::io::Output &output) const {
			output << "PERS=[ ";
			pers.print(output);
			output << "] MUST=[ ";
			must.print(output);
			output << "] MAY=[ ";
			may.print(output);
			output << "]";
		}

	private:
		PERSBranch::Domain pers;
		MUSTBranch::Domain must;
		MAYBranch::Domain may;
	};
	
public:
	BranchProblem(const int _size,  WorkSpace *_fw, const int _A, const int _row);

	const Domain& bottom(void) const { return bot; }
	const Domain& entry(void) const { return _top; }
	const Domain& top(void) const { return _top; }

	inline void lub(Domain &a, const Domain &b) const { a.lub(b); }
	inline void assign(Domain &a, const Domain &b) const { a = b; }
	inline bool equals(const Domain &a, const Domain &b) const { return (a.equals(b)); }
	
	void update(Domain& out, const Domain& in, Block* bb);
	void updateLBlock(Domain& acs, LBlock *lblock);
	
	inline void enterContext(Domain &dom, Block *header, dfa::hai::hai_context_t ctx) {
		persProb.enterContext(dom.pers, header, ctx);
		mustProb.enterContext(dom.must, header, ctx);
		mayProb.enterContext(dom.may, header, ctx);		
	}

	inline void leaveContext(Domain &dom, Block *header, dfa::hai::hai_context_t ctx) {
		persProb.leaveContext(dom.pers, header, ctx);
		mustProb.leaveContext(dom.must, header, ctx);
		mayProb.leaveContext(dom.may, header, ctx);
	}		

private:
	Domain bot, _top;
	MUSTBranch mustProb;
	PERSBranch persProb;
	MAYBranch mayProb;
	WorkSpace *fw;
	int row;
};

}

namespace elm { namespace io {
	inline io::Output& operator<<(io::Output& out, const otawa::BranchProblem::Domain& state) {
		state.print(out);
		return out; }
}}


#endif /*CACHE_MUSTPROBLEM_H_*/
