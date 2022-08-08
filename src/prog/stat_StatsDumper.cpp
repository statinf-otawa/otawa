/*
 *	StatsDumper class implementation.
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

#include <elm/sys/System.h>
#include <otawa/prog/features.h>
#include <otawa/stats/StatsDumper.h>
#include <otawa/stats/StatInfo.h>
#include <otawa/cfgio/Output.h>

namespace otawa {

class StatOutput: public StatCollector::Collector {
public:
	StatOutput(Output& out): _out(out) { }
	virtual ~StatOutput() { }
	virtual void collect(const Address &address, t::uint32 size, int value, const ContextualPath& ctx) {
		_out << value << "\t"
			 << address << "\t"
			 << size << "\t"
			 << ctx << io::endl;
	}

private:
	Output& _out;
};

/**
 * @class StatsDumper
 * Code processor dumping to disk statistics produced so far by
 * other code processors.
 * 
 * Recall that code processors only produce statis if the
 * configuration @ref Processor::COLLECT_STATS is set to true.
 * 
 * The files are saved as .csv in the directory DIR (as provided
 * by the workspace) appended with "stats". For each statistics
 * STAT, a file named STAT.csv is created.
 * 
 * The simple version has 4 columns:
 * * the statistics item (decimal integer),
 * * the code address (hexadecimal integer),
 * * the code size (decimal integer),
 * * the code context (text).
 * 
 * The BB version (prefixed by a line "#BB") has also 4 columns
 * with different meaning:
 * * the statistics item (decimal integer),
 * * the CFG number (decimal integer),
 * * the BB number (decimal integer),
 * * the code context (text).
 * 
 * The .csv files can contains definitions in the form of:
 * 
 * #ID : VALUE
 * 
 * That may be dependent on the statistics itself or standardized:
 * * "Label" - statistics label,
 * * "Unit" - statistics unit,
 * * "Description" - statistics description.
 * * "LineOp" - line aggregation operation (one of sum, max, min).
 * * "ConcatOp" - concatenation aggregation operation (as above).
 * * "ContextOp" - context aggregation operation (as above).
 * 
 * @par Provided features
 * * @ref STATS_DUMP_FEATURE
 * 
 * @ingroup stats
 */ 

///
StatsDumper::StatsDumper(p::declare& r): Processor(r) {}



void StatsDumper::configure(const PropList& props) {
	Processor::configure(props);
	path = STATS_PATH(props);
}

///
void StatsDumper::processWorkSpace(WorkSpace *ws) {

	// determine the path
	if(path.isEmpty())
		path = TASK_INFO_FEATURE.get(ws)->workDirectory();

	// clean old statistics
	Vector<cstring> to_remove;
	for(auto f: path.readDir())
		if(f.endsWith("-stat.csv"))
			to_remove += f;
	for(auto f: to_remove)
		(path / f).remove();
		
	// generate the statistics
	for(auto stat: StatInfo::get(ws)) {
		string id = string(stat->id()).replace("/", "-");
		io::OutStream *stream = sys::System::createFile(path / (id + "-stat.csv"));
		Output out(*stream);
		
		// dump additional information
		out << "#Label: " << stat->label() << io::endl;
		if(stat->unit())
			out << "#Unit: " << stat->unit() << io::endl;
		out << "#Total: " << stat->total() << io::endl;
		out << "#LineOp: " << stat->lineOperation() << io::endl;
		out << "#ConcatOp: " << stat->concatOperation() << io::endl;
		out << "#ContextOp: " << stat->contextOperation() << io::endl;
		if(stat->description()) {
			string s = stat->description();
			out << "#Description: " << s.replace("\n", " ") << io::endl;
		}
		ListMap<cstring, string> defs;
		stat->definitions(defs);
		for(auto k: defs.keys())
			out << '#' << k << ": " << defs.get(k, "") << io::endl; 
		
		// dump the data
		StatOutput sout(out);
		stat->collect(sout);
		
		delete stream;
	}

}

///
p::declare StatsDumper::reg
	= p::init("otawa::StatsDumper", Version(1, 0, 0))
	.require(TASK_INFO_FEATURE)
	.provide(STATS_DUMP_FEATURE)
	.make<StatsDumper>();


/**
 * Feature ensuring that analysis statistics has been dumped to files.
 * 
 * @warning Notice that only the code processor with configuration
 * @ref Processor::COLLECT_STATS set to true are generated.
 * 
 * @par Configuration
 * * @ref STATS_PATH
 * 
 * @par Implementation
 * * @ref StatsDumper (default)
 * 
 * @ingroup stats
 */
p::feature STATS_DUMP_FEATURE("otawa::STATS_DUMP_FEATURE", p::make<StatsDumper>());


/**
 * Configure for @ref STATS_DUMP_FEATURE. Specify the directory path that
 * will receive the statistics data.
 * @ingroup stats
 */
p::id<Path> STATS_PATH("otawa::STATS_PATH");

}	// otawa

