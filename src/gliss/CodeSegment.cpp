/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	gliss/CodeSegment.cpp -- gliss::CodeSegment class implementation.
 */

#include <elm/debug.h>
#include <otawa/gliss.h>
#include <otawa/gliss/Symbol.h>
#include <otawa/gliss/MemInst.h>

#define MAP_BITS	6
#define MAP_MASK	((1 << MAP_BITS) - 1)

using namespace elm;

namespace otawa { namespace gliss {

/**
 * @class CodeSegment
 * A segment of code with the PPC Gliss loader.
 */

/**
 * Constructor.
 * @param file		Container file.
 * @param name	Name of the segment.
 * @param memory	Gliss memory.
 * @param address	Address of the segment in the simulator memory.
 * @param size			Size of the segment.
 */
CodeSegment::CodeSegment(
	File& file,
	CString name,
	memory_t *memory,
	address_t address,
	size_t size)
:
	_file(file),
	_name(name),
	code(memory, address, size),
	built(false),
	map(0)
{
}


/**
 */
CodeSegment::~CodeSegment(void) {
	if(map)
		delete [] map;
}


// Overloaded
CString CodeSegment::name(void) {
	return _name.toCString();
}

// Overloaded
address_t CodeSegment::address(void) {
	return code.addr;
}

// Overloaded
size_t CodeSegment::size(void) {
	return code._size;
}

// Overloaded
Collection<ProgItem *>& CodeSegment::items(void) {
	if(!built)
		build();
	return _items;
}

// Overloaded
int CodeSegment::flags(void) {
	return EXECUTABLE;
}

/**
 * Build the code and the instructions in the current segment.
 */
void CodeSegment::build(void) {
	code_t buffer[20];
	instruction_t *inst;
	
	// Link the code
	built = true;
	_items.add(&code);
	
	// Build the instructions
	for(offset_t off = 0; off < code._size; off += 4) {
		address_t addr = code.addr + off;
		
		// Get the instruction
		inst = 0;
		iss_fetch((::address_t)addr, buffer);
		inst = iss_decode(_file.state(), (::address_t)addr, buffer);
		assert(inst);
	
		// Look for its kind
		Inst *result;
		if(inst->ident == ID_Instrunknown)
			result = new Inst(*this, addr);
		else {
			assert(iss_table[inst->ident].category <= 26);
			switch(iss_table[inst->ident].category) {
			case 5: 	// STORE
			case 6:		// LOAD
			case 22:	// FPLOAD
			case 23:	// FPSTORE
				result = new MemInst(*this, addr);
				break;
			case 8:		// BRANCH
			case 10:	// SYSTEM
			case 11:	// TRAP
				if(inst->ident == ID_BL_ && inst->instrinput[0].val.Int24 == 1)
					result = new Inst(*this, addr);
				else
					result = new ControlInst(*this, addr);
				break;
			
			default:
				result = new Inst(*this, addr);
				break;
			}
		}
				
		// Cleanup
		code._insts.addLast(result);
		iss_free(inst);
	}
		
	// Add symbols
	address_t lbound = address(), ubound = lbound + size();
	for(Iterator<otawa::Symbol *> sym(_file.symbols().visit()); sym; sym++) {
		address_t addr = sym->address();
		if(addr >= lbound && addr < ubound) {
			Inst *inst = (Inst *)findByAddress(addr);
			if(inst) {
				Identifier *id;
				switch(sym->kind()) {
				case Symbol::FUNCTION:
					inst->add<String>(File::ID_FunctionLabel, sym->name());
				case Symbol::LABEL:
					inst->add<String>( File::ID_Label, sym->name());
					break;
				}
			}
		} 
	}
}


/**
 * Find an instructions thanks to its address.
 * @param addr	Address of the instruction to find.
 * @return			Found instruction or null if not found.
 */
otawa::Inst *CodeSegment::findByAddress(address_t addr) {

	// Already built ?
	if(!built)
		build();
	
	// In the segment ?
	if(addr < code.address() || addr >= code.address() + code.size())
		return 0;
	
	// If required, build the map
	if(!map)
		buildMap();
	
	// Look in the instruction
	//int cnt = 0;
	for(otawa::Inst *inst = map[index(addr)];
	!inst->atEnd(); inst = inst->next() /*, cnt++*/)
		if(!inst->isPseudo() && inst->address() == addr) {
			//cout << "==> " << cnt << io::endl;
			return inst;
		}
	//cout << "==> NOT FOUND " << cnt << io::endl;		
	return 0;
}


/**
 * @class CodeSegment::Code
 * As, in this loader, there is no function identification, the entire segment is viewed as a monolithic piece of code
 * and only one code representation is required and embeded in the segment object.
 */

// Overloaded
CString CodeSegment::Code::name(void) {
	return "";
}

// Overloaded
address_t CodeSegment::Code::address(void) {
	return addr;
}

// Overloaded
size_t CodeSegment::Code::size(void) {
	return _size;
}


/**
 * Get first instruction of this code item.
 * @return First instruction.
 */
Inst *CodeSegment::Code::first(void) const {
	return (Inst *)_insts.first();
}


/**
 * Get the last instruction of this code item.
 * @return Last instruction.
 */
Inst *CodeSegment::Code::last(void) const {
	return (Inst *)_insts.last();
}


/**
 * Build a new code item.
 * @param memory	GLISS memory structure.
 * @param address	Segment address.
 * @param size	Segment size.
 */
CodeSegment::Code::Code(memory_t *memory, address_t address, size_t size)
: mem(memory), addr(address), _size(size) {
}


/**
 * Free all instructions.
 */
CodeSegment::Code::~Code(void) {
	while(!_insts.isEmpty()) {
		Inst *inst = (Inst *)_insts.first();
		_insts.removeFirst();
		delete inst;
	}
}

// Internal class
class InstIter: public IteratorInst<otawa::Inst *> {
	inhstruct::DLList& list;
	Inst *cur;
public:
	inline InstIter(inhstruct::DLList& _list): list(_list),
		cur((Inst *)_list.first()) { };
	virtual bool ended(void) const { return cur->atEnd(); };
	virtual Inst *item(void) const { return cur; };
	virtual void next(void) { cur = (Inst *)cur->next(); };
};

// Code overload
IteratorInst<otawa::Inst *> *CodeSegment::Code::insts(void) {
	return new InstIter(_insts);
}


/**
 * Compute the index of the address in the map.
 * @param addr	Address of the required instruction.
 * @return		Index in the map.
 */
inline int CodeSegment::index(address_t addr) {
	return (addr - code.address()) >> MAP_BITS;
}




/**
 * Build the map of instructions to improve access time.
 */
void CodeSegment::buildMap(void) {
	
	// Already done, let's go
	if(map)
		return;
	
	// Allocate the map
	map = new otawa::Inst *[(code.size() >> MAP_BITS) + 1];
	assert(map);
	
	// Fill the map
	for(otawa::Inst *inst = code.first(); !inst->atEnd(); inst = inst->next())
		if(!inst->isPseudo() && !((inst->address() - code.address()) & MAP_MASK))
			map[index(inst->address())] = inst;
}

} } // otawa::gliss
