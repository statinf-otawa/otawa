/*
 *	FastState class
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2011-12, IRIT UPS.
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

#include <otawa/dfa/FastState.h>

namespace otawa { namespace dfa {

/**
 * @class FastState
 * Fast implementation of abstract domain on the functional state of a microprocessor
 * including the registers and the memory content.
 * @param D		Domain of values.
 */

/**
 * @fn FastState::FastState(D *d, dfa::State *state, StackAllocator& alloc);
 * Build a state.
 * @param d		Domain manager.
 * @param proc	Analyzed process.
 * @param alloc	Stack allocator to use.
 */

/**
 * @fn int FastState::getMultiMax(void) const;
 * Get the max number of multiple load/store before jumping to top.
 * @return	Maximum number of effective load/store.
 */

/**
 * @fn void FastState::setMultiMax(int new_multi_max);
 * Set the maximum number of multiple load/store before approximating to top.
 * @param new_multi_max		New maximum number of multiple load/store.
 */

/**
 * @fn FastState::value_t FastState::get(t s, register_t r);
 * Get register value.
 * @param s	State to look at.
 * @param r	Register to get.
 * @return	Register value.
 */

/**
 * @fn FastState::t FastState::set(t s, register_t r, value_t v);
 * Set a register value.
 * @param s		State to change.
 * @param r		Register to set.
 * @param v		Value to set.
 * @return		State after change.
 */

/**
 * @fn FastState::t FastState::store(t s, address_t a, value_t v);
 * Perform a store to memory.
 * @param s		State to change.
 * @param a		Address to store to.
 * @param v		Value to store.
 * @return		New state with store performed.
 */

/**
 * FastState::value_t FastState::load(t s, address_t a);
 * Perform a load at the given address.
 * @param s		Current state.
 * @param a		Address to load from.
 * @return		Load value.
 */

/**
 * @fn FastState::t FastState::store(t s, address_t a, address_t b, ot::size off, value_t v);
 * Perform a multiple-store (joining on all memories that are modified).
 * @param s		State to store in.
 * @param a		Start address of the area.
 * @param b		Last address past the area.
 * @param off	Offset between stored objects.
 * @param v		Value to set.
 * @return		New state.
 */

/**
 * @fn FastState::t FastState::storeAtTop(t s);
 * Store to T address (set all memory to T).
 * @param s		State to store to.
 * @return		s with memory set to T.
 */

/**
 * @fn FastState::value_t FastState::load(t s, address_t a, address_t b, ot::size off);
 * Load multiple values from a memory area.
 * @param s		State to load from.
 * @param a		First address of the area.
 * @param b		Last address past the area.
 * @param off	Offset of objects between the area.
 * @return		Union of found objects.
 */

/**
 * @fn FastState::t FastState::join(t s1, t s2);
 * Perform join of both states.
 */

/**
 * @fn FastState::t FastState::map(t s, W& w);
 * Apply a function to transform a state.
 *
 * The worker object must match the following concept:
 *
 * @code
 * class Worker {
 * public:
 * 		value_t process(value_t v);
 * };
 * @endcode
 *
 * @param W		Type of the worker.
 * @param in	Input state.
 * @param w		Worker object.
 * @return		Output state.
 */

/**
 * @fn FastState::bool FastState::equals(t s1, t s2);
 * Test if both states are equal.
 * @param s1	First state.
 * @param s2	Second state.
 * @return		True if they equal, false else.
 */

/**
 * @fn void FastState::print(io::Output& out, t s);
 * Display the state.
 * @param out	Stream to display to.
 * @param s		State to display.
 */

/**
 * @fn FastState::t FastState::combine(t s1, t s2, W& w);
 * Apply a function to combine two states. This function assumes
 * that the applied operation is idempotent and monotonic and use it
 * to speed up the combination.
 *
 * The worker object must match the following concept:
 *
 * @code
 * class Worker {
 * public:
 * 		value_t process(value_t v1, value_t v2);
 * };
 * @endcode
 *
 * @param s1	First state.
 * @param s2	Second state.
 * @param w		Worker object.
 * @return		Output state.
 */

} }		// otawa::dfa
