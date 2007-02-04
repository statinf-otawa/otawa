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
#include <otawa/prop/AbstractIdentifier.h>

namespace otawa {

// Property description
class Property {
	friend class PropList;
	Property *_next;
	const AbstractIdentifier *_id;
protected:
	virtual ~Property(void) { };
	virtual Property *copy(void) { return new Property(_id); };
public:
	static const AbstractIdentifier *getID(elm::CString name);
	inline Property(const AbstractIdentifier *id): _id(id) { };
	inline Property(const AbstractIdentifier& id): _id(&id) { };
	inline Property(elm::CString name): _id(getID(name)) { };
	inline const AbstractIdentifier *id(void) const { return _id; };
	inline Property *next(void) const { return _next; };
	template <class T> inline const T& get(void) const;
	template <class T> inline void set(const T& value);
	virtual void print(elm::io::Output& out) const;
};


// GenericProperty class
template <class T>
class GenericProperty: public Property {
	T _value;
protected:
	inline GenericProperty(const AbstractIdentifier *id, T value)
		: Property(id), _value(value) { };
	inline GenericProperty(const AbstractIdentifier& id, T value)
		: Property(id), _value(value) { };
	inline GenericProperty(elm::CString name, T value)
		: Property(name), _value(value) { };
	virtual ~GenericProperty(void) { };
	virtual Property *copy(void)
		{ return new GenericProperty<T>(id(), value()); };
public:
	static GenericProperty<T> *make(const AbstractIdentifier *id, const T value) {
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
	inline LockedProperty(const AbstractIdentifier *id, T value)
		: GenericProperty<T>(id, value) { GenericProperty<T>::value()->lock(); };
	inline LockedProperty(const AbstractIdentifier& id, T value)
		: GenericProperty<T>(id, value) { GenericProperty<T>::value()->lock(); };
	inline LockedProperty(elm::CString name, T value)
		: GenericProperty<T>(name, value) { GenericProperty<T>::value()->lock(); };
};


// Property Inlines
template <class T>
inline const T& Property::get(void) const {
	return ((const GenericProperty<T> *)this)->value();
}

template <class T>
inline void Property::set(const T& value) {
	((GenericProperty<T> *)this)->value() = value;
}

inline elm::io::Output& operator<<(elm::io::Output& out, const Property *prop) {
	prop->print(out);
	return out;
}

} // otawa

#endif	// OTAWA_PROP_PROPERTY_H
