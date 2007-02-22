/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS.
 *
 *	DataItem class interface
 */
#ifndef OTAWA_PROG_DATA_ITEM_H
#define OTAWA_PROG_DATA_ITEM_H

#include <otawa/prog/ProgItem.h>

namespace otawa {

// Extern classes
class Type;

// Data class
class DataItem: public ProgItem {
public:
	inline Type *type(void) const { return _type; }

protected:
	DataItem(address_t address, Type *type, size_t size = 0);

private:
	Type *_type;
};

} // otawa

#endif // OTAWA_PROG_DATA_ITEM_H
