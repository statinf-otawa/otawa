/*
 *	otawa::Segment -- program segment class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-7, IRIT UPS.
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

#include <elm/assert.h>
#include <otawa/program.h>
#include <otawa/prog/Process.h>

// Configuration of the instruction map
#define MAP_BITS	6
#define MAP_MASK	((1 << MAP_BITS) - 1)
#define MAP_SIZE(s)	(((s) + MAP_MASK) >> MAP_BITS)
#define MAP_INDEX(a) (((a) - address()) >> MAP_BITS)
#define MAP_BASE(i)	 address_t(address() + ((i) << MAP_BITS))

namespace otawa {

/**
 * @class Segment
 * In usual file format like ELF, COFF and so on, the program file is
 * divided in segment according platform needs or memory propertes.
 * @par Usually, we find a ".text" segment containing program code,
 * ".data" containing initialized data, ".bss" containing uninitialized data,
 * ".rodata" containing read-only data. Yet, more segments may be available.
 * @ingroup prog
 */


/**
 * Build a segment with the given information.
 * @param name		Segment name in the file format (".text", ".data", ...).
 * @param address	Segment address in the process memory space.
 * @param size		Segment size (in bytes).
 * @param flags		As returned by the @ref flags() accessor.
 * 					A combination of @ref WRITABLE, @ref EXECUTABLE and @ref INITIALIZED .
 */
Segment::Segment(
	CString name,
	address_t address,
	ot::size size,
	flags_t flags)
:	_flags(flags),
	_name(name),
	_address(address),
	_size(size),
	map(new ProgItem *[MAP_SIZE(size)])
{
	// Removed : segment with 0 size seems to be normal
	// ASSERTP(size, "zero size segment");
	for(ot::size i = 0; i < MAP_SIZE(size); i++)
		map[i] = 0; 
}


/**
 * Protected destructor for avoiding implementation unexpected deletion.
 */
Segment::~Segment(void) {
	while(!items.isEmpty()) {
		ProgItem *item = (ProgItem *)items.first();
		items.removeFirst();
		delete item;
	}
	delete [] map;
}


/**
 * @fn CString Segment::name(void);
 * Get tne name of the segment.
 * @return Name of the segment.
 */

/**
 * @fn address_t Segment::address(void);
 * Get the base address of the segment.
 * @return Base address of the segment or 0 if no address has been assigned
 * to the segment.
 */
 
/**
 * @fn size_t Segment::size(void);
 * Get the size of the segment.
 * @return Size of the segment.
 */

/**
 * @fn int Segment::flags(void);
 * Get flag information about the segment. This flags are composed by OR'ing the constants
 * @ref EXECUTABLE, @ref WRITABLE and @ref INITIALIZED .
 * @return Flags value.
 */

/**
 * @fn bool Segment::isExecutable(void) const;
 * Test if the segment is executable.
 * @return	True if the segment is executable, false else.
 */

/**
 * @fn bool Segment::isWritable(void) const;
 * Test if the segment is writable.
 * @return	True if the segment is writable, false else.
 */


/**
 * @fn bool Segment::isInitialized(void) const;
 * Test if the segment is initialized.
 * @return	True if the segment is initialized, false else.
 */


/**
 * @var Segment::EXECUTABLE;
 * Segment flags denoting an executable segment.
 */


/**
 * @var Segment::WRITABLE;
 * Segment flags denoting a writable segment.
 */


/**
 * @var Segment::INITIALIZED;
 * Segment flags denoting an initialzied segment.
 */


/**
 * Find an instruction by its address.
 * @param addr	Address to find an instruction for.
 * @return		Found instruction or null.
 */
Inst *Segment::findInstAt(const Address& addr) {
	ProgItem *item = findItemAt(addr);
	if(!item) {
		Inst *inst = decode(addr);
		if(inst)
			insert(inst);
		return inst;
	}
	else
		return item->toInst();
}


/**
 * Find an item by its address.
 * @param addr	Address to find an instruction for.
 * @return		Found item or null.
 */
ProgItem *Segment::findItemAt(const Address& addr) {
	//cerr << "findItemAt(" << addr << ")\n";
	if(address().page() != addr.page()
	|| addr < address()
	|| addr >= topAddress())
		return 0;
	int index = MAP_INDEX(addr);
	ProgItem *item = map[index];
	if(item) {
		/*cerr << "index = " << index  << "(" << MAP_BASE(index) << ")->"
			 << (item ? item->address() : address_t(0)) << io::endl;*/
		while(item && item->address() <= addr) {
			//cerr << "> " << item->address() << "\n";
			if(item->address() == addr)
				return item;
			item = item->next();
		}
	}
	return 0;
}


/**
 * Decode the instruction at the given address. This method must overriden
 * by the ISA plugins to provide the actual decoding of the instruction.
 * This method returns null as the default.
 * @param address	Address of the instruction to decode.
 * @return			Decoded instruction or null if the address is invalid.
 */
Inst *Segment::decode(address_t address) {
	return 0;
}


/**
 * Insert the item in the list.
 * @param item	Item to insert.
 */
void Segment::insert(ProgItem *item) {
	ASSERTP(item->address() >= address() && item->address() < topAddress(),
		"attempt to insert an item at " << item->address()
		<< " with out-of-bound address [" << address() << ", " << topAddress() << ")");

	// compute map entry index
	int index = MAP_INDEX(item->address()), init = index;
	
	// find first used entry for insertion
	while(index > 0 && !map[index])
		index--;

	// find the insertion position
	ProgItem *cur = map[index];
	while(cur && cur->address() < item->address()) {
		ASSERT(cur->address() != item->address());
		cur = cur->next();
	}

	// end reached: add last
	if(!cur) {
		if(!map[index])
			items.addFirst(item);
		else
			items.addLast(item);
	}

	// perform insertion at this position
	else {
	
		// check memory overriding
		if(item->topAddress() > cur->address()) {
			throw DecodingException(_
				<< "instruction at " << item->address() << ':' << item->size()
				<< " in middle of instruction at " << cur->address() << ':' << cur->size()
				<< " before " << cur->previous()->address() << ':' << cur->previous()->size());
		}

		// insert the item
		cur->insertBefore(item);
	}

	// initialize the entry if needed
	if(!map[init] || cur == map[init])
		map[init]= item;

	ASSERT(!item->previous() || item->previous()->address() < item->address());
	ASSERT(!item->next() || item->address() < item->next()->address());
}

}; // namespace otawa
