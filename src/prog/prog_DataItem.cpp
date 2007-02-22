/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS.
 *
 *	DataItem class implementation
 */

#include <otawa/prog/DataItem.h>

using namespace elm;

namespace otawa {

/**
 * @class Data
 * This class represent a data item in the program. According information
 * available to Otawa, it may represent a block of anonymous data or 
 * may match some real variable from the underlying programming language.
 */

DataItem::DataItem(address_t address, Type *type, size_t size)
:	ProgItem(data, address, size)
{
}


/**
 * @fn Type *Data::type(void) const;
 * Get the type of the data block.
 * @return Type of the data block. Even when the type is not explicitly known,
 * a byte block type object is returned.
 */

}; // namespace otawa
