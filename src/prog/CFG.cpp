/*
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

#include <elm/array.h>

#include <otawa/cfg/CFG.h>
#include <otawa/cfg/features.h>
#include <otawa/flowfact/features.h>
#include <otawa/prog/File.h>

namespace otawa {

/**
 * @defgroup cfg	CFG (Control Flow Graph)
 *
 * This module allows to represents program as a Control Flow Graph (CFG).
 * The CFG is graph representation of the execution path of the program where
 * nodes represents either a basic block, or a synthetic blocks, and edges, @ref Edge,
 * the control flow transitions between blocks. A @ref BasicBlock, represents
 * blocks of sequential instructions. A synthetic block represents sub-CFG references a CFG
 * that may be a subprogram call or a separated subgraph of blocks. Whatever, CFG
 * have usually a single entry point and a single exit point. Only subprograms
 * with unresolved control target supports a second exit node, called "unknown":
 * unresolved branches are implemented as edges from the current basic block
 * to the unknown node.
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
 * A CFG is made mainly of 5 classes:
 * @li @ref CFG -- the CFG itself providing entry and exit virtual nodes and the list of BB,
 * @li @ref Edge -- an edge in the CFG linking a source BB with a target BB,
 * @li @ref Block -- a block with a list of input edges and list of output edges,
 * @li @ref BasicBlock -- subclass of @ref Block, sequence of instructions with address and size,
 * @li @ref SynthBlock -- subclass of @ref Block, sub-CFG, potentially a subprogram.
 *
 * These classes are subclasses of @ref PropList and therefore support properties.
 *
 * To visit a CFG, one has to start from either the entry, or the exit block (that are not virtual blocks
 * and does not match any code in the program) and follows the edges (be aware that a CFG may contain
 * loops !). To get the outputting edges of a BB, the following code based on the iterator @ref BasicBlock::OutIterator
 * may apply:
 * @code
 * for(BasicBlock::OutIter edge = bb->outs(); edge; edge++)
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
 * 		if(bb->isVirtual())
 * 			return;
 *		cout << "\t" << bb << io::endl;
 *		for(BasicBlock::InstIter inst = bb->insts(); inst; inst++)
 *			cout << "\t\t" << inst->address() << "\t" << *inst << io::endl;
 * 	}
 * };
 * @endcode
 *
 * One may also observe that the CFG representation fully supports transformations like inlining
 * of called sub-program or the representation of a possible exception raise. As shown in the section
 * on the transformation of CFG, these transformations does not duplicate code or changes the original
 * program. Only the representation is modified and aims at improving the precision in some particular
 * cases.
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
 * @li @ref NO_BLOCK -- avoid to involve in the CFGs the block whose first instruction is marked,
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
 * @li @ref TASK_ENTRY 	(in the configuration properties)
 * @li @ref ENTRY_CFG	(in the configuration properties or on the workspace)
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
 * @class Edge
 * Edge between two blocks of a CFG.
 * @ingroup cfg
 */

/**
 * @fn bool Edge::isNotTaken(void) const;
 * Test if the edge is marked as not-taken, that is, represents control
 * flow in sequence between the last instruction of source block and
 * the first instruction of sink block.
 * @return	True if it is not taken, false else.
 */

/**
 * @fn bool Edge::isTaken(void) const;
 * Test if the edge is marked as taken, that is, represents control flow
 * when the branch os a basic block is taken and the sink block is
 * the target of the taken branch.
 * @return	True if it is taken, false else.
 */

/**
 * @fn t::uint32 Edge::flags(void) const;
 * Get flags associated with the edge.
 * @return	Edge flags.
 */

/**
 * @fn bool Edge::isForward(void) const;
 * Test if the edge is a forward edge, that is, branch to an address bigger
 * than the branch instruction address.
 * @return	True if it is forward branch, false else.
 *
 */

/**
 * @fn bool Edge::isBackward(void) const;
 * Test if the edge is a backward edge, that is, branch to an address less
 * or equal to the branch instruction address. Backward branch represents
 * often back edge of loops (but it is not mandatory).
 * @return	True if it is back branch, false else.
 */



