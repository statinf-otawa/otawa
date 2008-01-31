/*
 *	$Id$
 *	octree command
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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

#include <elm/options.h>
#include <elm/genstruct/Vector.h>
#include <otawa/otawa.h>
#include <elm/assert.h>
#include <otawa/cfg/CFGBuilder.h>
#include <otawa/pcg/PCG.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/display/display.h>
#include <otawa/display/GenDrawer.h>
#include <otawa/pcg/PCGBuilder.h>
#include <elm/genstruct/VectorQueue.h>


using namespace elm;
using namespace elm::option;
using namespace elm::genstruct;
using namespace otawa;
using namespace otawa::display;


/**
 * @page opcg opcg Command
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


// PCGAdapter class
class PCGAdapter {
public:
	inline PCGAdapter(PCG *pcg)
		: _pcg(pcg) { ASSERT(pcg); }
	
	// DiGraph concept
	class Vertex {
	public:
		inline Vertex(void): blk(0) { }
		inline Vertex(PCGBlock *block): blk(block) { }
		inline int outDegree(void) const { return -1; }
		inline int inDegree(void) const { return -1; }
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
	 	inline Successor(const PCGAdapter& pcg, const Vertex &source)
	 		: blk(source.blk), iter(source.blk)	{  look(); }
	 	//Successor(const Forward &forward)		!!TODO!!
	 	inline bool ended(void) const { return iter.ended(); }
	 	inline Edge item (void) const { return Edge(blk, *iter); }
	 	void next(void) { iter.next(); look(); }
	private:
		void look(void) {
			while(iter) {
				if(SELECTED(iter))
					break;
				iter.next();
			}			
		}
		PCGBlock *blk;
		PCGBlock::PCGBlockOutIterator iter;
	};
	
	// DiGraphWithVertexMap concept
	template <class T>
	class VertexMap: public HashTable<Vertex, T> {
	public:
		VertexMap(const PCGAdapter& pcg) { }
	};

	// Collection<Vertex> concept
	inline int count(void) const { return _pcg->nbPCGBlocks(); }
	inline bool contains(const Vertex &item) const { return _pcg->pcgbs().contains(item.blk); }
	inline bool isEmpty (void) const { return _pcg->pcgbs(); }
	inline operator bool (void) const { return !isEmpty(); }

	class Iterator: public PreIterator<Iterator, Vertex> {
	public:
		inline Iterator(const PCGAdapter& ad)
			: iter(ad._pcg) { look(); }
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
		PCG::PCGIterator iter;
	};

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
		caption << "Program Call Graph of " << graph._pcg->getCFG()->label();
	}
	
	static void decorate(
		const PCGAdapter& graph,
		const PCGAdapter::Vertex& vertex,
		Output &content,
		ShapeStyle &style
	) {
		content << vertex.blk->getName();
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
class OctreeManager: public option::Manager {
public:
	
	// Options
	BoolOption no_int;
	StringOption chain;
	EnumOption<int> out;
	
	OctreeManager(void)
	:	no_int(*this, 'I', "no-internal",
			"do not include internal functions (starting with '_')", false),
		chain(*this, 'c', "chain", "generate calling chain to a function",
			"function", ""),
		out(*this, 'o', "out", "select the output", out_values)
	{
		program ="octree";
		version = "1.0.0";
		author = "H. Cassé";
		copyright = "Copyright (c) IRIT - UPS";
		description = "Draw the program call tree of the given executable.";
		free_argument_description = "program [entry_function...]";
	}
	
	bool accept(PCGBlock *block) {
		if(!no_int)
			return true;
		string name = block->getName();
		return name && !name.startsWith("_");		
	}
	
	void selectChain(PCG *pcg, string chain) {
		
		// Find the chain
		PCGBlock *to = 0;
		for(PCG::PCGIterator block(pcg); block; block++) {
			//cerr << "===>" << block->getName() << "<====\n";
			if(block->getName() == chain) {
				to = block;
				break;
			}
		}
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
	
	void run(int argc, char *argv[]) {
		
		// Parse option
		parse(argc, argv);
		if(!prog_name)
			throw OptionException("program name required.");
		if(!entry)
			entry = "main";
		
		// Open the file
		PropList config;
		//Processor::VERBOSE(config) = true;
		WorkSpace *ws = MANAGER.load(prog_name, config);
		ASSERT(ws);

		// Build the PCG
		//cerr << "BUILD THE PCG\n";
		PCGBuilder builder;
		TASK_ENTRY(config) = &entry;
		builder.process(ws, config);
		PCG *pcg = PCG::ID(ws);
		ASSERT(pcg);
		/*for(PCG::PCGIterator blk(pcg); blk; blk++)
			cout << "  * " << blk->getName() << io::endl;*/
	
		// Select the interesting PCG blocks
		if(chain)
			selectChain(pcg, chain);
		else
			for(PCG::PCGIterator block(pcg); block; block++)
				SELECTED(block) = accept(block);
		
		// Display the PCG
		//cerr << "PRODUCE THE OUTPUT\n";
		PCGAdapter ad(pcg);
		GenDrawer<PCGAdapter, PCGDecorator> drawer(ad);
		drawer.kind = (kind_t)out.value();
		drawer.path = entry + "." + out_values[out.value()].name;
		drawer.draw();
		
	}
	
protected:
	virtual void process(string arg) {
		if(!prog_name)
			prog_name = arg;
		else if(!entry)
			entry = arg;
		else
			throw OptionException("bad argument list");
	}
	
private:
	string prog_name;
	string entry;
};


/**
 * Entry point.
 */
int main(int argc, char *argv[]) {
	OctreeManager man;
	try {
		man.run(argc, argv);
	}
	catch(option::OptionException& e) {
		man.displayHelp();
		cerr << "ERROR: " << e.message() << io::endl;
	}
	catch(elm::Exception& e) {
		cerr << "ERROR: " << e.message() << io::endl;
	}
}
