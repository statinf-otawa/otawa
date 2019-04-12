/*
 *	DummySlicer class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2016, IRIT UPS.
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
#ifndef __OTAWA_OSLICE_SLICER_H__
#define __OTAWA_OSLICE_SLICER_H__

#include <elm/util/Option.h>
#include <otawa/proc/Processor.h>
#include <otawa/proc/Feature.h>
#include <otawa/cfg/features.h>

namespace otawa { namespace oslice {

class DummySlicer: public otawa::Processor {
public:
	static p::declare reg;
	DummySlicer(AbstractRegistration& _reg = reg);

protected:
	virtual void configure(const PropList &props);
	virtual void processWorkSpace(WorkSpace *fw);

};

} } // otawa::oslice

#endif
