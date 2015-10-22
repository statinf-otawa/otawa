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

#include <elm/util/array.h>
#include <otawa/cfg/CFG.h>
#include <otawa/prog/File.h>

namespace otawa {

/**
 * @defgroup cfg	CFG (Control Flow Graph)
 *
 * This module allows to represents program as a Control Flow Graph (CFG).
 * The CFG is graph representation of the execution path of the program where
 * nodes represents either a basic block, or a synthetic blocks, and edges, @ref Edge,
 * the control flow transitions between blocks. A @ref BasicBlock, represents
 * blocks of sequential instructions. A synthetic represents sub-CFG references a CFG
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
 * 		if(bb->isEnd())
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
 * @fn bool Block::isEnd(void)   const;
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
 * @fn Edge *Block::sequence(void) const;
 * Get the sequence output edge of the block (if any).
 * @return	Sequence output edge or null.
 */

/**
 * Build a CFG block.
 * @param type	Block type.
 */
Block::Block(t::uint16 type): _type(type), seq(0) {
}


/**
 */
io::Output& operator<<(io::Output& out, Block *block) {

	// end processing
	if(block->isEnd()) {
		if(block->isEntry())
			out << "entry";
		else if(block->isExit())
			out << "exit";
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
			out << sb->cfg();
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
 * @param type	Type of block (it will be at least marked as synthetic).
 */
SynthBlock::SynthBlock(t::uint32 type): Block(type | IS_SYNTH), _cfg(0) {
}

/**
 * @fn CFG *SynthBlock::cfg(void) const;
 * Get the CFG of a synthetic block (if any).
 * @return	Synthetic block CFG or null.
 */


/**
 * @class BasicBlock
 * A basic block of the CFG.
 * @ingroup cfg
 */

/**
 * Build a basic block.
 * @param instructions	Null-ended array of instructions.
 */
BasicBlock::BasicBlock(const Table<Inst *>& instructions): Block(IS_BASIC), insts(instructions) {
	ASSERTP(instructions, "unsupported empty array of instructions");
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
int BasicBlock::size(void) {
	return insts[insts.count() - 1]->topAddress() - insts[0]->address();
}

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
	for(InstIter i(this); i; i++)
		if(i->isControl())
			return i;
	return 0;
}


/**
 * Get the last instruction of the block.
 * @return	Block last instruction.
 */
Inst *BasicBlock::last(void) {
	Inst *r = 0;
	for(InstIter i(this); i; i++)
		r = i;
	return r;
}

/**
 * Count the number of instructions in the block.
 * @return	Instruction count.
 */
int BasicBlock::count(void) const {
	int c = 0;
	for(InstIter i(this); i; i++)
		c++;
	return c;
}


/**
 * @class BasicBlock::InstIter;
 * Iterator for instruction of a basic block.
 * @ingroup cfg
 */


/**
 * @class CFG;
 * Control Flow Graph of OTAWA.
 * @ingroup cfg
 */

/**
 * Build the CFG.
 * @param type		Type of CFG (one of SUBPROG, SYNTH or any user type).
 */
CFG::CFG(type_t type)
: idx(0), _type(type), fst(0), _exit(0), _unknown(0) {
}

/**
 */
CFG::~CFG(void) {

	// delete edges
	for(BlockIter b = this->vertices(); b; b++)
		for(Block::EdgeIter e = b->outs(); e; e++)
			delete e;

	// delete nodes
	for(BlockIter b = this->vertices(); b; b++) {
		if(b->isEnd())
			delete b;
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
		Inst *first = fst->first();
		id = FUNCTION_LABEL(first);
		if(!id)
			id = LABEL(first);
	}
	return id;
}


/**
 * Build a name that identifies this CFG and is valid C name.
 * @return	Name of the CFG.
 */
string CFG::name(void) {
	string name = label();
	if(!name)
		name = _ << "__0x" << fst->address();
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
 * @fn int CFG::index(void) const;
 * Get index of the CFG in the current task.
 * @return	CFG index.
 */

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
 * @fn BasicBlock *CFG::unknown(void);
 * Get the unknown basic block of the CFG (if any).
 * @return Unknown basic block or null.
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
 * @param first	First basic block of CFG.
 */
CFGMaker::CFGMaker(void)
: GenDiGraphBuilder<Block, Edge>(cfg = new CFG(), new Block(Block::IS_END | Block::IS_ENTRY)), u(0) {
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
Block *CFGMaker::exit(void) const {
	Block *e = cfg->exit();
	if(!e) {
		e = new Block(Block::IS_EXIT);
		cfg->_exit = e;
	}
	return e;
}

/**
 * Get the unknown edge.
 * @return	Unknown edge.
 */
Block *CFGMaker::unknown(void) {
	if(!u)
		u = new SynthBlock();
	return u;
}

/**
 * Build and return the CFG itself.
 * @return	Built CFG.
 */
CFG *CFGMaker::build(void) {
	if(cfg->exit())
		add(cfg->exit());
	if(u)
		add(u);
	return cfg;
}

/**
 * Add an edge for a sequence.
 * @param v		Source BB.
 * @param w		Sink BB.
 * @param e		Edge itself.
 */
void CFGMaker::seq(Block *v, Block *w, Edge *e) {
	GenDiGraphBuilder<Block, Edge>::add(v, w, e);
	v->seq = e;
}

/**
 * Add a basic block to the CFG. If it is the first,
 * it is considered as the entry point of the CFG.
 * @param v		Added block.
 */
void CFGMaker::add(Block *v) {
	sgraph::GenDiGraphBuilder<Block, Edge>::add(v);
	if(!cfg->fst && v->isBasic())
		cfg->fst = v->toBasic();
}

/**
 * Add a synthetic block.
 * @param v		Added synthetic block.
 * @param cfg	CFG of the synthetic block.
 */
void CFGMaker::add(SynthBlock *v, CFG *cfg) {
	sgraph::GenDiGraphBuilder<Block, Edge>::add(v);
	v->_cfg = cfg;
}

/**
 * Add a synthetic block.
 * @param v		Added synthetic block.
 * @param cfg	Maker of the CFG.
 */
void CFGMaker::add(SynthBlock *v, const CFGMaker& cfg) {
	sgraph::GenDiGraphBuilder<Block, Edge>::add(v);
	v->_cfg = cfg.cfg;
}

}	// otawa


#if 0
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
 * @fn Code *CFG::code(void) const;
 * Get the code containing the CFG.
 * @return Container code.
 */


/**
 * Get some label to identify the CFG.
 * @return	CFG label (if any) or empty string.
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
					new Edge(vbb, edge->target(), Edge::CALL);
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
#endif
