/*
 *	arm2 -- OTAWA loader to support ARMv5 ISA with GLISS2
 *	PowerPC OTAWA plugin
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2011, IRIT UPS.
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

#include <otawa/prog/Loader.h>
#include <otawa/hard.h>
#include <gel/gel.h>
#include <gel/gel_elf.h>
#include <gel/debug_line.h>
#include <otawa/prog/sem.h>
#include <otawa/loader/arm.h>
#include <elm/stree/MarkerBuilder.h>

extern "C" {
#	include <arm/grt.h>
#	include <arm/api.h>
#	include <arm/config.h>
#	include <arm/used_regs.h>
}

namespace otawa {

namespace arm {

#define VERSION "2.1.0"
OTAWA_LOADER_ID("arm2", VERSION, DAYDATE);

/**
 * @class Info
 * Information provided by an ARM loader supporting GLISS2 engine.
 */

/**
 * @fn void *Info::decode(Inst *inst);
 * Get the decode instruction by GLISS2.
 * The obtained data must be fried with free() method.
 * @param inst	Instruction to decode.
 * @return		Decoded data castable to arm_inst_t *.
 */

/**
 * @fn void Info::free(void *decoded);
 * Free an instruction previously obtained by decode() method.
 * @paramd decoded	Result of decode.
 */


/**
 * t::uint16 Info::multiMask(Inst *inst);
 * For a multiple register memory instruction,
 * get the mask of processed register (1 bit per register).
 * @param inst	Concerned instruction.
 * @return		Mask of processed registers.
 */


/**
 * Identifier used to retrieve ARM specific information.
 * Must be accessed dynamically because ARM loader is a plugin !
 *
 * @par Hooks
 * @li Process from ARM loader.
 */
Identifier <Info *> Info::ID("otawa::arm::Info::ID", 0);


/**
 */
Info::~Info(void) {
}

}

namespace arm2 {

#include "otawa_kind.h"
#include "otawa_target.h"
#include "otawa_multi.h"


/****** Platform definition ******/

// registers
static hard::PlainBank gpr("GPR", hard::Register::INT, 32, "r%d", 16);
static hard::Register sr("sr", hard::Register::BITS, 32);
static hard::MeltedBank misc("misc", &sr, 0);
static const hard::RegBank *banks_tab[] = { &gpr, &misc };
static genstruct::Table<const hard::RegBank *> banks_table(banks_tab, 2);

// register decoding
class RegisterDecoder {
public:
	RegisterDecoder(void) {

		// clear the map
		for(int i = 0; i < ARM_REG_COUNT; i++)
			map[i] = 0;

		// initialize it
		map[ARM_REG_UCPSR] = &sr;
		for(int i = 0; i < 16; i++)
			map[ARM_REG_GPR(i)] = gpr[i];
	}

	inline hard::Register *operator[](int i) const { ASSERT(i < ARM_REG_COUNT); return map[i]; }

private:
	hard::Register *map[ARM_REG_COUNT];
};
static RegisterDecoder register_decoder;


// semantics functions
#define _GPR(i)			gpr[i]->platformNumber()
#define _CPSR()			sr.platformNumber()
#define _BRANCH(t)		sem::branch(t)
#define _TRAP()			sem::trap(0)
#define _CONT()			sem::cont()
#define _IF(c, r, o)	sem::_if(c, r, o)
#define _LOAD(d, a, b)	sem::load(d, a, b)
#define _STORE(d, a, b)	sem::store(d, a, b)
#define _SCRATCH(d)		sem::scratch(d)
/*macro _set(d, a)			= "_SET"(d, a)*/
//#define _SETI(d, a)		sem::seti(d, a)
//#define _CMP(d, a, b)	sem::cmp(d, a, b)
/*macro cmpu(d, a, b)		= "_CMPU"(d, a, b)*/
//#define _ADD(d, a, b)	sem::add(d, a, b)
/*macro sub(d, a, b)		= "_SUB"(d, a, b)*/
//#define _SHL(d, a, b)	sem::shl(d, a, b)
//#define _SHR(d, a, b)	sem::shr(d, a, b)
//#define _ASR(d, a, b)	sem::asr(d, a, b)
/*macro neg(d, a)			= "_NEG"(d, a)
macro not(d, a)			= "_NOT"(d, a)
macro and(d, a, b)		= "_AND"(d, a, b)
macro or(d, a, b)		= "_AOR"(d, a, b)
macro xor(d, a, b)		= "_XOR"(d, a, b)
macro mul(d, a, b)		= "_MUL"(d, a, b)
macro mulu(d, a, b)		= "_MULU"(d, a, b)
macro div(d, a, b)		= "_DIV"(d, a, b)
macro divu(d, a, b)		= "_DIVU"(d, a, b)
macro mod(d, a, b)		= "_MOD"(d, a, b)
macro modu(d, a, b)		= "_MODU"(d, a, b)*/

#include "otawa_sem.h"


// platform
class Platform: public hard::Platform {
public:
	static const Identification ID;

