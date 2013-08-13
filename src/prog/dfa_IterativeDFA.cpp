/*
 *	$Id$
 *	IterativeDFA class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007-08, IRIT UPS.
 * 
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software 
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <otawa/dfa/IterativeDFA.h>

namespace otawa { namespace dfa {


/**
 * @defgroup dfa	Data Flow Analysis
 * This module provides classes to perform static analysis and Data Flow
 * Analyses (DFA). This includes DFA engines:
 *	@li @ref otawa::dfa::IterativeDFA,
 *	@li @ref otawa::dfa::XIterativeDFA,
 *	@li @ref otawa::util::HalfAbsInt.
 * 
 * This includes also tool classes:
 *	@li @ref otawa::dfa::BitSet to represents efficiently sets.
 */


/**
 * @class Predecessor
 * This iterator is used to traverse forward a CFG in an iterative DFA
 * analysis.
 * @ingroup dfa
 */


/**
 * @class Successor
 * This iterator is used to traverse backward a CFG in an iterative DFA
 * analysis.
 * @ingroup dfa
 */


/**
 * @class IterativeDFA
 * This class provides a generic facility to implement iterative Data
 * Flow Analysis (DFA) as presented in "Aho, Sethi, Ullman, Compilers:
 * Principles, Techniques, and Tools, Addison Wesley 1986".
 *
 * An iterative DFA perform a static analysis that computes for each BB of a CFG
 * two value sets (the set may be whatever value handled in the analyzed
 * program):
 * @li IN	set before the BB execution (union of previous BB sets),
 * @li OUT	set after the BB execution (IN after update of the BB).
 * 
 * To compute IN and OUT, the recursive formulae below are used:
 * 
 *		IN(BB) = U(BBi) / BBi is predecessor of BB
 * 
 * 		OUT(BB) = IN(BB) - KILL(BB) U GEN(BB)
 *
 * Where KILL(BB) represents the items removed by the execution of BB and
 * GEN(BB) the added items. Finally, we need default set value to initialize
 * the OUT of the BB and to perform a fixpoint computation on the whole CFG.
 * 
 * The final algorithm is described below:
 * @code
 * foreach BB do OUT(BB) <- INIT(BB)
 * change <- true
 * while change do
 *	change <- false
 *	foreach BB do
 *		old_out <- OUT(bb)
 *		IN(BB) = U(BBi) / BBi is predecessor of BB
 * 		OUT(BB) = IN(BB) - KILL(BB) U GEN(BB)
 *		if OUT(BB) <> old_out then change <- true
 *	done
 * done
 * @endcode
 * 
 * Notice that thanks to Iter type paremeter, we may have forward or backward
 * traversal of the CFG (IN may be union of predecessors or of successors).
 * 
 * An efficient way to implement the handled set is provided by the
 * @ref otawa::dfa::BitSet class. 
 * 
 * @param Problem	DFA problem to solve. It must implements the concept
 * 					@ref otawa::concept::IterativeDFAProblem.
 * @param Set		Type of set used to implements the problem (depends on
 *					the problem).
 * @param Iter		Kind of iterator to get predecessor sets of the current
 * 					computation (default to @ref Predecessor for forward
 * 					traversal, possibly @ref Successor for backward traversal).
 * @ingroup dfa
 */


/**
 * @fn IterativeDFA::IterativeDFA(Problem& problem, CFG& cfg);
 * @param problem	Problem to work on.
 * @param cfg		CFG to work on.
 */


/**
 * @fn void IterativeDFA::compute(void);
 * Perform the iterative DFA analysis.
 */
 
 
/**
 * @fn Set *IterativeDFA::inSet(BasicBlock *bb);
 * Get the IN(BB) set.
 * @param bb	Examined BB.
 * @return		IN(BB).
 */


/**
 * @fn Set *IterativeDFA::outSet(BasicBlock *bb);
 * Get the OUT(BB) set.
 * @param bb	Examined BB.
 * @return		OUT(BB).
 */


/**
 * @fn Set *IterativeDFA::genSet(BasicBlock *bb);
 * Get the GEN(BB) set.
 * @param bb	Examined BB.
 * @return		GEN(BB).
 */


/**
 * @fn Set *IterativeDFA::killSet(BasicBlock *bb);
 * Get the KILL(BB) set.
 * @param bb	Examined BB.
 * @return		KILL(BB).
 */

} } // otawa::dfa
