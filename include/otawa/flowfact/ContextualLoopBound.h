/*
 *	$Id$
 *	ContextualLoopBound class interface
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

#ifndef OTAWA_FLOWFACT_CONTEXTUALLOOPBOUND_H_
#define OTAWA_FLOWFACT_CONTEXTUALLOOPBOUND_H_

#include <otawa/base.h>
#include <elm/data/Tree.h>
#include <elm/data/Vector.h>
#include "../prop.h"

namespace otawa {

using namespace elm;

// ContextPath class
template <class T>
class ContextPath {
public:
	inline void push(const T& addr) { stack.push(addr); }
	inline T pop(void) { return stack.pop(); }
	inline const T& top(void) const { return stack.top(); }
	inline int count(void) const { return stack.length(); }
	inline const T& get(int i) const { return stack[stack.length() - i - 1]; }
	inline const T& operator[](int i) const { return get(i); }
	inline bool isEmpty(void) const { return stack.isEmpty(); }
	inline operator bool(void) const { return !isEmpty(); }
	inline void clear(void) { stack.clear(); }
	
private:
	Vector<T> stack;
};


// AmbiguousBoundException class
class AmbiguousBoundException: public MessageException {
public:
	inline AmbiguousBoundException(const string& message)
		: MessageException(message) { }
};


// ContextualLoopBound class
class ContextualLoopBound {
public:
	static const int undefined = -1;
	
	ContextualLoopBound(int max = undefined, int total = undefined);
	
	void addMax(const ContextPath<Address>& path, int max);
	void addTotal(const ContextPath<Address>& path, int total);
	int findMax(const ContextPath<Address>& path);
	int findTotal(const ContextPath<Address>& path);
	
private:
	typedef struct data_t {
		inline data_t(
			Address _fun = Address::null,
			int _max = undefined,
			int _total = undefined)
		: fun(_fun), max(_max), total(_total) { }
		Address fun;
		int max, total;
	} data_t;
	Tree<data_t> tree;
	
	Tree<data_t> *look(const ContextPath<Address>& path);
	int lookMax(Tree<data_t> *cur);
	int lookTotal(Tree<data_t> *cur);
	void print(Tree<data_t> *cur, int tab = 0);
};

extern Identifier<ContextualLoopBound *> CONTEXTUAL_LOOP_BOUND;

} // otawa

#endif /* OTAWA_FLOWFACT_CONTEXTUALLOOPBOUND_H_ */