	Platform(const PropList& props = PropList::EMPTY): hard::Platform(ID, props)
		{ setBanks(banks_table); }
	Platform(const Platform& platform, const PropList& props = PropList::EMPTY)
		: hard::Platform(platform, props)
		{ setBanks(banks_table); }

	// otawa::Platform overload
	virtual bool accept(const Identification& id) { return id.abi() == "eabi" && id.architecture() == "arm"; }
	virtual const hard::Register *getSP(void) const { return gpr[13]; }
};
const Platform::Identification Platform::ID("arm-eabi-");


/****** Instruction declarations ******/

class Process;

// Inst class
class Inst: public otawa::Inst {
public:

	inline Inst(Process& process, kind_t kind, Address addr, int size)
		: proc(process), _kind(kind), _addr(addr), isRegsDone(false), _size(size) {
		}

	// Inst overload
	virtual void dump(io::Output& out);
	virtual kind_t kind() { return _kind; }
	virtual address_t address() const { return _addr; }
	virtual t::uint32 size() const { return _size; }
	virtual Process &process() { return proc; }

	virtual const elm::genstruct::Table<hard::Register *>& readRegs() {
		if (!isRegsDone) {
			decodeRegs();
			isRegsDone = true;
		}
		return in_regs;
	}

	virtual const elm::genstruct::Table<hard::Register *>& writtenRegs() {
		if(!isRegsDone) {
			decodeRegs();
			isRegsDone = true;
		}
		return out_regs;
	}
	
	virtual void semInsts (sem::Block &block);
protected:
	Process &proc;
	kind_t _kind;
	int _size;

private:
	void decodeRegs(void);
	elm::genstruct::AllocatedTable<hard::Register *> in_regs;
	elm::genstruct::AllocatedTable<hard::Register *> out_regs;
	arm_address_t _addr;
	bool isRegsDone;
};


// BranchInst class
class BranchInst: public Inst {
public:

	inline BranchInst(Process& process, kind_t kind, Address addr, int size)
		: Inst(process, fixKind(process, addr, kind), addr, size), _target(0), isTargetDone(false)
		{ }

	virtual otawa::Inst *target();

protected:
	arm_address_t decodeTargetAddress(void);

private:

	static kind_t fixKind(Process& process, const Address& addr, kind_t kind);

	otawa::Inst *_target;
	bool isTargetDone;
};



/****** Segment class ******/
class Segment: public otawa::Segment {
public:
	Segment(Process& process,
		CString name,
		address_t address,
		t::uint32 size,
		int flags = EXECUTABLE)
	: otawa::Segment(name, address, size, flags), proc(process) { }

protected:
	virtual otawa::Inst *decode(address_t address);

private:
	Process& proc;
};


/****** Process class ******/

typedef enum { NONE = 0, ARM = 1, THUMB = 2, DATA = 3 } area_t;

io::Output& operator<<(io::Output& out, area_t area) {
	switch(area) {
	case NONE:	out << "none"; break;
	case ARM:	out << "arm"; break;
	case THUMB:	out << "thumb"; break;
	case DATA:	out << "data"; break;
	}
	return out;
}

class Process: public otawa::Process, public arm::Info {
	typedef elm::stree::Tree<Address::offset_t, area_t> area_tree_t;
public:
	static const t::uint32 IS_BL_0			= 0x08000000,
							 IS_BL_1 		= 0x04000000,
							 IS_BX_IP 		= 0x02000000,
							 IS_THUMB_BX	= 0x01000000;

