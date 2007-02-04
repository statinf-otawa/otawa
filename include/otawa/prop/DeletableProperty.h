/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/prop/DeletableProperty.h -- DeletableProperty class interface.
 */
#ifndef OTAWA_PROP_DELETABLE_PROPERTY_H
#define OTAWA_PROP_DELETABLE_PROPERTY_H

#include <elm/string.h>
#include <otawa/prop/Property.h>

namespace otawa {

// DeletableProperty class
template <class T>
class DeletableProperty: public GenericProperty<T> {
public:
protected:
	virtual ~DeletableProperty(void);
	//virtual Property *copy(void);
public:
	inline DeletableProperty(const AbstractIdentifier *id, T value);
	inline DeletableProperty(const AbstractIdentifier& id, T value);
	inline DeletableProperty(elm::CString name, T value);
};


// Inlines
template <class T>
DeletableProperty<T>::~DeletableProperty(void) {
	delete GenericProperty<T>::value();
}

/*template <class T>
Property *DeletableProperty<T>::copy(void) {
	return new GenericProperty<T>::copy();
}*/

template <class T>
inline DeletableProperty<T>::DeletableProperty(const AbstractIdentifier *id, T value) 
: GenericProperty<T>(id, value) {
}

template <class T>
inline DeletableProperty<T>::DeletableProperty(const AbstractIdentifier& id, T value) 
: GenericProperty<T>(id, value) {
}

template <class T>
inline DeletableProperty<T>::DeletableProperty(elm::CString name, T value)
: GenericProperty<T>(name, value) {
}

} // otawa

#endif	// OTAWA_PROP_DELETABLE_PROPERTY_H
