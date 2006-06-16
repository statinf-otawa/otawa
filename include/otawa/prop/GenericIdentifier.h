/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	otawa/prop/GenericIdentifier.h -- interface to GenericIdentifier class.
 */
#ifndef OTAWA_PROP_GENERIC_IDENTIFIER_H
#define OTAWA_PROP_GENERIC_IDENTIFIER_H

#include <otawa/prop/Identifier.h>
#include <otawa/prop/Property.h>
#include <otawa/prop/PropList.h>

namespace otawa {

// External class
class PropList;

// GenericIdentifier class
template <class T>
class GenericIdentifier: public Identifier {
	T def;
public:
	inline GenericIdentifier(elm::CString name);
	inline GenericIdentifier(elm::CString name, const T& default_value);
	
	inline T& value(PropList& list);
	inline void add(PropList& list, const T& value);
	inline void set(PropList& list, const T& value);
	inline elm::Option<T> get(PropList& list);
	inline T get(PropList& list, const T& def);
	inline T use(PropList& list);
	
	inline T& value(PropList *list);
	inline void add(PropList *list, const T& value);
	inline void set(PropList *list, const T& value);
	inline elm::Option<T> get(PropList *list);
	inline T get(PropList *list, const T& def);
	inline T use(PropList *list);
	
	// Operators
	inline T& operator()(PropList& list);
	inline T& operator()(PropList *list);
	
	// Identifier overload
	virtual void print(elm::io::Output& output, const Property& prop);
	virtual const Type& type(void) const;
	//virtual void scan(PropList& props, VarArg& args) const;
};

// Inlines
template <class T>
inline GenericIdentifier<T>::GenericIdentifier(elm::CString name)
: Identifier(name) {
}

template <class T>
inline GenericIdentifier<T>::GenericIdentifier(elm::CString name,
const T& default_value): Identifier(name), def(default_value) {
}

template <class T>
inline T& GenericIdentifier<T>::value(PropList& list) {
	Property *prop = list.getProp(this);
	if(!prop) {
		prop = GenericProperty<T>::make(this, def);
		list.addProp(prop);
	}
	return ((GenericProperty<T> *)prop)->value();
}

template <class T>
inline void GenericIdentifier<T>::add(PropList& list, const T& value) {
	list.add<T>(*this, value);
}

template <class T>
inline void GenericIdentifier<T>::set(PropList& list, const T& value) {
	list.set<T>(*this, value);
}

template <class T>
inline elm::Option<T> GenericIdentifier<T>::get(PropList& list) {
	return list.get<T>(*this);
}

template <class T>
inline T GenericIdentifier<T>::get(PropList& list, const T& def) {
	return list.get<T>(*this, def);
}

template <class T>
inline T GenericIdentifier<T>::use(PropList& list) {
	return list.use<T>(*this);
}

template <class T>
inline T& GenericIdentifier<T>::value(PropList *list) {
	assert(list);
	return value(*list);
}

template <class T>
inline void GenericIdentifier<T>::add(PropList *list, const T& value) {
	add(*this, value);
}

template <class T>
inline void GenericIdentifier<T>::set(PropList *list, const T& value) {
	set(*this, value);
}

template <class T>
inline elm::Option<T> GenericIdentifier<T>::get(PropList *list) {
	return get(*this);
}

template <class T>
inline T GenericIdentifier<T>::get(PropList *list, const T& def) {
	return get(*list, def);
}

template <class T>
inline T GenericIdentifier<T>::use(PropList *list) {
	return use(*this);
}

template <class T>
inline T& GenericIdentifier<T>::operator()(PropList& list) {
	return value(list);
}

template <class T>
inline T& GenericIdentifier<T>::operator()(PropList *list) {
	assert(list);
	return value(*list);
}

template <class T>	
void GenericIdentifier<T>::print(elm::io::Output& output, const Property& prop) {
	output << ((const GenericProperty<T> &)prop).value();
}

template <class T>
const Type& GenericIdentifier<T>::type(void) const {
	return Type::no_type;
}

/*template <class T>
void GenericIdentifier<T>::scan(PropList& props, VarArg& args) const {
	props.set(*this, args.next<T>());
}*/

} // otawa

#endif // OTAWA_PROP_GENERIC_IDENTIFIER_H
