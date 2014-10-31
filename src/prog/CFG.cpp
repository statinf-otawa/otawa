/*
 *	$Id$
 *	CFG class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-08, IRIT UPS.
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
#include <otawa/cfg.h>
#include <elm/debug.h>
#include <otawa/util/Dominance.h>
#include <otawa/dfa/BitSet.h>
#include <elm/genstruct/HashTable.h>
#include <elm/genstruct/VectorQueue.h>

using namespace elm::genstruct;

namespace otawa {

/**
 * @defgroup cfg	CFG (Control Flow Graph)
 *
 * This module allows to represents program as a Control Flow Graph (CFG).
 * The CFG is graph representation of the execution path of the program where
 * nodes, @ref BasicBlock, represents blocks of sequential instructions
 * and edges, @ref Edge, the control flow transitions between blocks.
 *
 * @section cfg-using Using the CFG
 *
 * Most analyses only read the CFG structure and either use the properties of the CFG,
 * or creates new properties. They only need to understand the structure of the CFG.
 *
 * Most classes, features and identifier are accessible in the header file:
 * @code
 * #include <otawa/cfg/features.h>
 * @endcode
 *
 * A CFG is made mainly of 3 classes:
 * @li @ref CFG -- the CFG itself providing entry and exit virtual nodes and the list of BB,
 * @li @ref BasicBlock -- a BB in the CFG with an address, a size, list of input edges and list of output edges,
 * @li @ref Edge -- an edge in the CFG linking a source BB with a target BB.
 *
 * These classes are subclasses of @ref PropList and therefore support properties.
 *
 * To visit a CFG, one has to start from either the entry, or the exit BB (that are not virtual blocks
 * and does not match any code in the program) and follows the edges (be aware that a CFG may contain
 * loops !). To get the outputting edges of a BB, the following code based on the iterator @ref BasicBlock::OutIterator
 * may apply:
 * @code
 * for(BasicBlock::OutIterator edge(bb); edge; edge++)
 * 		process(edge);
 * @endcode
 *
 * Another way to work CFG is to make the analysis class to inherit from one of the following standard
 * processors:
 * @li @ref CFGProcessor -- call method @ref CFGProcessor::processCFG() for each CFG in the task,
 * @li @ref BBProcessor -- call method @ref BBProcessor::processBB() for each BB of the CFGs of the task,
 * @li @ref EdgeProcessor -- call method @ref EdgeProcessor::processEdge() for each edge of the CFGs of the task.
 *
 * They will avoid the hassle work of traversing the different CFGs of the task and BB and edges inside the CFGs.
 *
 * As an example, the following analysis outputs the CFG and disassemble the instructions in the BB.
 * @code
 * class Disassembler: public BBProcessor {
 * protected:
 * 	virtual void processCFG(WorkSpace *ws, CFG *cfg) {
 * 		cout << "CFG " << cfg->label() << io::endl;
 * 		BBProcessor::processCFG(ws, cfg);
 * 	}
 *
 * 	virtual void processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {
 * 		if(bb->isEnd())
 * 			return;
 *		cout << "\t" << bb << io::endl;
 *		for(BasicBlock::InstIterator inst(bb); inst; inst++)
 *			cout << "\t\t" << inst->address() << "\t" << *inst << io::endl;
 * 	}
 * };
 * @endcode
 *
 * The traversal of the graph following edges requires the understanding of the kind of the edges:
 * @li  @ref Edge::NONE -- unused kind,
 * @li  @ref Edge::TAKEN -- represents a branch instruction that is taken (conditional or not),
 * @li  @ref Edge::NOT_TAKEN -- represents a branch instruction that is not-taken (only conditional),
 * @li  @ref Edge::CALL -- represents a sub-program (do not use the Edge::target() but Edge::calledCFG() to get the called CFG),
 * @li  @ref Edge::VIRTUAL -- one of special edges starting from the entry BB or leading to the exit BB,
 * @li  @ref Edge::VIRTUAL_CALL -- represents a sub-program call whose CFG has been inlined,
 * @li  @ref Edge::VIRTUAL_RETURN -- represents a sub-program return whose CFG has been inlined,
 * @li  @ref Edge::EXN_CALL -- represents an exception call whose CFG has been inlined,
 * @li  @ref Edge::EXN_RETURN represents an exception return whose CFG has been inlined,
 *
 * Notice that you shouldn't use the Edge::target() method for an edge of type Edge::CALL but instead
 * the Edge::calledCFG() that gives the CFG of the called sub-program.
 *
 * One may also observe that the CFG representation fully supports transformations like inlining
 * of called sub-program or the representation of a possible exception raise. As shown in the section
 * on the transformation of CFG, these transformations does not duplicate code or changes the original
 * program. Only the representation is modified and aims at improving the precision in some particular
 * cases. Whatevern the @ref Edge::VIRTUAL_CALL, @ref Edge::VIRTUAL_RETURN, @ref Edge::EXN_CALL and
 * @ref Edge::EXN_RETURN provides more information on the transformed CFG:
 * @li @ref CALLED_CFG -- put on virtual call and return edges, gives the original global CFG,
 * @li @ref RECURSIVE_LOOP -- put on the edge representing the recursive call of an inlined sub-program,
 * @li @ref VIRTUAL_RETURN_BLOCK -- put on the block performing the sub-program call, gives the basic block the sub-program returns to (useful to skip an inlined CFG).
 *
 * Finally, a CFG may be called at different points in the other CFGs of the program, the properties
 * @ref CALLED_BY hooked to the CFG allows to get the matching calling edges:
 * @code
 * for(Identifier<Edge *>::Getter edge(cfg, CALLED_BY); edge; edge++)
 * 		processCaller(cfg, edge);
 * @endcode
 *
 *
 * @section cfg-build Building the CFGs
 *
 * Once a program is loaded, OTAWA has only two information item on the structure of the program:
 * the block of bytes representing the program and the startup address of the program.
 * In addition, according to the option of linkage, other symbols may be available like
 * the entry points of the functions composing the program.
 *
 * From this, OTAWA must decode the instructions from these entry points and follows all execution
 * paths until fully decoding the program. This first phase is performed the @ref CFGBuilder analysis
 * that produces as an output, a @ref CFGInfo, the collection of all CFGs (one by function) of the
 * program. This phase is relatively complex and may fail because of:
 * @li errors in decoding the instructions (in this case, the error is signaled to the user and the execution path is aborted),
 * @li unresolved branches (mainly because of computed branch target as found in function pointer call or in some switch compilation schemes).
 *
 * For the latter case, you can help @ref CFGBuilder by providing information on the possible branches
 * using the properties below:
 * @li @ref IS_RETURN -- consider the marked instruction as a return,
 * @li @ref NO_RETURN -- mark the function as not returning,
 * @li @ref NO_CALL -- avoid to involve in the CFGs the function whose first instruction is marked,
 * @li @ref IGNORE_CONTROL -- ignore the control effect of the marked instruction,
 * @li @ref IGNORE_SEQ -- only on a conditional branching instruction, ignore the sequential path,
 * @li @ref BRANCH_TARGET -- gives the target(s) of an indirect branch instruction,
 * @li @ref CALL_TARGET -- gives the target(s) of an indirect call instruction.
 *
 * This properties are usually set by the @ref FlowFactLoader from textual or XML files. Once may also observe
 * that this properties allows also to change artificially the control flow according to the needs of a user.
 * For example, one can avoid the "printf" function to be called by putting a property @ref NO_CALL on its entry
 * instruction.
 *
 * Once the global CFGs of the program are built, the user is usually interested only in a subset
 * of CFGS corresponding to the task the WCET is computed for. This subset is gathered by the
 * @ref CFGCollector analysis from one of the following properties:
 * @li @ref TASK_ENTRY (in the configuration properties)
 * @li @ref ENTRY_CFG (in the configuration properties or on the workspace)
 * @li @ref CFGCollector::ADDED_CFG -- CFG to add to the collection,
 * @li @ref CFGCollection::ADD_FUNCTION -- function to add to the collection.
 *
 * As a result, the @ref INVOLVED_CFGS (type @ref CFGCollection) hooked to the workspace provides
 * this list of functions that is used by the following analyses to perform the WCET computation.
 * Notices that the first CFG of the collection is the entry point of the task. Remark also that
 * the obtained CFG are a copy of the original CFG of the global decoding of the program: this means
 * that you add as many properties to the collected CFG without impacting the CFGs of the global
 * analysis.
 *
 *
 * @section cfg-loop Loops in the CFGs
 *
 * Loops are a common shared issue when handling CFG. OTAWA provides several way to cope with
 * loops. First two features allows to identify loops: @ref LOOP_HEADERS_FEATURE and @ref LOOP_INFO_FEATURE.
 *
 * The @ref LOOP_HEADERS_FEATURE allows to identify the loops in the CFG. Basically, it finds the
 * back-edges (edges causing the looping) and, from these, the header of the loops. Each loop
 * is defined by its header basic block that is the first basic block traversed when entering
 * a loop. An header basic block is the header of only one loop and is a good way to identify the loop.
 * The following properties are set:
 * @li @ref LOOP_HEADER -- evaluates to true when the hooked basic block is a loop header,
 * @li @ref BACK_EDGE -- evaluates to true on a back-edge (the target of the edge is the loop header).
 *
 * This feature is implemented by the @ref Dominance analysis or @ref SpanningTreeBuilder.
 *
 * If @ref LOOP_HEADERS_FEATURE provides minimal information on a loop, @ref LOOP_INFO_FEATURE
 * allows to get full details sometimes for some analyses. The following properties are set:
 * @li @ref ENCLOSING_LOOP_HEADER -- gives the header of the loop enclosing the linked BB,
 * @li @ref LOOP_EXIT_EDGE -- on an edge, informs it is an exit edgen,
 * @li @ref EXIT_LIST -- put on the loop header, get the list of edges exiting the loop.
 *
 * As shown above, the loop handled by OTAWA supports only one entry point and header.
 * This is a classic definition of so-called "regular" loops. Even if this is the most common
 * loop found, some programs exhibit "irregular" loops (at least, two possible entry point).
 * A usual way to support such a loop is to choose one entry point and to duplicate the code
 * of the other entry points: this may be done in OTAWA using the analysis @ref LoopReductor.
 *
 *
 * @section cfg-transform Transforming the CFG
 *
 * CFGs in OTAWA are designed to be light representation of the program, that is, they support
 * duplication with a minimal cost in term of memory footprint. This means, for example,
 * that the precision of the computed WCET may be improved by duplicating some parts of the CFG
 * to cope more precisely with the context of the code. Several OTAWA code processors
 * provide such transformations.
 *
 * The @ref Virtualizer allows to inline the called function at the location they are called.
 * Therefore, if a function is called at several locations, this allows to duplicate the BB
 * and the edges representing the function and, instead to melt the context information,
 * to have a context as precise as possible. Yet, such a transformation may be costly in terms
 * of number of BB and must be used carefully. To prevent such a size explosion,
 * you can mark a CFG as @ref DONT_INLINE to prevent the inlining.
 *
 * Unrolling loops is another way to improve precision. As many loop body have a different
 * behavior between the first iteration and the other one (cache load, etc), it may be interesting
 * to unroll loops once. This may be performed using the code processor @ref LoopUnroller.
 *
 * Finally, some transformations are required by a particular architecture. This is the case
 * of instruction sets with delayed branches. Delayed branches executes the instruction following
 * the branch instruction before performing actually the control flow change. As an outcome,
 * the instruction following a branch is really part of the basic block the branch is contained in.
 * This policy is different from the usual way OTAWA builds CFGs and a transformation must be done
 * to fix the basic blocks: this is performed by @ref DelayedBuilder.
 *
 *
 * @section cfg-io Input / Output in XML
 * This modules provides two code processors allowing to perform input / output
 * on CFG. @ref CFGSaver save the CFG of the current task to a file while
 * @ref CFGLoader allows to load it back.
 *
 * Both processors uses the same XML format defined below:
 * @code
 * <?xml version="1.0" ?>
 * <otawa-workspace>
 *
 * 		<cfg id="ID" entry="ID" exit="ID">
 * 			<bb id="ID" address="ADDRESS"? size="INT"?/>*
 * 			<edge kind="KIND" source="ID" target="ID"? cfg="ID"?/>*
 *		</cfg>*
 *
 * </otawa-workspace>
 * @endcode
 *
 * The OTAWA workspace is viewed, in the CFG representation, as a set of CFG
 * that, in turn, is made of several basic blocks linked by edges. Each CFG and BB
 * is identified with the attribute "id", any textual unique value, allowing
 * to reference them.
 *
 * The cfg element must reference two specific BB (using the unique "id"), an entry BB
 * and an exit BB. The BB is defined by its address and its size (in bytes).
 * The edges are defined by their kind, the source BB and, depending on the type,
 * the target BB or the called CFG (@ref Edge::CALL). The latter attribute "cfg"
 * must give the identifier of an existing CFG.
 */


