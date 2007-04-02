/*
 *	$Id$
 *	Copyright (c) 2003-07, IRIT UPS.
 *
 *	otawa::Segment class implementation
 */

#include <elm/assert.h>
#include <otawa/program.h>

// Configuration of the instruction map
#define MAP_BITS	6
#define MAP_MASK	((1 << MAP_BITS) - 1)
#define MAP_SIZE(s)	(((s) + MAP_MASK - 1) >> MAP_BITS)
#define MAP_INDEX(a) (((a) - address()) >> MAP_BITS)

namespace otawa {

/**
 /**
 * @class Segment
 * @par In usual file format like ELF, COFF and so on, the program file is
 * divided in segment according platform needs or memory propertes.
 * @par Usually, we find a ".text" segment containing program code,
 * ".data" containing initialized data, ".bss" containing uninitialized data,
 * ".rodata" containing read-only data. Yet, more segments may be available.
 */


/**
 * Build a segment with the given information.
 * @param name		Segment name in the file format (".text", ".data", ...).
 * @param address	Segment address in the process memory space.
 * @param size		Segment size (in bytes).
 * @param flags		As returned by the @ref flags() accessor.
 */
Segment::Segment(
	CString name,
	address_t address,
	size_t size,
	unsigned long flags)
:	_name(name),
	_address(address),
	_size(size),
	_flags(flags),
	map(new ProgItem *[MAP_SIZE(size)])
{
	ASSERTP(size, "zero size segment");
	for(int i = 0; i < MAP_SIZE(size); i++)
		map[i] = 0; 
}


/**
 * Protected destructor for avoiding implementation unexpected deletion.
 */
Segment::~Segment(void) {
	while(!items.isEmpty()) {
		delete items.first();
		items.removeFirst();
	}
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
 * EXECUTABLE and WRITABLE.
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
 * Find an instruction by its address.
 * @param addr	Address to find an instruction for.
 * @return		Found instruction or null.
 */
Inst *Segment::findInstAt(address_t addr) {
	ProgItem *item = findItemAt(addr);
	if(!item) {
		Inst *inst = decode(addr);
		if(inst)
			insert(inst);
		return inst;
	}
	else {
		Inst *inst = item->toInst();
		while(inst && inst->isPseudo())
			inst = inst->nextInst();
		return inst;
	}
}


/**
 * Find an item by its address.
 * @param addr	Address to find an instruction for.
 * @return		Found item or null.
 */
ProgItem *Segment::findItemAt(address_t addr) {

	// In the segment ?
	if(addr < address() || addr >= address() + size())
		return 0;
	
	// Look in the instruction
	ProgItem *item = map[MAP_INDEX(addr)];
	if(item)
		while(item && item->address() <= addr) {
			if(item->address() == addr)
				return item;
			item = item->next();
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
		"attempt to insert an item with out-of-bound address");
	//cout << "Inserting item at " << item->address() << io::endl;
	int index = MAP_INDEX(item->address());
	
	// Empty map entry
	if(!map[index]) {
		map[index] = item;
		if(items.isEmpty()) {
			items.addFirst(item);
			return;
		}
		else {
			do
				index--;
			while(index >= 0 && !map[index]);
			if(index < 0) {
				items.addFirst(item);
				return;
			}
		}
	}
	
	// Find the position
	ProgItem *cur = map[index];
	while(cur && cur->address() < item->address())
		cur = cur->next();
	if(!cur)
		items.addLast(item);
	else {
		assert(item->topAddress() <= cur->address());
		cur->insertBefore(item);
	} 
}

}; // namespace otawa
