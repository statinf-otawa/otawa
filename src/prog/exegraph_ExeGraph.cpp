/*
 *	$Id$
 *	exegraph module implementation
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
#include <elm/assert.h>
#include <elm/debug.h>
#include <otawa/exegraph/ExeGraph.h>
#include <elm/debug.h>
#include <otawa/dfa/BitSet.h>
#include <elm/genstruct/DLList.h>
#include <otawa/otawa.h>

//#define DO_CHECK
#if defined(NDEBUG) || !defined(DO_CHECK)
#	define CHECK(c)
#else
#	define CHECK(c)	c
#endif


using namespace otawa;
using namespace otawa::hard;
using namespace elm;
using namespace elm::genstruct;

namespace otawa {


/**
 * @class ExeInst <ExeGraph.h>
 * An instruction represented in an ExeGraph. 
 * @param	Type of execution graph nodes.
 */

/**
 * @fn ExeInst::ExeInst(void);
 * Constructor.
 * @param inst	The instruction in the CFG.
 * @param bb	The basic block it belongs to.
 * @param part	The part (BEFORE_PROLOGUE, PROLOGUE, BODY, EPILOGUE) it
 * 				belongs to.
 * @param index	The index of the instruction in the execution graph. 
 */

/**
 * @fn Inst * ExeInst::inst(void);
 * @return	Instruction in the CFG.
 */

/**
 * @fn ExeGraph<N>::code_part_t ExeInst::codePart(void);
 * @return	Code part (BEFORE_PROLOGUE, PROLOGUE, BODY or EPILOGUE).
 */

/**
 * @fn int ExeInst::index(void);
 * @return	Index of the instruction.
 */

/**
 * @fn void ExeInst::setIndex(int index);
 * Sets the instruction index.
 * @param	 index	New instruction index.
 */

/**
 * @fn void ExeInst::addNode(N * node)
 * Adds a node to the list of nodes related to this instruction.
 * @param node	The node to add to the list of nodes related to the instruction.
 */

/**
 * @fn void ExeInst::deleteNodes(void);
 * Clears the list of nodes related to this instruction.
 */

/**
 * @fn N * ExeInst::firstNode(void);
 * @return 	First execution node related to the instruction.
 */

/**
 * @fn N * ExeInst::lastNode(void);
 * @return 	Last execution node related to the instruction.
 */

/**
 * @fn BasicBlock * ExeInst::basicBlock(void);
 * @return 	Basic block that contains the instruction.
 */

/**
 * @class ExeInst::NodeIterator <ExeGraph.h>
 * Iterator on the list of nodes related to the instruction.
 */


/**
 * @fn ExeInst::NodeIterator::NodeIterator(ExeInst<N> *inst)
 * Constructor.
 * @param	inst	The instruction.
 */

/**
 * @class ExeSequence <ExeGraph.h>
 * A sequence of instructions (@ref ExeInst)
 * @param	Type of execution graph nodes.
 */

/**
 * @fn ExeSequence::length();
 * @return	Length of the sequence.
 */

/**
 * @class ExeSequence::InstIterator <ExeGraph.h>
 * Iterator on the list of instructions of the sequence.
 */

/**
 * @fn ExeSequence::InstIterator::InstIterator(ExeSequence<N> *sequence);
 * Constructor.
 * @param	sequence	The sequence of instructions.
 */

/**
 * @class ExeGraph<N>::ExeEdge <ExeGraph.h>
 * An edge between two execution nodes (@ref ExeNode)
 */


/**
 * @fn ExeGraph<N>::ExeEdge::ExeEdge(void);
 * Constructor.
 * @param source	The source node.
 * @param target	The target node.
 * @param type		The type of the edge.
 */

/**
 * @fn ExeGraph<N>::ExeEdge::edge_type_t ExeGraph<N>::ExeEdge::type(void);
 * @return 	Edge type.
 */

/**
 * @fn elm::String ExeGraph<N>::ExeEdge::name(void);
 * Returns the name of the edge composed of the names of the source and target
 * nodes separated by an arrow (e.g. "CM(I1)->IF(I3)").
 * @return 	Edge name.
 */

