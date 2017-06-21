/*
 * OTAWA -- WCET computation framework
 * Copyright (C) 2009-17  IRIT - UPS <casse@irit.fr>
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
#include <elm/io.h>
#include <elm/ptr.h>

#include <otawa/base.h>
#include <otawa/prop/Property.h>
#include <otawa/prop/Identifier.h>

namespace otawa {

using namespace elm;

// pre-declaration
class ContextualPath;


// ContextualStep class
class ContextualStep {
public:
	typedef enum kind_t {
		NONE = 0,
		FUNCTION,
		CALL,
		FIRST_ITER,
		OTHER_ITER,
		AFTER,
	} kind_t;

	inline ContextualStep(void): _kind(NONE) { }
	inline ContextualStep(kind_t kind, const Address &address): _kind(kind), addr(address) { };
	inline ContextualStep(const ContextualStep& step): _kind(step._kind), addr(step.addr) { };
	static ContextualStep null;

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

	static bool exists(const PropList& props, const ContextualPath& path, const AbstractIdentifier& id);
	static PropList& ref(PropList& props, const ContextualPath& path, const AbstractIdentifier& id);

	static const PropList& find(const PropList& props, const ContextualPath& path, const AbstractIdentifier& id);
	static PropList& make(PropList& props, const ContextualPath& path);
	static void printFrom(io::Output& out, const PropList& props);

private:
	const PropList& findProps(const PropList& props, const ContextualPath& path, const AbstractIdentifier& id) const;
	PropList& makeProps(const ContextualPath& path);
	PropList& refProps(PropList& props, const ContextualPath& path, const AbstractIdentifier& id);
	void printRec(io::Output& out, const Node& node, int indent = 0) const;

	Node root;
	static AbstractIdentifier ID;
};


class ContextualList: public Lock {
public:
	inline ContextualList(const ContextualStep& step): s(step) { }
	inline ContextualList(const ContextualStep& step, const LockPtr<ContextualList> next)
		: s(step), n(next) { }

	inline const ContextualStep& step(void) const { return s; }
	inline LockPtr<ContextualList> next(void) const { return n; }
	int count(void) const;
	const ContextualStep& ith(int i) const;
	inline const ContextualStep& operator[](int i) const { return ith(i); }

private:
	ContextualStep s;
	LockPtr<ContextualList> n;
};


// ContextualPath class
class ContextualPath {
public:

	// Ref for ContextualPath
	template <class T>
	class Ref {
	public:
		inline Ref(const ContextualPath& _cp, PropList& _p, const Identifier<T>& _i): cp(_cp), p(_p), i(_i) { }
		inline Ref(const Ref<T>& r): cp(r.cp), p(r.p), i(r.i) { }
		inline Ref<T>& operator=(const Ref<T>& r) { cp = r.cp; p = r.p; i = r.i; return *this; }

		// immutable part
		inline const ContextualPath& path(void) const { return cp; }
		inline const PropList &proplist (void) const { return p; }
		inline const Identifier<T>& identifier (void) const { return i; }
		inline bool exists(void) const { return ContextualProperty::exists(p, cp, i); };
		inline const T &get (void) const { return cp.get(i, p); }
		inline operator const T &(void) const { return get(); }
		inline T operator->(void) const { return get(); }
		inline void print(io::Output& out) { i.print(out, find(p, cp, i).getProp(&i)); }

		// mutable part
		inline const Ref<T> &add(const T &value) const { return i(ContextualProperty::make(p, cp, i)).add(value); }
		inline void remove(void) const { ContextualProperty::make(p, cp).removeProp(i); }
		inline T& ref(void) const { return i.ref(ContextualProperty::ref(p, cp, i)); }
		inline T& 	operator&(void) const { return ref(); }
		const Ref<T> &operator=(const T &v) const { ref() = v; return *this; }
		inline Ref<T> &operator+= (const T &v) const { ref() += v; return *this; }
		inline Ref<T> &operator-= (const T &v) const { ref() -= v; return *this; }
		inline Ref<T> &operator *= (const T &v) const { ref() *= v; return *this; }
		inline Ref<T> &operator/= (const T &v) const { ref() /= v; return *this; }
		inline Ref<T> &operator%= (const T &v) const { ref() %= v; return *this; }
		inline Ref<T> &operator &= (const T &v) const { ref() &= v; return *this; }
		inline Ref<T> &operator|= (const T &v) const { ref() |= v; return *this; }
		inline Ref<T> &operator^= (const T &v) const { ref() ^= v; return *this; }
		inline Ref<T> &operator<<= (const T &v) const { ref() <<= v; return *this; }
		inline Ref<T> &operator>>= (const T &v) const { ref() >>= v; return *this; }
		inline Ref<T> &operator++ (void) const { ref()++; return *this; }
		inline Ref<T> &operator-- (void) const { ref()--; return *this; }
		inline Ref<T> &operator++ (int) const { ref()++; return *this; }
		inline Ref<T> &operator-- (int) const { ref()--; return *this; }

	private:
		const ContextualPath& cp;
		PropList& p;
		const Identifier<T>& i;
	};

	// constructors
	inline ContextualPath(void) { }

	inline void clear(void) { p = 0; }
	inline void push(const ContextualStep& step) { p = new ContextualList(step, p); }
	inline void push(ContextualStep::kind_t kind, const Address& addr) { push(ContextualStep(kind, addr)); }
	inline void pop(void) { ASSERT(&p); p = p->next(); }

	inline bool isEmpty(void) const { return p.isNull(); }
	inline int count(void) const { return p.isNull() ? 0 : p->count(); }
	inline const ContextualStep& step(int i) const { return p->ith(p->count() - i - 1); }
	inline const ContextualStep& operator[](int i) const { return step(i); }
	inline operator bool(void) const { return !isEmpty(); }
	void print(io::Output& out) const;
	inline const ContextualList *list(void) const { return &p; }

	Address getEnclosingFunction(void) const;

	template <class T> inline const T& get(const Identifier<T>& id, const PropList& props) const {
		const PropList& fprops = ContextualProperty::find(props, *this, id);
		return id.value(fprops);
	}

	template <class T> inline Ref<T> ref(const Identifier<T>& id, PropList& props) const
		{ return Ref<T>(*this, props, id); }
	template <class T> inline const T& get(const Identifier<T>& id, const PropList *props) const
		{ return get(id, *props); }
	template <class T> inline Ref<T> ref(const Identifier<T>& id, PropList *props) const
		{ return ref(id, *props); }

	template <class T> inline const T& operator()(const Identifier<T>& id, const PropList *props) const
		{ return get(id, *props); }
	template <class T> inline const T& operator()(const Identifier<T>& id, const PropList& props) const
		{ return get(id, props); }
	template <class T> inline Ref<T> operator()(const Identifier<T>& id, PropList *props) const
		{ return ref(id, *props); }
	template <class T> inline Ref<T> operator()(const Identifier<T>& id, PropList& props) const
		{ return ref(id, props); }

private:
	LockPtr<ContextualList> p;
};

// I/O
inline io::Output& operator<<(io::Output& out, const ContextualPath& path)
	{ path.print(out); return out; }
template <class T>
inline io::Output& operator<<(io::Output& out, const ContextualPath::Ref<T>& r)
	{ r.print(out); return out; }

}	// otawa

#endif /* OTAWA_PROP_CONTEXTUALPROPERTY_H_ */
