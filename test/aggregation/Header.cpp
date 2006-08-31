/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	Header.cpp -- Header class implementation.
 */
#include "Header.h"

namespace otawa { namespace ipet {

/**
 * @author G. Cavaignac
 * @class Header
 * This class is an empty basic block, used as header.
 * A header have a child basic block. It is the header of this basic block
 */

/**
 * Constructs a new header of the given basic block
 */
Header::Header(BasicBlock *bb)
: _child(bb){
	_head = bb->head();
}

} } // otawa::ipet
