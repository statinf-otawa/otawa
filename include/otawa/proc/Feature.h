/*
 *	$Id$
 *	feature module interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-08, IRIT UPS.
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
#ifndef OTAWA_PROC_FEATURE_H
#define OTAWA_PROC_FEATURE_H

#include <elm/string.h>
#include <elm/util/Cleaner.h>
#include <elm/genstruct/DAGNode.h>
#include <otawa/prop/PropList.h>
#include <otawa/prop/Identifier.h>
#include <otawa/proc/Processor.h>

namespace otawa {
	
using namespace elm;
class WorkSpace;


class FeatureDependency;
	
// AbstractFeature class
class AbstractFeature: public Identifier<Processor *> {
public:
	AbstractFeature(CString name = "");
	inline void incUseCount();
	inline void decUseCount();
	virtual void process(WorkSpace *fw,
		const PropList& props = PropList::EMPTY) const = 0;
	virtual void check(WorkSpace *fw) const = 0;
	virtual void clean(WorkSpace *ws) const = 0;
};


// default_checker_t structure
typedef struct default_checker_t {
	static inline void check(WorkSpace *fw) { }
	static inline void clean(WorkSpace *ws) { };
} default_checker_t;


// Feature class
template <class T, class C = default_checker_t>
class Feature: public AbstractFeature {
public:
	Feature(CString name = "");
	virtual void process(WorkSpace *fw,
		const PropList& props = PropList::EMPTY) const;
	
	// Abstract feature overload
	virtual void check(WorkSpace *fw) const { C::check(fw); }
	virtual void clean(WorkSpace *ws) const { C::clean(ws); }
};

// Inline
template <class T, class C>
Feature<T, C>::Feature(CString name): AbstractFeature(name) {
}

template <class T, class C>
void Feature<T, C>::process(WorkSpace *fw, const PropList& props) const {
	Processor *proc = (*this)(props);
	if(proc)
		proc->process(fw, props);
	else {
		T proc;
		proc.process(fw, props);
	}
}


// FeatureDependency class
class FeatureDependency: public elm::CleanList {
	int refcount;
	bool invalidated;
	const AbstractFeature *feature;
	
public:
	genstruct::DAGNode<FeatureDependency*> *graph;
	
	inline FeatureDependency(const AbstractFeature *_feature);
	inline ~FeatureDependency();
	inline void addChild(FeatureDependency *fdep);
	inline void removeChild(FeatureDependency *fdep);
	inline bool isInUse();
	inline bool isInvalidated();
	inline void setInvalidated(bool inv);
	inline const AbstractFeature *getFeature() const;
};

inline FeatureDependency::FeatureDependency(const AbstractFeature *_feature)
	: refcount(0), invalidated(false), feature(_feature), graph(new genstruct::DAGNode<FeatureDependency*>(this))
	{ }

inline FeatureDependency::~FeatureDependency() {
	delete graph;
}

inline const AbstractFeature *FeatureDependency::getFeature() const {
	return feature;
}

inline void FeatureDependency::addChild(FeatureDependency *fdep)  {	
	graph->addChild(fdep->graph);
	fdep->refcount++;
}

inline void FeatureDependency::removeChild(FeatureDependency *fdep)  {
	fdep->refcount--;
	graph->removeChild(fdep->graph);
	ASSERT(fdep->refcount >= 0);
	if (fdep->refcount == 0) {
		delete fdep;
	}
}
inline bool FeatureDependency::isInUse() {
	return (refcount > 0);
}

inline bool FeatureDependency::isInvalidated() {
	return invalidated;
}

inline void FeatureDependency::setInvalidated(bool inv) {
	invalidated = inv;
	if (invalidated && (refcount == 0))
		delete this;
}


// Feature
extern Feature<NoProcessor> NULL_FEATURE;

} // otawa

#endif /* OTAWA_PROC_FEATURE_H */