/**
 * Identifier used for storing and retrieving the CFG on its entry BB.
 *
 * @ingroup cfg
 */
Identifier<CFG *> ENTRY("otawa::ENTRY", 0);


/**
 * Identifier used for storing in each basic block from the CFG its index.
 * Also used for storing each CFG's index.
 *
 * @par Hooks
 * @li @ref BasicBlock
 * @li @ref CFG
 *
 * @ingroup cfg
 */
Identifier<int> INDEX("otawa::INDEX", -1);


/**
 * @class CFG
 * Control Flow Graph representation. Its entry basic block is given and
 * the graph is built using following taken and not-taken properties of the block.
 *
 * @ingroup cfg
 */


/**
 * Format the display of the given address relativelt to the given CFG.
 * @param addr	Address to format.
 * @return		Formatted address.
 */
string CFG::format(const Address& addr) {
	string lab = LABEL(this);
	if(!lab)
		return _ << addr;
	else {
		t::int32 off = addr - address();
		if(off >= 0)
			return _ << lab << " + 0x" << io::hex(off);
		else
			return _ << lab << " - 0x" << io::hex(-off);
	}
}


/**
 * Constructor. Add a property to the basic block for quick retrieval of
 * the matching CFG.
 */
CFG::CFG(Segment *seg, BasicBlock *entry):
	flags(0),
	_entry(BasicBlock::FLAG_Entry),
	_exit(BasicBlock::FLAG_Exit),
	_seg(seg),
	ent(entry)
{
	ASSERT(seg && entry);

	// Get label
	BasicBlock::InstIter inst(entry);
	String label = FUNCTION_LABEL(inst);
	if(label)
		LABEL(this) = label;
}


