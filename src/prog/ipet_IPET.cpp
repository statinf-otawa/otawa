/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/ipet_IPET.cpp -- IPET class implementation.
 */

 #include <otawa/ipet/IPET.h>
 
namespace otawa {

/**
 * @class IPET
 * This static class is used for storing ressources (property identifiers)
 * used by IPET processors.
 */


/**
 * This identifier is used for storing the time of execution in cycle of a
 * basic block.
 */
Identifier IPET::ID_BB_Time("ipet.bb_time");

} // otawa
