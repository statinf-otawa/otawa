/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	otawa::Segment class implementation
 */

#include <otawa/program.h>

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
	_flags(flags)
{
	assert(size);
	items.addFirst(new ProgItem(address, size));
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
Inst *Segment::findByAddress(address_t addr) {
	for(ItemIter item(this); item; item++) {
		CodeItem *code = item->toCode();
		if(code) {
			Inst *inst = code->findByAddress(addr);
			if(inst)
				return inst;
		}
	}
	return 0;
}

/**
 * Replace the current item by the given list of sub-items.
 * @param old_item	Item to split.
 * @param new_items	List of items to replace.
 * @warning Current item and list must occupy the same memory range.
 */
void Segment::splitItem(ProgItem *old_item, inhstruct::DLList& new_items) {
	assert(!new_items.isEmpty());
	assert(((ProgItem *)new_items.first())->address() == old_item->address());
	assert(((ProgItem *)new_items.first())->address()
		+ ((ProgItem *)new_items.last())->size()
		== old_item->address() + old_item->size());
	
	// Add all replacers
	ProgItem *item = (ProgItem *)new_items.first();
	while(item->atEnd()) {
		ProgItem *next = (ProgItem *)item->next();
		item->remove();
		old_item->insertBefore(item);
		item = next;
	}
	
	// Remove itself
	old_item->remove();
	delete old_item; 
}


/**
 * Find an item containing the given address.
 * @param addr	Address of the item to find.
 * @return		Matching item or null.
 */ 
ProgItem *Segment::findItemByAddress(address_t addr) {
	for(ItemIter item(this); item; item++)
		if(addr >= item->address() && addr < item->topAddress())
			return item;
	return 0; 
}


/**
 * Put an item in the segment. This function may be used to replace a blank
 * item by an identified one. This function fails if the item makes a conflict
 * with existing items.
 * @param new_item	Item to put.
 * @param old_item	Item to put in.
 */
void Segment::putItem(ProgItem *new_item, ProgItem *old_item) {
	assert(new_item);
	
	// Find old item if required
	if(!old_item) {
		old_item = findItemByAddress(new_item->address());
		assert(old_item);
	}
	assert(old_item->isBlank());

	// Blank before
	if(old_item->address() <  new_item->address()) {
		size_t rem_size = old_item->size();
		old_item->_size = new_item->address() - old_item->address();
		old_item->insertAfter(new_item);
		rem_size -= old_item->size() + new_item->size();
		if(rem_size > 0)
			new_item->insertAfter(new ProgItem(new_item->topAddress(), rem_size));
	}
	
	// Blank after only
	else if(old_item->size() < new_item->size()) {
		old_item->_size = old_item->size() - new_item->size();
		old_item->_address = old_item->address() + new_item->size();
		old_item->insertBefore(new_item); 
	}
	
	// Simple replacement
	else {
		assert(old_item->size() == new_item->size());
		old_item->insertAfter(new_item);
		old_item->remove();
		delete old_item;
	}
}

}; // namespace otawa