	Process(Manager *manager, hard::Platform *pf, const PropList& props = PropList::EMPTY)
	:	otawa::Process(manager, props),
	 	_start(0),
	 	oplatform(pf),
		_memory(0),
		init(false),
		map(0),
		no_stack(true),
		_file(0)
	{
		ASSERTP(manager, "manager required");
		ASSERTP(pf, "platform required");

		// gliss2 ppc structs
		_platform = arm_new_platform();
		ASSERTP(_platform, "cannot create an arm_platform");
		_decoder = arm_new_decoder(_platform);
		ASSERTP(_decoder, "cannot create an arm_decoder");
		_memory = arm_get_memory(_platform, ARM_MAIN_MEMORY);
		ASSERTP(_memory, "cannot get main arm_memory");
		arm_lock_platform(_platform);
#		ifdef ARM_THUMB
			arm_set_cond_state(_decoder, 0);
#		endif

		// build arguments
		static char no_name[1] = { 0 };
		static char *default_argv[] = { no_name, 0 };
		static char *default_envp[] = { 0 };
		argc = ARGC(props);
		if (argc < 0)
			argc = 1;
		argv = ARGV(props);
		if (!argv)
			argv = default_argv;
		else
			no_stack = false;
		envp = ENVP(props);
		if (!envp)
			envp = default_envp;
		else
			no_stack = false;

		// handle features
		provide(MEMORY_ACCESS_FEATURE);
		provide(SOURCE_LINE_FEATURE);
		provide(CONTROL_DECODING_FEATURE);
		provide(REGISTER_USAGE_FEATURE);
		provide(MEMORY_ACCESSES);
		Info::ID(this) = this;
	}

	virtual ~Process() {
		arm_delete_decoder(_decoder);
		arm_unlock_platform(_platform);
		if(_file)
			gel_close(_file);
	}

	// Process overloads
	virtual int instSize(void) const {
#		ifdef ARM_THUMB
			return 0;
#		else
			return 4;
#		endif
	}
	virtual hard::Platform *platform(void) { return oplatform; }
	virtual otawa::Inst *start(void) { return _start; }

