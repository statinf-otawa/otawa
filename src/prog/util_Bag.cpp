/*
 *	otawa::Bag class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2014, IRIT UPS.
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

#include <otawa/util/Bag.h>

namespace otawa {

/**
 * @class Bag
 * @param T		Type of stored items.
 * This light parametric class is dedicated to the storage of small sets
 * of simple items hooked to a property. It may be used as a normal array
 * but the stored objects will be deleted when the property is removed.
 *
 * Normal work of the Bag class is to perform a copy of the arguments passed
 * to the constructor and it will manage the new allocated array.
 * If a transfer of array is needed (the array has been allocated by another class
 * but the responsibility for releasing the array is transfered to the Bag),
 * the array must be passed through a small @ref Give object. @ref Give objects
 * may be built from a pair (array count, array pointer) but also from
 * a @ref elm::genstruct::Vector (in this case, the array is detached).
 *
 * @ingroup props
 */

/**
 * @fn Bag::Bag(void);
 * Empty array constructor.
 */

/**
 * @fn Bag::Bag(const Bag& bag);
 * Clone constructor.
 * @param bag	Bag to clone.
 */

/**
 * @fn Bag::Bag(int c, const T *a);
 * Constructor using a raw array.
 * @param c		Count of items in the array.
 * @param a		Base pointer of the array.
 */

/**
 * @fn Bag::Bag(int c, T *a);
 * Constructor using a raw array.
 * @param c		Count of items in the array.
 * @param a		Base pointer of the array.
 */

/**
 * @fn Bag::Bag(const genstruct::Vector<T>& v);
 * Constructor from a vector.
 * @param v		Vector to copy items from.
 */

/**
 * @fn Bag::Bag(Pair<int, T *> p);
 * Construction from a pair (array count, array base).
 * @param p		Pair to build from.
 */

/**
 * @fn Bag::Bag(const Give<T>& g);
 * Construction from a given array. The current Bag object becomes responsible
 * for freeing the array passed in g.
 * @param g		Given array.
 */

/**
 * @fn int Bag::count(void) const;
 * Get the count of items in the bag.
 * @return	Count of items.
 */

/**
 * @fn int Bag::size(void) const;
 * Same as @ref count().
 */


/**
 * @fn const T& Bag::get(int i) const;
 * Get the ith item of the bag.
 * @param i		Index of the looked item.
 * @return		ith item.
 */

/**
 * @fn T& Bag::get(int i);
 * Get the ith item of the bag.
 * @param i		Index of the looked item.
 * @return		ith item.
 */

/**
 * @fn void Bag::clear(void);
 * Clear the content of the bag. After this call, the bag is empty.
 */

/**
 * @fn bool Bag::isEmpty(void) const;
 * Test if the bag is empty.
 * @return	True if the bag is empty, false else.
 */

}	// otawa
