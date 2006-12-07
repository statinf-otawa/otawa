/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	otawa/prop/GenericIdentifier.h -- interface to GenericIdentifier class.
 */
#ifndef OTAWA_PROP_GENERIC_IDENTIFIER_H
#define OTAWA_PROP_GENERIC_IDENTIFIER_H

#include <elm/rtti.h>
#include <elm/meta.h>
#include <otawa/prop/Identifier.h>
#include <otawa/prop/Property.h>
#include <otawa/prop/PropList.h>

namespace otawa {

using namespace elm;
using namespace elm::io;

// External class
class PropList;

// GenericIdentifier class
template <class T>
class GenericIdentifier: public Identifier {
	T def;
	inline const T& get(const Property& prop) const;

	typedef struct {
		static inline void scan(const GenericIdentifier<T>& id, PropList& props,
			VarArg& args);
	} __scalar;

	typedef struct {
		static inline void scan(const GenericIdentifier<T>& id, PropList& props,
			VarArg& args);
	} __class;

public:

	// Value class
	class Value {
		PropList& prop;
		const GenericIdentifier<T>& id;
	public:
		inline Value(PropList& prop, const GenericIdentifier<T>& id);
		inline Value(PropList *prop, const GenericIdentifier<T>& id);
		inline operator T(void) const;
		inline Value& operator=(const T& value);
		inline Value& operator+=(const T& value);
		inline T operator->(void) const; 
	};

	// Constructors
	inline GenericIdentifier(elm::CString name, NameSpace& ns = ROOT_NS);
	inline GenericIdentifier(elm::CString name, const T& default_value, NameSpace& = ROOT_NS);
	
	// PropList& Accessors
	inline void add(PropList& list, const T& value) const;
	inline void set(PropList& list, const T& value) const;
	//inline elm::Option<T> get(const PropList& list) const;
	inline const T& get(const PropList& list, const T& def) const;
	inline const T& use(const PropList& list) const;
	inline const T& value(const PropList& list) const;
	inline Value value(PropList& list) const;
	
	// PropList* Accessors
	inline void add(PropList *list, const T& value) const;
	inline void set(PropList *list, const T& value) const;
	//inline elm::Option<T> get(const PropList *list) const;
	inline const T& get(const PropList *list, const T& def) const;
	inline const T& use(const PropList *list) const;
	inline const T& value(const PropList *list) const;
	inline Value value(PropList *list) const;

	// Operators
	inline const T& operator()(const PropList& props) const;
	inline const T& operator()(const PropList *props) const;
	inline Value operator()(PropList& props) const;
	inline Value operator()(PropList *props) const;
	
