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
#include <elm/data/List.h>
#include <otawa/proc/AbstractFeature.h>

namespace otawa {
	
using namespace elm;

// FeatureDependency class
class FeatureDependency: public elm::CleanList {
	typedef List<FeatureDependency *> list_t;
public:
	
	// constructors
	FeatureDependency(const AbstractFeature *_feature);
	~FeatureDependency(void);
	void add(FeatureDependency *to);
	void remove(FeatureDependency *from);

	// accessors
	inline const AbstractFeature *getFeature(void) const { return feature; }

	// Iterator on dependencies
	class Dependent: public list_t::Iter {
	public:
		inline Dependent(FeatureDependency *d): list_t::Iter(d->children) { }
		inline Dependent(const Dependent& i): list_t::Iter(i) { }
	};

private:
	const AbstractFeature *feature;
	list_t parents;
	list_t children;
};

} // otawa

#endif /* OTAWA_PROC_FEATURE_DEPENDENCY_H */
