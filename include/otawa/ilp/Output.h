/*
 *	ilp::Output class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2013, IRIT UPS.
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
#ifndef OTAWA_ILP_OUTPUT_H_
#define OTAWA_ILP_OUTPUT_H_

#include <otawa/proc/Processor.h>
#include "features.h"

namespace otawa { namespace ilp {

class Output: public Processor {
public:
	static p::declare reg;
	Output(p::declare &r = reg);
	virtual ~Output(void);
	virtual void configure(const PropList &props);
protected:
	virtual void setup(WorkSpace *ws);
	virtual void processWorkSpace(WorkSpace *ws);
	virtual void cleanup(WorkSpace *ws);
private:
	sys::Path path;
	format_t format;
	elm::io::OutStream *stream;
	bool do_free;
};

} }

#endif /* OTAWA_ILP_OUTPUT_H_ */
