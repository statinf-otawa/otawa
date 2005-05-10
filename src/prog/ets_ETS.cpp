/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/ets_ETS.cpp -- ETS class implementation.
 */

 #include <otawa/ets/ETS.h>
 
namespace otawa { namespace ets {

/**
 * @class ETS
 * This static class is used for storing ressources (property identifiers)
 * used by ETS processors.
 */


/**
 * This identifier is used for storing the number of iteration for each loop
 * of the program.
 */
Identifier ETS::ID_LOOP_COUNT("ets.loopCount");


/**
 * Identifier of annotation used for storing the WCET value (int)
 * in the ETS of the computed function.
 */
Identifier ETS::ID_WCET("ets.wcet");

} }// otawa::ets