/**
 * Build an empty CFG.
 */
CFG::CFG(void):
	 flags(0),
	_entry(BasicBlock::FLAG_Entry),
	_exit(BasicBlock::FLAG_Exit),
	_seg(0),
	ent(0)
{
}


/**
 * @fn BasicBlock *CFG::entry(void);
 * Get the entry basic block of the CFG.
 * @return Entry basic block.
 */


/**
 * @fn BasicBlock *CFG::exit(void);
 * Get the exit basic block of the CFG.
 * @return Exit basic block.
 */


/**
 * @fn Code *CFG::code(void) const;
 * Get the code containing the CFG.
 * @return Container code.
 */


/**
 * Get some label to identify the CFG.
 * @return	CFG label or any other identification way.
 */
String CFG::label(void) {
	if(!ent) {
		BasicBlock::OutIterator out(entry());
		if(!out)
			return "";
		ent = out->target();
	}
	string id = LABEL(this);
	if(!id) {
		Inst *first = ent->firstInst();
		if(first) {
			id = FUNCTION_LABEL(first);
			if(!id)
				id = LABEL(first);
		}
		if(!id)
			id = _ << "__0x" << ent->address();
	}
	return id;
}


/**
 * Get the address of the first instruction of the CFG.
 * @return	Return address of the first instruction.
 */
