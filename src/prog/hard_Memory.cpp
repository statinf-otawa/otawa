/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	src/prog/hard_Memory.h -- Memory class implementation.
 */

#include <elm/util/VarArg.h>
#include <otawa/hard/Memory.h>

using namespace elm;
using namespace elm::genstruct;

namespace otawa { namespace hard {

/**
 * @class AddressSegment
 * This class represents a range of addresses in an address space that shares
 * common features.
 */


/**
 * Build a new address segment.
 * @param address	Base address of the segement.
 * @param size		Size of the segment (in bytes).
 * @param flags		Some flags for accessing the segement (combination of
 * 					IO, READABLE, WRITABLE and CACHEABLE). As default, it is
 * 					readable and cacheable.
 * @param read_time		Read time in the segment in cycles (default 10).
 * @param write_time	Write time in the segement in cycles (default equal to
 * 						read time).
 */
AddressSegment::AddressSegment(address_t address, size_t size, int _flags,
int read_time, int write_time): addr(address), _size(size), flags(_flags),
rtime(read_time), wtime(write_time < 0 ? read_time : write_time) {
	assert(read_time >= 0);
}


/**
 * @fn address_t AddressSegment::address(void) const;
 * Get the base address of the segment.
 * @return Segment base address.
 */


/**
 * @fn size_t AddressSegment::size(void) const;
 * Get the size of the segment.
 * @return Segment size.
 */


/**
 * @fn bool AddressSegment::isReadable(void) const;
 * Test if the segement is readable.
 * @return True if the segment is readable, false else.
 */


/**
 * @fn bool AddressSegment::isWritable(void) const;
 * Test if the segement is writable.
 * @return True if the segment is writable, false else.
 */


/**
 * @fn bool AddressSegment::isCacheable(void) const;
 * Test if the segement is cacheable.
 * @return True if the segment is cacheable, false else.
 */


/**
 * @fn bool AddressSegment::isIO(void) const;
 * Test if the segement is mapped to input/output registers.
 * @return True if the segment is mapped to I/O, false else.
 */


/**
 * @fn int AddressSegment::readTime(void) const;
 * Get the read time in cycle for this segement.
 * @return	Read time in cycles.
 */


/**
 * @fn int AddressSegment::writeTime(void) const;
 * Get the write time in cycle for this segement.
 * @return	Write time in cycles.
 */


/**
 * @fn bool AddressSegment::contains(address_t address) const;
 * Test if an address is contained in the segement.
 * @param address	Address to check.
 * @return			True if it is contained, false else.
 */


/**
 * @class AddressSpace
 * This class represents a full address space composed by different segements
 * with differents features.
 * @par
 * Many processor have a unique address space, we call it generally "main".
 * Processors with seperated address space for IO, x86 family for example, may
 * have another one called "io". There are some processors that separates code
 * and data in two address space (AVR family for example). In this case, two
 * address spaces are provided and may be named "code" and "data".
 */


/**
 * Build an address space from its name and a list of address segments.
 * @param name		Address space name.
 * @param segments	Null-ended table of address segment pointers.
 */
AddressSpace::AddressSpace(elm::CString name, AddressSegment *segments[])
: _name(name) {
	int cnt = 0;
	while(segments[cnt])
		cnt++;
	segs = Table<AddressSegment *>(segments, cnt);
}


/**
 * @fn const elm::CString AddressSpace::name(void) const;
 * Get the name of the address space.
 * @return Address space name.
 */


/**
 * @fn const elm::genstruct::Table<AddressSegment *>& AddressSpace::segments(void) const;
 * Get the space segments.
 * @return	Space segements.
 */


/**
 * Get a segment containing the given address.
 * @param addr	Address to look for.
 * @return		Container segment or null if none matches.
 */
AddressSegment *AddressSpace::segmentOf(address_t addr) const {
	for(int i = 0; i < segs.count(); i++)
		if(segs[i]->contains(addr))
			return segs[i];
	return 0;
}


/**
 * @class Memory
 * This class represents the full description of a memory of a platform.
 */
 

/**
 * Build a memory.
 * @param spaces	Null-ended table of address spaces.
 */
Memory::Memory(AddressSpace *spaces[]) {
	int cnt = 0;
	while(spaces[cnt])
		cnt++;
	_spaces = Table<AddressSpace *>(spaces, cnt);
}


/* regular data */
static AddressSegment regular_seg(0, 0xffffffff, READABLE | WRITABLE | CACHEABLE);
static AddressSegment *regular_segs[] = { &regular_seg, 0 };
static AddressSpace regular_space("main", regular_segs);
static AddressSpace *regular_spaces[] = { &regular_space, 0 };


/**
 * This simple memory provides a unique address space containing a unique
 * segement readable, writable and cacheable.
 */
Memory Memory::regular(regular_spaces);
	
/*
 * @fn const elm::genstruct::Table<AddressSpace *>& spaces(void) const;
 * Return the address spaces in this memory.
 * @return	Address spaces.
 */


/**
 * Return the space matching the given name.
 * @param name	Looked space name.
 * @return		Matching address space or null if not found.
 */
AddressSpace *Memory::spaceOf(elm::CString name) const {
	for(int i = 0; i < _spaces.count(); i++)
		if(_spaces[i]->name() == name)
			return _spaces[i];
	return 0;
}


/**
 * Find the segment in the named address space mathcing the given address.
 * @param space	Space to look in.
 * @param name	Address to look for.
 * @return Matching segment or null.
 */
AddressSegment *Memory::segmentOf(elm::CString name, address_t addr) const {
	AddressSpace *space = spaceOf(name);
	if(!space)
		return 0;
	else
		return space->segmentOf(addr);
}

} } // otawa::hard

