/*
 *	$Id$
 *	DynFeature class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009, IRIT UPS.
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
#ifndef OTAW_PROC_DYNFEATURE_H_
#define OTAW_PROC_DYNFEATURE_H_

#include <otawa/proc/Feature.h>

namespace otawa {

// FeatureNotFound exception
class FeatureNotFound: public otawa::Exception {
public:
	FeatureNotFound(string name);
	inline const string& name(void) const { return _name; }

private:
	string _name;
};

// DynFeature class
class DynFeature {
public:
	DynFeature(string name);
	inline operator AbstractFeature *(void) const
		{ if(!feature) init(); return feature; }
	inline AbstractFeature *operator*(void) const
		{ if(!feature) init(); return feature; }
	inline operator AbstractFeature&(void) const
		{ if(!feature) init(); return *feature; }

private:
	mutable AbstractFeature *feature;
	string _name;
	void init(void) const;
};

} // otawa

#endif /* OTAW_PROC_ DYNFEATURE_H_ */