/**
 */
io::Output& operator<<(io::Output& out, Edge *edge) {
	out << edge->source() << " -> " << edge->sink();
	return out;
}


/**
 * @class Block;
 * A node in the CFG. This class is abstract and usually subclassed
 * as BasicBlock or SynthBlock.
 * @ingroup cfg
 */

/**
 * @fn bool Block::isVirtual(void)   const;
 * Test if the block is an end, i.e. an entry, exit or unknown block.
 * @return	True if block is an end.
 */

/**
 * @fn Block::isEntry(void) const;
 * Test if a block is an entry.
 * @return	True if block is an entry.
 */

/**
 * @fn Block::isExit(void) const;
 * Test if a block is an exit.
 * @return	True if block is an exit.
 */

/**
 * @fn Block::isUnknown(void) const;
 * Test if a block is unknown.
 * @return	True if block is unknown.
 */

/**
 * @fn Block::isSynth(void) const;
 * Test if a block is synthetic, that is, may be subclassed to SynthBlock.
 * @return	True if block is synthetic.
 */

/**
 * @fn Block::isBasic(void) const;
 * Test if a block is basic, that is, may be subclassed to BasicBlock.
 * @return	True if block is basic.
 */

/**
 * @fn bool Block::isCall(void) const;
 * Test if a synthetic block is subprogram call.
 * @return	True if block is a synthetic subprogram call.
 */

/**
 * @fn int Block::id(void) const;
 * Returns a Block identifier that is unique to the whole program
 */

/**
 * @fn Block::operator BasicBlock *(void);
 * Convert current block to BasicBlock. An assertion failure is raised
 * if it can't be.
 * @return	Matching basic block.
 */

 /**
  * @fn Block::operator SynthBlock *(void);
  * Convert current block to SynthBlock. An assertion failure is raised
  * if it can't be.
  * @return	Matching synthetic block.
  */

/**
 * @fn int Block::id(void) const;
 * Returns a Block identifier that is unique to the whole program.
 * This number if positive or null and less than the countBB() of the CFG
 * collection containing the parent CFG of this block.
 * @return	Unique identifier.
*/



/**
 * Build a CFG block.
 * @param type	Block type.
 */
Block::Block(t::uint16 type): _type(type), _cfg(0) {
}


/**
 */
io::Output& operator<<(io::Output& out, Block *block) {

	// null pointer
	if(!block)
		out << "<null>";

	// end processing
	else if(block->isVirtual()) {
		if(block->isEntry())
			out << "entry";
		else if(block->isExit())
			out << "exit";
		else if(block->isPhony())
			out << "BB " <<block->index() << " (phony)";
		else
			out << "unknown";
	}

	// common block processing
	else {
		out << "BB " << block->index() << " (";
		if(block->isBasic()) {
			BasicBlock *bb = *block;
			out << bb->address();
		}
		else {
			SynthBlock *sb = *block;
			if(sb->callee())
				out << sb->callee();
			else
				out << "unknown";
		}
		out << ')';
	}

	return out;
}


/**
 * @class SynthBlock;
 * A CFG block representing a branch or a call to a sub-CFG.
 * @ingroup cfg
 */

/**
 * Build a synthetic block.
 * @param type		Type of block (it will be at least marked as synthetic).
 */
SynthBlock::SynthBlock(t::uint32 type):
	Block(type | IS_SYNTH), _callee(0)
{
}


/**
 * If the current synthetic block is a call, return the address of the instruction
 * performing the call.
 * @return	Calling instruction or null (synthetic is not a function call).
 */
Inst *SynthBlock::callInst(void) {
	EdgeIter i = ins();
	if(!i)
		return 0;
	else if(!i->source()->isBasic())
		return 0;
	else
		return i->source()->toBasic()->control();
}


/**
 * @fn CFG *SynthBlock::callee(void) const;
 * Get the CFG called in a synthetic block (if any).
 * @return	Synthetic block CFG or null.
 */

/**
 * @fn CFG *SynthBlock::caller(void) const;
 * Get the CFG owner of a synthetic block (if any).
 * @return	Caller / owner CFG.
 */


