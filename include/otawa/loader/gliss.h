/*
 *	Info interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2012, IRIT UPS <casse@irit.fr>
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
#ifndef OTAWA_LOADER_GLISS_H
#define OTAWA_LOADER_GLISS_H

#include <otawa/prop/Identifier.h>
#include <otawa/proc/Feature.h>

namespace otawa {

class Inst;

namespace gliss {

class Info {
public:
	virtual bool check(t::uint32 checksum) = 0;
	virtual void *decode(Inst *inst) = 0;
	virtual void free(void *desc) = 0;
	template <class T> inline void decode(Inst *inst, T *&desc)
		{ desc = static_cast<T *>(decode(inst)); }
protected:
	virtual ~Info(void);
};

extern Feature<NoProcessor> INFO_FEATURE;
extern Identifier<Info *> INFO;

} } // otawa::gliss

#endif // OTAWA_LOADER_GLISS_H

