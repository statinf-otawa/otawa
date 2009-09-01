/*
 * OTAWA -- WCET computation framework
 * Copyright (C) 2009  IRIT - UPS <casse@irit.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.package ojawa;
 */
#ifndef OTAWA_PROP_CONTEXTUALPROPERTY_H_
#define OTAWA_PROP_CONTEXTUALPROPERTY_H_

#include <elm/inhstruct/Tree.h>
#include <elm/genstruct/Vector.h>
#include <elm/io.h>
#include <otawa/base.h>
#include <otawa/prop/Property.h>
#include <otawa/prop/Identifier.h>

namespace otawa {

using namespace elm;

// ContextualStep class
class ContextualStep {
public:
	typedef enum kind_t {
		NONE = 0,
		FUNCTION,
		CALL,
		FIRST_ITER,
		OTHER_ITER
	} kind_t;

	inline ContextualStep(void): _kind(NONE) { }
	inline ContextualStep(kind_t kind, const Address &address): _kind(kind), addr(address) { };
	inline ContextualStep(const ContextualStep& step): _kind(step._kind), addr(step.addr) { };
	inline ContextualStep& operator=(const ContextualStep& step)
		{ _kind = step._kind; addr = step.addr; return *this; }

	inline kind_t kind(void) const { return _kind; }
	inline const Address& address(void) const { return addr; }

	inline bool equals(const ContextualStep& step) const
		{ return _kind == step.kind() && addr == step.address(); }
	inline bool operator==(const ContextualStep& step) const { return equals(step); }
	inline bool operator!=(const ContextualStep& step) const { return !equals(step); }

	void print(io::Output& out) const;

private:
	kind_t _kind;
	Address addr;
};
inline io::Output& operator<<(io::Output& out, const ContextualStep& path)
	{ path.print(out); return out; }


// ContextualPath class
class ContextualPath {
public:
	inline ContextualPath(void) { }
	ContextualPath(const ContextualPath& path);
	ContextualPath& operator=(const ContextualPath& path);

	inline void clear(void) { stack.clear(); }
	inline void push(const ContextualStep& step) { stack.push(step); }
	inline void push(ContextualStep::kind_t kind, const Address& addr)
		{ stack.push(ContextualStep(kind, addr)); }
	inline void pop(void) {stack.pop(); }

	inline bool isEmpty(void) const { return stack.isEmpty(); }
	inline int count(void) const { return stack.count(); }
	inline const ContextualStep& step(int i) const { return stack[i]; }
	inline const ContextualStep& operator[](int i) const { return step(i); }
	inline operator bool(void) const { return !isEmpty(); }
	void print(io::Output& out) const;

	template <class T> inline const T& get(const Identifier<T>& id, const PropList& props) const;
	template <class T> inline Ref<T, Identifier<T> > ref(const Identifier<T>& id, PropList& props) const;
	template <class T> inline const T& operator()(const Identifier<T>& id, const PropList& props) const
		{ return get(id, props); }
	//template <class T> inline Ref<T, Identifier<T> > operator()(const Identifier<T>& id, PropList& props)
	//	{ return ref(id, props); }

	template <class T> inline const T& get(const Identifier<T>& id, const PropList *props) const
		{ return get(id, *props); }
	template <class T> inline Ref<T, Identifier<T> > ref(const Identifier<T>& id, PropList *props) const
		{ return ref(id, *props); }
	template <class T> inline const T& operator()(const Identifier<T>& id, const PropList *props) const
		{ return get(id, *props); }

private:
	genstruct::Vector<ContextualStep> stack;
};
inline io::Output& operator<<(io::Output& out, const ContextualPath& path)
	{ path.print(out); return out; }


// ContextualProperty class
class ContextualProperty: public Property {

	// Node class
	class Node: public PropList, public inhstruct::Tree {
	public:
		inline Node(void) { }
		inline Node(const ContextualStep& _step): step(_step) { }
		ContextualStep step;
	};

public:
	ContextualProperty(void);

	static const PropList& find(const PropList& props, const ContextualPath& path, const AbstractIdentifier& id);
	static PropList& make(PropList& props, const ContextualPath& path);
	static void print(io::Output& out, const PropList& props);

private:
	const PropList& findProps(const PropList& props, const ContextualPath& path, const AbstractIdentifier& id) const;
	PropList& makeProps(const ContextualPath& path);
	void print(io::Output& out, const Node& node, int indent = 0) const;

	Node root;
	static AbstractIdentifier ID;
};

// inlines
template <class T>
inline const T& ContextualPath::get(const Identifier<T>& id, const PropList& props) const {
	const PropList& fprops = ContextualProperty::find(props, *this, id);
	return id.value(fprops);
}

template <class T>
inline Ref<T, Identifier<T> > ContextualPath::ref(const Identifier<T>& id, PropList& props) const
	{ return Ref<T, Identifier<T> >(ContextualProperty::make(props, *this), id); }

}	// otawa

#endif /* OTAWA_PROP_CONTEXTUALPROPERTY_H_ */
