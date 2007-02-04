/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/cache_lblockset.cpp -- implementation of lblockset class.
 */

#include <otawa/cache/LBlockSet.h>

namespace otawa {

//int LBlockSet::counter = 0;

/**
 * This property is used for storing the list of L-Blocks. The type of its
 * date is LBlockSet *[] with a size equal to the line count of the instruction
 * cache.
 */
Identifier<LBlockSet **> LBLOCKS("lblocks", 0, otawa::NS);

/**
 */
int LBlockSet::add(LBlock *node){
	int id = listelbc.length();
	listelbc.add(node);
	return id;
}

/**
 */
int LBlockSet::count(void){
	return listelbc.length();
}

/**
 */
LBlock *LBlockSet::lblock(int i) {
		return listelbc[i];
}

/**
 */
int LBlockSet::line(void) {
	return linenumber;
}

} // otawa
