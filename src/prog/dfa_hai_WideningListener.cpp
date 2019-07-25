/*
 *	This file is part of OTAWA
 *	Copyright (c) 2006-11, IRIT UPS.
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

namespace otawa { namespace dfa { namespace hai {

/**
 * @class WideningListener
 *
 * This listener gathers in an array the LUB of the in-states for all analyzed basic blocks.
 * At the end of the analysis, you can access result[CFGNUMBER][BBNUMBER] to obtain
 * the in-state of the basic block BBNUMBER of cfg CFGNUMBER.
 * 
 */
 
/**
 * @fn void WideningListener::blockInterpreted (const DefaultFixPoint< WideningListener > *fp, BasicBlock *bb, const typename Problem::Domain &in, const typename Problem::Domain &out, CFG *cur_cfg) const
 * This is called whenever a block is processed. In the WideningListener, we do the LUB of all the in-states
 * of each block.
 * @param fp The FixPoint object
 * @param bb The Basic Block
 * @param in In-state
 * @param out Out-state (not used in this class)
 * @param cur_cfg Current CFG   
 */
 
/**
 * @fn void WideningListener::fixPointReached (const DefaultFixPoint< WideningListener > *fp, BasicBlock *bb)
 * This is called whenever a loop's fixpoint is reached.
 * @param fp The FixPoint object
 * @param bb The loop header. 
 */

/**
  * Problem & WideningListener::getProb ()
  * This function is for getting the Problem associated with this listener.
  * @return The problem.
 */


} } }	// otawa::dfa::hai