	virtual File *loadFile(elm::CString path) {

		// check if there is not an already opened file !
		if(program())
			throw LoadException("loader cannot open multiple files !");

		// make the file
		File *file = new otawa::File(path);
		addFile(file);

		// build the environment
		gel_env_t genv = *gel_default_env();
		genv.argv = argv;
		genv.envp = envp;
		if(no_stack)
			genv.flags = GEL_ENV_NO_STACK;

		// build the GEL image
		_file = gel_open(&path, NULL, 0);
		if(!_file)
			throw LoadException(_ << "cannot load \"" << path << "\": " << gel_strerror());
		gel_image_t *gimage = gel_image_load(_file, &genv, 0);
		if(!gimage) {
			gel_close(_file);
			throw LoadException(_ << "cannot build image of \"" << path << "\": " << gel_strerror());
		}

		// build the GLISS image
		gel_image_info_t iinfo;
		gel_image_infos(gimage, &iinfo);
		for(int i = 0; i < iinfo.membersnum; i++) {
			gel_cursor_t cursor;
			gel_block2cursor(iinfo.members[i], &cursor);
			arm_mem_write(_memory,
				gel_cursor_vaddr(cursor),
				gel_cursor_addr(&cursor),
				gel_cursor_avail(cursor));
		}

		// cleanup image
		gel_image_close(gimage);

		// build segments
		gel_file_info_t infos;
		gel_file_infos(_file, &infos);
		for (int i = 0; i < infos.sectnum; i++) {
			gel_sect_info_t infos;
			gel_sect_t *sect = gel_getsectbyidx(_file, i);
			ASSERT(sect);
			gel_sect_infos(sect, &infos);
			if(infos.vaddr != 0 && infos.size != 0) {
				int flags = 0;
				if(infos.flags & SHF_EXECINSTR)
					flags |= Segment::EXECUTABLE;
				if(infos.flags & SHF_WRITE)
					flags |= Segment::WRITABLE;
				if(infos.type == SHT_PROGBITS)
					flags |= Segment::INITIALIZED;
				Segment *seg = new Segment(*this, infos.name, infos.vaddr, infos.size, flags);
				file->addSegment(seg);
			}
		}

		// Initialize symbols
#		ifdef ARM_THUMB
			stree::MarkerBuilder<Address::offset_t, area_t> area_builder;
			area_builder.add(0UL, (infos.entry & 0x1) ? THUMB : ARM);
			area_builder.add(0xffffffffUL, NONE);
#		endif
		gel_sym_iter_t iter;
		gel_sym_t *sym;
		for(sym = gel_sym_first(&iter, _file); sym; sym = gel_sym_next(&iter)) {

			// get the symbol description
			//gel_sym_t *sym = gel_find_file_symbol(_file, name);
			//assert(sym);
			gel_sym_info_t infos;
			gel_sym_infos(sym, &infos);

			// handle the $a, $t or $d symbols
#			ifdef ARM_THUMB
				if(infos.name[0] == '$') {
					area_t area = NONE;
					switch(infos.name[1]) {
					case 'a': area = ARM; break;
					case 'd': area = DATA; break;
					case 't': area = THUMB; break;
					}
					if(area != NONE) {
						area_builder.add(Address::offset_t(infos.vaddr & 0xfffffffe), area);
						continue;
					}
				}
#			endif

			// compute the kind
			Symbol::kind_t kind = Symbol::NONE;
			t::uint32 mask = 0xffffffff;
			switch(ELF32_ST_TYPE(infos.info)) {
			case STT_FUNC:
				kind = Symbol::FUNCTION;
				mask = 0xfffffffe;
				break;
			case STT_NOTYPE:
				kind = Symbol::LABEL;
				break;
			case STT_OBJECT:
				kind = Symbol::DATA;
				break;
			default:
				continue;
			}

			// Build the label if required
			String label(infos.name);
			Symbol *symbol = new Symbol(*file, label, kind, infos.vaddr & mask, infos.size);
			file->addSymbol(symbol);
		}
		//gel_enum_free(iter);
#		ifdef ARM_THUMB
			area_builder.make(area_tree);
#		endif

		// Last initializations
		_start = findInstAt(Address(infos.entry & 0xfffffffe));
		return file;
	}

	// internal work
	void decodeRegs(Inst *oinst,
		elm::genstruct::AllocatedTable<hard::Register *> *in,
		elm::genstruct::AllocatedTable<hard::Register *> *out)
	{
		// Decode instruction
		arm_inst_t *inst = decode_raw(oinst->address());
		if(inst->ident == ARM_UNKNOWN) {
			free_inst(inst);
			return;
		}

		// get the registers
		arm_used_regs_read_t rds;
		arm_used_regs_write_t wrs;
		arm_used_regs(inst, rds, wrs);

		// convert registers to OTAWA
		elm::genstruct::Vector<hard::Register *> reg_in;
		elm::genstruct::Vector<hard::Register *> reg_out;
		for (int i = 0; rds[i] != -1; i++ ) {
			hard::Register *r = register_decoder[rds[i]];
			if(r)
				reg_in.add(r);
		}
		for (int i = 0; wrs[i] != -1; i++ ) {
			hard::Register *r = register_decoder[wrs[i]];
			if(r)
				reg_out.add(r);
		}

		// make the in and the out
		in->allocate(reg_in.length());
		for(int i = 0 ; i < reg_in.length(); i++)
			in->set(i, reg_in.get(i));
		out->allocate(reg_out.length());
		for (int i = 0 ; i < reg_out.length(); i++)
			out->set(i, reg_out.get(i));

		// Free instruction
		free_inst(inst);
	}

