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
 * This identifier (int) is used for storing the number of iteration for each loop
 * of the program.
 */
Identifier ETS::ID_LOOP_COUNT("ets.loopCount");

/**
 * Identifier of annotation (int) used for storing the WCET value
 * in the ETS of the computed function.
 */
Identifier ETS::ID_WCET("ets.wcet");

/**
 * This identifier (AbstractCacheState *) is used for storing the abstract cache line for each node.
 */
Identifier ETS::ID_ACS("ets.acs");

/**
 * This identifier (int) is used for storing the number of hit accesses for each node.
 */
Identifier ETS::ID_HITS("ets.hits");

/**
 * This identifier (int) is used for storing the number of miss accesses for each node.
 */
Identifier ETS::ID_MISSES("ets.misses");

/**
 * This identifier (int) is used for storing the number of first miss accesses for each node.
 */
Identifier ETS::ID_FIRST_MISSES("ets.firstMisses");

/**
 * This identifier (int) is used for storing the number of conflict accesses for each node.
 */
Identifier ETS::ID_CONFLICTS("ets.conflicts");

} }// otawa::ets
