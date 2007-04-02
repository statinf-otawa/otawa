/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	prog/prog_ProgItem.cpp -- ProgItem class implementation.
 */

#include <otawa/prog/ProgItem.h>

namespace otawa {
	
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
 * Get the next program item.
 * @return	Next program item, null at the end of the list.
 */
ProgItem *ProgItem::next(void) const {
	ProgItem *result = (ProgItem *)inhstruct::DLNode::next(); 
	if(result->atEnd())
		return 0;
	else
		return result;
}


/**
 * Get the previous program item.
 * @return	Previous program item, null at the begin of the list.
 */
ProgItem *ProgItem::previous(void) const {
	ProgItem *result = (ProgItem *)inhstruct::DLNode::previous(); 
	if(result->atBegin())
		return 0;
	else
		return result;
}


/**
 * @fn address_t ProgItem::address(void) const;
 * Get the address of the item .
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
 * @fn address_t ProgItem::topAddress(void) const;
 * Compute the address of the item immediately following the current item.
 * @return	Top address of the item.
 */


/**
 * Return the instruction matching the current item.
 * @return	Matching instruction or null if the current program item is not
 * an instruction.
 */
Inst *ProgItem::toInst(void) {
	return 0;
}

} // otawa
