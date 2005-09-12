/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/cache_lblockset.cpp -- implementation of lblockset class.
 */

#include <otawa/cache/LBlockSet.h>

namespace otawa {


int LBlockSet::counter = 0;

Identifier LBlockSet::ID_LBlockSet("ipet.LBlockSet");

int LBlockSet::addLBLOCK (LBlock *node){
		listelbc.add(node);
		return cptr ++;		
	}


int LBlockSet::returnCOUNTER(void){
	return listelbc.length();
}
LBlock* LBlockSet::returnLBLOCK(int i){
		return listelbc[i];
}
bool LBlockSet::Iterator::ended(void) const {
	return pos >= lbs.length();
}
void LBlockSet::Iterator::next(void) {
	pos ++;
}

LBlock *LBlockSet::Iterator::item(void) const {
	return lbs[pos];
}
int LBlockSet::cacheline(void) {
	return linenumber;
}
} // otawa
