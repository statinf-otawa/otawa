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
 * @fn ProgItem::~ProgItem(void);
 * Protected destructor for avoiding implementation unexpected deletion.
 */
ProgItem::~ProgItem(void) {
}
 
/**
 * @fn CString ProgItem::name(void);
 * Get the name of the program item if some is defined. It may be the name
 * of a function for a piece of code or the name of a data.
 * @return Name ofthis item or an empty string else.
 */

/**
 * @fn address_t ProgItem::address(void);
 * Get the address of the item if some has been assigned.
 * @return Address of the item or address 0 if none is assigned.
 * @note In workstation systems, it is commonly accepted that the address
 * 0 is ever invalid because it is the usual value of NULL in C. It should also
 * work the same for embedded systems.
 */
 
 /**
  * @fn size_t ProgItem::size(void);
  * Get the size of the item in bytes.
  * @return	Size of the item.
  */

/**
 *	@fn Code *ProgItem::toCode(void);
 *	Get the code program item if it is, null else.
 *	@return Code program item or null.
 */
CodeItem *ProgItem::toCode(void) {
	return 0;
}

/**
 *	@fn Data *ProgItem::toData(void);
 *	Get the data program item if it is, null else.
 *	@return Data program item or null.
 */
Data *ProgItem::toData(void) {
	return 0;
}

} // otawa