Address CFG::address(void) {
	if(!ent) {
		BasicBlock::OutIterator edge(entry());
		if(edge)
			ent = edge->target();
		if(!ent)
			return Address::null;
	}
	return ent->address();
}


/**
 * @fn elm::Collection<BasicBlock *>& CFG::bbs(void);
 * Get an iterator on basic blocks of the CFG.
 * @return	Basic block iterator.
 */


/**
 * Scan the CFG for finding exit and builds virtual edges with entry and exit.
 * For memory-place and time purposes, this method is only called when the CFG
 * is used (call to an accessors method).
 */
void CFG::scan(void) {

	// Prepare data
	typedef HashTable<BasicBlock *, BasicBlock *> map_t;
	map_t map;
	VectorQueue<BasicBlock *> todo;
	todo.put(ent);

	// Find all BB
	_bbs.add(&_entry);
	while(todo) {
		BasicBlock *bb = todo.get();
		ASSERT(bb);

		// second case : calling jump to a function
		if(map.exists(bb) || (bb != ent && ENTRY(bb)))
			continue;

		// build the virtual BB
		BasicBlock *vbb = new VirtualBasicBlock(bb);
		_bbs.add(vbb);
		map.put(bb, vbb);
		ASSERTP(map.exists(bb), "not for " << bb->address());

		// resolve targets
		for(BasicBlock::OutIterator edge(bb); edge; edge++) {
			ASSERT(edge->target());
			if(edge->kind() != Edge::CALL)
				todo.put(edge->target());
		}
	}

	// Relink the BB
	BasicBlock *vent = map.get(ent, 0);
	ASSERT(vent);
	new Edge(&_entry, vent, Edge::VIRTUAL);
	for(bbs_t::Iterator vbb(_bbs); vbb; vbb++) {
		if(vbb->isEnd())
			continue;
		BasicBlock *bb = ((VirtualBasicBlock *)*vbb)->bb();
		if(bb->isReturn())
			new Edge(vbb, &_exit, Edge::VIRTUAL);

		for(BasicBlock::OutIterator edge(bb); edge; edge++) {

			// A call
			if(edge->kind() == Edge::CALL) {
				Edge *vedge = new Edge(vbb, edge->target(), Edge::CALL);
				vedge->toCall();
			}

			// Pending edge
			else if(!edge->target()) {
				new Edge(vbb, 0, edge->kind());
			}

			// Possibly a not explicit call
			else {
				ASSERT(edge->target());
				BasicBlock *vtarget = map.get(edge->target(), 0);
				if(vtarget)
					new Edge(vbb, vtarget, edge->kind());
				else {		// calling jump to a function
					Edge *nedge = new Edge(vbb, edge->target(), Edge::CALL);
					vbb->flags |= BasicBlock::FLAG_Call;
					new Edge(vbb, &_exit, Edge::VIRTUAL);
				}
			}

		}
	}
	_bbs.add(&_exit);

	// Number the BB
	for(int i = 0; i < _bbs.length(); i++) {
		INDEX(_bbs[i]) = i;
		_bbs[i]->_cfg = this;
	}
	flags |= FLAG_Scanned;

}


