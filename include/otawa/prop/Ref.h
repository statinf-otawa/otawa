/*
 *	$Id$
 *	Ref class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009, IRIT UPS.
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
#ifndef OTAWA_PROP_REF_H_
#define OTAWA_PROP_REF_H_

#include <otawa/prop/PropList.h>

namespace otawa {

// ImmutableRef class
template <class T, class I>
class ImmutableRef {
public:
	inline ImmutableRef(const PropList& _prop, const I& _id): prop(_prop), id(_id) { }
	inline ImmutableRef(const PropList *_prop, const I& _id): prop(*_prop), id(_id) { }
	inline ImmutableRef(const ImmutableRef<T, I>& ref): prop(ref.prop), id(ref.id)  { }
	inline const PropList& proplist(void) const { return prop; }
	inline const I& identifier(void) const { return id; }

	// accessors
	inline bool exists(void) const { return prop.hasProp(id); }
	inline const T& get(void) const { return id.value(prop); }
	inline void print(io::Output& out) const { id.print(out, prop.getProp(&id)); }
	inline operator const T&(void) const { return id.get(prop, id.defaultValue()); }
	inline T operator->(void) const { return id.get(prop, id.defaultValue()); }

protected:
	const PropList& prop;
	const I& id;
};


// Ref class
template <class T, class I>
class Ref: public ImmutableRef<T, I> {
public:
	inline Ref(PropList& _prop, const I& _id): ImmutableRef<T, I>(_prop, _id) { }
	inline Ref(PropList *_prop, const I& _id): ImmutableRef<T, I>(_prop, _id) { }
	inline Ref(const Ref<T, I>& ref): ImmutableRef<T, I>(ref)  { }

	// accessors
	inline PropList& props(void) const { return const_cast<PropList&>(ImmutableRef<T, I>::proplist()); }
	inline const I& id(void) const { return ImmutableRef<T, I>::identifier(); }

	// mutators
	inline const Ref& add(const T& value) const { id().add(props(), value); return *this; }
	inline void remove(void) const { props().removeProp(id()); }
	inline T& ref(void) const { return id().ref(props()); }

	inline T& operator*(void) const { return ref(); }
	inline const Ref<T, I>& operator=(const T& value) const { id().set(props(), value); return *this; }
	inline Ref<T, I>& operator=(const Ref<T, I>& value) { id().set(props(), value.get()); return *this; }
	inline Ref<T, I>& operator+=(const T& v) { ref() +=  v; return *this; }
	inline Ref<T, I>& operator-=(const T& v) { ref() -=  v; return *this; }
	inline Ref<T, I>& operator*=(const T& v) { ref() *=  v; return *this; }
	inline Ref<T, I>& operator/=(const T& v) { ref() /=  v; return *this; }
	inline Ref<T, I>& operator%=(const T& v) const { ref() %=  v; return *this; }
	inline Ref<T, I>& operator&=(const T& v) const { ref() &=  v; return *this; }
	inline Ref<T, I>& operator|=(const T& v) const { ref() |=  v; return *this; }
	inline Ref<T, I>& operator^=(const T& v) const { ref() ^=  v; return *this; }
	inline Ref<T, I>& operator<<=(const T& v) const { ref() <<=  v; return *this; }
	inline Ref<T, I>& operator>>=(const T& v) const { ref() >>=  v; return *this; }
	inline Ref<T, I>& operator++(void) const { ref()++; return *this; }
	inline Ref<T, I>& operator--(void) const { ref()--; return *this; }
	inline Ref<T, I>& operator++(int) const { ref()++; return *this; }
	inline Ref<T, I>& operator--(int) const { ref()--; return *this; }
};

// output
template <class T, class I>
io::Output& operator<<(io::Output& out, const Ref<T, I>& ref) { ref.print(out); return out; }
template <class T, class I>
io::Output& operator<<(io::Output& out, const ImmutableRef<T, I>& ref) { ref.print(out); return out; }

}	// otawa

#endif /* OTAWA_PROP_REF_H_ */
