/*
 *	$Id$
 *	AbstractFeature class interface
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
#ifndef OTAWA_PROC_ABSTRACT_FEATURE_H
#define OTAWA_PROC_ABSTRACT_FEATURE_H

#include <elm/string.h>
#include <otawa/prop/PropList.h>
#include <otawa/prop/Identifier.h>
#include <otawa/proc/Processor.h>

namespace otawa {
	
using namespace elm;
class WorkSpace;
	
// AbstractFeature class
class AbstractFeature: public Identifier<Processor *> {
public:
	static AbstractFeature& null;
	AbstractFeature(cstring name = "");
	virtual ~AbstractFeature(void);
	virtual void process(WorkSpace *ws, const PropList& props = PropList::EMPTY) const = 0;
};


// feature class
namespace p {

	class feature: public AbstractFeature {
	public:
		feature(cstring name, AbstractMaker *maker);
		feature(cstring name, p::declare& reg);
		~feature(void);
		virtual void process(WorkSpace *ws, const PropList& props) const;
	private:
		AbstractMaker *_maker;
	};

	template <class T>
	static inline p::declare& make(void) { return T::reg; }

	void *get_impl(WorkSpace *ws, const AbstractFeature& feature);

	template <class T = void, class I = void>
	class interfaced_feature: public feature {
	public:
		inline interfaced_feature(cstring name): feature(name, make<T>()) { }
		inline interfaced_feature(cstring name, AbstractMaker *maker): feature(name, maker) { }
		inline interfaced_feature(cstring name, p::declare& reg): feature(name, reg) { }

		template <class P> inline const I *give(const P *p) const { return p; }
		const I *get(WorkSpace *ws) const
			{ void *p = get_impl(ws, *this); if(p == nullptr) return nullptr;
			  else return static_cast<const I *>(p); }
	};
}

} // otawa

#endif /* OTAWA_PROC_ABSTRACT_FEATURE_H */