/**
 * @class PhonyBlock;
 * A phony block does not represent any code in the program but may be used
 * to structure the CFG in a more regular way. It is member of the "end"
 * family blocks but does not really represent a end.
 *
 * For most analyzes, a virtual block can be considered as transparent.
 *
 * @ingroup cfg
 */


/**
 * @class BasicBlock
 * A basic block of the CFG.
 * @ingroup cfg
 */

/**
 * Build a basic block.
 * @param instructions	Array of instructions.
 */
BasicBlock::BasicBlock(const Array<Inst *>& instructions): Block(IS_BASIC), _insts(instructions) {
	ASSERTP(instructions.count() > 0, "unsupported empty array of instructions");
}

/**
 */
BasicBlock::~BasicBlock(void) {
}

/**
 * @fn Address BasicBlock::address(void) const;
 * Get initial address of the block.
 * @return Block initial address.
 */

/**
 * Get size of the basic block.
 * @return	Block size (in bytes).
 */
int BasicBlock::size(void) const {
	return _insts[_insts.count() - 1]->topAddress() - _insts[0]->address();
}

/**
 * Test if the current BB contains the given instruction.
 * Beware: as some instruction sets melt opcode of instructions,
 * it is not enough to test containment with addresses.
 * Hence, this function test really if the given instruction
 * is member of the BB list of instructions.
 * @param i		Instruction to look for.
 * @return		True if the instruction is in the BB list, false else.
 */
bool BasicBlock::contains(Inst *i) {
	for(auto mi: *this)
		if(mi == i)
			return true;
	return false;
}

/**
 * @fn bool BasicBlock::contains(Address addr);
 * Test if the given address is in the area covered
 * by the BB.
 * @param addr	Address to test.
 * @return		True of the address in the BB, false else.
 */


/**
 * @fn bool BasicBlock::overlap(const MemArea& area) const;
 * Test if the area of the current BB overlaps the given memory area.
 * @param area	Memory area to test with.
 * @return		True if both memory areas overlap, false else.
 */


/**
 * @fn bool BasicBlock::overlap(BasicBlock *bb) const;
 * Test if the area of the current BB overlaps the memory area of the given BB.
 * @param bb	BB to test for overlapping.
 * @return		True if both BB memory areas overlap, false else.
 */


/**
 * @fn Address BasicBlock::topAddress(void);
 * Get top address, that is, the address of the byte following the basic block.
 * @param	Block top address.
 */

/**
 * @fn Inst *BasicBlock::first(void) const;
 * Get the first instruction of the block.
 * @return	Block first instruction?
 */

/**
 * Get the control instruction of the block (if any).
 * @return	Block control instruction.
 */
Inst *BasicBlock::control(void) {
	for(InstIter i(this); i(); i++)
		if(i->isControl() && !IGNORE_CONTROL(*i))
			return *i;
	return 0;
}


/**
 * Count the number of instructions in the block.
 * @return	Instruction count.
 */
int BasicBlock::count(void) const {
	int c = 0;
	for(InstIter i(this); i(); i++)
		c++;
	return c;
}


/**
 * Compute the predecessors of the current basic block that (a) are basic
 * blocks and (b)for which transition can be accounted by an edge.
 * The result predecessor vector is empty if there is at least one
 * predecessor basic block that cannot be accounted by an edge.
 *
 * This function is useful when an analysis requires to work on instructions
 * of chains of basic blocks.
 *
 * The result is a vector of pair (b, e) where b is a predecessor basic blocks
 * and e the edge that account for the number of iterations of the basic block.
 *
 * @param preds	List of predecessors basic blocks and accounting edge.
 *
 */