	otawa::Inst *decode(Address addr) {
		arm_inst_t *inst = decode_raw(addr);
		Inst::kind_t kind = 0;
		otawa::Inst *result = 0;
		kind = arm_kind(inst);
		int size = arm_get_inst_size(inst) >> 3;
		if(kind & Inst::IS_CONTROL)
			result = new BranchInst(*this, kind, addr, size);
		else
			result = new Inst(*this, kind, addr, size);
		t::uint16 multi = arm_multi(inst);
		if(multi)
			otawa::arm::NUM_REGS_LOAD_STORE(result) = elm::ones(multi);
		char buf[256];
		arm_disasm(buf, inst);
		t::uint8 b0, b1;
		get(addr, b0);
		get(addr + 1, b1);
		free_inst(inst);
		return result;
	}


	// GLISS2 ARM access
	inline int opcode(Inst *inst) const {
		arm_inst_t *i = decode_raw(inst->address());
		int code = i->ident;
		free_inst(i);
		return code;
	}

	inline ::arm_inst_t *decode_raw(Address addr) const {
#		ifdef ARM_THUMB
			if(isThumb(addr))
				return arm_decode_THUMB(decoder(), ::arm_address_t(addr.offset()));
			else
				return arm_decode_ARM(decoder(), ::arm_address_t(addr.offset()));
#		else
			return arm_decode(decoder(), ::arm_address_t(addr.offset()));
#		endif
	}

	inline void free_inst(arm_inst_t *inst) const { arm_free_inst(inst); }
	virtual gel_file_t *file(void) const { return _file; }
	virtual arm_memory_t *memory(void) const { return _memory; }
	inline arm_decoder_t *decoder() const { return _decoder; }
	inline void *platform(void) const { return _platform; }

	virtual Option<Pair<cstring, int> > getSourceLine(Address addr) throw (UnsupportedFeatureException) {
		setup_debug();
		if (!map)
			return none;
		const char *file;
		int line;
		if (!map || gel_line_from_address(map, addr.offset(), &file, &line) < 0)
			return none;
		return some(pair(cstring(file), line));
	}

	virtual void getAddresses(cstring file, int line, Vector<Pair<Address, Address> >& addresses) throw (UnsupportedFeatureException) {
		setup_debug();
		addresses.clear();
		if (!map)
			return;
		gel_line_iter_t iter;
		gel_location_t loc, ploc = { 0, 0, 0, 0 };
		// TODO		Not very performant but it works.
		for (loc = gel_first_line(&iter, map); loc.file; loc = gel_next_line(&iter)) {
			cstring lfile = loc.file;
			if (file == loc.file || lfile.endsWith(file)) {
				if (line == loc.line)
					addresses.add(pair(Address(loc.low_addr), Address(loc.high_addr)));
				else if(loc.file == ploc.file && line > ploc.line && line < loc.line)
					addresses.add(pair(Address(ploc.low_addr), Address(ploc.high_addr)));
			}
			ploc = loc;
		}
	}

