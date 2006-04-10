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
#include <elm/Iterator.h>
#include <otawa/prop/Property.h>
#include <otawa/prop/DeletableProperty.h>

namespace otawa {
	
// PropList class
class PropList {
	friend class PropFilter;
	mutable Property *head;
protected:
	virtual Property *getDeep(Identifier *id) const;
public:
	static const PropList EMPTY;
	inline PropList(const PropList& props) { addProps(props); };
	inline PropList(void): head(0) { };
	inline ~PropList(void) { clearProps(); };

	// Property access
	Property *getProp(Identifier *id) const;
	void setProp(Property *prop);
	void removeProp(Identifier *id);
	inline void setProp(Identifier *id) { setProp(new Property(id)); };
	void addProp(Property *prop);
	void removeAllProp(Identifier *id);
	inline bool hasProp(Identifier& id);
	
	// Property value access with identifier pointer (DEPRECATED)
	template <class T> inline T get(Identifier *id, const T def_value) const;
	template <class T> inline elm::Option<T> get(Identifier *id) const;
	template <class T> inline T use(Identifier *id) const;
	template <class T> inline void set(Identifier *id, const T value);
	template <class T> inline void add(Identifier *id, const T value);
	template <class T> inline void addLocked(Identifier *id, const T value);

	// Property value access with identifier reference
	template <class T> inline T get(Identifier& id, const T def_value) const;
	template <class T> inline elm::Option<T> get(Identifier& id) const;
	template <class T> inline T use(Identifier& id) const;
	template <class T> inline void set(Identifier& id, const T value);
	template <class T> inline void add(Identifier& id, const T value);
	template <class T> inline void addLocked(Identifier& id, const T value);
	template <class T> inline void addDeletable(Identifier& id, const T value);

	// Global management
	void clearProps(void);
	void addProps(const PropList& props);

	// PropIter class
	class PropIter: public PreIterator<PropIter, Property *> {
		Property *prop;
	public:
		inline PropIter(const PropList& list);
		inline PropIter(const PropList *list);
		inline void next(void);
		inline bool ended(void) const;
		inline Property *item(void) const;
		template <class T> inline T get(void) const;
		inline bool operator==(Identifier *id) const;
		inline bool operator!=(Identifier *id) const;
		inline bool operator==(Identifier& id) const;
		inline bool operator!=(Identifier& id) const;
	};

	// PropGetter class
	template <class T>
	class PropGetter: public PreIterator<PropGetter<T>, T> {
		PropIter iter;
		Identifier& _id;
		inline void look(void);
	public:
		inline PropGetter(const PropList *list, Identifier& id);
		inline PropGetter(const PropList& list, Identifier& id);
		inline bool ended(void) const;
		inline T item(void) const;
		inline void next(void);
	};

};


// PropList inlines
inline bool PropList::hasProp(Identifier& id) {
	return getProp(&id) != 0;	
}

template <class T> T PropList::get(Identifier *id, const T def_value) const {
	Property *prop = getProp(id);
	return !prop ? def_value : ((GenericProperty<T> *)prop)->value();
};

template <class T> elm::Option<T> PropList::get(Identifier *id) const {
	Property *prop = getProp(id);
	return !prop ? elm::Option<T>() : elm::Option<T>(((GenericProperty<T> *)prop)->value());
};

template <class T> T PropList::use(Identifier *id) const {
	Property *prop = getProp(id);
	if(!prop)
		assert(0);
	return ((const GenericProperty<T> *)prop)->value();
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

template <class T> T PropList::get(Identifier& id, const T def_value) const {
	Property *prop = getProp(&id);
	return !prop ? def_value : ((GenericProperty<T> *)prop)->value();
};

template <class T> elm::Option<T> PropList::get(Identifier& id) const {
	Property *prop = getProp(&id);
	return !prop ? elm::Option<T>() : elm::Option<T>(((GenericProperty<T> *)prop)->value());
};

template <class T> T PropList::use(Identifier& id) const {
	Property *prop = getProp(&id);
	if(!prop)
		assert(0);
	return ((GenericProperty<T> *)prop)->value();
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


// PropList::PropIter inlines
inline PropList::PropIter::PropIter(const PropList& list): prop(list.head) {
}

inline PropList::PropIter::PropIter(const PropList *list): prop(list->head) {
}

inline void PropList::PropIter::next(void) {
	assert(prop);
	prop = prop->next();
}

inline bool PropList::PropIter::ended(void) const {
	return prop == 0;
}

inline Property *PropList::PropIter::item(void) const {
	assert(prop);
	return prop;
}

inline bool PropList::PropIter::operator==(Identifier *id) const {
	return item()->id() == id;
}

inline bool PropList::PropIter::operator!=(Identifier *id) const {
	return item()->id() == id;
}

inline bool PropList::PropIter::operator==(Identifier& id) const {
	return item()->id() == &id;
}

inline bool PropList::PropIter::operator!=(Identifier& id) const {
	return item()->id() == &id;
}

template <class T>
inline T PropList::PropIter::get(void) const {
	return ((GenericProperty<T> *)prop)->value();
}

// PropList::PropGetter inlines
template <class T>
inline void PropList::PropGetter<T>::look(void) {
	for(; iter; iter++)
		if(iter->id() == &_id)
			return;
}

template <class T>
inline PropList::PropGetter<T>::PropGetter(const PropList *list, Identifier& id)
: iter(*list), _id(id) {
	look();
}

template <class T>
inline PropList::PropGetter<T>::PropGetter(const PropList& list, Identifier& id)
: iter(list), _id(id) {
	look();
}

template <class T>
inline bool PropList::PropGetter<T>::ended(void) const {
	return iter.ended();
}

template <class T>
inline T PropList::PropGetter<T>::item(void) const {
	return ((GenericProperty<T> *)iter.item())->value();
}

template <class T>
inline void PropList::PropGetter<T>::next(void) {
	iter++;
	look();
}

};	// otawa

#endif		// OTAWA_PROP_PROPLIST_H
