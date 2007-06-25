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
	static NameSpace NS;
	AbstractFeature(CString name = "");
	FeatureDependency *dependency;	
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
	
public:
	genstruct::DAGNode<const AbstractFeature*> *graph;
	
	inline FeatureDependency();
	inline void incUseCount();
	inline void decUseCount();
	
};

inline FeatureDependency::FeatureDependency() : graph(NULL), refcount(0) {
}

inline void FeatureDependency::incUseCount()  {
	refcount++;
}

inline void FeatureDependency::decUseCount()  {
	refcount--;
	if (refcount == 0) {
		delete graph;
		graph = NULL;
	}
}




} // otawa

#endif /* OTAWA_PROC_FEATURE_H */