/**
 * Number the basic block of the CFG, that is, hook a property with ID_Index
 * identifier and the integer value of the number to each basic block. The
 * entry get the number 0 et the exit the last number.
 */
void CFG::numberBB(void) {
	for(int i = 0; i < _bbs.length(); i++)
		INDEX(_bbs[i]) = i;
}


/**
 */
CFG::~CFG(void) {

	// remove edges
	for(int i = 0; i < _bbs.length() - 1; i++) {
		BasicBlock *bb = _bbs[i];
		while(true) {
			BasicBlock::OutIterator edge(bb);
			if(edge)
				delete *edge;
			else
				break;
		}
	}

	// remover basic blocks
	for(int i = 1; i < _bbs.length() - 1; i++)
		delete _bbs[i];
}


/**
 * Get the first basic block of the CFG.
 * @return	First basic block.
 */
BasicBlock *CFG::firstBB(void) {
	if(!(flags & FLAG_Scanned))
		scan();
	return _bbs[1];
}


/**
 * Get the first instruction of the CFG.
 * @return	First instruction of the CFG.
 */
Inst *CFG::firstInst(void) {
	if(!(flags & FLAG_Scanned))
		scan();
	BasicBlock *bb = firstBB();
	BasicBlock::InstIter inst(bb);
	return *inst;
}


/**
 * Print a reference for the CFG.
 * @param out	Output stream.
 */
void CFG::print(io::Output& out) {
	out << label();
}

} // namespace otawa
