/*
 *	$Id$
 *	Copyright (c) 2005-06, IRIT UPS.
 *
 *	AbstractFeature and Feature class interface.
 */
#ifndef OTAWA_PROC_FEATURE_H
#define OTAWA_PROC_FEATURE_H

#include <elm/string.h>
#include <otawa/prop/PropList.h>
#include <otawa/prop/GenericIdentifier.h>
#include <otawa/proc/Processor.h>

namespace otawa {
	
using namespace elm;
class FrameWork;
	
// AbstractFeature class
class AbstractFeature: public GenericIdentifier<Processor *> {
public:
	static NameSpace NS;
	AbstractFeature(CString name = "");
	virtual void process(FrameWork *fw,
		const PropList& props = PropList::EMPTY) const = 0;
	virtual void check(FrameWork *fw) const = 0;
};

// default_checker_t structure
typedef struct default_checker_t {
	static inline void check(FrameWork *fw) { }
} default_checker_t;

// Feature class
template <class T, class C = default_checker_t>
class Feature: public AbstractFeature {
public:
	Feature(CString name = "");
	virtual void process(FrameWork *fw,
		const PropList& props = PropList::EMPTY) const;
	virtual void check(FrameWork *fw) const;
};

// Inline
template <class T, class C>
Feature<T, C>::Feature(CString name): AbstractFeature(name) {
}

template <class T, class C>
void Feature<T, C>::process(FrameWork *fw, const PropList& props) const {
	Processor *proc = (*this)(props);
	if(proc)
		proc->process(fw, props);
	else {
		T proc;
		proc.process(fw, props);
	}
}

template <class T, class C>
void Feature<T, C>::check(FrameWork *fw) const {
	C::check(fw);
}

} // otawa

#endif /* OTAWA_PROC_FEATURE_H */
