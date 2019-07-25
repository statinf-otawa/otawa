/*
 *	otawa::dcache module features
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2013, IRIT UPS.
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
#ifndef OTAWA_DCACHE_FEATURES_H_
#define OTAWA_DCACHE_FEATURES_H_

#include <otawa/cache/features.h>
#include <otawa/prop/PropList.h>
#include <otawa/dfa/BitSet.h>
#include <otawa/hard/Cache.h>
#include <otawa/util/Bag.h>

namespace otawa {

namespace ilp { class Var; }
class Inst;

namespace dcache {

//#define OLD_IMPLEMENTATION

// type of unrolling
typedef enum data_fmlevel_t {
		DFML_INNER = 0,
		DFML_OUTER = 1,
		DFML_MULTI = 2,
		DFML_NONE
} data_fmlevel_t;


// ACS class
class ACS {
public:
	inline ACS(const int _size, const int _A, int init = -1) : A (_A), size(_size), age(new int [size])
		{ for (int i = 0; i < size; i++) age[i] = init; }
	inline ~ACS() { delete [] age; }

	inline int getSize(void) const { return size; }
	inline int getA(void) const { return A; }

	inline ACS(const ACS &source) : A(source.A), size(source.size), age(new int [size])
		{ for (int i = 0; i < size; i++) age[i] = source.age[i]; }

	inline ACS& operator=(const ACS& src) { set(src); return *this; }

	inline bool equals(const ACS& dom) const {
		ASSERT((A == dom.A) && (size == dom.size));
		for (int i = 0; i < size; i++) if (age[i] != dom.age[i]) return false;
		return true;
	}

	inline void empty(void) { for (int i = 0; i < size; i++) age[i] = -1; }
	inline bool contains(const int id) const { ASSERT((id < size) && (id >= 0)); return(age[id] != -1); }

	void print(elm::io::Output &output) const;

	inline int getAge(int id) const { ASSERT(id < size); return(age[id]); }
	inline void setAge(const int id, const int _age)
		{ ASSERT(id < size); ASSERT((_age < A) || (_age == -1)); age[id] = _age; }
	inline void set(const ACS& dom)
		{ ASSERT(A == dom.A); ASSERT(size == dom.size); for(int i = 0; i < size; i++) age[i] = dom.age[i]; }

	inline const int& operator[](int i) const { return age[i]; }
	inline int& operator[](int i) { return age[i]; }

protected:
	int A, size;
	int *age;
};
inline io::Output& operator<<(io::Output& out, const ACS& acs) { acs.print(out); return out; }


// Block class
class Block {
public:
	inline Block(void): _set(-1), idx(-1) { }
	inline Block(int set, int index, const Address& address): _set(set), idx(index), addr(address) { ASSERT(!address.isNull()); }
	inline Block(const Block& block): _set(block._set), idx(block.idx), addr(block.addr) { }
	inline int set(void) const { return _set; }
	inline int index(void) const { return idx; }
	inline const Address& address(void) const { return addr; }
	void print(io::Output& out) const;

private:
	int _set;
	int idx;
	Address addr;
};
inline io::Output& operator<<(io::Output& out, const Block& block) { block.print(out); return out; }


// BlockCollection class
class BlockCollection {
public:
	~BlockCollection(void);
	inline const Block& operator[](int i) const { return *blocks[i]; }
	const Block& obtain(const Address& addr);
	inline void setSet(int set) { _set = set; }

	inline int count(void) const { return blocks.count(); }
	inline int cacheSet(void) const { return _set; }

private:
	int _set;
	Vector<Block *> blocks;
};


// Data
class BlockAccess: public PropList {
public:
	typedef enum action_t {
		NONE = 0,
		LOAD = 1,
		STORE = 2,
		PURGE = 3
	} action_t;
	typedef enum kind_t {
		ANY = 0,
		BLOCK = 1,
		RANGE = 2
	} kind_t;

	inline BlockAccess(void): inst(0), _kind(ANY), _action(NONE), id(0) { }
	inline BlockAccess(Inst *instruction, action_t action): inst(instruction), _kind(ANY), _action(action) { ASSERT(instruction); count++; id = count;}
	inline BlockAccess(Inst *instruction, action_t action, const Block& block): inst(instruction), _kind(BLOCK), _action(action)
		{ ASSERT(instruction); data.blk = &block; count++; id = count;}
	inline BlockAccess(Inst *instruction, action_t action, Address::offset_t first, Address::offset_t last): inst(instruction), _kind(RANGE), _action(action)
		{ ASSERT(instruction); data.range.first = first; data.range.last = last; count++; id = count; }
	inline BlockAccess(const BlockAccess& acc): inst(acc.inst), _kind(acc._kind), _action(acc._action), blocks(acc.blocks), id(acc.id)
		{ data = acc.data; }
	inline BlockAccess& operator=(const BlockAccess& acc)
		{ inst = acc.inst; _kind = acc._kind; _action = acc._action; data = acc.data; blocks = acc.blocks; id = acc.id; return *this; }

	inline Inst *instruction(void) const { return inst; }
	inline kind_t kind(void) const { return kind_t(_kind); }
	inline bool isAny(void) const { return _kind == ANY; }
	inline action_t action(void) const { return action_t(_action); }
	inline const Block& block(void) const { ASSERT(_kind == BLOCK); return *data.blk; }
	inline int first(void) const { ASSERT(_kind == RANGE); return data.range.first; }
	inline int last(void) const { ASSERT(_kind == RANGE); return data.range.last; }
	inline bool inRange(int block) const { if(first() < last()) return first() <= block && block <= last(); else return block <= first() || last() <= block; }
	bool inSet(int set, const hard::Cache *cache) const;
	bool in(const Block& block) const;

	void print(io::Output& out) const;

	inline void addBlock(const Block* block) { blocks.addLast(block); }
	const Vector<const Block*>& getBlocks(void) const { return blocks; }

	static unsigned int count;
	inline unsigned int getID(void) const { return id; }

private:
	Inst *inst;
	t::uint8 _kind, _action;
	union {
		const Block *blk;
		struct { hard::Cache::block_t first, last; } range;
	} data;
	Vector<const Block*> blocks;
	unsigned int id;
};
inline io::Output& operator<<(io::Output& out, const BlockAccess& acc) { acc.print(out); return out; }
inline io::Output& operator<<(io::Output& out, const Pair<int, BlockAccess *>& v) { return out; }
io::Output& operator<< (io::Output& out, BlockAccess::action_t action);


class NonCachedAccess: public PropList {
public:
	typedef enum kind_t {
		ANY = 0,
		SINGLE = 1,
		MULTIPLE = 2
	} kind_t;

	inline NonCachedAccess(void): inst(0), _kind(ANY), _action(NONE) { }
	inline NonCachedAccess(Inst* instx, BlockAccess::action_t actx): inst(instx), _kind(SINGLE), _action(actx) { ASSERT(instx); }
	inline NonCachedAccess(Inst* instx, BlockAccess::action_t actx, Address addrx): inst(instx), _kind(SINGLE), _action(actx) { ASSERT(instx); addresses.add(addrx); }
	inline NonCachedAccess(const NonCachedAccess& acc): inst(acc.inst), _kind(acc._kind), _action(acc._action), addresses(acc.addresses) { }
	inline NonCachedAccess& operator=(const NonCachedAccess& acc) { inst = acc.inst; _kind = acc._kind; _action = acc._action; addresses = acc.addresses; return *this; }

	inline Inst *instruction(void) const { return inst; }
	inline kind_t kind(void) const { return kind_t(_kind); }
	inline bool isAny(void) const { return _kind == ANY; }
	inline BlockAccess::action_t action(void) const { return BlockAccess::action_t(_action); }
	inline void addAddress(Address addr) { addresses.addLast(addr); _kind = MULTIPLE;}
	const Vector<Address>& getAddresses(void) const { return addresses; }
	inline void print(io::Output& out) const {
		out << inst->address() << " (" << inst << "): " << BlockAccess::action_t(_action) << ' ';
		if(addresses.count() == 1)
			out << addresses[0];
		else if(addresses.count() == 0)
			out << "multiple addresses";
		else
			out << "multiple addresses";
	}

private:
	Inst *inst;
	t::uint8 _kind, _action;
	Vector<Address> addresses;
};

inline io::Output& operator<<(io::Output& out, const NonCachedAccess& acc) { acc.print(out); return out; }

// DirtyManager class
class DirtyManager {
public:

	typedef struct t {
		friend class DirtyManager;
	public:
		inline t(void) { }
		inline t(int s): _may(s), _must(s) { }
		inline t(const t& i): _may(i._may), _must(i._must) { }
		inline const dfa::BitSet& may(void) const { return _may; }
		inline const dfa::BitSet& must(void) const { return _must; }
		void print(io::Output& out) const;
		inline bool mayBeDirty(int b) const { return _may.contains(b); }
		inline bool mustBeDirty(int b) const { return _must.contains(b); }
	private:
		inline dfa::BitSet& may(void) { return _may; }
		inline dfa::BitSet& must(void) { return _must; }
		dfa::BitSet _may, _must;
	} t;

	DirtyManager(const BlockCollection& coll);
	bool mayBeDirty(const t& value, int block) const;
	bool mustBeDirty(const t& value, int block) const;
	const t& bottom(void) const;
	const t& top(void) const;
	void set(t& d, const t& s) const;
	void update(t& d, const BlockAccess& acc);
	void join(t& d, const t& s) const;
	bool equals(const t& s1, const t& s2) const;

private:
	t bot, _top;
	const BlockCollection& _coll;
};
inline io::Output& operator<<(io::Output& out, const DirtyManager::t& v) { v.print(out); return out; }

// useful typedefs
typedef AllocArray<ACS *> acs_stack_t;
typedef Vector<ACS *> acs_table_t;
typedef Vector<acs_stack_t> acs_stack_table_t;


// block analysis
extern p::feature DATA_BLOCK_FEATURE;
extern p::feature CLP_BLOCK_FEATURE;
extern p::id<Pair<int, BlockAccess *> > DATA_BLOCKS;
extern p::id<Pair<int, NonCachedAccess *> > NC_DATA_ACCESSES;
extern p::id<const BlockCollection *> DATA_BLOCK_COLLECTION;
extern p::id<Address> INITIAL_SP;

// MUST analysis
extern p::feature MUST_ACS_FEATURE;
extern p::feature PERS_ACS_FEATURE;
extern p::id<acs_table_t *> MUST_ACS;
extern p::id<acs_table_t *> ENTRY_MUST_ACS;
extern p::id<acs_table_t *> PERS_ACS;
extern p::id<acs_table_t *> ENTRY_PERS_ACS;
extern p::id<acs_stack_table_t *> LEVEL_PERS_ACS;
extern p::id<bool> DATA_PSEUDO_UNROLLING;
extern p::id<data_fmlevel_t> DATA_FIRSTMISS_LEVEL;

// categories build
extern p::id<cache::category_t> WRITETHROUGH_DEFAULT_CAT;
extern p::feature CATEGORY_FEATURE;
extern p::id<cache::category_t> CATEGORY;
extern p::id<otawa::Block *> CATEGORY_HEADER;

// ILP constraint build
extern p::feature CONSTRAINTS_FEATURE;
extern p::id<ilp::Var *> MISS_VAR;

// MAY analysis
extern Identifier<Vector<ACS *> *> ENTRY_MAY_ACS;
extern p::feature MAY_ACS_FEATURE;
extern p::id<Vector<ACS *> *> MAY_ACS;

// Dirty analysis
extern p::feature DIRTY_FEATURE;
extern p::id<AllocArray<DirtyManager::t> > DIRTY;

// Purge analysis
typedef enum {
	INV_PURGE = 0,
	NO_PURGE = 1,
	PERS_PURGE = 2,
	MAY_PURGE = 3,
	MUST_PURGE = 4
} purge_t;
io::Output& operator<<(io::Output& out, purge_t purge);
extern p::feature PURGE_FEATURE;
extern p::id<purge_t> PURGE;
extern p::id<ot::time> PURGE_TIME;

// WCET builder
extern p::feature WCET_FUNCTION_FEATURE;

} }		// otawa::dcache

#endif /* OTAWA_DCACHE_FEATURES_H_ */
