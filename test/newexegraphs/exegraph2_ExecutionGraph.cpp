/*
 *	$Id$
 *	Interface to the ExecutionGraph, EGNode, EGEdge classes.
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


#include "ExecutionGraph.h"

using namespace  otawa;
using namespace otawa::exegraph2;

// ----------------------------------------------------------------

ExecutionGraph::~ExecutionGraph() {
}


/**
 * Manage the attribute dump.
 * Must be called before the first attribute is generated.
 */
static void dumpAttrBegin(io::Output& out, bool& first) {
	first = true;
}

/**
 * Manage the attribute dump.
 * Must be called before displaying an attribute.
 */
static void dumpAttr(io::Output& out, bool& first) {
	if(first)
		out << " [ ";
	else
		out << ", ";
	first = false;
}

/**
 * Manage the attribute dump.
 * Must be called after the last attribute has been generated.
 */
static void dumpAttrEnd(io::Output& out, bool& first) {
	if(!first)
		out << " ]";
}


// ---------------------------------------
void ExecutionGraph::dump(elm::io::Output& dotFile, const string& info) {
    int i=0;
    bool first_line = true;
    int width = 5;
    dotFile << "digraph G {\n";

    // display information if any
    if(info)
    	dotFile << "\"info\" [shape=record, label=\"{" << info << "}\"];\n";

/*    // display ressources
    dotFile << "\"legend\" [shape=record, label= \"{ ";
    for (elm::genstruct::Vector<Resource *>::Iterator res(_resources) ; res ; res++) {
		if (i == 0) {
			if (!first_line)
				dotFile << " | ";
			first_line = false;
			dotFile << "{ ";
		}
		dotFile << res->name();
		if (i < width-1){
			dotFile << " | ";
			i++;
		}
		else {
			dotFile << "} ";
			i = 0;
		}
    }
    if (i!= 0)
		dotFile << "} ";
    dotFile << "} ";
    dotFile << "\"] ; \n";*/

    // display instruction sequence
    dotFile << "\"code\" [shape=record, label= \"\\l";
    bool body = true;
    BasicBlock *bb = 0;
    for (EGInstSeq::InstIterator inst(_inst_seq) ; inst ; inst++) {
		if(inst->part() == BLOCK && body) {
			body = false;
			dotFile << "------\\l";
		}
    	BasicBlock *cbb = inst->basicBlock();
    	if(cbb != bb) {
    		bb = cbb;
    		dotFile << bb << "\\l";
    	}
    	dotFile << "I" << inst->index() << ": ";
		dotFile << "0x" << fmt::address(inst->inst()->address()) << ":  ";
		inst->inst()->dump(dotFile);
		dotFile << "\\l";
    }
    dotFile << "\"] ; \n";

    // edges between info, legend, code
    if(info)
    	dotFile << "\"info\" -> \"legend\";\n";
    dotFile << "\"legend\" -> \"code\";\n";

    // display nodes
    for (EGInstSeq::InstIterator inst(_inst_seq) ; inst ; inst++) {
		// dump nodes
		dotFile << "{ rank = same ; ";
		for (EGInst::NodeIterator node(inst) ; node ; node++) {
			dotFile << "\"" << node->stage()->name();
			dotFile << "I" << node->inst()->index() << "\" ; ";
		}
		dotFile << "}\n";
		// again to specify labels
		for (EGInst::NodeIterator node(inst) ; node ; node++) {
			dotFile << "\"" << node->stage()->name();
			dotFile << "I" << node->inst()->index() << "\"";
			dotFile << " [shape=record, ";
			if (node->inst()->part() == BLOCK)
				dotFile << "color=blue, ";
			dotFile << "label=\"" << node->stage()->name();
			dotFile << "(I" << node->inst()->index() << ") [" << node->latency() << "]\\l" << node->inst()->inst();
			dotFile << "| { ";
/*
			int i=0;
			int num = _resources.length();
			while (i < num) {
				int j=0;
				dotFile << "{ ";
				while ( j<width ) {
					if ( (i<num) && (j<num) ) {
						if (node->e(i))
							dotFile << node->d(i);
					}
					if (j<width-1)
						dotFile << " | ";
					i++;
					j++;
				}
				dotFile << "} ";
				if (i<num)
					dotFile << " | ";
			}
*/
			dotFile << "} ";
			dotFile << "\"] ; \n";
		}
		dotFile << "\n";
    }

    // display edges
    int group_number = 0;
    for (EGInstSeq::InstIterator inst(_inst_seq) ; inst ; inst++) {
		// dump edges
		for (EGInst::NodeIterator node(inst) ; node ; node++) {
			for (Successor next(node) ; next ; next++) {
				if ( node != inst->firstNode()
					 ||
					 (node->stage()->category() != EGStage::EXECUTE)
					 || (node->inst()->index() == next->inst()->index()) ) {

					// display edges
					dotFile << "\"" << node->stage()->name();
					dotFile << "I" << node->inst()->index() << "\"";
					dotFile << " -> ";
					dotFile << "\"" << next->stage()->name();
					dotFile << "I" << next->inst()->index() << "\"";

					// display attributes
					bool first;
					dumpAttrBegin(dotFile, first);

					// latency if any
					if(next.edge()->latency()) {
						dumpAttr(dotFile, first);
						dotFile << "label=\"" << next.edge()->latency() << "\"";
					}

					// edge style
					switch( next.edge()->type()) {
					case EGEdge::SOLID:
						if (node->inst()->index() == next->inst()->index()) {
							dumpAttr(dotFile, first);
							dotFile << "minlen=4";
						}
						break;
					case EGEdge::SLASHED:
						dumpAttr(dotFile, first);
						dotFile << " style=dotted";
						if (node->inst()->index() == next->inst()->index()) {
							dumpAttr(dotFile, first);
							dotFile << "minlen=4";
						}
						break;
					default:
						break;
					}

					// dump attribute end
					dumpAttrEnd(dotFile, first);
					dotFile << ";\n";

					// group
					if ((node->inst()->index() == next->inst()->index())
						|| ((node->stage()->index() == next->stage()->index())
							&& (node->inst()->index() == next->inst()->index()-1)) ) {
						dotFile << "\"" << node->stage()->name();
						dotFile << "I" << node->inst()->index() << "\"  [group=" << group_number << "] ;\n";
						dotFile << "\"" << next->stage()->name();
						dotFile << "I" << next->inst()->index() << "\" [group=" << group_number << "] ;\n";
						group_number++;
					}
				}
			}
		}
		dotFile << "\n";
    }
    dotFile << "}\n";
}

/**
 * Build a parametric execution graph.
 * @param ws	Current workspace.
 * @param proc	Processor description.
 * @param seq	Instruction sequence to compute.
 * @param props	Other parameters.
 */
ExecutionGraph::ExecutionGraph(
	WorkSpace *ws,
	EGProc *proc,
	EGInstSeq *inst_seq,
	EGNodeFactory * node_factory,
	const PropList& props
)
: _inst_seq(inst_seq)
{}



