/*
 *	$Id$
 *	Registration class interface
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
#ifndef OTAWA_PROC_REGISTRATION_H_
#define OTAWA_PROC_REGISTRATION_H_

#include <elm/genstruct/SLList.h>
#include <elm/util/Initializer.h>
#include <elm/util/Version.h>
#include <otawa/prop/Identifier.h>

namespace otawa {

using namespace elm;
using namespace elm::genstruct;

class Processor;
class AbstractFeature;

// FeatureUsage class
class FeatureUsage {
public:
	typedef enum kind_t {
		none,
		require,
		provide,
		invalidate
	} kind_t;
	
	inline FeatureUsage(void): _kind(none), _feature(0) { }
	inline FeatureUsage(kind_t kind, const AbstractFeature& feature)
		: _kind(kind), _feature(&feature) { }
	inline FeatureUsage(const FeatureUsage& usage)
		: _kind(usage._kind), _feature(usage._feature) { }
	inline FeatureUsage& operator=(const FeatureUsage& usage)
		{ _kind = usage._kind; _feature = usage._feature; return *this; }
	
	inline kind_t kind(void) const { return _kind; }
	inline const AbstractFeature& feature(void) const { return *_feature; }

private:
	kind_t _kind;
	const AbstractFeature *_feature;
};


// AbstractRegistration class
class AbstractRegistration {
public:	
	inline string name(void) const { return _name; }
	inline Version version(void) const { return _version; }
	inline AbstractRegistration& base(void) const { return *_base; }
	virtual Processor *make(void) const = 0;
	virtual bool isFinal(void) const = 0;
	
	// Private use only
	void initialize(void);

protected:
	AbstractRegistration(void);
	virtual ~AbstractRegistration(void) { }

private:
	template <class T, class B, class C> friend class Registered;
	friend class Processor;
	friend class ConfigIter;
	friend class FeatureIter;
	string _name;
	Version _version;
	AbstractRegistration *_base;
	SLList<AbstractIdentifier *> configs;
	SLList<FeatureUsage> features;
	void record(void);
};


// NullRegistration class
class NullRegistration: public AbstractRegistration {
public:
	virtual Processor *make(void) const { return 0; }
	virtual bool isFinal(void) const { return false; }
};


// Registration class
template <class T> class Registration: public AbstractRegistration {
public:
	virtual Processor *make(void) const { return new T; }
	virtual bool isFinal(void) const { return true; }
};


// Registered class
template <class T, class B, class C = Registration<T> >
struct Registered: public B {
	typedef Registered<T, B, C> super;
	
	static struct __reg_init: C {
		__reg_init(void) {
			T::__reg.record();
		 	T::__reg._base = &B::__reg;
			T::init();
		};
	} __reg;

	Registered(void): B(__reg) { }
	Registered(AbstractRegistration& reg): B(reg) { }
	Registered(cstring name, const Version& version)
		: B(name, version, __reg) { }
	Registered(cstring name, const Version& version, AbstractRegistration& reg)
		: B(name, version, reg) { }

protected:
	inline static void _name(cstring name)
		{ __reg._name = name; }
	inline static void _version(int major, int minor, int release)
		{ __reg._version = Version(major, minor, release); }
	inline static void _config(AbstractIdentifier& id)
		{ __reg.configs.add(&id); }
	inline static void _require(AbstractFeature& feature)
		{ __reg.features.add(FeatureUsage(FeatureUsage::require, feature)); }
	inline static void _provide(AbstractFeature& feature)
		{ __reg.features.add(FeatureUsage(FeatureUsage::provide, feature)); }
	inline static void _invalidate(AbstractFeature& feature)
		{ __reg.features.add(FeatureUsage(FeatureUsage::invalidate, feature)); }
};

template <class T, class B, class C>
typename Registered<T, B, C>::__reg_init Registered<T, B, C>::__reg;

} // otawa

#endif /* OTAWA_PROC_REGISTRATION_H_ */