	// Identifier overload
	virtual void print(elm::io::Output& output, const Property& prop) const;
	virtual const Type& type(void) const;
	virtual void scan(PropList& props, VarArg& args) const;	
};

// Inlines
template <class T>
inline const T& GenericIdentifier<T>::get(const Property& prop) const {
	return ((const GenericProperty<T> &)prop).value();
}

template <class T>
inline GenericIdentifier<T>::GenericIdentifier(elm::CString name, NameSpace& ns)
: Identifier(name, ns) {
}

template <class T>
inline GenericIdentifier<T>::GenericIdentifier(
	elm::CString name,
	const T& default_value,
	NameSpace& ns)
: Identifier(name, ns), def(default_value) {
}

template <class T>
inline void GenericIdentifier<T>::add(PropList& list, const T& value) const {
	return list.add(*this, value);
}

template <class T>
inline void GenericIdentifier<T>::set(PropList& list, const T& value) const {
	list.set(*this, value);
}

/*template <class T>
inline elm::Option<T> GenericIdentifier<T>::get(const PropList& list) const {
	return list.get<T>(*this);
}*/

template <class T>
inline const T& GenericIdentifier<T>::get(const PropList& list, const T& def) const {
	return list.get(*this, def);
}

template <class T>
inline const T& GenericIdentifier<T>::use(const PropList& list) const {
	return list.use<T>(*this);
}

template <class T>
inline const T& GenericIdentifier<T>::value(const PropList& list) const {
	Property *prop = list.getProp(this);
	if(!prop)
		return def;
	else
		return ((GenericProperty<T> *)prop)->value();
}

template <class T>
inline class GenericIdentifier<T>::Value
GenericIdentifier<T>::value(PropList& list) const {
	return Value(list, *this);
}
	

template <class T>
inline void GenericIdentifier<T>::add(PropList *list, const T& value) const {
	return list->add(*this, value);
}

template <class T>
inline void GenericIdentifier<T>::set(PropList *list, const T& value) const {
	list->set(*this, value);
}

/*template <class T>
inline elm::Option<T> GenericIdentifier<T>::get(const PropList *list) const {
	return list->get<T>(*this);
}*/

template <class T>
inline const T& GenericIdentifier<T>::get(const PropList *list, const T& def) const {
	return list->get(*this, def);
}

template <class T>
inline const T& GenericIdentifier<T>::use(const PropList *list) const {
	return list->use<T>(*this);
}

template <class T>
inline const T& GenericIdentifier<T>::value(const PropList *list) const {
	Property *prop = list->getProp(*this);
	if(!prop)
		return def;
	else
		return ((GenericProperty<T> *)prop)->value();
}

template <class T>
inline class GenericIdentifier<T>::Value
GenericIdentifier<T>::value(PropList *list) const {
	return Value(list, *this);
}

template <class T>	
void GenericIdentifier<T>::print(elm::io::Output& output, const Property& prop) const {
	output << get(prop);
}

template <class T>
const Type& GenericIdentifier<T>::type(void) const {
	return Type::no_type;
}

template <class T>
inline const T& GenericIdentifier<T>::operator()(const PropList& props) const {
	return value(props);
}

template <class T>
inline const T& GenericIdentifier<T>::operator()(const PropList *props) const {
	return value(props);
}

template <class T>
inline class GenericIdentifier<T>::Value
GenericIdentifier<T>::operator()(PropList& props) const {
	return value(props);
}

template <class T>
inline class GenericIdentifier<T>::Value
GenericIdentifier<T>::operator()(PropList *props) const {
	return value(props);
}


// GenericIdentifier<T>::print Specializations
template <>
void GenericIdentifier<char>::print(elm::io::Output& out, const Property& prop) const;
template <>
void GenericIdentifier<elm::CString>::print(elm::io::Output& out, const Property& prop) const;
template <>
void GenericIdentifier<elm::String>::print(elm::io::Output& out, const Property& prop) const;
template <>
void GenericIdentifier<PropList *>::print(elm::io::Output& out, const Property& prop) const;


// GenericIdentifier<T>::scan Specializations
template <>
void GenericIdentifier<elm::CString>::scan(PropList& props, VarArg& args) const;
template <>
void GenericIdentifier<elm::String>::scan(PropList& props, VarArg& args) const;


// Value class
template <class T>
inline GenericIdentifier<T>::Value::Value(PropList& _prop,
const GenericIdentifier<T>& _id): prop(_prop), id(_id) {
}

template <class T>
inline GenericIdentifier<T>::Value::Value(PropList *_prop,
const GenericIdentifier<T>& _id): prop(*_prop), id(_id) {
}

template <class T>
inline GenericIdentifier<T>::Value::operator T(void) const {
	return prop.get<T>(id, id.def);
}

template <class T>
inline class GenericIdentifier<T>::Value&
GenericIdentifier<T>::Value::operator=(const T& value) {
	prop.set(id, value);
	return *this;
}

template <class T>
inline class GenericIdentifier<T>::Value&
GenericIdentifier<T>::Value::operator+=(const T& value) {
	prop.add(id, value);
	return *this;
}

template <class T>
inline T GenericIdentifier<T>::Value::operator->(void) const {
	return prop.get<T>(id, id.def);
} 


// VarArg management
template <class T>
inline void GenericIdentifier<T>::__scalar::scan(const GenericIdentifier<T>& id,
PropList& props, VarArg& args) {
	props.set(id, args.next<T>());
}

template <class T>
inline void GenericIdentifier<T>::__class::scan(const GenericIdentifier<T>& id,
PropList& props, VarArg& args) {
	T *ptr = args.next<T *>();
	props.set(id, *ptr);
}

template <class T>
void GenericIdentifier<T>::scan(PropList& props, VarArg& args) const {
	_if<type_info<T>::is_scalar, __scalar, __class>::_::scan(*this, props, args);
}

} // otawa

#endif // OTAWA_PROP_GENERIC_IDENTIFIER_H