	virtual void get(Address at, t::int8& val)
		{ val = arm_mem_read8(_memory, at.offset()); }
	virtual void get(Address at, t::uint8& val)
		{ val = arm_mem_read8(_memory, at.offset()); }
	virtual void get(Address at, t::int16& val)
		{ val = arm_mem_read16(_memory, at.offset()); }
	virtual void get(Address at, t::uint16& val)
		{ val = arm_mem_read16(_memory, at.offset()); }
	virtual void get(Address at, t::int32& val)
		{ val = arm_mem_read32(_memory, at.offset()); }
	virtual void get(Address at, t::uint32& val)
		{ val = arm_mem_read32(_memory, at.offset()); }
	virtual void get(Address at, t::int64& val)
		{ val = arm_mem_read64(_memory, at.offset()); }
	virtual void get(Address at, t::uint64& val)
		{ val = arm_mem_read64(_memory, at.offset()); }
	virtual void get(Address at, Address& val)
		{ val = arm_mem_read32(_memory, at.offset()); }
	virtual void get(Address at, string& str) {
		Address base = at;
		while(!arm_mem_read8(_memory, at.offset()))
			at = at + 1;
		int len = at - base;
		char buf[len];
		get(base, buf, len);
		str = String(buf, len);
	}
	virtual void get(Address at, char *buf, int size)
		{ arm_mem_read(_memory, at.offset(), buf, size); }

	// otawa::arm::Info overload
	virtual void *decode(otawa::Inst *inst) { return decode_raw(inst->address()); }
	virtual void free(void *decoded) { free_inst(static_cast<arm_inst_t *>(decoded)); }

	virtual t::uint16 multiMask(otawa::Inst *inst) {
		arm_inst_t *i = decode_raw(inst->address());
		t::uint16 r = arm_multi(i);
		free_inst(i);
		return r;
	}

private:

	/**
	 * Test if the given address matches a thumb code area.
	 * @param address	Address to test.
	 * @return			True if the address in the thumb area, false else.
	 */
	bool isThumb(const Address& address) const {
#		ifdef ARM_THUMB
			area_t kind = area_tree.get(address.offset());
			return kind == THUMB;
#		else
			return false;
#		endif
	}

	void setup_debug(void) {
		ASSERT(_file);
		if(init)
			return;
		init = true;
		map = gel_new_line_map(_file);
	}

