/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	properties.h -- object properties interface.
 */
#ifndef OTAWA_PROPERTIES_H
#define OTAWA_PROPERTIES_H

#include <assert.h>
#include <elm/string.h>
#include <elm/utility.h>
using namespace elm;

namespace otawa {

// Base types
typedef unsigned long id_t;
const id_t INVALID_ID = 0;

// Property description
class Property {
	friend class PropList;
	Property *next;
	id_t id;
protected:
	virtual ~Property(void) { };
	virtual Property *copy(void) { return new Property(id); };
public:
	static id_t getID(CString name);
	inline Property(id_t _id): id(_id) { };
	inline Property(CString name): id(getID(name)) { };
	inline id_t getID(void) const { return id; };
	inline Property *getNext(void) const { return next; };
};

// GenericProperty class
template <class T> class GenericProperty: public Property {
	T value;
protected:
	inline GenericProperty(id_t id, T _value): Property(id), value(_value) { };
	inline GenericProperty(CString name, T _value): Property(name), value(_value) { };
	virtual ~GenericProperty(void) { };
	virtual Property *copy(void) { return new GenericProperty<T>(getID(), value); };
public:
	static GenericProperty<T> *make(id_t id, const T value) {
		return new GenericProperty(id, value);
	};
	inline T& getValue(void) { return value; };
};

// LockedProperty class
template <class T> class LockedProperty: public GenericProperty<T> {
protected:
	virtual ~LockedProperty(void) {
		getValue()->unlock();
	};
	virtual Property *copy(void) {
		return new LockedProperty<T>(getID(), getValue());
	};
public:
	inline LockedProperty(id_t id, T _value): GenericProperty<T>(id, _value) {
		getValue()->lock();
	};
	inline LockedProperty(CString name, T _value): GenericProperty<T>(name, _value) {
		getValue()->lock();
	};
};

// PropList class
class PropList {
	Property *head;
protected:
	virtual Property *getDeep(id_t id);
public:
	inline PropList(const PropList& props) { addProps(props); };
	inline PropList(void): head(0) { };
	inline ~PropList(void) { clearProps(); };

	// Property access
	Property *getProp(id_t id);
	void setProp(Property *prop);
	void removeProp(id_t id);
	inline void setProp(id_t id) { setProp(new Property(id)); };
	
	// Property value access
	template <class T> inline T get(id_t id, const T def_value);
	template <class T> inline Option<T> get(id_t id);
	template <class T> inline T& use(id_t id);
	template <class T> inline void set(id_t id, const T value);

	// Global management
	void clearProps(void);
	void addProps(const PropList& props);
};


// PropList methods
template <class T> T PropList::get(id_t id, const T def_value) {
	Property *prop = getProp(id);
	return !prop ? def_value : ((GenericProperty<T> *)prop)->getValue();
};
template <class T> Option<T> PropList::get(id_t id) {
	Property *prop = getProp(id);
	return !prop ? Option<T>() : Option<T>(((GenericProperty<T> *)prop)->getValue());
};
template <class T> T& PropList::use(id_t id) {
	Property *prop = getProp(id);
	if(!prop)
		assert(0);
	return ((GenericProperty<T> *)prop)->getValue();
};
template <class T> void PropList::set(id_t id, const T value) {
	setProp(GenericProperty<T>::make(id, value));
};


// Lock Specialization
/*template <> inline GenericProperty<Lock *>::~GenericProperty(void) {
	value->unlock();
};
template <> inline GenericProperty<Lock *>::GenericProperty(id_t id, Lock *_value)
: Property(id), value(_value) {
	value->lock();
};
template <> inline GenericProperty<Lock *>::GenericProperty(CString name, Lock *_value)
: Property(name), value(_value) {
	value->lock();
};*/

};	// otawa

#endif		// PROJECT_PROPERTIES_H
