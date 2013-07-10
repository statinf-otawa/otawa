/*
 *	$Id$
 *	Identifier class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-09, IRIT UPS.
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

#include <elm/rtti.h>
#include <elm/meta.h>
#include <otawa/type.h>
#include <otawa/prop/Property.h>
#include <otawa/prop/PropList.h>
#include <otawa/prop/AbstractIdentifier.h>
#include <otawa/prop/Ref.h>
#include <elm/sys/Path.h>

namespace otawa {

using namespace elm;
using namespace elm::io;

// External class
class PropList;


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
	inline const T& use(const PropList& list) const;
	inline const T& value(const PropList& list) const;
	inline Ref<T, Identifier> value(PropList& list) const;
	inline void remove(PropList& list) const { list.removeProp(this); }
	inline bool exists(PropList& list) const { return list.getProp(this); }
	inline void copy(PropList& list, Property *prop)
		{ list.addProp(GenericProperty<T>::make(this, get(prop))); }

	// PropList* Accessors
	inline void add(PropList *list, const T& value) const { add(*list, value); }
	inline void set(PropList *list, const T& value) const { set(*list, value); }
	inline elm::Option<T> get(const PropList *list) const { return get(*list); }
	inline const T& get(const PropList *list, const T& def) const { return get(*list, def); }
	inline T& ref(PropList *list) const { return ref(*list); }
	inline const T& use(const PropList *list) const { return use(*list); }
	inline const T& value(const PropList *list) const { return value(*list); }
	inline Ref<T, Identifier<T> > value(PropList *list) const { return value(*list); }
	inline void remove(PropList *list) const { list->removeProp(this); }
	inline bool exists(PropList *list) const { return list->getProp(this); }
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
		{ out << (!prop ? def : get(*prop)); }
	virtual void printFormatted(io::Output& out, const Property *prop) const
		{ out << (!prop ? def : get(*prop)); }
	virtual const Type& type(void) const { return otawa::type<T>(); }
	virtual void fromString(PropList& props, const string& str) const;
	virtual Property *copy(Property& prop) const
		{ return GenericProperty<T>::make(this, get(&prop)); }

	// Getter class
	class Getter: public PreIterator<Getter, T> {
	public:
		inline Getter(const PropList *list, Identifier<T>& id): getter(list, id) { }
		inline Getter(const PropList& list, Identifier<T>& id): getter(list, id) { }
		inline const T& item(void) const { return ((GenericProperty<T> *)*getter)->value(); }
		inline bool ended(void) const { return getter.ended(); }
		inline void next(void) { getter.next(); }
	private:
		PropList::Getter getter;
	};

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


// Identifier<T>::printFormatted specializations
template <> void Identifier<char>::printFormatted(io::Output& out, const Property *prop) const;
template <> void Identifier<string>::printFormatted(io::Output& out, const Property *prop) const;
template <> void Identifier<cstring>::printFormatted(io::Output& out, const Property *prop) const;
template <> void Identifier<PropList>::printFormatted(io::Output& out, const Property *prop) const;
template <> void Identifier<const PropList *>::printFormatted(io::Output& out, const Property *prop) const;


// Identifier<T>::scan Specializations
template <class T>
inline void Identifier<T>::__class::scan(const Identifier<T>& id,
PropList& props, VarArg& args) {
	T *ptr = args.next<T *>();
	id.set(props, *ptr);
}

template <class T>
inline void Identifier<T>::__simple::scan(const Identifier<T>& id, PropList& props, VarArg& args) {
	T ptr = args.next<T>();
	id.set(props, ptr);
}

/*template <class T>
void Identifier<T>::scan(PropList& props, VarArg& args) const {
	_if<type_info<T>::is_scalar || type_info<T>::is_ptr, __simple, __class>
	::_::scan(*this, props, args);
}

template <> void Identifier<elm::CString>::scan(PropList& props, VarArg& args) const;
template <> void Identifier<cstring>::scan(PropList& props, VarArg& args) const;
template <> void Identifier<elm::String>::scan(PropList& props, VarArg& args) const;*/


// GenericIdentifier<T>::fromString
template <class T> inline void Identifier<T>::fromString(PropList& props, const string& str) const
	{ T v; StringInput in(str); in >> v; set(props, v); }
//	{ throw io::IOException("type not supported for Identifier::fromString() call"); }
template <> void Identifier<bool>::fromString(PropList& props, const string& str) const;
template <> void Identifier<int>::fromString(PropList& props, const string& str) const;
template <> void Identifier<unsigned int>::fromString(PropList& props, const string& str) const;
template <> void Identifier<long>::fromString(PropList& props, const string& str) const;
template <> void Identifier<unsigned long>::fromString(PropList& props, const string& str) const;
template <> void Identifier<long long>::fromString(PropList& props, const string& str) const;
template <> void Identifier<unsigned long long>::fromString(PropList& props, const string& str) const;
template <> void Identifier<double>::fromString(PropList& props, const string& str) const;
template <> void Identifier<string>::fromString(PropList& props, const string& str) const;
template <> void Identifier<Address>::fromString(PropList& props, const string& str) const;
template <> void Identifier<elm::sys::Path>::fromString(PropList& props, const string& str) const;

} // otawa

#endif	// OTAWA_PROP_IDENTIFIER_H
