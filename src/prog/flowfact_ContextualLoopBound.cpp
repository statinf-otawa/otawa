/*
 *	$Id$
 *	ContextualLoopBound class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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

#include <otawa/flowfact/ContextualLoopBound.h>
#include <otawa/flowfact/features.h>

namespace otawa {

/**
 * @class ContextPath
 * This class is a stack to store call chain in the program call graph.
 * The top level element (index 0) is the current call and  previous elements
 * represents the call that has driven to the current call.
 * 
 * @param T		Representation of a function call.
 */


/**
 * @fn void ContextPath::push(const T& addr);
 * Add a call to the call stack (becoming the new top call).
 * @param addr	Function call to push.
 */


/**
 * @fn T ContextPath::pop(void)
 * Remove the top call from the call stack.
 * @return	Popped function call.
 */


/**
 * @fn const T& ContextPath::top(void) const
 * Return the top function call.
 * @return	Top function call.
 */


/**
 * @fn int ContextPath::count(void) const
 * Get the count of calls in the path.
 * @return	Number of function calls.
 */


/**
 * @fn const T& ContextPath::get(int i) const
 * Get the function call at position i (0 for the top function call).
 * @param	i	Position of the function call to get.
 * @return		Function call at position i.
 * @warning		i must be in [0, @ref count() - 1].
 */


/**
 * @fn const T& ContextPath::operator[](int i) const
 * Shortcut to get().
 */


/**
 * @class ContextualLoopBound
 * A contextual loop bound is a collection of loop bound (including max iteration
 * and total iteration number) for the different call contexts of a loop.
 * 
 * The contextual loop bound does not require to contain all possible contexts
 * but enough information for the top part of the call context. At worse, it
 * may only contain the max loop bound for any context. The collection of the
 * collections does not need to be exhaustive if safe loop count is given
 * for less precise contexts. 
 */



/**
 * @typedef ContextualLoopBounds::bound_t
 * This structure contains information about loop bounds for a given context.
 */


/**
 * Build a contextual loop bound with the given bounds.
 * @param max	Maximum iteration.
 * @param total	Total iteration.
 */
ContextualLoopBound::ContextualLoopBound(int max, int total)
: tree(data_t(Address::null, max, total)) {	
}



/**
 * Look for the subtree matching the given path (build the subtree if required).
 * @param path	Path to look in.
 * @return		Matching subtree.
 */
genstruct::Tree<ContextualLoopBound::data_t> *
ContextualLoopBound::look(const ContextPath<Address>& path) {
	genstruct::Tree<data_t> *cur = &tree;
	
	// Build the path
	for(int i = 0; i < path.count(); i++) {
		
		// Look for the address
		genstruct::Tree<data_t> *child;
		for(child = cur->children(); child; child = child->sibling())
			if(child->data().fun == path[i])
				break;
		if(child) {
			cur = child;
			continue;
		}
		
		// Create the subtree
		child = new genstruct::Tree<data_t>(data_t(path[i]));
		cur->add(child);
		cur = child;
	}
	
	return cur;
}

/**
 * Add a context max bound.
 * @param path	Call context.
 * @param max	Max bound.
 * @throw AmbiguousBoundException	Thrown when there is already a bound
 * 									for the given context.
 */
void ContextualLoopBound::addMax(const ContextPath<Address>& path, int max)
throw(AmbiguousBoundException) {
	genstruct::Tree<data_t> *cur = look(path);
	if(cur->data().max != -1 && cur->data().max != max) {
		cur->data().max = elm::max(max, cur->data().max);
		// !!FIX!!
		//throw AmbiguousBoundException("ambiguous maximum bound");
	}
	else
		cur->data().max = max;
}


/**
 * Add a context total bound.
 * @param path	Call context.
 * @param total	Total bound.
 * @throw AmbiguousBoundException	Thrown when there is already a bound
 * 									for the given context.
 */
void ContextualLoopBound::addTotal(const ContextPath<Address>& path, int total)
throw(AmbiguousBoundException) {
	genstruct::Tree<data_t> *cur = look(path);
	if(cur->data().total != -1 && cur->data().total != total) {
		cur->data().total = elm::max(total, cur->data().total);
		// !!FIX!!
		//throw AmbiguousBoundException("ambiguous total bound");
	}
	else
		cur->data().total = total;
}


/**
 * Look to find the maximum in the tree children.
 * @param cur		Tree to look in.
 * @return			Maximum.
 */
int ContextualLoopBound::lookMax(genstruct::Tree<data_t> *cur) {
	ASSERT(cur);
	if(cur->data().max != undefined)
		return cur->data().max;
	int tmax = undefined;
	for(genstruct::Tree<data_t> *child = cur->children();
	child;
	child = child->sibling()) {
		int cmax = lookMax(child);
		tmax = max(tmax, cmax);
	}
	return tmax;
}


/**
 * Look to find the total in the tree children.
 * @param cur		Tree to look in.
 * @return			Total.
 */
int ContextualLoopBound::lookTotal(genstruct::Tree<data_t> *cur) {
	ASSERT(cur);	
	if(cur->isEmpty())
		return undefined;
	int ttotal = 0;
	for(genstruct::Tree<data_t> *child = cur->children();
	child;
	child = child->sibling()) {
		int ctotal = lookTotal(child);
		if(ctotal == undefined) {
			ttotal = undefined;
			break;
		}
		ttotal += ctotal;
	}
	return ttotal;
}


/**
 * Find the maximum bound for the given context.
 * @param	path	Context path.
 * @return			Maxmimum value (may be undefined).
 */
int ContextualLoopBound::findMax(const ContextPath<Address>& path) {
	int cmax = undefined;
	genstruct::Tree<data_t> *cur = &tree;
	
	// Build the path
	for(int i = 0; i < path.count(); i++) {
		genstruct::Tree<data_t> *child;
		for(child = cur->children(); child; child = child->sibling()) {
			if(child->data().fun == path[i])
				break;
		}
		if(!child)
			break;
		cur = child;
		if(cmax == undefined
		|| (cur->data().max != undefined && cur->data().max < cmax))
			cmax = cur->data().max;
			if(cmax != undefined)
				return cmax;
	}
	
	// Look for a max
	return max(cmax, lookMax(cur));
}


/**
 * Find the total bound for the given context.
 * @param	path	Context path.
 * @return			Total value (may be undefined).
 */
int ContextualLoopBound::findTotal(const ContextPath<Address>& path) {
	int ctotal = undefined;
	genstruct::Tree<data_t> *cur = &tree;
	
	// Build the path
	for(int i = 0; i < path.count(); i++) {
		genstruct::Tree<data_t> *child;
		for(child = cur->children(); child; child = child->sibling())
			if(child->data().fun == path[i])
				break;
		if(!child)
			break;
		cur = child;
		if(cur->data().total != undefined) {
			ctotal = cur->data().total;
			if(ctotal != undefined)
				return ctotal;
		}
	}
	
	// Look for a max
	return max(ctotal, lookTotal(cur));
}


/**
 * This property provides context-dependent loop bounds. It is part of information
 * given by @ref FLOW_FACT_FEATURE.
 * 
 * @par Hooks
 * @li @ref Inst
 */
Identifier<ContextualLoopBound *> CONTEXTUAL_LOOP_BOUND("otawa::CONTEXTUAL_LOOP_BOUND", 0);

} // otawa
