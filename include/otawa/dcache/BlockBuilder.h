/*
 * DataBlockBuilder.h
 *
 *  Created on: 9 juil. 2009
 *      Author: casse
 */

#ifndef DATABLOCKBUILDER_H_
#define DATABLOCKBUILDER_H_

#include <otawa/prog/Inst.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/proc/Feature.h>

namespace otawa {

namespace hard {
	class Cache;
	class Memory;
}

// Block class
class Block {
public:
	inline Block(void): _set(-1), idx(-1)  { }
	inline Block(int set, int index, const Address& address): _set(set), idx(index), addr(address) { }
	inline Block(const Block& block): _set(block._set), idx(block.idx), addr(block.addr) { }
	inline int set(void) const { return _set; }
	inline int index(void) const { return idx; }
	inline const Address& address(void) const { return addr; }

	inline void print(io::Output& out) const {
		if(_set == -1)
			out << "ANY";
		else
			out << addr << " (" << idx << ", " << _set << ")";
	}

private:
	int _set;
	int idx;
	Address addr;
};
inline io::Output& operator<<(io::Output& out, const Block& block) { block.print(out); return out; }


// BlockCollection class
class BlockCollection {
public:
	const Block& get(int set, const Address& addr) {
		for(int i = 0; i < blocks.count(); i++)
			if(addr == blocks[i].address())
				return blocks[i];
		blocks.add(Block(set, blocks.count(), addr));
		return blocks[blocks.count() - 1];
	}
	inline void setSet(int set) { _set = set; }

	inline int count(void) const { return blocks.count(); }
	inline int set(void) const { return _set; }

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

	inline void print(io::Output& out) const {
		out << inst << " access ";
		switch(_kind) {
		case ANY: out << "ANY"; break;
		case BLOCK: out << *data.blk; break;
		case RANGE: out << '[' << data.range.first << ", " << data.range.last << ']'; break;
		}
	}

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


// BlockBuilder class
class BlockBuilder: public BBProcessor {
public:
	BlockBuilder(void);
	virtual void configure(const PropList &props);

protected:
	virtual void setup(WorkSpace *ws);
	virtual void processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb);

private:
	const hard::Cache *cache;
	const hard::Memory *mem;
	genstruct::Vector<BlockAccess> blocks;
	Address sp;
	BlockCollection *colls;
};

// features
extern Feature<BlockBuilder> DATA_BLOCK_FEATURE;
extern Identifier<Pair<int, BlockAccess *> > DATA_BLOCKS;
extern Identifier<const BlockCollection *> DATA_BLOCK_COLLECTION;

// configuration
extern Identifier<Address> INITIAL_SP;;

} // otawa

#endif /* DATABLOCKBUILDER_H_ */
