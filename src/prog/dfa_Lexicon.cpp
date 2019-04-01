/*
 *	dfa::Lexicon class
 *	Copyright (c) 2019, IRIT UPS.
 *
 *	This file is part of OTAWA
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */

#include <otawa/dfa/Lexicon.h>


namespace otawa { namespace dfa {

/**
 * @class Lexicon
 * A lexicon is devoted to speed up and reduce the place occupied by a static
 * analysis. Firstly, it ensures that each will be represented once and,
 * secondly, it allows to speed up transition between states that already arised.
 *
 * Basically, it records in a unique instance each state and record transition
 * between states to replay them quickly.
 *
 * Initial states can be recorded using the Lexicon::add() function. Then the production
 * of new states along transition is performed using Lexicon::apply().
 * If the transition is already known, the state is immediately returned. If the
 * transition is not known, the resulting state is obtained by calling
 * function Lexicon::compute() that has to be overridden by the user.
 *
 * Lexicon supports optional compilation:
 *	* If the symbol OTAWA_LEXICON_STAT is defined before inclusion, statistics function
 *	functions are provided (Lexicon::stateCount(), Lexicon::minUpdateTrans(),
 *	Lexicon::maxUpdateTtrans()).
 *	* If the symbol OTAWA_LEXICON_DUMP is defined before inclusion, Lexicon::dump() function
 *	is made available.
 * In addition, in OTAWA_LEXICON_STAT mode, the function Lexicon::countAlive() is available:
 * once the analysis is performed, the application can mark each useful state handle with
 * Handle::setAlive() to mark handle as alive and this function will return the number
 * of states that are alive.
 *
 * @param S		Type of state (a hash for this type must be available).
 * @param A		Type of action that triggers transitions.
 * @param M		(optional) Type of hash manager.
 * @ingroup		dfa
 */

/**
 * @var const int Lexicon::default_size;
 * Default size of the hashmap used by this class.
 */

/**
 * @fn Lexicon::Lexicon(int size = default_size);
 * Build a lexicon.
 * @param size	Size of the used hash map (default to @ref Lexicon::default_size).
 */

/**
 * @fn Lexicon::Lexicon(M& m, int size);
 * Build a lexicon using the given hash manager.
 * @param m		Hash manager to use.
 * @param size	Size of the used hash map (default to @ref Lexicon::default_size).
 */

/**
 * @fn Handle *Lexicon::add(const S& s);
 * Add the given state to the lexicon. Notice that if the state is already in the
 * lexicon, it is not duplicated.
 * @param s		State to store.
 * @return		Handle containing the state.
 */

/**
 * Handle *Lexicon::update(Handle *ha, const A& a);
 * Build a new state by the updating the given state with the given action.
 * Notice that if the update is new, a call to Lexicon::doUpdate() is performed
 * in order to actually compute the new state.
 * @param ha	Input state handle.
 * @param a		Action causing the update.
 * @return		Updated output state handle.
 */

/**
 * void Lexicon::doUpdate(const S& s, const A& a, S& r);
 * Function to overload to provide a way to compute a new state after an update.
 * @param s	Input state.
 * @param a	Upadting action.
 * @param r	Resulting state (output parameter).
 */

/**
 * Handle *Lexicon::join(Handle *ha, Handle *hb);
 * Build a new state by join the first state with the second state.
 * Notice that if the join is new, a call to Lexicon::doJoin() is performed
 * in order to actually compute the new state.
 * @param ha	First state handle.
 * @param hb	Second state handle.
 * @return		Joined output state handle.
 */

/**
 * void Lexicon::doJoin(const S& s1, const S& s2, S& r);
 * Function to overload to provide a way to compute a new state after a join.
 * @param s1	First input state.
 * @param s2	Second input state.
 * @param r		Resulting state (output parameter).
 */

/**
 * void Lexicon::dump(sys::Path p);
 * Dump states, update and join transition as a single .dot graph
 * in the file which path is p.
 *
 * Only available if OTAWA_LEXICON_DUMP is defined.
 * @param p		Path of the file to dump to.
 */

/**
 * @fn int Lexicon::stateCount() const;
 * Get the count of states in the lexicon.
 * @return	Count of states.
 */

/**
 * @fn int Lexicon::minUpdateTrans() const;
 * Get the minimum number of update transition over all states.
 * @return	Minimum number of transitions.
 */

/**
 * @fn int Lexicon::maxUpdateTrans() const;
 * Get the maximum number of update transition over all states.
 * @return	Maximum number of transitions.
 */

/**
 * @fn int Lexicon::countAlive() const;
 * Get the count of states that are still alive.
 * @return	Count of alive states.
 */

} }	// otawa::dfa
