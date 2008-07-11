/*
 *	$Id$
 *	Copyright (c) 2005-06, IRIT UPS.
 *
 *	AbstractFeature and Feature class interface.
 */
#ifndef OTAWA_PROC_FEATURE_H
#define OTAWA_PROC_FEATURE_H

#include <elm/string.h>
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
};

// default_checker_t structure
typedef struct default_checker_t {
	static inline void check(WorkSpace *fw) { }
} default_checker_t;



// Feature class
template <class T, class C = default_checker_t>
class Feature: public AbstractFeature {
public:
	Feature(CString name = "");
	virtual void process(WorkSpace *fw,
		const PropList& props = PropList::EMPTY) const;
	virtual void check(WorkSpace *fw) const;
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

template <class T, class C>
void Feature<T, C>::check(WorkSpace *fw) const {
	C::check(fw);
}

// FeatureDependency class
class FeatureDependency {
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
