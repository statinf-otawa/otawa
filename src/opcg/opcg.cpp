/*
 *	opcg command
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

#include <elm/assert.h>
#include <elm/data/HashMap.h>
#include <elm/data/Vector.h>
#include <elm/data/VectorQueue.h>
#include <elm/options.h>

#include <otawa/otawa.h>
#include <otawa/pcg/PCG.h>
#include <otawa/display/display.h>
#include <otawa/display/GenDrawer.h>
#include <otawa/pcg/PCGBuilder.h>
#include <otawa/app/Application.h>

using namespace elm;
using namespace elm::option;
using namespace otawa;
using namespace otawa::display;


/**
 * @addtogroup commands
 * @section opcg opcg Command
 * 
 * opcg is a simple OTAWA tool to display the Program Call Graph (PCG) of
 * a binary program.
 * 
 * @par Syntax
 * 
 * @code
 * $ opcg [options] binary_file [entry function]
 * @endcode
 * This program output in postscript the PCG of a binary program rooted by
 * the given function or, as a default, by the "main" function.
 * 
 * The output file use the entry function name where the file type extension is
 * appended. If you do not select a special file output and no entry function,
 * the output file is named "main.ps".
 * 
 * The options includes:
 * 
 * @li -I|--no_intern -- do not dump C internal functions (starting by '_' in the PCG).
 * 
 * @li -c|--chain @i function -- bound the PCG to network between the entry function
 * 		and the given @i function (useful when the PCG is too big).

 * -o|--output @i type -- select the type of output (the @i type may be one of
 * 		ps, pdf, png, gif, jpg, svg, dot).
 */


// Selection tag
Identifier<bool> SELECTED("", false);

// BBCounter analysis
#if 0
class BBCounter: public Processor {
public:
	static Identifier<int> COUNT;
	static Identifier<int> TOTAL;

	BBCounter(void): Processor("BBCounter", Version(1, 0, 0)) {
		require(PCG_FEATURE);
	}

protected:

	void eval(PCGBlock *block) {
		if(TOTAL(block) >= 0)
			return;
		TOTAL(block) = 0;
		int total = 0;
		for(PCGBlock::PCGBlockOutIterator called(block); called; called++) {
			eval(called);
			total += TOTAL(*called);
		}
		int cnt = block->getCFG()->count();
		COUNT(block) = cnt;
		TOTAL(block) = total + cnt;
	}

	virtual void processWorkSpace(WorkSpace *ws) {
		PCG *pcg = PCG::ID(ws);
		ASSERT(pcg);
		for(PCG::PCGIterator child(pcg); child; child++)
			eval(child);
	}

private:
};
Identifier<int> BBCounter::COUNT("BBCounter::COUNT", -1);
Identifier<int> BBCounter::TOTAL("BBCounter::TOTAL", -1);
#endif

// PCGAdapter class

class PCGAdapter {
public:
	inline PCGAdapter(PCG *pcg): _pcg(pcg) { ASSERT(pcg); }

	// DiGraph concept
	class Vertex {
	public:
		inline Vertex(void): blk(0) { }
		inline Vertex(PCGBlock *block): blk(block) { }
		inline bool operator==(const Vertex& vertex) const { return blk == vertex.blk; }
		PCGBlock *blk;
	};

	class Edge {
	public:
		inline Edge(const Vertex& source, const Vertex& sink)
			: src(source.blk), snk(sink.blk) { }
		inline Vertex source(void) const { return src; }
		inline Vertex sink(void) const { return snk; }
	private:
		PCGBlock *src, *snk;
	};

	class Successor: public PreIterator<Successor, Edge> {
	public:
	 	inline Successor(const PCGAdapter& pcg, const Vertex &source): blk(source.blk), iter(source.blk->outs()) { }
	 	inline bool ended(void) const { return iter.ended(); }
	 	inline Edge item (void) const { return Edge(blk, iter->sink()); }
	 	void next(void) { iter.next(); }
	private:
		PCGBlock *blk;
		PCGBlock::EdgeIter iter;
	};
	
	// DiGraphWithVertexMap concept
	template <class T>
	class VertexMap: public HashMap<Vertex, T> {
	public:
		VertexMap(const PCGAdapter& pcg) { }
	};

	// Collection<Vertex> concept
	inline int count(void) const { return _pcg->count(); }
	//inline bool contains(const Vertex &item) const { return _pcg->pcgbs().contains(item.blk); }
	//inline bool isEmpty (void) const { return _pcg->pcgbs(); }
	//inline operator bool (void) const { return !isEmpty(); }

	class Iter: public PreIterator<Iter, Vertex> {
	public:
		inline Iter(const PCGAdapter& ad): iter(ad._pcg->blocks()) { look(); }
	 	inline bool ended(void) const { return iter.ended(); }
	 	inline Vertex item(void) const { return *iter; }
	 	void next(void) { iter.next(); look(); }
	private:
		void look(void) {
			while(iter) {
				if(SELECTED(iter))
					break;
				iter.next();
			}
		}
		typename PCG::Iter iter;
	};

