/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	otawa/hard/Memory.h -- Memory class interface.
 */
#ifndef OTAWA_HARD_MEMORY_H
#define OTAWA_HARD_MEMORY_H

#include <assert.h>
#include <elm/genstruct/Table.h>
#include <elm/string.h>
#include <otawa/base.h>

namespace otawa { namespace hard {

// Memory flags
const int 	IO = 0x0001,
			READABLE = 0x0002,
			WRITABLE = 0x0004,
			CACHEABLE = 0x0008;

// AddressSegment class
class AddressSegment {
public:
private:
	int flags;
	int rtime, wtime;
	address_t addr;
	size_t _size;
public:
	AddressSegment(address_t address, size_t size,
		int flags = READABLE | CACHEABLE, int read_time = 10, int write_time = -1);
	
	// Accessors
	inline address_t address(void) const;
	inline size_t size(void) const;
	inline bool isReadable(void) const;
	inline bool isWritable(void) const;
	inline bool isCacheable(void) const;
	inline bool isIO(void) const;
	inline int readTime(void) const;
	inline int writeTime(void) const;
	inline bool contains(address_t addr) const;
};

// AddressSpace class
class AddressSpace {
	elm::CString _name;
	elm::genstruct::Table<AddressSegment *> segs;
public:
	AddressSpace(elm::CString name, AddressSegment *segments[]);
	
	// Accessors
	inline const elm::CString name(void) const;
	inline const elm::genstruct::Table<AddressSegment *>& segments(void) const;
	AddressSegment *segmentOf(address_t addr) const;
};


// Memory class
class Memory {
	elm::genstruct::AllocatedTable<AddressSpace *> _spaces;
public:
	Memory(AddressSpace *space[]);
	static Memory regular;
	
	// Accessors
	inline const elm::genstruct::Table<AddressSpace *>& spaces(void) const;
	AddressSpace *spaceOf(elm::CString name) const;
	AddressSegment *segmentOf(elm::CString space, address_t addr) const;
};


// AddressSegment inlines
inline address_t AddressSegment::address(void) const {
	return addr;
}

inline size_t AddressSegment::size(void) const {
	return _size;
}

inline bool AddressSegment::isReadable(void) const {
	return flags & READABLE;
}

inline bool AddressSegment::isWritable(void) const {
	return flags & WRITABLE;
}

inline bool AddressSegment::isCacheable(void) const {
	return flags & CACHEABLE;
}

inline bool AddressSegment::isIO(void) const {
	return flags & IO;
}

inline int AddressSegment::readTime(void) const {
	return rtime;
}

inline int AddressSegment::writeTime(void) const {
	return wtime;
}

inline bool AddressSegment::contains(address_t address) const {
	return address >= addr && address < addr + _size;
}

// AddressSpace inlines
inline const elm::CString AddressSpace::name(void) const {
	return _name;
}

inline const elm::genstruct::Table<AddressSegment *>& AddressSpace::segments(void) const {
	return segs;
}

} } // otawa::hard

#endif // OTAWA_HARD_MEMORY_H_