void BasicBlock::basicPreds(basic_preds_t& preds) {
	Vector<Edge *> todo;

	// prepare to-do list
	for(auto e: inEdges())
		todo.push(e);

	// work on the todo list
	while(todo) {
		auto p = todo.pop();
		Block *b = p->source();

		// BB case
		if(b->isBasic())
			preds.add(pair(b->toBasic(), p));

		// phony case
		else if(b->isPhony()) {
			for(auto e: b->inEdges())
				todo.push(e);
		}

		// entry case
		else if(b->isEntry()) {
			if(b->cfg()->callCount() == 0)
				preds.push(pair(null<BasicBlock>(), p));
			else
				for(auto c: b->cfg()->callers())
					todo.push(*c->inEdges().begin());
		}

		// synthetic case
		else {
			SynthBlock *sb = b->toSynth();
			if(sb->callee() == nullptr
			|| (sb->callee()->exit()->countIns() != 1 && sb->callee()->callCount() > 1)) {
				preds.clear();
				preds.push(pair<BasicBlock *, Edge *>(nullptr, p));
				return;
			}
			for(auto e: sb->callee()->exit()->inEdges())
				todo.push(e);
		}
	}
}


/**
 * @class BasicBlock::InstIter;
 * Iterator for instruction of a basic block.
 */


/**
 * @class CFG;
 * Control Flow Graph of OTAWA.
 * @ingroup cfg
 */

/**
 * Build the CFG.
 * @param first		First instruction of CFG.
 * @param type		Type of CFG (one of SUBPROG, SYNTH or any user type).
 */
CFG::CFG(Inst *first, type_t type)
:	idx(0),
	_offset(0),
	_type(type),
	fst(first),
	_entry(nullptr),
	_exit(nullptr),
	_unknown(nullptr)
{ }

/**
 */
CFG::~CFG(void) {

	// delete edges
	for(BlockIter b = this->vertices(); b(); b++) {
		Block::EdgeIter e = b->outs();
		while(e()) {
			delete *e;
			e = b->outs();
		}
	}

	// delete nodes
	for(BlockIter b = this->vertices(); b(); b++) {
		if(b->isVirtual())
			delete *b;
		else if(b->isBasic()) {
			BasicBlock *bb = **b;
			delete bb;
		}
		else {
			SynthBlock *sb = **b;
			delete sb;
		}
	}

}

/**
 * Get some label associated with CFG.
 * @return	CFG label (if any) or empty string.
 */
String CFG::label(void) {
	string id = LABEL(this);
	if(!id) {
		id = FUNCTION_LABEL(fst);
		if(!id)
			id = LABEL(fst);
	}
	return id;
}


/**
 * Build a name that identifies this CFG and is valid C name.
 * @return	Name of the CFG.
 */
string CFG::name(void) {
	string name = LABEL(this);
	if(!name) {
		name = FUNCTION_LABEL(fst);
		if(!name)
			name = LABEL(fst);
		if(!name)
			name = _ << "__0x" << fst->address();
	}
	return name;
}


/**
 * Format the display of the given address relativelt to the given CFG.
 * @param addr	Address to format.
 * @return		Formatted address.
 */