/**
 * @class ExeGraph<N>::ExeNode <ExeGraph.h>
 * An execution node.
 */


/**
 * @fn ExeGraph<N>::ExeNode::ExeNode(void);
 * Constructor.
 * @param 	graph	The execution graph the node is attached to.
 * @param	stage	The pipeline stage the node is related to.
 * @param	inst	The instruction the node is related to.
 */

/**
 * @fn PipelineStage<N> ExeGraph<N>::ExeNode::pipelineStage(void);
 * @return 	The pipeline stage the node is related to.
 */

/**
 * @fn ExeInst<N> * ExeGraph<N>::ExeNode::inst(void);
 * @return 	The instruction the node is related to.
 */

/**
 * @fn int ExeGraph<N>::ExeNode::latency();
 * @return 	The minimum node latency.
 */

/**
 * @fn int ExeGraph<N>::ExeNode::latency(time_bound_t bound);
 * @param	bound	A time bound (MIN or MAX).
 * @return 	The node latency.
 */

/**
 * @fn void ExeGraph<N>::ExeNode::setLatency(time_bound_t bound, int latency);
 * @param	bound	 A time bound (MIN or MAX).
 * @param	latency	The new value of the latency.
 */

/**
 * @fn bool ExeGraph<N>::ExeNode::needsOperands(void);
 * @return 	Whether the node depends on input operands.
 */

/**
 * @fn void ExeGraph<N>::ExeNode::setNeedsOperands(bool val);
 * Sets whether the node depends on input operands.
 */

/**
 * @fn bool ExeGraph<N>::ExeNode::producesOperands(void);
 * @return 	Whether the node produces output operands.
 */

/**
 * @fn void ExeGraph<N>::ExeNode::setProducesOperands(bool val);
 * Sets whether the node produces output operands.
 */

/**
 * @fn void ExeGraph<N>::ExeNode::addContender(N* cont);
 * Adds a contender to the lists of contenders for the node.
 * @param	cont	The new contender for the node.
 */

/**
 * @fn elm::genstruct::Vector<N *> * ExeGraph<N>::ExeNode::contenders(void);
 * @return	The list of contenders of the node.
 */

/**
 * @fn elm::String ExeGraph<N>::ExeNode::name(void);
 * Returns the name of the node, composed of the name of the pipeline stage
 * and the name of the instruction (e.g. "CM(I8)").
 * @return 	Node name.
 */

/**
 * @class ExeGraph<N>::ExeNode::ContenderIterator <ExeGraph.h>
 * Iterator on the list of contenders of the node.
 */

/**
 * @fn ExeGraph<N>::ExeNode::ContenderIterator::ContenderIterator(N *node)
 * Constructor.
 * @param	node	The execution node.
 */

/**
 * @class ExeGraph <ExeGraph.h>
 * @brief An execution graph that expresses precedence constraints on the execution 
 * of a sequence of instructions in a pipelined processor.
 * See "X. Li, A. Roychoudhury, T. Mitra, Modeling out-of-order processors for WCET 
 * analysis, Real-Time Systems, 34(3), 2006" and "J. Barre, C. Landet, C. Rochange, P. Sainrat, 
 * Modeling Instruction-Level Parallelism for WCET Evaluation, 12th IEEE Int'l Conf. on Embedded 
 * and Real-Time Systems and Applications, August 2006" for an introduction to execution graphs.
 * ExeGraph does not come with a timing computation method. It is instead meant to be derived
 * into classes that include particular solving methods (e.g. LiExeGraph).
 * @param N Type of nodes.
 */


/**
 * @fn ExeGraph<N>::ExeGraph(void);
 * Constructor.
 */

/**
 * @fn GenGraph<N, typename ExeGraph<N>::ExeEdge>  * ExeGraph<N>::graph(void);
 * @return The graph.
 */

/**
 * @fn void ExeGraph<N>::setEntryNode(N * node);
 * Sets the entry node (used for topological graph exploration).
 * @param	node	The entry node.
 */

/**
 * @fn N * ExeGraph<N>::entryNode(void);
 * @return	The entry node.
 */

