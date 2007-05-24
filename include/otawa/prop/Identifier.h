/*
 *	$Id$
 *	Copyright (c) 2003-07, IRIT UPS.
 *
 *	Identifier class interface
 */
#ifndef OTAWA_PROP_IDENTIFIER_H
#define OTAWA_PROP_IDENTIFIER_H

#include <elm/rtti.h>
#include <elm/meta.h>
#include <otawa/type.h>
#include <otawa/prop/AbstractIdentifier.h>
#include <otawa/prop/Property.h>
#include <otawa/prop/PropList.h>

namespace otawa {

using namespace elm;
using namespace elm::io;

// External class
class PropList;


// GenericIdentifier class
template <class T>
class Identifier: public AbstractIdentifier {
	T def;
	inline const T& get(const Property& prop) const;

	typedef struct {
		static inline void scan(const Identifier<T>& id, PropList& props,
			VarArg& args);
	} __scalar;

	typedef struct {
		static inline void scan(const Identifier<T>& id, PropList& props,
			VarArg& args);
	} __class;

public:

	// Value class
	class Value {
		PropList& prop;
		const Identifier<T>& id;
	public:
		inline Value(PropList& prop, const Identifier<T>& id);
		inline Value(PropList *prop, const Identifier<T>& id);

		inline Value& add(const T& value);
		inline void remove(void);
		inline bool exists(void) const;
		inline T& ref(void) const;

		inline operator const T&(void) const;
		inline Value& operator=(const T& value);
		inline T operator->(void) const;
		inline T& operator&(void) const { return ref(); }
		
		inline Value& operator+=(const T& v) const { ref() +=  v; return *this; } 
		inline Value& operator-=(const T& v) const { ref() -=  v; return *this; } 
		inline Value& operator*=(const T& v) const { ref() *=  v; return *this; } 
		inline Value& operator/=(const T& v) const { ref() /=  v; return *this; } 
		inline Value& operator%=(const T& v) const { ref() %=  v; return *this; } 
		inline Value& operator&=(const T& v) const { ref() &=  v; return *this; } 
		inline Value& operator|=(const T& v) const { ref() |=  v; return *this; } 
		inline Value& operator^=(const T& v) const { ref() ^=  v; return *this; } 
		inline Value& operator<<=(const T& v) const { ref() <<=  v; return *this; } 
		inline Value& operator>>=(const T& v) const { ref() >>=  v; return *this; } 
		inline Value& operator++(void) const { ref()++; } 
		inline Value& operator--(void) const { ref()--; } 
		inline Value& operator++(int) const { ref()++; } 
		inline Value& operator--(int) const { ref()--; } 
	};
	
	// Constructors
	inline Identifier(elm::CString name, NameSpace& ns = ::NS);
	inline Identifier(elm::CString name, const T& default_value, NameSpace& = ::NS);
		
	// PropList& Accessors
	inline void add(PropList& list, const T& value) const;
	inline void set(PropList& list, const T& value) const;
	inline elm::Option<T> get(const PropList& list) const;
	inline const T& get(const PropList& list, const T& def) const;
	inline const T& use(const PropList& list) const;
	inline const T& value(const PropList& list) const;
	inline Value value(PropList& list) const;
	inline void remove(PropList& list) const { list.removeProp(this); }
	inline bool exists(PropList& list) const { return list.getProp(this); } 
	
	// PropList* Accessors
	inline void add(PropList *list, const T& value) const;
	inline void set(PropList *list, const T& value) const;
	inline elm::Option<T> get(const PropList *list) const;
	inline const T& get(const PropList *list, const T& def) const;
	inline const T& use(const PropList *list) const;
	inline const T& value(const PropList *list) const;
	inline Value value(PropList *list) const;
	inline void remove(PropList *list) const { list->removeProp(this); }
	inline bool exists(PropList *list) const { return list->getProp(this); } 

	// Operators
	inline const T& operator()(const PropList& props) const;
	inline const T& operator()(const PropList *props) const;
	inline Value operator()(PropList& props) const;
	inline Value operator()(PropList *props) const;
	
	// Identifier overload
	virtual void print(elm::io::Output& output, const Property& prop) const;
	virtual const Type& type(void) const;
	virtual void scan(PropList& props, VarArg& args) const;	

