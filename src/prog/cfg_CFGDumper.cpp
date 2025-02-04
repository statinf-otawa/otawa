/*
 *	CFGDumper class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2022, IRIT UPS.
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

#include <otawa/cfg/CFG.h>
#include <otawa/cfg/features.h>
#include <otawa/proc/Processor.h>
#include <otawa/prog/Process.h>
#include <otawa/prog/File.h>

namespace otawa {

class CFGDumper: public Processor {
public:
	static p::declare reg;
	CFGDumper(p::declare& r = reg): Processor(r) {}

protected:
	
	void configure(const PropList& props) override {
		Processor::configure(props);
	}
	
	void processWorkSpace(WorkSpace * ws) override {
		
		// compute path
		Path path = this->path;
		if(path.isEmpty())
			path = TASK_INFO_FEATURE.get(ws)->workDirectory();
		path = path / "cfg.csv";
		
		try {
			// open the file
			auto stream = path.write();
			io::Output out(*stream);
			
			// dump definitions
			out << "#Task: " << TASK_INFO_FEATURE.get(ws)->entryName() << io::endl;
			out << "#Exec: " << ws->process()->program()->name() << io::endl;
			
			// dump the CFGs
			for(auto g: *COLLECTED_CFG_FEATURE.get(ws)) {
				out << "G\t" << g->name()
					<< '\t' << g->address()
					<< '\t' << *CONTEXT(g) << io::endl;
				for(auto v: g->vertices()) {
					
					// dump blocks
					if(v->isBasic()) {
						auto bb = v->toBasic();
						out << "B\t" << bb->address() << '\t' << bb->size() << io::endl;
					}
					else if(v->isSynth()) {
						auto cb = v->toSynth();
						out << 'C';
						if(cb->callee() != nullptr)
							out << '\t' << cb->callee()->index();
						out << io::endl;
					}
					else {
						if(v->isEntry())
							out << 'N' << io::endl;
						else if(v->isExit())
							out << 'X' << io::endl;
						else if(v->isUnknown())
							out << 'U' << io::endl;
						else
							out << 'P' << io::endl;
					}
				}
				
				// dump edges
				for(auto v: g->vertices())
					for(auto e: v->outEdges())
						out << "E\t" << v->index()
							<< '\t' << e->sink()->index()
							<< '\t' << (e->isTaken() ? 'T' : 'N')
							<< io::endl;
			}
			
			// close the file
			delete stream;
		}
		catch(sys::SystemException& e) {
			throw ProcessorException(*this, _ << "cannot create " << path << ": " << e.message());
		}
	}

private:
	Path path;
};


///
p::declare CFGDumper::reg
	= p::init("otawa::CFGDumper", Version(1, 0, 0))
	.require(COLLECTED_CFG_FEATURE)
	.require(TASK_INFO_FEATURE)
	.provide(CFG_DUMP_FEATURE)
	.make<CFGDumper>();

	
/**
 * Feature ensuring that the CFG representation has been dumped.
 * 
 * @ingroup cfg
 */
p::feature CFG_DUMP_FEATURE("otawa::CFG_DUMP_FEATURE", p::make<CFGDumper>());
	
} // orawa
