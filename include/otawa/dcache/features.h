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

#include <otawa/proc/SilentFeature.h>
#include <otawa/cache/categories.h>
#include <otawa/prop/PropList.h>

namespace otawa {

namespace ilp { class Var; }
class Inst;

namespace dcache {

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

protected:
	int A, size;
	int *age;
};
inline io::Output& operator<<(io::Output& out, const ACS& acs) { acs.print(out); return out; }


// Block class
class Block {
public:
	inline Block(void): _set(-1), idx(-1)  { }
	inline Block(int set, int index, const Address& address): _set(set), idx(index), addr(address) { }
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
	const Block& get(int set, const Address& addr);
	inline void setSet(int set) { _set = set; }

	inline int count(void) const { return blocks.count(); }
	inline int cacheSet(void) const { return _set; }

private:
	int _set;
	genstruct::Vector<Block> blocks;
};


// Data
class BlockAccess: public PropList {
public:
	typedef enum kind_t {
		ANY,
		BLOCK,
		RANGE
	} kind_t;

	inline BlockAccess(void): inst(0), _kind(ANY) { }
	inline BlockAccess(Inst *instruction): inst(instruction), _kind(ANY) { }
	inline BlockAccess(Inst *instruction, const Block& block): inst(instruction), _kind(BLOCK)
		{ data.blk = &block; }
	inline BlockAccess(Inst *instruction, int first, int last): inst(instruction), _kind(RANGE)
		{ data.range.first = first; data.range.last = last; }
	inline BlockAccess(const BlockAccess& acc): inst(acc.inst), _kind(acc._kind)
		{ data = acc.data; }
	inline BlockAccess& operator=(const BlockAccess& acc)
		{ inst = acc.inst; _kind = acc._kind; data = acc.data; return *this; }

	inline Inst *instruction(void) const { return inst; }
	inline kind_t kind(void) { return _kind; }
	inline const Block& block(void) const { ASSERT(_kind == BLOCK); return *data.blk; }
	inline int first(void) const { ASSERT(_kind == RANGE); return data.range.first; }
	inline int last(void) const { ASSERT(_kind == RANGE); return data.range.last; }

	void print(io::Output& out) const;

private:
	Inst *inst;
	kind_t _kind;
	union {
		const Block *blk;
		struct { int first, last; } range;
	} data;
};
inline io::Output& operator<<(io::Output& out, const BlockAccess& acc) { acc.print(out); return out; }
inline io::Output& operator<<(io::Output& out, const Pair<int, BlockAccess *>& v) { return out; }

// useful typedefs
typedef genstruct::AllocatedTable<ACS *> acs_stack_t;
typedef genstruct::Vector<ACS *> acs_table_t;
typedef genstruct::Vector<acs_stack_t> acs_stack_table_t;


// block analysis
extern SilentFeature DATA_BLOCK_FEATURE;
extern Identifier<Pair<int, BlockAccess *> > DATA_BLOCKS;
extern Identifier<const BlockCollection *> DATA_BLOCK_COLLECTION;
extern Identifier<Address> INITIAL_SP;

// MUST analysis
extern SilentFeature MUST_ACS_FEATURE;
extern SilentFeature PERS_ACS_FEATURE;
extern Identifier<acs_table_t *> MUST_ACS;
extern Identifier<acs_table_t *> ENTRY_MUST_ACS;
extern Identifier<acs_table_t *> PERS_ACS;
extern Identifier<acs_table_t *> ENTRY_PERS_ACS;
extern Identifier<acs_stack_table_t *> LEVEL_PERS_ACS;
extern Identifier<bool> DATA_PSEUDO_UNROLLING;
extern Identifier<data_fmlevel_t> DATA_FIRSTMISS_LEVEL;

// categories build
extern SilentFeature CATEGORY_FEATURE;
extern Identifier<cache::category_t> CATEGORY;
extern Identifier<BasicBlock*> CATEGORY_HEADER;

// ILP constraint build
extern SilentFeature CONSTRAINTS_FEATURE;
extern Identifier<ilp::Var *> MISS_VAR;

// MAY analysis
extern Identifier<Vector<ACS *> *> ENTRY_MAY_ACS;
extern SilentFeature ACS_MAY_FEATURE;
extern Identifier<Vector<ACS *> *> MAY_ACS;

} }		// otawa::dcache

#endif /* OTAWA_DCACHE_FEATURES_H_ */
