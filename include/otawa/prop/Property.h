/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	otawa/prop/Property.h -- interface to Property class and derived.
 */
#ifndef OTAWA_PROP_PROPERTY_H
#define OTAWA_PROP_PROPERTY_H

#include <assert.h>
#include <elm/string.h>
#include <elm/utility.h>
#include <otawa/prop/Identifier.h>

namespace otawa {

// Classes
class Identifier;


// Property description
class Property {
	friend class PropList;
	friend class PropIter;
	friend class PropFilter;
	Property *next;
	Identifier *id;
protected:
	virtual ~Property(void) { };
	virtual Property *copy(void) { return new Property(id); };
public:
	static Identifier *getID(elm::CString name);
	inline Property(Identifier *_id): id(_id) { };
	inline Property(Identifier& _id): id(&_id) { };
	inline Property(elm::CString name): id(getID(name)) { };
	inline Identifier *getID(void) const { return id; };
	inline Property *getNext(void) const { return next; };
};


// GenericProperty class
template <class T>
class GenericProperty: public Property {
	T value;
protected:
	inline GenericProperty(Identifier *id, T _value)
		: Property(id), value(_value) { };
	inline GenericProperty(Identifier& id, T _value)
		: Property(id), value(_value) { };
	inline GenericProperty(elm::CString name, T _value)
		: Property(name), value(_value) { };
	virtual ~GenericProperty(void) { };
	virtual Property *copy(void)
		{ return new GenericProperty<T>(getID(), value); };
public:
	static GenericProperty<T> *make(Identifier *id, const T value) {
		return new GenericProperty(id, value);
	};
	inline T& getValue(void) { return value; };
};


// LockedProperty class
template <class T>
class LockedProperty: public GenericProperty<T> {
protected:
	virtual ~LockedProperty(void) {
		getValue()->unlock();
	};
	virtual Property *copy(void) {
		return new LockedProperty<T>(getID(), getValue());
	};
public:
	inline LockedProperty(Identifier *id, T _value)
		: GenericProperty<T>(id, _value) { getValue()->lock(); };
	inline LockedProperty(Identifier& id, T _value)
		: GenericProperty<T>(id, _value) { getValue()->lock(); };
	inline LockedProperty(elm::CString name, T _value)
		: GenericProperty<T>(name, _value) { getValue()->lock(); };
};


} // otawa

#endif	// OTAWA_PROP_PROPERTY_H
