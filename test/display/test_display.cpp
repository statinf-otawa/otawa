#include <stdlib.h>
#include <otawa/otawa.h>
#include <otawa/prog/Loader.h>
#include <otawa/util/Dominance.h>
#include <elm/io.h>
#include <otawa/display/GenDrawer.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/ipet.h>
#include <otawa/ipet/BBTimeSimulator.h>
#include <otawa/gensim/GenericSimulator.h>
#include <otawa/cfg/Virtualizer.h>
#include <otawa/display/CFGAdapter.h>
#include <otawa/display/GenGraphAdapter.h>

using namespace elm;
using namespace otawa;
using namespace otawa::ipet;


void usage_error(){
	cerr << "error\n";
	cerr << "Usage: test_display file\n";
	exit(1);
}


// CFGDecorator class
class CFGDecorator {
public:
	static inline void decorate(
		const display::CFGAdapter& graph,
		Output& caption,
		display::TextStyle& text,
		display::FillStyle& fill)
	{
		caption << "my graph : " << graph.cfg->address();
	}
	
	static inline void decorate(
		const display::CFGAdapter& graph,
		const display::CFGAdapter::Vertex vertex,
		Output& content,
		display::ShapeStyle& style)
	{
		style.shape = display::ShapeStyle::SHAPE_MRECORD;
		if(vertex.bb->isEntry())
			content << "Entry";
		else if(vertex.bb->isExit())
			content << "Exit";
		else {
			content << " BB " << vertex.bb->number()
				<< " (" << vertex.bb->address() << ")\n"
				<< "---\n";
			for(BasicBlock::InstIter inst(vertex.bb); inst; inst++)
				content << inst->address() << '\t' << *inst << io::endl;
			content << "---\n"
				<< (PropList *)vertex.bb;
		}
	}

	static inline void decorate(
		const display::CFGAdapter& graph,
		const display::CFGAdapter::Edge edge,
		Output& label,
		display::TextStyle& text,
		display::LineStyle& line)
	{
		label << edge.edge->source()->number()
			  << " -> "
			  << edge.edge->target()-> number();
	}
};

// MyGraph
class MyEdge;
class MyNode: public GenGraph<MyNode, MyEdge>::Node {
};
class MyEdge: public GenGraph<MyNode, MyEdge>::Edge {
public:
	MyEdge(MyNode *src, MyNode *snk)
		: GenGraph<MyNode, MyEdge>::Edge(src, snk) { }
};
class MyGraph: public GenGraph<MyNode, MyEdge> {	
};


// MyDecorator
class MyDecorator {
public:
	static inline void decorate(
		const display::GenGraphAdapter<MyGraph>& graph,
		Output& caption,
		display::TextStyle& text,
		display::FillStyle& fill)
	{ caption << "my graph : " << (void *)graph.graph; }
	
	static inline void decorate(
			const display::GenGraphAdapter<MyGraph>& graph,
		const display::GenGraphAdapter<MyGraph>::Vertex vertex,
		Output& content,
		display::ShapeStyle& style)
	{
		content << "my node : " <<  (void *)vertex.node;
		style.shape = display::ShapeStyle::SHAPE_DIAMOND;
	}

	static inline void decorate(
			const display::GenGraphAdapter<MyGraph>& graph,
		const display::GenGraphAdapter<MyGraph>::Edge edge,
		Output& label,
		display::TextStyle& text,
		display::LineStyle& line)
	{
		label << edge.edge->source()->index()
			  << " -> "
			  << edge.edge->target()-> index();
		line.style = display::LineStyle::DASHED;
	}
	
};

int main(int argc, char **argv){
	CString file = argv[1];
	if(argc < 2){
		usage_error();
	}
	
	Manager manager;
	try{
		PropList props;
		PROCESSOR_PATH(props) = "../../data/procs/op1.xml";
		SIMULATOR(props) = &gensim_simulator;
		ipet::EXPLICIT(props) = true;
		WorkSpace *fw = manager.load(file, props);
		
		Virtualizer virt;
		virt.process(fw, props);
		BBTimeSimulator bbts;
        bbts.process(fw, props);

        // Display the CFG
        display::CFGAdapter adapter(ENTRY_CFG(fw));
        display::GenDrawer<display::CFGAdapter, CFGDecorator> drawer(adapter);
        drawer.path = "ici.ps";
        drawer.draw();
        
        // Display a GenGraph
        MyGraph graph;
        MyNode *node1 = new MyNode;
        graph.add(node1);
        MyNode *node2 = new MyNode;
        graph.add(node2);
        new MyEdge(node1, node2);
        new MyEdge(node2, node2);
        display::GenGraphAdapter<MyGraph> adapter2(&graph);
        display::GenDrawer<display::GenGraphAdapter<MyGraph>, MyDecorator>
        	drawer2(adapter2);
        drawer2.path = "la.ps";
        drawer2.draw();
	}
	catch(elm::Exception& e){
		cerr << "ERROR: " << e.message() << '\n';
		exit(2);
	}
}
