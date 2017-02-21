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
#include <otawa/proc/AbstractFeature.h>
#include <otawa/prop/info.h>
#include <otawa/prog/WorkSpace.h>

namespace otawa {
	
using namespace elm;
class WorkSpace;


class FeatureDependency;
	
// default_handler_t structure
typedef struct default_handler_t {
	static inline void check(WorkSpace *fw) { }
	static inline void clean(WorkSpace *ws) { };
} default_handler_t;


// Feature class
template <class T, class C = default_handler_t>
class Feature: public AbstractFeature {
public:
	Feature(CString name = "");
	virtual void process(WorkSpace *fw,
		const PropList& props = PropList::EMPTY) const;
	
	// Abstract feature overload
	virtual void check(WorkSpace *fw) const { C::check(fw); }
	virtual void clean(WorkSpace *ws) const { C::clean(ws); }
};


// identifier property
extern Identifier<const AbstractFeature *> DEF_BY;
inline Property *defBy(const AbstractFeature *feature)
	{ return make(DEF_BY, feature); }


// Inline
template <class T, class C>
Feature<T, C>::Feature(CString name): AbstractFeature(name) {
}

template <class T, class C>
void Feature<T, C>::process(WorkSpace *ws, const PropList& props) const {
	bool del_proc = false;
	Processor *proc = (*this)(props);
	if(!proc) {
		proc = new T();
		del_proc = true;
	}
	ws->run(proc, props, del_proc);
}

} // otawa

#endif /* OTAWA_PROC_FEATURE_H */
