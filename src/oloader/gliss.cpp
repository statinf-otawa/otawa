/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS.
 *
 *	implementation of the GLISS simulation interface
 */

typedef void state_t;
#include <otawa/loader/gliss.h>

namespace otawa { namespace gliss {

/**
 * @internal Identifier giving access to the GLISS state of the loaded program.
 */
Identifier<state_t *> GLISS_STATE("otawa::gliss::gliss_state", 0);


} } // otawa::gliss