/**
 * @fn void ExeGraph<N>::build(FrameWork *fw,  Microprocessor<N> * microprocessor, ExeSequence<N> *sequence);
 * @param	fw	The framework.
 * @param	microprocessor	The microprocessor.
 * @param	sequence	The sequence of instructions the graph should represent.
 */

/**
 * @fn elm::String ExeGraph<N>::partName(code_part_t part)
 * @return	The name of the code part (BEFORE_PROLOGUE, PROLOGUE, BODY or EPILOGUE).
 */

/**
 * @fn void ExeGraph<N>::setFirstNode(code_part_t part, N * node)
 * @param	part	The code part.
 * @param	node	The first node of this code part.
 * Sets the first node of the specified code part.
 */

/**
 * @fn void ExeGraph<N>::setLastNode(code_part_t part, N * node)
 * @param	part	The code part.
 * @param	node	The last node of this code part.
 * Sets the last node of the specified code part.
 */

/**
 * @fn N * ExeGraph<N>::firstNode(code_part_t part)
 * @param	part	The code part.
 * @return	The first node of the specified code part.
 */

/**
 * @fn N * ExeGraph<N>::lastNode(code_part_t part)
 * @param	part	The code part.
 * @return	The last node of the specified code part.
 */

/**
 * @class ExeGraph<N>::PipelineIterator <ExeGraph.h>
 * Iterator on the pipeline stage of a microprocessor.
 */

/**
 * @fn ExeGraph<N>::PipelineIterator::PipelineIterator(Microprocessor *processor)
 * Constructor.
 * @param	processor	The microprocessor.
 */

/**
 * @class ExeGraph<N>::FunctionalUnitIterator <ExeGraph.h>
 * Iterator on the functional units of a pipeline stage.
 */

/**
 * @fn ExeGraph<N>::FunctionalUnitIterator::FunctionalUnitIterator(PipelineStage<N> * stage)
 * Constructor.
 * @param	stage	The pipeline stage.
 */

/**
 * @class ExeGraph<N>::FunctionalUnitPipelineIterator <ExeGraph.h>
 * Iterator on the pipeline stages of a functional unit.
 */

/**
 * @fn ExeGraph<N>::FunctionalUnitPipelineIterator::FunctionalUnitPipelineIterator(PipelineStage<N>::FunctionalUnit * fu)
 * Constructor.
 * @param	fu	The functional unit.
 */

/**
 * @class ExeGraph<N>::InstIterator <ExeGraph.h>
 * Iterator on the instructions of a sequence.
 */

/**
 * @fn ExeGraph<N>::InstIterator::InstIterator(ExeSequence<N> *sequence)
 * Constructor.
 * @param	sequence	The sequence.
 */

/**
 * @class ExeGraph<N>::InstNodeIterator <ExeGraph.h>
 * Iterator on the execution nodes related to an instruction.
 */

/**
 * @fn ExeGraph<N>::InstNodeIterator::InstNodetIerator(ExeInst<N> * inst)
 * Constructor.
 * @param	inst	The instruction.
 */

/**
 * @class ExeGraph<N>::Predecessor <ExeGraph.h>
 * An iterator on the predecessors of an execution node.
 */

/**
 * @fn ExeGraph<N>::Predecessor::Predecessor(N * node)
 * Constructor.
 * @param	node	The execution node.
 */

/**
 * @class ExeGraph<N>::Successor <ExeGraph.h>
 * An iterator on the successors of an execution node.
 */

/**
 * @fn ExeGraph<N>::Successor::Successor(N * node)
 * Constructor.
 * @param	node	The execution node.
 */

/**
 * @class ExeGraph<N>::PreorderIterator <ExeGraph.h>
 * An iterator on the nodes of an execution graph that follows a topological order (i.e. a node
 * is handled after all its predecessors.
 */

/**
 * @fn ExeGraph<N>::PreorderIterator::PreorderIterator(ExeGraph<N> * graph, N * node)
 * Constructor.
 * @param	graph	The execution graph.
 * @param	node	The entry node.
 */

Identifier<bool> START("otawa.exegraph.start", false);



} // namespace otawa
