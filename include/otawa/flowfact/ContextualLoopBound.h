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
#include <otawa/properties.h>
#include <elm/genstruct/Tree.h>
#include <elm/genstruct/Vector.h>

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
	genstruct::Vector<T> stack;
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
	
	void addMax(const ContextPath<Address>& path, int max)
		throw(AmbiguousBoundException);
	void addTotal(const ContextPath<Address>& path, int total)
		throw(AmbiguousBoundException);
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
	genstruct::Tree<data_t> tree;
	
	genstruct::Tree<data_t> *look(const ContextPath<Address>& path);
	int lookMax(genstruct::Tree<data_t> *cur);
	int lookTotal(genstruct::Tree<data_t> *cur);
	void print(genstruct::Tree<data_t> *cur, int tab = 0);
};

} // otawa

#endif /* OTAWA_FLOWFACT_CONTEXTUALLOOPBOUND_H_ */
