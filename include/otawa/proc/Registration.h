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

// Pre-declaration
class Processor;
class AbstractFeature;
namespace proc { class declare; }

// fast initialization
namespace p {
	const int end = 0;
	const int require = 1;		// AbstractFeature *
	const int provide = 2;		// AbstractFeature *
	const int invalidate = 3;	// AbstractFeature *
	const int use = 4;			// AbstractFeature *
	const int base = 5;			// AbstractRegistration *
	const int config = 6;		// AbstractIdentifier *
} // p

// FeatureUsage class
class FeatureUsage {
public:
	typedef enum kind_t {
		none,
		require,
		provide,
		invalidate,
		use
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
	inline const string& name(void) const { return _name; }
	inline const Version& version(void) const { return _version; }
	inline AbstractRegistration& base(void) const { return *_base; }
	virtual Processor *make(void) const = 0;
	virtual bool isFinal(void) const = 0;
	bool provides(const AbstractFeature& feature);
	bool requires(const AbstractFeature& feature);
	bool invalidates(const AbstractFeature& feature);
	bool uses(const AbstractFeature& feature);

	// Private use only
	void initialize(void);

protected:
	AbstractRegistration(void);
	AbstractRegistration(AbstractRegistration *base);
	AbstractRegistration(string name, Version version, AbstractRegistration *base);
	virtual ~AbstractRegistration(void) { }
	void init(cstring name, const Version& version, int tag, VarArg& args);
	void setFeatures(const SLList<FeatureUsage>& features);
	void setConfigs(const SLList<AbstractIdentifier *>& configs);
	void record(void);

private:
	friend class declare;
	friend class Processor;
	friend class ConfigIter;
	friend class FeatureIter;
	string _name;
	Version _version;
	AbstractRegistration *_base;
	SLList<AbstractIdentifier *> configs;
	SLList<FeatureUsage> features;
};


// NullRegistration class
class NullRegistration: public AbstractRegistration {
public:
	virtual Processor *make(void) const { return 0; }
	virtual bool isFinal(void) const { return false; }
};


// MetaRegistration class
class MetaRegistration: public AbstractRegistration {
public:
	inline MetaRegistration(cstring name, const Version& version, int tag, ...)
		{ VARARG_BEGIN(args, tag) init(name, version, tag, args); VARARG_END }
	inline MetaRegistration(cstring name, const Version& version, int tag, VarArg& args)
		{ init(name, version, tag, args); }
	virtual Processor *make(void) const { return 0; }
	virtual bool isFinal(void) const { return false; }
};

// Registration class
template <class T> class Registration: public AbstractRegistration {
public:
	inline Registration(void) { }
	inline Registration(cstring name, const Version& version, int tag, ...)
		{ VARARG_BEGIN(args, tag) init(name, version, tag, args); VARARG_END }
	inline Registration(cstring name, const Version& version, int tag, VarArg& args)
		{ init(name, version, tag, args); }
	virtual Processor *make(void) const { return new T; }
	virtual bool isFinal(void) const { return true; }
};


// AbstractMake class
class AbstractMaker {
public:
	virtual ~AbstractMaker(void) { }
	virtual Processor *make(void) const = 0;
};


// Make class
template <class C>
class Maker: public AbstractMaker {
public:
	virtual Processor *make(void) const { return new C(); }
};

// useful
extern AbstractMaker *null_maker, *no_maker;

namespace p {

// make class
class init {
	friend class declare;

	/*class AbstractMaker {
	public:
		virtual ~AbstractMaker(void) { }
		virtual Processor *make(void) = 0;
	};

	template <class T> class Maker: public AbstractMaker {
	public:
		virtual Processor *make(void) { return new T(); }
	};*/

public:
	inline init(string name, Version version)
		: _name(name), _version(version), _base(0), _maker(0) { }
	inline init(string name, Version version, AbstractRegistration& base)
		: _name(name), _version(version), _base(&base), _maker(0) { }
	inline init& require(const AbstractFeature& feature)
		{ features.add(FeatureUsage(FeatureUsage::require, feature)); return *this; }
	inline init& provide(const AbstractFeature& feature)
		{ features.add(FeatureUsage(FeatureUsage::provide, feature)); return *this; }
	inline init& invalidate(const AbstractFeature& feature)
		{ features.add(FeatureUsage(FeatureUsage::invalidate, feature)); return *this; }
	inline init& use(const AbstractFeature& feature)
		{ features.add(FeatureUsage(FeatureUsage::use, feature)); return *this; }
	inline init& config(AbstractIdentifier& id) { configs.add(&id); return *this; }
	inline init& base(AbstractRegistration& base) { _base = &base; return *this; }
	template <class T> inline init& maker(void) { _maker = new Maker<T>(); return *this; }

private:
	string _name;
	Version _version;
	AbstractRegistration *_base;
	SLList<AbstractIdentifier *> configs;
	SLList<FeatureUsage> features;
	AbstractMaker *_maker;
};


// declare class
class declare: public AbstractRegistration {
public:
	declare(otawa::p::init& i);
	virtual ~declare(void);
	virtual Processor *make(void) const;
	virtual bool isFinal(void) const;
private:
	AbstractMaker *_maker;
};


}	// proc

} // otawa

#endif /* OTAWA_PROC_REGISTRATION_H_ */