	inline Vertex sinkOf(const Edge& e) const { return e.sink(); }

private:
	PCG *_pcg;
	friend class PCGDecorator;
};


// PCGDecorator class
class PCGDecorator {
public:
	static void decorate(
		const PCGAdapter& graph,
		Output &caption,
		TextStyle &text,
		FillStyle &fill
	) {
		caption << "PCG of " << graph._pcg->entry()->cfg()->label();
	}
	
	static void decorate(
		const PCGAdapter& graph,
		const PCGAdapter::Vertex& vertex,
		Output &content,
		ShapeStyle &style
	) {
		content << vertex.blk->getName();
		//int cnt = BBCounter::COUNT(vertex.blk),
		//	total = BBCounter::TOTAL(vertex.blk);
		//if(cnt >= 0)
		//	content << "\nBB: " << cnt << " / " << total;
	}
	
	static void decorate(
		const PCGAdapter& graph,
		const PCGAdapter::Edge& edge,
		Output &label,
		TextStyle &text,
		LineStyle &line
	) {
	}
};


// Enumerated type
EnumOption<int>::value_t out_values[] = {
	{ "type of output", OUTPUT_DOT },
	{ "ps", OUTPUT_PS },
	{ "pdf", OUTPUT_PDF }, 	
	{ "png", OUTPUT_PNG }, 	
	{ "gif", OUTPUT_GIF }, 	
	{ "jpg", OUTPUT_JPG }, 	
	{ "svg", OUTPUT_SVG }, 	
	{ "dot", OUTPUT_DOT },
	{ 0, 0 }
};


/**
 * Manager for the application.
 */
class OPCG: public Application {
	SwitchOption no_int;
	ValueOption<string> chain;
	ValueOption<display::output_mode_t> out;
	SwitchOption bb_cnt;
	
public:

	OPCG(void):	Application(Application::Make("opcg", Version(1, 1, 0))
		.author("F. Nemer")
		.description("Draw the program call tree of the given executable.")
		.copyright("Copyright (c) 2010 IRIT - UPS")),
		no_int(SwitchOption::Make(*this).cmd("-I").cmd("--no-internal").description("do not include internal functions (starting with '_')")),
		chain(ValueOption<string>::Make(*this).cmd("-c").cmd("--chain").description("generate calling chain to a function").argDescription("function")),
		out(ValueOption<display::output_mode_t>::Make(*this).cmd("-o").cmd("--out").description("select the output")),
		bb_cnt(SwitchOption::Make(*this).cmd("--count-bb").description("display BB counts/total for each function"))
	{ }
	
protected:

	bool accept(PCGBlock *block) {
		if(!no_int)
			return true;
		string name = block->getName();
		return name && !name.startsWith("_");		
	}

#	if 0
	void selectChain(PCG *pcg, string chain) {
		
		// Find the chain
		PCGBlock *to = 0;
		/*for(PCG::PCGIterator block(pcg); block; block++) {
			//cerr << "===>" << block->getName() << "<====\n";
			if(block->getName() == chain) {
				to = block;
				break;
			}
		}*/
		if(!to)
			throw OptionException(_ << "no looked function \"" << chain << "\".");
		
		// Build the selection
		VectorQueue<PCGBlock *> todo;
		todo.put(to);
		while(todo) {
			
			// Select the current one
			PCGBlock *block = todo.get();
			SELECTED(block) = true;
			
			// Look for parent
			for(PCGBlock::PCGBlockInIterator parent(block); parent; parent++)
				if(!SELECTED(parent) && accept(parent))
					todo.put(parent);
		}
	}
#	endif

	virtual void work(const string &entry, PropList &props) throw(elm::Exception) {

		// Build the PCG
		PCGBuilder builder;
		TASK_ENTRY(props) = &entry;
		builder.process(workspace(), props);
		PCG *pcg = PROGRAM_CALL_GRAPH(workspace());
		ASSERT(pcg);

		//for(PCG::Iter i(pcg); i; i++)
		//	cerr << "DEBUG: " << *i << io::endl;
	
		// Select the interesting PCG blocks
		/*if(chain)
			selectChain(pcg, chain);
		else*/
			for(PCG::Iter block = pcg->blocks(); block; block++)
				SELECTED(block) = accept(block);
		
		// count BB if required
		/* if(bb_cnt) {
			BBCounter counter;
			counter.process(workspace(), props);
		} */

		// Display the PCG
		PCGAdapter ad(pcg);
		GenDrawer<PCGAdapter, PCGDecorator> drawer(ad);
		drawer.kind = (kind_t)out.value();
		drawer.path = entry + "." + out_values[out.value()].name;
		drawer.draw();
		
	}
	
private:
	string prog_name;
	string entry;
};

OTAWA_RUN(OPCG);
