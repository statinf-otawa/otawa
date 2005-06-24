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
 * This identifier is used for storing the time of execution in cycles (int)
 * of the program area it applies to.
 */
Identifier IPET::ID_Time("ipet.time");


/**
 * This identifier is used for storing in basic blocks and edges the variables
 * (otawa::ilp::Var *) used in ILP resolution.
 */
Identifier IPET::ID_Var("ipet.var");


/**
 * Identifier of annotations used for storing ILP system (otawa::ilp::System *)
 * in the CFG object.
 */
Identifier IPET::ID_System("ipet.system");


/**
 * Identifier of annotation used for storing for storing the WCET value (int)
 * in the CFG of the computed function.
 */
Identifier IPET::ID_WCET("ipet.wcet");


/**
 * Identifier of a boolean property requiring that explicit names must be used.
 * The generation of explicit names for variables may be time-consuming and
 * must only be activated for debugging purposes.
 */
Identifier IPET::ID_Explicit("ipet.explicit");

} // otawa
