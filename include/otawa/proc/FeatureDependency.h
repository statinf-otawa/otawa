/*
 *	$Id$
 *	FeatureDependency class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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
#ifndef OTAWA_PROC_FEATURE_DEPENDENCY_H
#define OTAWA_PROC_FEATURE_DEPENDENCY_H

#include <elm/string.h>
#include <elm/util/Cleaner.h>
#include <elm/genstruct/DAGNode.h>
#include <otawa/proc/AbstractFeature.h>

namespace otawa {
	
using namespace elm;
	
// FeatureDependency class
class FeatureDependency: public elm::CleanList {
public:
	
	// constructors
	FeatureDependency(const AbstractFeature *_feature);
	~FeatureDependency(void);
	void addChild(FeatureDependency *fdep);
	void removeChild(FeatureDependency *fdep);
	void setInvalidated(bool inv);

	// accessors
	inline bool isInUse(void) const { return (refcount > 0); }
	inline bool isInvalidated() const {	return invalidated; }
	inline const AbstractFeature *getFeature(void) const { return feature; }

	// Iterator on dependencies
	class Children: public PreIterator<Children, FeatureDependency *> {
	public:
		inline Children(FeatureDependency *dep): iter(*dep->graph) { }
		inline Children(const Children& i): iter(i.iter) { }
		inline bool ended(void) const { return iter.ended(); }
		inline void next(void) { iter.next(); }
		inline FeatureDependency *item(void) const { return (*iter)->useValue(); }
	private:
		genstruct::DAGNode<FeatureDependency *>::Iterator iter;
	};

private:
	int refcount;
	bool invalidated;
	const AbstractFeature *feature;
	genstruct::DAGNode<FeatureDependency*> *graph;
};

} // otawa

#endif /* OTAWA_PROC_FEATURE_DEPENDENCY_H */
