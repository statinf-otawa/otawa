/*
 *	$Id$
 *	Implementation of the EGDisplayer class.
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

#include "EGDisplay.h"
#include <elm/io/OutFileStream.h>

using namespace otawa;
using namespace otawa::exegraph2;

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

EGGenericDotDisplay::EGGenericDotDisplay(ExecutionGraph *graph,
											elm::io::Output& out_file,
											const string& info)
: EGDisplay(graph,out_file,info)
											{
    int i=0;
    bool first_line = true;
    int width = 5;
    out_file << "digraph G {\n";

    // display information if any
    if(info)
    	out_file << "\"info\" [shape=record, label=\"{" << info << "}\"];\n";

   // display instruction sequence
    out_file << "\"code\" [shape=record, label= \"\\l";
    bool body = true;
    BasicBlock *bb = 0;
    for (EGInstSeq::InstIterator inst(graph->instSeq()) ; inst ; inst++) {
		if(inst->part() == BLOCK && body) {
			body = false;
			out_file << "------\\l";
		}
    	BasicBlock *cbb = inst->basicBlock();
    	if(cbb != bb) {
    		bb = cbb;
    		out_file << bb << "\\l";
    	}
    	out_file << "I" << inst->index() << ": ";
		out_file << "0x" << fmt::address(inst->inst()->address()) << ":  ";
		inst->inst()->dump(out_file);
		out_file << "\\l";
    }
    out_file << "\"] ; \n";

    // edges between info, legend, code
    if(info)
    	out_file << "\"info\" -> \"legend\";\n";
    out_file << "\"legend\" -> \"code\";\n";

    // display nodes
    for (EGInstSeq::InstIterator inst(graph->instSeq()) ; inst ; inst++) {
		// dump nodes
		out_file << "{ rank = same ; ";
		for (EGInst::NodeIterator node(inst) ; node ; node++) {
			out_file << "\"" << node->stage()->name();
			out_file << "I" << node->inst()->index() << "\" ; ";
		}
		out_file << "}\n";
		// again to specify labels
		for (EGInst::NodeIterator node(inst) ; node ; node++) {
			out_file << "\"" << node->stage()->name();
			out_file << "I" << node->inst()->index() << "\"";
			out_file << " [shape=record, ";
			if (node->inst()->part() == BLOCK)
				out_file << "color=blue, ";
			out_file << "label=\"" << node->stage()->name();
			out_file << "(I" << node->inst()->index() << ") [" << node->latency() << "]\\l" << node->inst()->inst();
			out_file << "\"] ; \n";
		}
		out_file << "\n";
    }

    // display edges
    int group_number = 0;
    for (EGInstSeq::InstIterator inst(graph->instSeq()) ; inst ; inst++) {
		// dump edges
		for (EGInst::NodeIterator node(inst) ; node ; node++) {
			for (ExecutionGraph::Successor next(node) ; next ; next++) {
				if ( node != inst->firstNode()
					 ||
					 (node->stage()->category() != EGStage::EXECUTE)
					 || (node->inst()->index() == next->inst()->index()) ) {

					// display edges
					out_file << "\"" << node->stage()->name();
					out_file << "I" << node->inst()->index() << "\"";
					out_file << " -> ";
					out_file << "\"" << next->stage()->name();
					out_file << "I" << next->inst()->index() << "\"";

					// display attributes
					bool first;
					dumpAttrBegin(out_file, first);

					// latency if any
					if(next.edge()->latency()) {
						dumpAttr(out_file, first);
						out_file << "label=\"" << next.edge()->latency() << "\"";
					}

					// edge style
					switch( next.edge()->type()) {
					case EGEdge::SOLID:
						if (node->inst()->index() == next->inst()->index()) {
							dumpAttr(out_file, first);
							out_file << "minlen=4";
						}
						break;
					case EGEdge::SLASHED:
						dumpAttr(out_file, first);
						out_file << " style=dotted";
						if (node->inst()->index() == next->inst()->index()) {
							dumpAttr(out_file, first);
							out_file << "minlen=4";
						}
						break;
					default:
						break;
					}

					// dump attribute end
					dumpAttrEnd(out_file, first);
					out_file << ";\n";

					// group
					if ((node->inst()->index() == next->inst()->index())
						|| ((node->stage()->index() == next->stage()->index())
							&& (node->inst()->index() == next->inst()->index()-1)) ) {
						out_file << "\"" << node->stage()->name();
						out_file << "I" << node->inst()->index() << "\"  [group=" << group_number << "] ;\n";
						out_file << "\"" << next->stage()->name();
						out_file << "I" << next->inst()->index() << "\" [group=" << group_number << "] ;\n";
						group_number++;
					}
				}
			}
		}
		out_file << "\n";
    }
    out_file << "}\n";
}

EGGenericDotDisplay::EGGenericDotDisplay(ExecutionGraph *graph,
											const Path& out_file_path,
											const string& info)
: EGDisplay(graph,out_file_path,info) {
	OutFileStream out_file_stream(out_file_path);
	elm::io::Output out_file(out_file_stream);

    int i=0;
    bool first_line = true;
    int width = 5;
    out_file << "digraph G {\n";

    // display information if any
    if(info)
    	out_file << "\"info\" [shape=record, label=\"{" << info << "}\"];\n";

   // display instruction sequence
    out_file << "\"code\" [shape=record, label= \"\\l";
    bool body = true;
    BasicBlock *bb = 0;
    for (EGInstSeq::InstIterator inst(graph->instSeq()) ; inst ; inst++) {
		if(inst->part() == BLOCK && body) {
			body = false;
			out_file << "------\\l";
		}
    	BasicBlock *cbb = inst->basicBlock();
    	if(cbb != bb) {
    		bb = cbb;
    		out_file << bb << "\\l";
    	}
    	out_file << "I" << inst->index() << ": ";
		out_file << "0x" << fmt::address(inst->inst()->address()) << ":  ";
		inst->inst()->dump(out_file);
		out_file << "\\l";
    }
    out_file << "\"] ; \n";

    // edges between info, legend, code
    if(info)
    	out_file << "\"info\" -> \"legend\";\n";
    out_file << "\"legend\" -> \"code\";\n";

    // display nodes
    for (EGInstSeq::InstIterator inst(graph->instSeq()) ; inst ; inst++) {
		// dump nodes
		out_file << "{ rank = same ; ";
		for (EGInst::NodeIterator node(inst) ; node ; node++) {
			out_file << "\"" << node->stage()->name();
			out_file << "I" << node->inst()->index() << "\" ; ";
		}
		out_file << "}\n";
		// again to specify labels
		for (EGInst::NodeIterator node(inst) ; node ; node++) {
			out_file << "\"" << node->stage()->name();
			out_file << "I" << node->inst()->index() << "\"";
			out_file << " [shape=record, ";
			if (node->inst()->part() == BLOCK)
				out_file << "color=blue, ";
			out_file << "label=\"" << node->stage()->name();
			out_file << "(I" << node->inst()->index() << ") [" << node->latency() << "]\\l" << node->inst()->inst();
			out_file << "\"] ; \n";
		}
		out_file << "\n";
    }

    // display edges
    int group_number = 0;
    for (EGInstSeq::InstIterator inst(graph->instSeq()) ; inst ; inst++) {
		// dump edges
		for (EGInst::NodeIterator node(inst) ; node ; node++) {
			for (ExecutionGraph::Successor next(node) ; next ; next++) {
				if ( node != inst->firstNode()
					 ||
					 (node->stage()->category() != EGStage::EXECUTE)
					 || (node->inst()->index() == next->inst()->index()) ) {

					// display edges
					out_file << "\"" << node->stage()->name();
					out_file << "I" << node->inst()->index() << "\"";
					out_file << " -> ";
					out_file << "\"" << next->stage()->name();
					out_file << "I" << next->inst()->index() << "\"";

					// display attributes
					bool first;
					dumpAttrBegin(out_file, first);

					// latency if any
					if(next.edge()->latency()) {
						dumpAttr(out_file, first);
						out_file << "label=\"" << next.edge()->latency() << "\"";
					}

					// edge style
					switch( next.edge()->type()) {
					case EGEdge::SOLID:
						if (node->inst()->index() == next->inst()->index()) {
							dumpAttr(out_file, first);
							out_file << "minlen=4";
						}
						break;
					case EGEdge::SLASHED:
						dumpAttr(out_file, first);
						out_file << " style=dotted";
						if (node->inst()->index() == next->inst()->index()) {
							dumpAttr(out_file, first);
							out_file << "minlen=4";
						}
						break;
					default:
						break;
					}

					// dump attribute end
					dumpAttrEnd(out_file, first);
					out_file << ";\n";

					// group
					if ((node->inst()->index() == next->inst()->index())
						|| ((node->stage()->index() == next->stage()->index())
							&& (node->inst()->index() == next->inst()->index()-1)) ) {
						out_file << "\"" << node->stage()->name();
						out_file << "I" << node->inst()->index() << "\"  [group=" << group_number << "] ;\n";
						out_file << "\"" << next->stage()->name();
						out_file << "I" << next->inst()->index() << "\" [group=" << group_number << "] ;\n";
						group_number++;
					}
				}
			}
		}
		out_file << "\n";
    }
    out_file << "}\n";
}

