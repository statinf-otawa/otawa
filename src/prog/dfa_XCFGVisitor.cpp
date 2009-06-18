/*
 * $Id$
 * Copyright (c) 2006 IRIT-UPS
 *
 * XCFGVisitor class implementation.
 */

#include <otawa/dfa/XCFGVisitor.h>

namespace otawa { namespace dfa {

/**
 * @class XCFGVisitor
 * This class implements the visitor concept of @ref XIterativeDFA class and
 * allows applying an analysis on a collection of CFG in a flow-insensitive way.
 *
 * @param P	Problem to resolve.
 *
 * The problem concept must implements the following concept:
 * @code
 * 	typedef domain_t;							// domain type
 *  domain_t *empty(void);						// return an empty domain
 * 	domain_t *gen(CFG *cfg, BasicBlock *bb);	// return the GEN value
 *	domain_t *kill(CFG *cfg, BasicBlock *bb);	// return the KILL value
 *	void free(domain_t *d);						// free the given domain
 * @endcode
 *
 * The domain_t type must implements the following concept:
 * @code
 *	void reset(void);			// reset the domain
 *  void join(domain_t *d);		// join between current domain and given one.
 *  void meet(domain_t *d);		// meet between current domain and given one.
 *	bool equals(domain_t *d);	// test if both domains are equals
 * @endcode
 */


/**
 * @fn XCFGVisitor::XCFGVisitor(CFGCollection& cfgs, P& problem)
 * Build the visitor to apply on the given domain and CFG collection.
 * @param cfgs		CFG collection to work on.
 * @param problem	Problem to resolve.
 */


/**
 * Identifier used for internal use.
 */
Identifier<int> INDEX("otawa::dfa::I", -1);


} } // otawa::dfa
