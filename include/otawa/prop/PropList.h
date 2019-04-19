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
#include <otawa/base.h>

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
	friend class RCURunnable;
	friend class WorkSpace;
public:
	static const AbstractIdentifier *getID(elm::CString name);
	inline Property(const AbstractIdentifier *id): _id(id), _next(0) { };
	inline Property(const AbstractIdentifier& id): _id(&id), _next(0) { };
	inline Property(elm::CString name): _id(getID(name)) { };
	inline const AbstractIdentifier *id(void) const { return _id; };
	inline Property *next(void) const { return _next; };
	template <class T> inline const T& get(void) const;
	template <class T> inline void set(const T& value);
	virtual void print(elm::io::Output& out) const;
protected:
	virtual ~Property(void) { };
	virtual Property *copy(void) { return new Property(_id); };
private:
	const AbstractIdentifier *_id;
	mutable Property *_next;
};


// PropList class
class PropList {
	mutable Property *head;
public:
	static rtti::Type& __type;
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
	void takeProps(PropList& props);
	void print(elm::io::Output& out) const;
	inline PropList& operator=(const PropList& props)
		{ clearProps(); addProps(props); return *this; }

	// Iter class
	class Iter: public elm::PreIterator<Iter, Property *> {
	public:
		inline Iter(void): prop(nullptr) { }
		inline Iter(const PropList& list): prop(list.head) { }
		inline Iter(const PropList *list): prop(list->head) { }
		inline void next(void) { ASSERT(prop); prop = prop->next(); }
		inline bool ended(void) const { return prop == 0; }
		inline Property *item(void) const { ASSERT(prop); return prop; }
		inline bool equals(const Iter& i) const { return prop == i.prop; }

		inline bool operator==(const AbstractIdentifier *id) const
			{ return item()->id() == id; }
		inline bool operator!=(const AbstractIdentifier *id) const
			{ return item()->id() != id; }
		inline bool operator==(const AbstractIdentifier& id) const
			{ return item()->id() == &id; }
		inline bool operator!=(const AbstractIdentifier& id) const
			{ return item()->id() != &id; }
	private:
		Property *prop;
	};

	// PropRange class
	class PropRange {
	public:
		inline PropRange(const PropList& props): _props(props) { }
		inline Iter begin(void) const { return Iter(_props); }
		inline Iter end(void) const { return Iter(); }
	private:
		const PropList& _props;
	};
	inline PropRange properties(void) const { return PropRange(*this); }

	// Getter class
	class Getter: public elm::PreIterator<Getter, Property *> {
	public:
		inline Getter(void): _id(nullptr) { }
		inline Getter(const PropList *list, const AbstractIdentifier& id)
			: iter(*list), _id(&id) { look(); }
		inline Getter(const PropList& list, const AbstractIdentifier& id)
			: iter(list), _id(&id) { look(); }
		inline bool ended(void) const { return iter.ended(); }
		inline Property *item(void) const { return iter.item(); }
		inline void next(void) { iter.next(); look(); }
		inline bool equals(const Getter& g) const { return iter.equals(g.iter); }
	private:
		Iter iter;
		const AbstractIdentifier *_id;
		inline void look(void)
			{ for(; iter(); iter++) if(iter->id() == _id) return; }
	};

	// GetterRange class
	class GetterRange {
	public:
		inline GetterRange(const AbstractIdentifier& id, const PropList& props)
			: _id(id), _props(props) { }
		inline Getter begin(void) const { return Getter(_props, _id); }
		inline Getter end(void) const { return Getter(); }
	private:
		const AbstractIdentifier& _id;
		const PropList& _props;
	};
	inline GetterRange all(const AbstractIdentifier& id) const { return GetterRange(id, *this); }

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
