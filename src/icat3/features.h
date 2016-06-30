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

#include <elm/data/Table.h>
#include <otawa/hard/Cache.h>
#include <otawa/proc/Feature.h>

namespace otawa { namespace icat3 {

using namespace elm;

// LBlocks
class LBlock {
public:
	LBlock(void): _index(-1) { }
	LBlock(Address address, int index);
	inline Address address(void) const { return _address; }
	inline int index(void) const { return _index; }

private:
	Address _address;
	int _index;
};

class LBlockCollection;
class LBlockSet: public Bag<LBlock> {
	friend class LBlockCollection;
public:
	inline LBlockSet(void): _coll(0), _set(-1) { }
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

class ACS: public AllocTable<age_t> {
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
	ACSStack(int n);
	inline const genstruct::Vector<ACS>& stack(void) const { return _stack; }
	inline genstruct::Vector<ACS>& stack(void) { return _stack; }
	inline const ACS& whole(void) const { return _whole; }
	inline ACS& whole(void) { return _whole; }
	inline bool isBottom(void) const { return _bot; }
	inline void setBottom(bool bot) { _bot = bot; }
private:
	bool _bot;
	ACS _whole;
	genstruct::Vector<ACS> _stack;
};

template <class T>
class Container {
public:
	inline Container(void): vals(0) { }
	inline Container(const LBlockCollection& c): vals(new T[c.sets()]) { }
	inline ~Container(void) { if(vals) delete [] vals; }
	inline T& operator[](int i) { return vals[i]; }
	inline const T& operator[](int i) const { return vals[i]; }
private:
	T *vals;
};

extern p::id<Container<ACS> > MUST_INIT;
extern p::id<Container<ACS> > PERS_INIT;
extern p::id<Container<ACS> > MAY_INIT;
extern p::feature MUST_PERS_ANALYSIS_FEATURE;
extern p::feature MAY_ANALISYS_FEATURE;
extern p::id<Container<ACS> > MUST_STATE;
extern p::id<Container<ACSStack> > PERS_STATE;
extern p::id<Container<ACS> > MAY_STATE;

extern p::feature EDGE_EVENTS_FEATURE;

} }		// otawa::icat3

#endif /* ICAT3_CMAKEFILES_FEATURES_H_ */
