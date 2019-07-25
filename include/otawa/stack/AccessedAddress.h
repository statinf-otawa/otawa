/*
 *	AccessAddress and AccessesAddresses classes interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009, IRIT UPS.
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
#ifndef OTAWA_ACCESSEDADDRESS_H_
#define OTAWA_ACCESSEDADDRESS_H_

#include <otawa/base.h>
#include <otawa/prop/Identifier.h>
#include <otawa/proc/Feature.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/stack/features.h>

namespace otawa {

class Inst;

typedef struct address_stat_t {
	int all, sprel, abs, total;
} address_stat_t;

class AccessedAddress {
public:
	typedef enum {
		ANY,
		SP,
		ABS
	} kind_t;

	inline AccessedAddress(Inst *instruction, bool is_store, kind_t kind = ANY)
		: inst(instruction), store(is_store), knd(kind) { }
	inline kind_t kind(void) const { return (kind_t )knd; }
	inline bool isStore(void) const { return store; }
	inline bool isLoad(void) const { return !store; }
	inline Inst *instruction(void) const { return inst; }

	void print(io::Output& out) const;

private:
	Inst *inst;
	unsigned char store;
	unsigned char knd;
};


class SPAddress: public AccessedAddress {
public:
	inline SPAddress(Inst *instruction, bool is_store, t::int32 offset)
		: AccessedAddress(instruction, is_store, SP), off(offset) { }
	inline t::int32 offset(void) const { return off; }

private:
	t::int32 off;
};


class AbsAddress: public AccessedAddress {
public:
	inline AbsAddress(Inst *instruction, bool is_store, const Address& address)
		: AccessedAddress(instruction, is_store, ABS), addr(address) { }
	inline const Address& address(void) const { return addr; }

private:
	Address addr;
};
inline io::Output& operator<< (io::Output& out, const AccessedAddress *addr) { addr->print(out); return out; }


// AccessedAddresses class
class AccessedAddresses {
public:
	AccessedAddresses(): _size(0), addrs(0) { }
	inline ~AccessedAddresses(void) { clear(); }

	template <class C>
	void set(const C& coll) {
		clear();
		_size = coll.count();
		addrs = new AccessedAddress *[_size];
		int i = 0;
		for(typename C::Iter aa(coll); aa(); aa++, i++)
			addrs[i] = *aa;
	}

	void clear(void) {
		if(_size) {
			for(int i = 0; i < _size; i++)
				delete addrs[i];
			delete [] addrs;
			_size = 0;
		}
	}

	inline int size(void) const { return _size; }
	inline AccessedAddress *get(int index) { ASSERT(index < _size); return addrs[index]; }

	void print(io::Output& out) const {
		for(int i = 0; i < _size; i++)
			addrs[i]->print(out);
	}

private:
	int _size;
	AccessedAddress **addrs;
};
inline io::Output& operator<<(io::Output& out, const AccessedAddresses *aa) { aa->print(out); return out; }
inline io::Output& operator<<(io::Output& out, AccessedAddresses *aa) { aa->print(out); return out; }


// AccessedAddressFromStack class
class AccessedAddressFromStack: public BBProcessor {
public:
	static Identifier<bool> DISPLAY;

	static p::declare reg;
	AccessedAddressFromStack(p::declare& r = reg);
	virtual void configure(const PropList& props);

protected:
	virtual void setup (WorkSpace *fw);
	virtual void cleanup (WorkSpace *fw);
	virtual void processBB (WorkSpace *fw, CFG *cfd, Block *bb);

private:
	address_stat_t istats, *stats;
	bool display;
};

io::Output& operator<<(io::Output& out, address_stat_t *s);

extern p::feature ADDRESS_ANALYSIS_FEATURE;
extern Identifier<AccessedAddresses *> ADDRESSES;
extern Identifier<address_stat_t *> ADDRESS_STATS;

}	//otawa

#endif /* OTAWA_ACCESSEDADDRESS_H_ */

