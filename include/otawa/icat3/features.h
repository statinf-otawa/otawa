/*
 *	features of icat3
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
#ifndef ICAT3_CMAKEFILES_FEATURES_H_
#define ICAT3_CMAKEFILES_FEATURES_H_

#include <elm/data/Array.h>

#include <otawa/hard/Cache.h>
#include <otawa/icache/features.h>
#include <otawa/proc/Feature.h>

namespace otawa {

class Block;

namespace icat3 {

using namespace elm;

// LBlocks
class LBlock {
public:
	LBlock(void): _index(-1), _set(-1) { }
	LBlock(Address address, int index, int set);
	inline Address address(void) const { return _address; }
	inline int index(void) const { return _index; }
	inline int set(void) const { return _set; }

private:
	Address _address;
	int _index;
	int _set;
};

class LBlockCollection;
class LBlockSet: public Bag<LBlock *> {
	friend class LBlockCollection;
public:
	inline LBlockSet(void): _coll(0), _set(-1) { }
	~LBlockSet(void);
	inline const LBlockCollection& collection(void) const { return *_coll; }
	inline int index(void) const { return _set; }
private:
	const LBlockCollection *_coll;
	int _set;
};

class LBlockCollection: public Bag<LBlockSet> {
public:
	LBlockCollection(int sets, const hard::Cache *cache);
	inline const hard::Cache *cache(void) const { return _cache; }
	inline int A(void) const { return _cache->wayCount(); }
	inline int sets(void) const { return _cache->setCount(); }
private:
	const hard::Cache *_cache;
};

extern p::feature LBLOCKS_FEATURE;
extern p::id<LBlockCollection *> LBLOCKS;
extern p::id<LBlock *> LBLOCK;


// ACS building
typedef t::int8 age_t;
const int BOT_AGE = -1;

class ACS: public AllocArray<age_t> {
public:
	ACS(void);
	ACS(int n, age_t d = BOT_AGE);
	ACS(const ACS& a);
	ACS& operator=(const ACS& a);
	void print(int set, const LBlockCollection& coll, io::Output& out = cout) const;
};
io::Output& operator<<(io::Output& out, const ACS& a);


class ACSStack {
public:
	ACSStack(void);
	ACSStack(int n, age_t d = BOT_AGE);
	ACSStack(const ACS& a);
	inline ACSStack(const ACSStack& a) { copy(a); }
	inline const Vector<ACS>& stack(void) const { return _stack; }
	inline Vector<ACS>& stack(void) { return _stack; }
	inline const ACS& whole(void) const { return _whole; }
	inline ACS& whole(void) { return _whole; }
	inline bool isBottom(void) const { return _bot; }
	inline void setBottom(bool bot) { _bot = bot; }
	void print(int set, const LBlockCollection& coll, io::Output& out = cout) const;
	inline int depth(void) const { return _stack.length(); }
	inline const ACS& get(int i) const { return _stack[i]; }
	inline ACS& get(int i) { return _stack[i]; }
	inline const ACS& operator[](int i) const { return get(i); }
	inline ACS& operator[](int i) { return get(i); }
	inline void push(const ACS& a) { _stack.add(a); }
	inline void pop(void) { _stack.setLength(_stack.length() - 1); }
	void copy(const ACSStack& a);
private:
	bool _bot;
	ACS _whole;
	Vector<ACS> _stack;
};

template <class T>
class Container: public AllocArray<T> {
public:
	inline Container(void) { }
	inline Container(const LBlockCollection& c): elm::AllocArray<T>(new T[c.sets()]) { }
	inline Container(const Container& c): elm::AllocArray<T>(c) { }
	inline void configure(const LBlockCollection& c) { AllocArray<T>::set(c.sets(), new T[c.sets()]); }
};

class CacheState;
class ACSManager {
public:
	ACSManager(WorkSpace *ws);
	~ACSManager(void);
	void start(Block *b);
	void update(const icache::Access& acc);
	int mustAge(const LBlock *lb);
	int mayAge(const LBlock *lb);
	int depth(const LBlock *lb);
	int persAge(const LBlock *lb, int depth);
	void print(const LBlock *lb, Output& out = cout);

private:
	CacheState& use(int set);
	const LBlockCollection& coll;
	Vector<int> used;
	Block *_b;
	CacheState **_states;
	bool _has_may;
};


extern p::id<Container<ACS> > MUST_INIT;
extern p::id<Container<ACS> > PERS_INIT;
extern p::id<Container<ACS> > MAY_INIT;
extern p::feature MUST_PERS_ANALYSIS_FEATURE;
extern p::feature MAY_ANALYSIS_FEATURE;
extern p::id<Container<ACS> > MUST_IN;
extern p::id<Container<ACSStack> > PERS_IN;
extern p::id<Container<ACS> > MAY_IN;

typedef enum category_t {
	UC = 0,
	AH = 1,
	PE = 2,
	AM = 3,
	NC = 4
} category_t;
io::Output& operator<<(io::Output& out, category_t cat);

// event-oriented results
extern p::feature EDGE_EVENTS_FEATURE;

// category results
extern p::feature CATEGORY_FEATURE;
extern p::id<category_t> CATEGORY;
extern p::id<Block *> HEADER;

} }		// otawa::icat3

#endif /* ICAT3_CMAKEFILES_FEATURES_H_ */
