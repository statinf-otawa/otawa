/*
 *	$Id$
 *	hard::Dumper class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2010, IRIT UPS.
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

#include <otawa/hard/Dumper.h>
#include <otawa/hard.h>

namespace otawa { namespace hard {

/**
 * @class Dumper
 * Dump to output the hardware configuration details.
 */

Registration<Dumper> Dumper::reg(
	"otawa::hard::Dumper",
	Version(1, 0, 0),
	p::end
);


/**
 */
Dumper::Dumper(void): Processor(reg) {
}


/**
 */
void Dumper::processWorkSpace(WorkSpace *ws) {
	const Platform& pf = *ws->process()->platform();

	// display memory
	out << "MEMORY\n";
	const hard::Memory *mem = hard::MEMORY(ws);
	if(!mem)
		mem = &pf.memory();
	for(int i = 0; i < mem->banks().count(); i++) {
		const Bank& bank = *mem->banks()[i];
		out << "\tBANK " << bank.name() << io::endl;
		out << "\t\ttype = ";
		switch(bank.type()) {
		case hard::Bank::NONE: out << "none"; break;
		case hard::Bank::ROM: out << "ROM"; break;
		case hard::Bank::SPM: out << "SPM"; break;
		case hard::Bank::DRAM: out << "DRAM"; break;
		case hard::Bank::IO: out << "IO"; break;
		default: out << "unknown"; break;
		}
		out << io::endl;
		out << "\t\trange = " << bank.address() << "-" << bank.topAddress() << io::endl;
		out << "\t\tlatency = " << bank.latency() << " cycles\n";
		if(bank.writeLatency())
			out << "\t\twrite latency = " << bank.writeLatency() << " cycles\n";
	}
}

} }		// otawa::hard

