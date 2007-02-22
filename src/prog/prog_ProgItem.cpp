/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	prog/prog_ProgItem.cpp -- ProgItem class implementation.
 */

#include <otawa/prog/ProgItem.h>

namespace otawa {
	
/**
 * @class ProgObject
 * This is the base class of all objects representing program parts.
 * It provides facilities for storing and retrieving bound special properties.
 */

/**
 * @class ProgItem
 * Base class of the components of a program file segment.
 */


/**
 * Build a blank progam item.
 * @param address	Program item address.
 * @param size		Program item size.
 */
ProgItem::ProgItem(address_t address, size_t size)
:	_kind(blank),
	_address(address),
	_size(size)
{
	assert(size);
}


/**
 * Build a program item.
 * @param kind		Kind of program item.
 * @param address	Program item address.
 * @param size		Program item size.
 */
ProgItem::ProgItem(kind_t kind, address_t address, size_t size)
:	_kind(kind),
	_address(address),
	_size(size)
{
	assert(size);
}

/**
 * @fn ProgItem::~ProgItem(void);
 * Protected destructor for avoiding implementation unexpected deletion.
 */
ProgItem::~ProgItem(void) {
}
 
/**
 * @fn address_t ProgItem::address(void) const;
 * Get the address of the item if some has been assigned.
 * @return Address of the item or address 0 if none is assigned.
 * @note In workstation systems, it is commonly accepted that the address
 * 0 is ever invalid because it is the usual value of NULL in C. It should also
 * work the same for embedded systems.
 */
 
 /**
  * @fn size_t ProgItem::size(void) const;
  * Get the size of the item in bytes.
  * @return	Size of the item.
  */

/**
 * @fn CodeItem *ProgItem::toCode(void);
 *	Get the code program item if it is, null else.
 *	@return Code program item or null.
 */

/**
 * @fn DataItem *ProgItem::toData(void);
 *	Get the data program item if it is, null else.
 *	@return Data program item or null.
 */

/**
 * @fn bool ProgItem::isBlank(void);
 * Test if the code item is blank.
 * @return True if it is blank, false else.
 */

} // otawa
