/*
 *	Identifier class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-18, IRIT UPS.
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
#ifndef OTAWA_PROP_IDENTIFIER_H
#define OTAWA_PROP_IDENTIFIER_H

#include <elm/meta.h>
#include <otawa/type.h>
#include <otawa/prop/Property.h>
#include <otawa/prop/PropList.h>
#include <otawa/prop/AbstractIdentifier.h>
#include <otawa/prop/Ref.h>
#include <otawa/util/Bag.h>
#include <elm/sys/Path.h>

namespace otawa {

using namespace elm;
using namespace elm::io;

// External class
class PropList;


namespace p {
	template <class T> inline void print(Output& out, const T& v) { out << v; }
	template <class T> inline void print(Output& out, T *p) { if(!p) out << "<null>"; else out << p; }
}


// GenericIdentifier class
template <class T>
class Identifier: public AbstractIdentifier {
public:

	// Constructors
	inline Identifier(cstring name): AbstractIdentifier(name) { }
	inline Identifier(cstring name, const T& default_value)
		: AbstractIdentifier(name), def(default_value) { }

	inline Identifier(cstring name, const T& default_value, Property *prop, ...)
		: AbstractIdentifier(name), def(default_value)
		{ VARARG_BEGIN(args, prop); initProps(prop, args); VARARG_END }

	inline Identifier(cstring name, const T& default_value, Property *prop, VarArg& args)
		: AbstractIdentifier(name, prop, args), def(default_value) { }

	// intrinsic accessor
	inline const T& defaultValue(void) const { return def; }

	// PropList& Accessors
	inline void add(PropList& list, const T& value) const;
	inline void set(PropList& list, const T& value) const;
	inline elm::Option<T> get(const PropList& list) const;
	inline const T& get(const PropList& list, const T& def) const;
	inline T& ref(PropList& list) const;
	inline T *addr(PropList& list) const;
	inline const T& use(const PropList& list) const;
	inline const T& value(const PropList& list) const;
	inline Ref<T, Identifier> value(PropList& list) const;
	inline void remove(PropList& list) const { list.removeProp(this); }
	inline bool exists(const PropList& list) const { return list.getProp(this); }
	inline void copy(PropList& list, Property *prop)
		{ list.addProp(GenericProperty<T>::make(this, get(prop))); }

	// PropList* Accessors
	inline void add(PropList *list, const T& value) const { add(*list, value); }
	inline void set(PropList *list, const T& value) const { set(*list, value); }
	inline elm::Option<T> get(const PropList *list) const { return get(*list); }
	inline const T& get(const PropList *list, const T& def) const { return get(*list, def); }
	inline T& ref(PropList *list) const { return ref(*list); }
	inline T *addr(PropList *list) const { return addr(*list); }
	inline const T& use(const PropList *list) const { return use(*list); }
	inline const T& value(const PropList *list) const { return value(*list); }
	inline Ref<T, Identifier<T> > value(PropList *list) const { return value(*list); }
	inline void remove(PropList *list) const { list->removeProp(this); }
	inline bool exists(const PropList *list) const { return list->getProp(this); }
	inline void copy(PropList *list, Property *prop) { copy(*list, prop); }

	// Property accessors
	inline const T& get(const Property *prop) const
		{ return static_cast<const GenericProperty<T> *>(prop)->value(); }
	inline void set(Property *prop, const T& value) const
		{ static_cast<GenericProperty<T> *>(prop)->value() = value; }

	// Operators
	inline const T& operator()(const PropList& props) const { return value(props); }
	inline const T& operator()(const PropList *props) const { return value(props); }
	inline Ref<T, Identifier<T> > operator()(PropList& props) const { return value(props); }
	inline Ref<T, Identifier<T> > operator()(PropList *props) const { return value(props); }
	inline const T& operator()(Property *prop) const { return get(prop); }

	// Identifier overload
	virtual void print(elm::io::Output& out, const Property *prop) const
		{ p::print(out, !prop ? def : get(*prop)); }
	virtual void printFormatted(io::Output& out, const Property *prop) const
		{ p::print(out, !prop ? def : get(*prop)); }
	virtual const Type& type(void) const { return otawa::type<T>(); }
	virtual void fromString(PropList& props, const string& str) const;
	virtual Property *copy(Property& prop) const
		{ return GenericProperty<T>::make(this, get(&prop)); }

	// Getter class
	class Getter: public PreIterator<Getter, const T&> {
	public:
		inline Getter(void) { }
		inline Getter(const PropList *list, const Identifier<T>& id): getter(list, id) { }
		inline Getter(const PropList& list, const Identifier<T>& id): getter(list, id) { }
		inline const T& item(void) const { return ((GenericProperty<T> *)*getter)->value(); }
		inline bool ended(void) const { return getter.ended(); }
		inline void next(void) { getter.next(); }
		inline bool equals(const Getter& g) const { return getter.equals(g.getter); }
	private:
		PropList::Getter getter;
	};

	// GetterRange class
	class GetterRange {
	public:
		inline GetterRange(const PropList& props, const Identifier<T>& id)
			: _props(props), _id(id) { }
		inline Getter begin(void) const { return Getter(_props, _id); }
		inline Getter end(void) const { return Getter(); }
	private:
		const PropList& _props;
		const Identifier<T>& _id;
	};
	inline GetterRange all(const PropList& props) const { return GetterRange(props, *this); }
	inline GetterRange all(const PropList *props) const { return GetterRange(*props, *this); }

private:
	T def;
	inline const T& get(const Property& prop) const;

	typedef struct {
		static inline void scan(const Identifier<T>& id, PropList& props, VarArg& args);
	} __class;

	typedef struct {
		static inline void scan(const Identifier<T>& id, PropList& props, VarArg& args);
	} __simple;
};


// Inlines
template <class T>
inline void Identifier<T>::add(PropList& list, const T& value) const {
	list.addProp(GenericProperty<T>::make(this, value));
}

template <class T>
inline const T& Identifier<T>::get(const Property& prop) const {
	return ((const GenericProperty<T> &)prop).value();
}

template <class T>
inline void Identifier<T>::set(PropList& list, const T& value) const {
	Property *p = list.getProp(this);
	if(p == 0)
		add(list, value);
	else
		set(p, value);
}

template <class T>
inline elm::Option<T> Identifier<T>::get(const PropList& list) const {
	Property *prop = list.getProp(this);
	if(!prop)
		return none;
	else
		return get(prop);
}

template <class T>
inline const T& Identifier<T>::get(const PropList& list, const T& def) const {
	Property *prop = list.getProp(this);
	if(!prop)
		return def;
	else
		return ((GenericProperty<T> *)prop)->value();
}

template <class T>
inline const T& Identifier<T>::use(const PropList& list) const {
	Property *prop = list.getProp(this);
	ASSERT(prop);
	return ((GenericProperty<T> *)prop)->value();
}

template <class T>
inline const T& Identifier<T>::value(const PropList& list) const {
	Property *prop = list.getProp(this);
	if(!prop)
		return def;
	else
		return ((GenericProperty<T> *)prop)->value();
}

template <class T>
inline class Ref<T, Identifier<T> > Identifier<T>::value(PropList& list) const
	{ return Ref<T, Identifier<T> >(list, *this); }

template <class T>
inline T& Identifier<T>::ref(PropList& list) const {
	GenericProperty<T> *_prop = (GenericProperty<T> *)list.getProp(this);
	if(!_prop) {
		_prop = GenericProperty<T>::make(this, def);
		list.addProp(_prop);
	}
	return _prop->value();
}

template <class T>
inline T *Identifier<T>::addr(PropList& list) const {
	GenericProperty<T> *_prop = (GenericProperty<T> *)list.getProp(this);
	if(!_prop) {
		_prop = GenericProperty<T>::make(this, def);
		list.addProp(_prop);
	}
	return &_prop->value();
}


// Identifier<T>::printFormatted specializations
template <> void Identifier<char>::printFormatted(io::Output& out, const Property *prop) const;
template <> void Identifier<string>::printFormatted(io::Output& out, const Property *prop) const;
template <> void Identifier<cstring>::printFormatted(io::Output& out, const Property *prop) const;
template <> void Identifier<PropList>::printFormatted(io::Output& out, const Property *prop) const;
template <> void Identifier<const PropList *>::printFormatted(io::Output& out, const Property *prop) const;


// Identifier<T>::scan Specializations
template <class T>
inline void Identifier<T>::__class::scan(const Identifier<T>& id, PropList& props, VarArg& args) {
	T *ptr = args.next<T *>();
	id.set(props, *ptr);
}

template <class T>
inline void Identifier<T>::__simple::scan(const Identifier<T>& id, PropList& props, VarArg& args) {
	T ptr = args.next<T>();
	id.set(props, ptr);
}

// GenericIdentifier<T>::fromString
template <class T> void from_string(const string& s, T& v)
	{ StringInput in(s); in >> v; }
template <class T> struct scalar_init { T _ = T(0); };
template <class T> struct object_init { T _; };
template <class T> inline void Identifier<T>::fromString(PropList& props, const string& str) const
	{ typename elm::_if<type_info<T>::is_scalar, scalar_init<T>, object_init<T> > v;
	from_string(str, v._);
	set(props, v._); }

template <> void from_string(const string& str, bool& v);
template <> void from_string(const string& str, int& v);
template <> void from_string(const string& str, unsigned int& v);
template <> void from_string(const string& str, long& v);
template <> void from_string(const string& str, unsigned long& v);
template <> void from_string(const string& str, long long& v);
template <> void from_string(const string& str, unsigned long long& v);
template <> void from_string(const string& str, double& v);
template <> void from_string(const string& str, string& v);
template <> void from_string(const string& str, Address& v);
template <> void from_string(const string& str, sys::Path& v);

class SymAddress;
template <> void from_string(const string& str, SymAddress*& v);

void from_string_split(const string& str, Vector<string>& items);
template <class T> void from_string(const string& str, Bag<T>& bag) {
	Vector<string> strs; from_string_split(str, strs);
	Vector<T> list(strs.length()); list.setLength(strs.length());
	for(int i = 0; i < strs.length(); i++) from_string(strs[i], list[i]);
	bag = list;
}

namespace p {

template <class T>
class id: public Identifier<T> {
public:
	inline id(cstring name): Identifier<T>(name) { }
	inline id(cstring name, const T& def): Identifier<T>(name, def) { }
};

} // p

} // otawa

#endif	// OTAWA_PROP_IDENTIFIER_H
