/*
 *	$Id$
 *	(Abstract)ParamProcessor class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2010, IRIT UPS <casse@irit.fr>
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
#ifndef OTAWA_PROC_PARAMPROCESSOR_H_
#define OTAWA_PROC_PARAMPROCESSOR_H_

#include <otawa/prop/Identifier.h>
#include <otawa/proc/Processor.h>

namespace otawa {

// external definitions
class ActualFeature;


// AbstractParamProcessor class
class AbstractParamProcessor: public AbstractIdentifier {
public:
	AbstractParamProcessor(cstring name, Version version);
	virtual ~AbstractParamProcessor(void);
	virtual Processor *instantiate(const ActualFeature& feature) const = 0;
	inline const Version& version(void) const { return vers; }

private:
	Version vers;
};

// ParamProcessor class
class ParamProcessor: public AbstractParamProcessor {
public:

	class AbstractMaker {
	public:
		virtual Processor *instantiate(cstring name, Version version, const ActualFeature& feature) const = 0;
	};

	template <class T>
	class Maker: public AbstractMaker {
	public:
		virtual Processor *instantiate(cstring name, Version version, const ActualFeature& feature) const
			{ return new T(name, version, feature); }
	};

	ParamProcessor(cstring name, Version version, AbstractMaker *maker)
		: AbstractParamProcessor(name, version), _maker(maker) { }
	~ParamProcessor(void)
		{ delete _maker; }
	virtual Processor *instantiate(const ActualFeature& feature) const
		{ return _maker->instantiate(&name(), version(), feature); }

private:
	AbstractMaker *_maker;
};

} // otawa

#endif /* OTAWA_PROC_PARAMPROCESSOR_H_ */

