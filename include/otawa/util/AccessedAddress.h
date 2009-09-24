/*
 * AccessedAddress.h
 *
 *  Created on: 2 juil. 2009
 *      Author: casse
 */

#ifndef ACCESSEDADDRESS_H_
#define ACCESSEDADDRESS_H_

#include <otawa/base.h>
#include <otawa/prop/Identifier.h>
#include <otawa/proc/Feature.h>

namespace otawa {

class Inst;

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
	inline SPAddress(Inst *instruction, bool is_store, long offset)
		: AccessedAddress(instruction, is_store, SP), off(offset) { }
	inline long offset(void) const { return off; }

private:
	long off;
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


class AccessedAddresses {
public:
	AccessedAddresses(): _size(0) { }
	inline ~AccessedAddresses(void) { clear(); }

	template <class C>
	void set(const C& coll) {
		clear();
		_size = coll.count();
		addrs = new AccessedAddress *[_size];
		int i = 0;
		for(typename C::Iterator aa(coll); aa; aa++, i++)
			addrs[i] = aa;
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

extern Feature<NoProcessor> ADDRESS_ANALYSIS_FEATURE;
extern Identifier<AccessedAddresses *> ADDRESSES;

}	//otawa

#endif /* ACCESSEDADDRESS_H_ */
