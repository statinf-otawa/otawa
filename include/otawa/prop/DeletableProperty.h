/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/prop/DeletableProperty.h -- DeletableProperty class interface.
 */
#ifndef OTAWA_PROP_DELETABLE_PROPERTY_H
#define OTAWA_PROP_DELETABLE_PROPERTY_H

#include <otawa/prop/Property.h>

namespace otawa {

// DeletableProperty class
template <class T>
class DeletableProperty: public GenericProperty<T> {
public:
protected:
	virtual ~DeletableProperty(void);
	virtual Property *copy(void);
public:
	inline DeletableProperty(Identifier *id, T _value);
	inline DeletableProperty(Identifier& id, T _value);
	inline DeletableProperty(elm::CString name, T _value);
};


// Inlines
template <class T>
DeletableProperty<T>::~DeletableProperty(void) {
	delete GenericProperty<T>::getValue();
}

template <class T>
Property *DeletableProperty<T>::copy(void) {
	return new DeletableProperty<T>(
		GenericProperty<T>::getID(),
		GenericProperty<T>::getValue());
}

template <class T>
inline DeletableProperty<T>::DeletableProperty(Identifier *id, T _value) 
: GenericProperty<T>(id, _value) {
}

template <class T>
inline DeletableProperty<T>::DeletableProperty(Identifier& id, T _value) 
: GenericProperty<T>(id, _value) {
}

template <class T>
inline DeletableProperty<T>::DeletableProperty(elm::CString name, T _value)
: GenericProperty<T>(name, _value) {
}

} // otawa

#endif	// OTAWA_PROP_DELETABLE_PROPERTY_H