	// Getter class
	class Getter: public PropList::Getter<T> {
	public:
		inline Getter(const PropList *list, Identifier<T>& id):
			PropList::Getter<T>(list, id) { }
		inline Getter(const PropList& list, Identifier<T>& id):
			PropList::Getter<T>(list, id) { }
	};
};


// Inlines
template <class T>
inline const T& Identifier<T>::get(const Property& prop) const {
	return ((const GenericProperty<T> &)prop).value();
}

template <class T>
inline Identifier<T>::Identifier(elm::CString name, NameSpace& ns)
: AbstractIdentifier(name, ns) {
}

template <class T>
inline Identifier<T>::Identifier(
	elm::CString name,
	const T& default_value,
	NameSpace& ns)
: AbstractIdentifier(name, ns), def(default_value) {
}

template <class T>
inline void Identifier<T>::add(PropList& list, const T& value) const {
	return list.add(*this, value);
}

template <class T>
inline void Identifier<T>::set(PropList& list, const T& value) const {
	list.set(*this, value);
}

template <class T>
inline elm::Option<T> Identifier<T>::get(const PropList& list) const {
	return list.get<T>(*this);
}

template <class T>
inline const T& Identifier<T>::get(const PropList& list, const T& def) const {
	return list.get(*this, def);
}

template <class T>
inline const T& Identifier<T>::use(const PropList& list) const {
	return list.use<T>(*this);
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
inline class Identifier<T>::Value
Identifier<T>::value(PropList& list) const {
	return Value(list, *this);
}
	

template <class T>
inline void Identifier<T>::add(PropList *list, const T& value) const {
	return list->add(*this, value);
}

template <class T>
inline void Identifier<T>::set(PropList *list, const T& value) const {
	list->set(*this, value);
}

template <class T>
inline elm::Option<T> Identifier<T>::get(const PropList *list) const {
	return list->get<T>(*this);
}

template <class T>
inline const T& Identifier<T>::get(const PropList *list, const T& def) const {
	return list->get(*this, def);
}

template <class T>
inline const T& Identifier<T>::use(const PropList *list) const {
	return list->use<T>(*this);
}

template <class T>
inline const T& Identifier<T>::value(const PropList *list) const {
	Property *prop = list->getProp(*this);
	if(!prop)
		return def;
	else
		return ((GenericProperty<T> *)prop)->value();
}

template <class T>
inline class Identifier<T>::Value
Identifier<T>::value(PropList *list) const {
	return Value(list, *this);
}

template <class T>	
void Identifier<T>::print(elm::io::Output& output, const Property& prop) const {
	output << get(prop);
}

template <class T>
const Type& Identifier<T>::type(void) const {
	return otawa::type<T>();
}

template <class T>
inline const T& Identifier<T>::operator()(const PropList& props) const {
	return value(props);
}

template <class T>
inline const T& Identifier<T>::operator()(const PropList *props) const {
	return value(props);
}

template <class T>
inline class Identifier<T>::Value
Identifier<T>::operator()(PropList& props) const {
	return value(props);
}

template <class T>
inline class Identifier<T>::Value
Identifier<T>::operator()(PropList *props) const {
	return value(props);
}


// GenericIdentifier<T>::print Specializations
template <>
void Identifier<char>::print(elm::io::Output& out, const Property& prop) const;
template <>
void Identifier<elm::CString>::print(elm::io::Output& out, const Property& prop) const;
template <>
void Identifier<elm::String>::print(elm::io::Output& out, const Property& prop) const;
template <>
void Identifier<PropList *>::print(elm::io::Output& out, const Property& prop) const;


// GenericIdentifier<T>::scan Specializations
template <>
void Identifier<elm::CString>::scan(PropList& props, VarArg& args) const;
template <>
void Identifier<elm::String>::scan(PropList& props, VarArg& args) const;


// Value class
template <class T>
inline Identifier<T>::Value::Value(PropList& _prop,
const Identifier<T>& _id): prop(_prop), id(_id) {
}

template <class T>
inline Identifier<T>::Value::Value(PropList *_prop, const Identifier<T>& _id)
: prop(*_prop), id(_id) {
}

template <class T>
inline class Identifier<T>::Value& Identifier<T>::Value::add(const T& value) {
	prop.add(id, value);
	return *this;
}

template <class T>
inline void Identifier<T>::Value::remove(void) {
	prop.removeProp(id);
}

template <class T>
inline bool Identifier<T>::Value::exists(void) const {
	return prop.hasProp(id);	
}

template <class T>
inline T& Identifier<T>::Value::ref(void) const {
	GenericProperty<T> *_prop = (GenericProperty<T> *)prop.getProp(&id);
	if(!_prop) {
		_prop = GenericProperty<T>::make(&id, id.def); 
		prop.addProp(_prop);
	}
	return _prop->value();
} 
 
template <class T>
inline Identifier<T>::Value::operator const T&(void) const {
	return prop.get<T>(id, id.def);
}

template <class T>
inline class Identifier<T>::Value&
Identifier<T>::Value::operator=(const T& value) {
	prop.set(id, value);
	return *this;
}

template <class T>
inline T Identifier<T>::Value::operator->(void) const {
	return prop.get<T>(id, id.def);
}


// VarArg management
template <class T>
inline void Identifier<T>::__scalar::scan(const Identifier<T>& id,
PropList& props, VarArg& args) {
	props.set(id, args.next<T>());
}

template <class T>
inline void Identifier<T>::__class::scan(const Identifier<T>& id,
PropList& props, VarArg& args) {
	T *ptr = args.next<T *>();
	props.set(id, *ptr);
}

template <class T>
void Identifier<T>::scan(PropList& props, VarArg& args) const {
	_if<type_info<T>::is_scalar, __scalar, __class>::_::scan(*this, props, args);
}

} // otawa

#endif	// OTAWA_PROP_IDENTIFIER_H
