/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	prog/util_DFABitSet.cpp -- DFABitSet class implementation.
 */

#include <otawa/util/DFABitSet.h>

namespace otawa {


/**
 * @class DFABitSet
 * This class defines a set of values represented as a bit vector and usable
 * as a @ref DFASet in a DFA problem resolution.
 */


/**
 */
DFABitSet::~DFABitSet(void) {
}


/**
 * @fn DFABitSet::DFABitSet(int size);
 * Build a new set.
 * @param size	Size of the set.
 */


/**
 * @fn void DFABitSet::add(int index);
 * Add an item to the set using its index.
 * @param index	Index of the item to add.
 */


/**
 * @fn void DFABitSet::remove(int index);
 * Remove an item from the set using its index.
 * @param index	Index of the item to remove.
 */


/**
 * @fn bool DFABitSet::contains(int index);
 * Test if the item is contained in the set.
 * @param index	Index of the item.
 * @return		True if the item is in the set, false else.
 */


/**
 */
bool DFABitSet::equals(DFASet *set) {
	return vec.equals(((DFABitSet *)set)->vec);
}


/**
 */
void DFABitSet::add(DFASet *set) {
	vec.applyOr(((DFABitSet *)set)->vec);	
}


/**
 */
void DFABitSet::remove(DFASet *set) {
	vec.applyReset(((DFABitSet *)set)->vec);	
}


/**
 * Perform the output of a DFABitSet.
 */
elm::io::Output& operator<<(elm::io::Output& output, DFABitSet& bits) {
	output << "{";
	bool first = true;
	for(int i = 0; i < bits.size(); i++)
		if(bits.contains(i)) {
			if(first)
				first = false;
			else
				output << ", ";
			output << i;
		}
	output << "}";
	return output;
}

/**
 * this function is done by TAWk
 * it returns the number of the true bits in the vector
 */
 int DFABitSet::counttruebit(void){
 	int size = this->size();
 	int count = 0;	
	for (int i=0;i < size;i++){
		if (this->contains(i))
			count = count +1;
	}
	return count;	
 }

} // otawa