string CFG::format(const Address& addr) {
	string lab = label();
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
 * Count the number of calls to this CFG.
 * @return	Number of calls to this CFG.
 */
int CFG::callCount(void) const {
	int c = 0;
	for(auto i: callers()) {
		(void)i;
		c++;
	}
	return c;
}


/**
 * Remove the provided identifier from all blocks
 * in the CFG.
 * @param id	Identifier to remove.
 */
void CFG::clean(const AbstractIdentifier& id) {
	for(auto v: *this)
		v->removeAllProp(&id);
}


/**
 * @fn bool CFG::isTop() const;
 * Test if the current CFG is the top CFG, that is, the CFG representing
 * the start of the analysis (usually of a task).
 * @return	True if the CFG is the top one, false else.
 */


/**
 * @fn int CFG::index(void) const;
 * Get index of the CFG in the current task.
 * @return	CFG index.
 */

/**
 * @fn BasicBlock *CFG::exit(void);
 * Get the exit basic block of the CFG.
 * @return Exit basic block.
 */

/**
 * @fn BasicBlock *CFG::unknown(void);
 * Get the unknown basic block of the CFG (if any).
 * @return Unknown basic block or null.
 */

/**
 * @fn CallerIter CFG::callers(void) const;
 * Get the list of synthetic block performing a call to this CFG.
 * @return	Caller synthetic block iterator.
 */

/**
 * Address CFG::address(void);
 * Get the address of the first instruction of the CFG.
 * @return	Return address of the first instruction.
 */

io::Output& operator<<(io::Output& out, CFG *cfg) {
	out << cfg->name();
	return out;
}


/**
 * @class CFGMaker
 * Constructor for a CFG. Notice it is the only way to build
 * a CFG. After the construction, CFG can not be modified any more.
 * Modification must be done by cloning and modifying a particular CFG.
 * @ingroup cfg
 */


/**
 * Build a CFG.
 * @param first	First instruction of CFG.
 * @param fix	If true, unknown and exit vertices addition (and numbering)
 * 				is not delayed.
 */
CFGMaker::CFGMaker(Inst *first, bool fix)
: GenDiGraphBuilder<Block, Edge>(cfg = new CFG(first)),
  _fix(fix) {
	cfg->_entry = new Block(Block::IS_VIRTUAL | Block::IS_ENTRY);
	add(cfg->_entry);
}

/**
 * @fn Block *CFGMaker::entry(void) const;
 * Get entry block.
 * @return Entry block.
 */

/**
 * Get the exit block.
 * @eturn	Exit block.
 */
Block *CFGMaker::exit(void) {
	Block *e = cfg->exit();
	if(!e) {
		e = new Block(Block::IS_EXIT);
		cfg->_exit = e;
		e->_cfg = cfg;
		if(_fix)
			add(e);
	}
	return e;
}

/**
 * Get the unknown edge.
 * @return	Unknown edge.
 */
Block *CFGMaker::unknown(void) {
	if(!cfg->_unknown) {
		cfg->_unknown = new Block(Block::IS_VIRTUAL | Block::IS_UNKN);
		cfg->_unknown->_cfg = cfg;
		if(_fix)
			add(cfg->_unknown);
	}
	return cfg->_unknown;
}

/**
 * Get the count of vertices (without unknown).
 * @return	Count of vertices.
 */
int CFGMaker::count(void) {
	return cfg->count();
}


/**
 * Build and return the CFG itself.
 * @return	Built CFG.
 */
CFG *CFGMaker::build(void) {

	// add exit block if required
	if(!cfg->exit())
		exit();
	if(!_fix)
		add(cfg->exit());

	// add unknown block if required
	if(cfg->_unknown && !_fix)
		add(cfg->_unknown);

	// copy properties
	cfg->takeProps(*this);

	return cfg;
}

/**
 * @fn void CFGMaker::add(Block *v);
 * Add a basic block to the CFG. If it is the first,
 * it is considered as the entry point of the CFG.
 * @param v		Added block.
 */

/**
 * Add a synthetic block.
 * @param v			Added synthetic block.
 * @param callee	CFG of the synthetic block.
 */
void CFGMaker::call(SynthBlock *v, CFG *callee) {
	v->_callee = callee;
	if(callee != nullptr)
		callee->_callers.add(v);
	add(v);
}


///
void CFGMaker::add(Block *v) {
	graph::GenDiGraphBuilder<Block, Edge>::add(v);
	v->_cfg = cfg;
}


/**
 * Add a synthetic block.
 * @param v		Added synthetic block.
 * @param maker	Maker of the CFG.
 */
void CFGMaker::call(SynthBlock *v, CFGMaker& maker) {
	v->_callee = maker.cfg;
	maker.cfg->_callers.add(v);
	add(v);
}


/**
 * Fix a synthetic block which CFG was not defined.
 * @param v	Synthetic block to fix.
 * @param g	CFG to fix with.
 */
void CFGMaker::fix(SynthBlock *v, CFGMaker *g) {
	ASSERTP(v->_callee == nullptr, "fixed synthetic block must not have already a CFG!");
	v->_callee = g->cfg;
}


/**
 * Fix a synthetic block which CFG was not defined.
 * @param v	Synthetic block to fix.
 * @param g	CFG to fix with.
 */
void CFGMaker::fix(SynthBlock *v, CFG *g) {
	ASSERTP(v->_callee == nullptr, "fixed synthetic block must not have already a CFG!");
	v->_callee = g;
}

}	// otawa
