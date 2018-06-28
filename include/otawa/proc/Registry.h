/*
 *	$Id$
 *	Registry class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-7, IRIT UPS.
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
#ifndef OTAWA_PROC_PROC_REGISTRY_H
#define OTAWA_PROC_PROC_REGISTRY_H

#include <elm/data/HashMap.h>
#include <elm/PreIterator.h>
#include <otawa/proc/Registration.h>

namespace otawa {

// Registry class
class Registry: public Initializer<AbstractRegistration> {
	friend class AbstractRegistration;
	typedef HashMap<String, const AbstractRegistration *> htab_t;

public:
	static const AbstractRegistration *find(string name);

	// Iter class
	class Iter: public htab_t::Iter {
	public:
		inline Iter(void): htab_t::Iter(_.procs) { }
	};

private:
	htab_t procs;
	static Registry _;
	Registry(void);
};


// Macro
template <class T>
inline const AbstractRegistration& registration(void) { return T::__reg; }


// ConfigIter class
class ConfigIter: public PreIterator<ConfigIter, AbstractIdentifier *> {
public:
	inline ConfigIter(const AbstractRegistration& registration, bool all = true)
		: reg(&registration), iter(reg->configs), _all(all) { step(); }
	inline AbstractIdentifier *item(void) const { return iter.item(); }
	inline void next(void) { iter.next(); step(); }
	inline bool ended(void) const { return !reg; }
	
private:
	void step(void);
	const AbstractRegistration *reg;
	List<AbstractIdentifier *>::Iter iter;
	bool _all;
};


// FeatureIter class
class FeatureIter: public PreIterator<FeatureIter, const FeatureUsage *> {
public:
	inline FeatureIter(const AbstractRegistration& registration)
		: reg(&registration), iter(reg->_feats) { step(); }
	inline const FeatureUsage *item(void) const { return &iter.item(); }
	inline void next(void) { iter.next(); step(); }
	inline bool ended(void) const { return iter.ended(); }
	
private:
	void step(void);
	const AbstractRegistration *reg;
	List<FeatureUsage>::Iter iter;
};

} // otawa

#endif	// OTAWA_PROC_PROC_REGISTER_H 
