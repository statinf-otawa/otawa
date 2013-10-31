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
 * @class FirstUnrollingFixPoint
 *
 * FixPoint class for HalfAbsInt
 * Like the DefaultFixPoint, but: 
 * Makes the distinction between the first iteration and the other iterations of any loop.
 * getIter() method enables the Problem to know which iteration it is.
 */
 

/**
 * @class FirstUnrollingFixPoint::FixPointState
 *
 * State info class for FirstUnrollingFixPoint
 * This FixPoint needs to remember:
 * - The loop header state (headerState)
 * - The current iteration number for the loop (numIter)
 * - The resulting state of the first iteration (firstIterState), that is, the union of the back edge states
 *    at the end of the first iteration.
 */


} } }	// otawa::dfa::hai
