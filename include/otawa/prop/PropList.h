/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	otawa/prop/PropList.h -- interface to PropList class.
 */
#ifndef OTAWA_PROP_PROPLIST_H
#define OTAWA_PROP_PROPLIST_H

#include <assert.h>
#include <elm/utility.h>
#include <otawa/prop/Property.h>
#include <otawa/prop/DeletableProperty.h>

namespace otawa {
	
// PropList class
class PropList {
	friend class PropIter;
	friend class PropFilter;
	Property *head;
protected:
	virtual Property *getDeep(Identifier *id);
public:
	inline PropList(const PropList& props) { addProps(props); };
	inline PropList(void): head(0) { };
	inline ~PropList(void) { clearProps(); };

	// Property access
	Property *getProp(Identifier *id);
	void setProp(Property *prop);
	void removeProp(Identifier *id);
	inline void setProp(Identifier *id) { setProp(new Property(id)); };
	void addProp(Property *prop);
	void removeAllProp(Identifier *id);
	
	// Property value access with identifier pointer (DEPRECATED)
	template <class T> inline T get(Identifier *id, const T def_value);
	template <class T> inline elm::Option<T> get(Identifier *id);
	template <class T> inline T& use(Identifier *id);
	template <class T> inline void set(Identifier *id, const T value);
	template <class T> inline void add(Identifier *id, const T value);
	template <class T> inline void addLocked(Identifier *id, const T value);

	// Property value access with identifier reference
	template <class T> inline T get(Identifier& id, const T def_value);
	template <class T> inline elm::Option<T> get(Identifier& id);
	template <class T> inline T& use(Identifier& id);
	template <class T> inline void set(Identifier& id, const T value);
	template <class T> inline void add(Identifier& id, const T value);
	template <class T> inline void addLocked(Identifier& id, const T value);
	template <class T> inline void addDeletable(Identifier& id, const T value);

	// Global management
	void clearProps(void);
	void addProps(const PropList& props);
};


// PropIter class
class PropIter {
	Property *prop;
public:
	inline PropIter(PropList& list): prop(list.head) { };
	inline void next(void) { assert(prop); prop = prop->next; };
	inline bool ended(void) const { return prop == 0; };
	inline Property *get(void) const { assert(prop); return prop; };
	
	// Operators
	inline operator bool(void) const { return !ended(); };
	inline operator Property *(void) const { return get(); };
	inline PropIter& operator++(void) { next(); return *this; };
	inline Property *operator->(void) const { return get(); };
	inline bool operator==(Identifier *id) const
		{ return get()->getID() == id; };
	inline bool operator!=(Identifier *id) const
		{ return get()->getID() == id; };
	inline bool operator==(Identifier& id) const
		{ return get()->getID() == &id; };
	inline bool operator!=(Identifier& id) const
		{ return get()->getID() == &id; };
};


// PropFilter class
class PropFilter {
	Identifier *id;
	Property *prop;
	inline void look(void) { while(prop && prop->getID() != id) prop->next; };
public:
	inline PropFilter(PropList& list, Identifier *_id)
		: id(_id), prop(list.head) { look(); };
	inline PropFilter(PropList& list, Identifier& _id)
		: id(&_id), prop(list.head) { look(); };
	inline void next(void) { assert(prop);  prop = prop->next; look(); };
	inline bool ended(void) const { return prop == 0; };
	inline Property *get(void) const { assert(prop); return prop; };
	
	// Operators
	inline operator bool(void) const { return !ended(); };
	inline operator Property *(void) const { return get(); };
	inline PropFilter& operator++(void) { next(); return *this; };
	inline Property *operator->(void) const { return get(); };
};


// PropList inlines
template <class T> T PropList::get(Identifier *id, const T def_value) {
	Property *prop = getProp(id);
	return !prop ? def_value : ((GenericProperty<T> *)prop)->getValue();
};

template <class T> elm::Option<T> PropList::get(Identifier *id) {
	Property *prop = getProp(id);
	return !prop ? elm::Option<T>() : elm::Option<T>(((GenericProperty<T> *)prop)->getValue());
};

template <class T> T& PropList::use(Identifier *id) {
	Property *prop = getProp(id);
	if(!prop)
		assert(0);
	return ((GenericProperty<T> *)prop)->getValue();
};

template <class T> void PropList::set(Identifier *id, const T value) {
	setProp(GenericProperty<T>::make(id, value));
};

template <class T>
inline void PropList::add(Identifier *id, const T value) {
	addProp(GenericProperty<T>::make(id, value));
};

template <class T>
inline void PropList::addLocked(Identifier *id, const T value) {
	addProp(LockedProperty<T>::make(id, value));
}

template <class T> T PropList::get(Identifier& id, const T def_value) {
	Property *prop = getProp(&id);
	return !prop ? def_value : ((GenericProperty<T> *)prop)->getValue();
};

template <class T> elm::Option<T> PropList::get(Identifier& id) {
	Property *prop = getProp(&id);
	return !prop ? elm::Option<T>() : elm::Option<T>(((GenericProperty<T> *)prop)->getValue());
};

template <class T> T& PropList::use(Identifier& id) {
	Property *prop = getProp(&id);
	if(!prop)
		assert(0);
	return ((GenericProperty<T> *)prop)->getValue();
};

template <class T> void PropList::set(Identifier& id, const T value) {
	setProp(GenericProperty<T>::make(&id, value));
};

template <class T>
inline void PropList::add(Identifier& id, const T value) {
	addProp(GenericProperty<T>::make(&id, value));
};

template <class T>
inline void PropList::addLocked(Identifier& id, const T value) {
	addProp(LockedProperty<T>::make(&id, value));
}

template <class T>
inline void PropList::addDeletable(Identifier& id, const T value) {
	addProp(new DeletableProperty<T>(id, value));
}

};	// otawa

#endif		// OTAWA_PROP_PROPLIST_H