	otawa::Inst *_start;
	hard::Platform *oplatform;
	arm_platform_t *_platform;
	arm_memory_t *_memory;
	arm_decoder_t *_decoder;
	gel_line_map_t *map;
	gel_file_t *_file;
	int argc;
	char **argv, **envp;
	bool no_stack;
	bool init;
#	ifdef ARM_THUMB
	area_tree_t area_tree;
#	endif
};


/****** Instructions implementation ******/

void Inst::dump(io::Output& out) {
	char out_buffer[200];
	arm_inst_t *inst = proc.decode_raw(_addr);
	arm_disasm(out_buffer, inst);
	proc.free_inst(inst);
	out << out_buffer;
}

void Inst::decodeRegs(void) {

	// Decode instruction
	arm_inst_t *inst = proc.decode_raw(address());
	if(inst->ident == ARM_UNKNOWN)
		return;

	// get register infos
	arm_used_regs_read_t rds;
	arm_used_regs_write_t wrs;
	elm::genstruct::Vector<hard::Register *> reg_in;
	elm::genstruct::Vector<hard::Register *> reg_out;
	arm_used_regs(inst, rds, wrs);
	for (int i = 0; rds[i] != -1; i++ ) {
		hard::Register *r = register_decoder[rds[i]];
		if(r)
			reg_in.add(r);
	}
	for (int i = 0; wrs[i] != -1; i++ ) {
		hard::Register *r = register_decoder[wrs[i]];
		if(r)
			reg_out.add(r);
	}

	// store results
	in_regs.allocate(reg_in.length());
	for(int i = 0 ; i < reg_in.length(); i++)
		in_regs.set(i, reg_in.get(i));
	out_regs.allocate(reg_out.length());
	for (int i = 0 ; i < reg_out.length(); i++)
		out_regs.set(i, reg_out.get(i));

	// Free instruction
	arm_free_inst(inst);
}


arm_address_t BranchInst::decodeTargetAddress(void) {

	// get the target
	arm_inst_t *inst= proc.decode_raw(address());
	Address target_addr = arm_target(inst);

	// thumb-1 case
#		ifdef ARM_THUMB

		// blx/0; blx/1
		if(_kind & Process::IS_BL_1) {
			arm_inst_t *pinst = proc.decode_raw(address() - 2);
			Inst::kind_t pkind = arm_kind(pinst);
			if(pkind & Process::IS_BL_0)
				target_addr = arm_target(pinst) + target_addr.offset();
			proc.free_inst(pinst);
		}

		// ldr ip, [pc, #k]; bx ip
		else if(kind() & Process::IS_BX_IP) {

			// look current and previous instruction words
			t::uint32 cur_word, pre_word;
			proc.get(address() - 4, pre_word);
			proc.get(address(), cur_word);

			// is it ldr ip, [pc, #k] with same condition ?
			if(((pre_word & 0x0ffff000) == 0x059fc000)
			&& ((pre_word & 0xf0000000) == (cur_word & 0xf0000000))) {
				// load address from M[pc + 8 + k]
				Address addr = address() + 4 + (pre_word & 0xfff);
				t::uint32 target;
				proc.get(addr, target);
				target_addr = target & 0xfffffffe;
			}
		}
#		endif

	// cleanup
	proc.free_inst(inst);
	return target_addr;
}


/**
 * Fix possibly the kind of a branch instruction.
 * @param process	Current process.
 * @param kind		Original kind.
 * @return			Fixed kind.
 */
Inst::kind_t BranchInst::fixKind(Process& process, const Address& addr, kind_t kind) {

	// pop { ..., ri, ... }; bx ri
	if(kind && Process::IS_THUMB_BX) {
		t::uint16 pre_half, cur_half;
		process.get(addr, cur_half);
		process.get(addr - 2, pre_half);
		int r = (cur_half >> 3) & 0xf;
		if(r < 8
		&& (pre_half & 0xff00) == 0xbc00
		&& (pre_half & (1 << r)))
			kind |= IS_RETURN;
	}

	// no modification
	return kind;
}


otawa::Inst *BranchInst::target() {
	if (!isTargetDone) {
		isTargetDone = true;

		// try to decode the address
		arm_address_t a = decodeTargetAddress();
		if (a)
			_target = process().findInstAt(a);

		// else try to scan if it is a call
		// TODO		Seems a bit strange and bad documented
		else if(size() == 4) {
			otawa::Inst *prev = this->prevInst();
			if(prev && prev->size() == 4) {
				t::uint32 my_word, prev_word;
				proc.get(address(), my_word);
				proc.get(prev->address(), prev_word);
				if((prev_word & 0x0fffffff) == 0x01a0e00f 					// test previous opcode
				&& (prev_word & 0xf0000000) == (my_word & 0xf0000000))		// test same condition
					_kind |= IS_CALL;
			}
		}
	}
	return _target;
}


otawa::Inst *Segment::decode(address_t address) {
	return proc.decode(address);
}


void Inst::semInsts (sem::Block &block) {
	arm_inst_t *inst = proc.decode_raw(address());
	if(inst->ident == ARM_UNKNOWN)
		return;
	arm_sem(inst, block);
	arm_free_inst(inst);

}

/****** loader definition ******/

// alias table
static string table[] = { "elf_40", "arm2" };
static elm::genstruct::Table<string> loader_aliases(table, 1);

// loader definition
class Loader: public otawa::Loader {
public:
	Loader(void): otawa::Loader("arm", Version(VERSION), OTAWA_LOADER_VERSION, loader_aliases) {
	}

	virtual CString getName(void) const { return "arm"; }

	virtual otawa::Process *load(Manager *man, CString path, const PropList& props) {
		otawa::Process *proc = create(man, props);
		if (!proc->loadProgram(path)) {
			delete proc;
			return 0;
		}
		else
			return proc;
	}

	virtual otawa::Process *create(Manager *man, const PropList& props) {
		return new Process(man, new Platform(props), props);
	}
};


} }		// otawa::arm2

// hooks
otawa::arm2::Loader OTAWA_LOADER_HOOK;
otawa::arm2::Loader& arm2_plugin = OTAWA_LOADER_HOOK;

