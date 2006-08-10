/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	prog/util_BitSet.h -- BitSet class implementation.
 */

#include <otawa/util/BitSet.h>

namespace otawa {

/**
 * @class BitSet otawa/util/BitSet.h
 * This class implements a set as a bit vector. You must assign each
 * object of the set to an index in the bit vector and use it to process
 * sets containing it.
 */


/**
 * @fn BitSet::BitSet(const elm::BitVector& ivec);
 * @internal Buid a bit set from a @ref elm::BitVector.
 */


/**
 * @fn BitSet::BitSet(int size);
 * Build a bitset with the given size.
 * @param size	Maximum set item count.
 */


/**
 * @fn BitSet::BitSet(const BitSet& set);
 * Build a bit set from by copying an existing one.
 * @param set	Set to copy.
 */


/**
 * @fn bool BitSet::isEmpty(void) const;
 * Test if the bit set is empty.
 * @return	True if it is empty, false else.
 */


/**
 * @fn bool BitSet::isFull(void) const;
 * Test if the bit set contains all existing items.
 * @return	True if it is full, false else.
 */


/**
 * @fn void BitSet::fill(void);
 * Fill the bit set with all items.
 */


/**
 * @fn void BitSet::empty(void);
 * Remove all items from the bit set.
 */


/**
 * @fn bool BitSet::contains(int index) const;
 * Test if the bit set contains an item.
 * @param index	Index identifying the looked item.
 * @return	True if the item is in the bit set, false else.
 */


/**
 * @fn void BitSet::add(int index);
 * Add an item to the bit set.
 * @param index	Index identifying the added item.
 */


/**
 * @fn void BitSet::remove(int index);
 * Remove an item from the bit set.
 * @param index	Index identifying the removed item.
 */

/**
 * @fn bool BitSet::equals(const BitSet& set) const;
 * Test if two bit sets are equals.
 * @param set	Bit set to compare the current one with.
 * @return		True if they are equal, false else.
 */


/**
 * @fn bool BitSet::includes(const BitSet& set) const;
 * Test if the current bit set contains the given one.
 * @param set	Set to test for inclusion.
 * @return		True if the current bit set contains the given one.
 */


/**
 * @fn bool BitSet::includesStrictly(const BitSet& set) const;
 * Test if the current bit set contains strictly the given one, that is,
 * both bit sets are equals.
 * @param set	Set to test for inclusion.
 * @return		True if the current bit set contains the given one.
 */


/**
 * @fn void BitSet::add(const BitSet& set);
 * Add the items of the given bit set to the current one.
 * @param set	Set to add.
 */


/**
 * @fn void BitSet::remove(const BitSet& set);
 * Remove the items contained in the given bit set from the current one.
 * @param set	Set containing items to remove.
 */


/**
 * @fn void BitSet::mask(const BitSet& set);
 * Keep only in this set the items also containted in the given one
 * (performs intersection operation).
 * @param set	Set to intersect with.
 */


/**
 * @fn BitSet BitSet::doUnion(const BitSet& set) const;
 * Build a new bit set as union between the current and the given one.
 * @param set	Set to do union with.
 * @return		Union of both sets.
 */


/**
 * @fn BitSet BitSet::doInter(const BitSet& set) const;
 * Build a new bit set as intersection between the current and the given one.
 * @param set	Set to do intersection with.
 * @return		Intersection of both sets.
 */


/**
 * @fn BitSet BitSet::doDiff(const BitSet& set) const;
 * Build a new bit set as difference between the current and the given one.
 * @param set	Set to do difference with.
 * @return		Difference of both sets.
 */


/**
 * @fn BitSet& BitSet::operator=(const BitSet& set);
 * Assignment with bit sets.
 * @param set	Set to assign.
 */


/**
 * @fn BitSet& BitSet::operator+=(int index);
 * Same as @ref add(int).
 */


/**
 * @fn BitSet& BitSet::operator+=(const BitSet& set);
 * Same as @ref add(const BitSet&).
 */


/**
 * @fn BitSet& BitSet::operator-=(int index);
 * Same as @ref remove(int).
 */

/**
 * @fn BitSet& BitSet::operator-=(const BitSet& set);
 * Same as @ref remove(const BitSet&).
 */


/**
 * @fn BitSet BitSet::operator+(const BitSet& set);
 * Same as @ref doUnion().
 */


/**
 * @fn BitSet BitSet::operator-(const BitSet& set);
 * Same as @ref doDiff().
 */


/**
 * @fn BitSet& BitSet::operator|=(const BitSet& set);
 * Same as @ref add(const BitSet&).
 */
 

/**
 * @fn BitSet& BitSet::operator&=(const BitSet& set);
 * Same as @ref remove(const BitSet&).
 */


/**
 * @fn BitSet BitSet::operator|(const BitSet& set);
 * Same as @ref doUnion().
 */


/**
 * @fn BitSet BitSet::operator&(const BitSet& set);
 * Same as @ref doInter().
 */


/**
 * @fn bool BitSet::operator==(const BitSet& set) const;
 * Same as @ref equals().
 */


/**
 * @fn bool BitSet::operator!=(const BitSet& set) const;
 * Same as !@ref equals().
 */


/**
 * @fn bool BitSet::operator>=(const BitSet& set) const;
 * Same as @ref includes().
 */


/**
 * @fn bool BitSet::operator<=(const BitSet& set) const;
 * Same as !@ref includesStrictly().
 */


/**
 * @fn bool BitSet::operator>(const BitSet& set) const;
 * Same as @ref includesStrictly().
 */


/**
 * @fn bool BitSet::operator<(const BitSet& set) const;
 * Same as !@ref includes().
 */


/**
 * @fn void BitSet::complement(void);
 * Complement the current bit set.
 */


/**
 * @fn BitSet BitSet::doComp(void) const;
 * Compute a bit set complement of the current one.
 * @return	Complemented bit set.
 */


/**
 * @class BitSet::Iterator
 * Iterator on the item contained in a bit set.
 */


/**
 * @fn BitSet::Iterator::Iterator(const BitSet& set);
 * Build an iterator on the items of the given bit set.
 * @param set	Set to iterate on items of.
 */

} // otawa
