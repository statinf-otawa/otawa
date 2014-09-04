/*
 *	PropList class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-8, IRIT UPS.
 *
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef OTAWA_PROP_PROPLIST_H
#define OTAWA_PROP_PROPLIST_H

#include <elm/utility.h>
#include <elm/PreIterator.h>
#include <elm/util/VarArg.h>

namespace elm { template <class T> class Initializer; }

namespace otawa {

// pre-declaration
class AbstractIdentifier;
class Type;


// Constants
extern const AbstractIdentifier END;


// Property description
class Property {
	friend class PropList;
	mutable Property *_next;
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


// PropList class
class PropList {
	mutable Property *head;
public:
	static const PropList EMPTY;
	inline PropList(const PropList& props): head(0) { addProps(props); };
	inline PropList(void): head(0) { };
	inline ~PropList(void) { clearProps(); };

	// Property access
	Property *getProp(const AbstractIdentifier *id) const;
	void setProp(Property *prop);
	void removeProp(const AbstractIdentifier *id);
	inline void removeProp(const AbstractIdentifier& id) { removeProp(&id); }
	Property *extractProp(const AbstractIdentifier *id);
	inline Property *extractProp(const AbstractIdentifier& id) { return extractProp(&id); }
	inline void setProp(const AbstractIdentifier *id) { setProp(new Property(id)); };
	void addProp(Property *prop);
	void removeAllProp(const AbstractIdentifier *id);
	inline bool hasProp(const AbstractIdentifier& id) const
		{ return getProp(&id) != 0; }

	// Global management
	void clearProps(void);
	void addProps(const PropList& props);
	void print(elm::io::Output& out) const;
	inline PropList& operator=(const PropList& props)
		{ clearProps(); addProps(props); return *this; }

	// Iter class
	class Iter: public elm::PreIterator<Iter, Property *> {
		Property *prop;
	public:
		inline Iter(const PropList& list): prop(list.head) { }
		inline Iter(const PropList *list): prop(list->head) { }
		inline void next(void) { ASSERT(prop); prop = prop->next(); }
		inline bool ended(void) const { return prop == 0; }
		inline Property *item(void) const { ASSERT(prop); return prop; }
		inline bool operator==(const AbstractIdentifier *id) const
			{ return item()->id() == id; }
		inline bool operator!=(const AbstractIdentifier *id) const
			{ return item()->id() != id; }
		inline bool operator==(const AbstractIdentifier& id) const
			{ return item()->id() == &id; }
		inline bool operator!=(const AbstractIdentifier& id) const
			{ return item()->id() != &id; }
	};

	// Getter class
	class Getter: public elm::PreIterator<Getter, Property *> {
	public:
		inline Getter(const PropList *list, const AbstractIdentifier& id)
			: iter(*list), _id(id) { look(); }
		inline Getter(const PropList& list, const AbstractIdentifier& id)
			: iter(list), _id(id) { look(); }
		inline bool ended(void) const { return iter.ended(); }
		inline Property *item(void) const { return iter.item(); }
		inline void next(void) { iter.next(); look(); }
	private:
		Iter iter;
		const AbstractIdentifier& _id;
		inline void look(void)
			{ for(; iter; iter++) if(iter->id() == &_id) return; }
	};

};


// output
inline elm::io::Output& operator<<(elm::io::Output& out, const PropList& props) {
	props.print(out);
	return out;
}

inline elm::io::Output& operator<<(elm::io::Output& out, const PropList *props) {
	props->print(out);
	return out;
}

};	// otawa

#endif		// OTAWA_PROP_PROPLIST_H
