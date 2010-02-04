/*
 *	$Id$
 *	ParamFeature and ActualFeature classes interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2010, IRIT UPS <casse@irit.fr>
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
#ifndef OTAWA_PROC_PARAMFEATURE_H_
#define OTAWA_PROC_PARAMFEATURE_H_

#include <elm/genstruct/Vector.h>
#include <otawa/proc/Feature.h>
#include <otawa/prog/WorkSpace.h>

namespace otawa {

// external definition
class AbstractParamIdentifier;
class AbstractParamProcessor;
class ParamFeature;

// ActualFeature class
class ActualFeature: public AbstractFeature {
public:
	ActualFeature(const ParamFeature& feature, const AbstractParamProcessor& def);
	inline const ParamFeature& feature(void) const { return feat; }
	inline const AbstractParamProcessor& processor(void) { return proc; }

	// AbstractFeature overload
	virtual void process(WorkSpace *ws, const PropList &props=PropList::EMPTY) const;
	virtual void check(WorkSpace*) const { }
	virtual void clean(WorkSpace*) const { }

private:
	~ActualFeature(void);
	void lock(void);
	void unlock(void);

	const ParamFeature& feat;
	const AbstractParamProcessor& proc;
	int cnt;
};

// ParamFeature class
class ParamFeature: public Identifier<AbstractParamProcessor *> {
public:
	static Identifier<AbstractParamIdentifier *> PARAM_ID;

	ParamFeature(cstring id, AbstractParamProcessor& def);
	ActualFeature *instantiate(const PropList& props);

private:
	bool matches(ActualFeature *feat, const PropList& def);

	const AbstractParamProcessor& proc;
	genstruct::Vector<ActualFeature *> feats;
};

// AbstractParamIdentifier class
class AbstractParamIdentifier: public Identifier<AbstractIdentifier *> {
public:
	static AbstractIdentifier ACTUAL_ID;

	AbstractParamIdentifier(ParamFeature& feature, cstring name);
	virtual AbstractIdentifier *instantiate(ActualFeature& feature) const = 0;
};

// ParamIdentifier class
template <class T>
class ParamIdentifier: public AbstractParamIdentifier {
public:
	inline ParamIdentifier(ParamFeature& feature, cstring name)
		: AbstractParamIdentifier(feature, name) { }
	inline ParamIdentifier(ParamFeature& feature, cstring name, const T& def)
		: AbstractParamIdentifier(feature, name), _def(def) { }

	inline Identifier<T>& operator[](const ActualFeature& feat) const
		{ return *static_cast<Identifier<T> *>(use(feat)); }
	inline Identifier<T>& operator[](const ActualFeature *feat) const
		{ return *static_cast<Identifier<T> *>(use(feat)); }

	virtual AbstractIdentifier *instantiate(ActualFeature& feature) const { return new Identifier<T>("", _def); }

private:
	T _def;
};

}	// otawa

#endif /* OTAWA_PROC_PARAMFEATURE_H_ */
