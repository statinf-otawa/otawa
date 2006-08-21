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
#include <elm/io.h>
#include <otawa/prop/Identifier.h>

namespace otawa {

// Classes
class Identifier;


// Property description
class Property {
	friend class PropList;
	Property *_next;
	const Identifier *_id;
protected:
	virtual ~Property(void) { };
	virtual Property *copy(void) { return new Property(_id); };
public:
	static const Identifier *getID(elm::CString name);
	inline Property(const Identifier *id): _id(id) { };
	inline Property(const Identifier& id): _id(&id) { };
	inline Property(elm::CString name): _id(getID(name)) { };
	inline const Identifier *id(void) const { return _id; };
	inline Property *next(void) const { return _next; };
	virtual void print(elm::io::Output& out) const;
};


// GenericProperty class
template <class T>
class GenericProperty: public Property {
	T _value;
protected:
	inline GenericProperty(const Identifier *id, T value)
		: Property(id), _value(value) { };
	inline GenericProperty(const Identifier& id, T value)
		: Property(id), _value(value) { };
	inline GenericProperty(elm::CString name, T value)
		: Property(name), _value(value) { };
	virtual ~GenericProperty(void) { };
	virtual Property *copy(void)
		{ return new GenericProperty<T>(id(), value()); };
public:
	static GenericProperty<T> *make(const Identifier *id, const T value) {
		return new GenericProperty(id, value);
	};
	inline const T& value(void) const { return _value; };
	inline T& value(void) { return _value; };
};


// LockedProperty class
template <class T>
class LockedProperty: public GenericProperty<T> {
protected:
	virtual ~LockedProperty(void) {
		GenericProperty<T>::value()->unlock();
	};
	virtual Property *copy(void) {
		return new LockedProperty<T>(
			GenericProperty<T>::id(),
			GenericProperty<T>::value());
	};
public:
	inline LockedProperty(const Identifier *id, T value)
		: GenericProperty<T>(id, value) { GenericProperty<T>::value()->lock(); };
	inline LockedProperty(const Identifier& id, T value)
		: GenericProperty<T>(id, value) { GenericProperty<T>::value()->lock(); };
	inline LockedProperty(elm::CString name, T value)
		: GenericProperty<T>(name, value) { GenericProperty<T>::value()->lock(); };
};


// Property Inlines
inline elm::io::Output& operator<<(elm::io::Output& out, const Property *prop) {
	prop->print(out);
	return out;
}

} // otawa

#endif	// OTAWA_PROP_PROPERTY_H
