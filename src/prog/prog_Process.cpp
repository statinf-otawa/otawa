/*
 *	Process class implementation
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

#include <elm/deprecated.h>
#include <elm/stree/SegmentBuilder.h>
#include <elm/xom.h>

#include <otawa/prog/Process.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Platform.h>
#include <otawa/prog/Loader.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/prog/Manager.h>
#include <otawa/prog/FixedTextDecoder.h>
#include <otawa/proc/Feature.h>
#include <otawa/prog/File.h>

using namespace elm;

namespace otawa {

/**
 * @addtogroup prog
 * @section prog_process Processes
 *
 * A process is the representation of the program at the start of the execution.
 * A process mainly provides the content of the different memories composing
 * the platform updated to with the bits of the program. In addition,
 * the process provides also details bout the underlying architecture
 * (instruction set, endianness, registers, branching model, etc) or
 * about the capacities of the loader.
 *
 * A simple way to get information about capabilities of an architecture
 * is to look to features provided by the process. They are automatically
 * transfered to any workspace using the process. For example,
 * to test if the architecture supports "delayed branches", one can write:
 * ~~~~
 * 	WorkSpace *ws = ...;
 * 	if(ws->provides(DELAYED_BRANCHEES)
 * 		...;
 * ~~~~
 *
 * The following features are usually provided by processes:
 *	* otawa::CONDITIONAL_INSTRUCTIONS_FEATURE
 *	* otawa::CONTROL_DECODING_FEATURE
 *	* otawa::DELAYED_FEATURE
 *	* otawa::DELAYED2_FEATURE
 *	* otawa::FLOAT_MEMORY_ACCESS_FEATURE
 *	* otawa::MEMORY_ACCESS_FEATURE
 *	* otawa::REGISTER_USAGE_FEATURE
 *	* otawa::SEMANTICS_INFO
 *	* otawa::SEMANTICS_INFO_EXTENDED
 *	* otawa::SEMANTICS_INFO_FLOAT
 *	* otawa::SOURCE_LINE_FEATURE
 *	* otawa::VLIW_SUPPORTED
 */

/**
 * This feature is put on processes by the loader to inform that the
 * control instruction of the current instruction contains delayed branches
 *
 * @par Provider
 * @li program loader
 *
 * @par Properties
 * @li @ref otawa::DELAYED
 *
 * @ingroup prog
 */
p::feature DELAYED_FEATURE("otawa::DELAYED_FEATURE", new Maker<NoProcessor>());


/**
 * @class SimState
 * This abstract must be used to encapsulate the state of processor functional
 * simulator.
 */


/**
 * Build the simulator state.
 * @param process	Owner processor (to check consistency).
 */
SimState::SimState(Process *process): proc(process) {
	ASSERTP(proc, "no process given");
}


/**
 */
SimState::~SimState(void) {
}


/**
 * @fn Process *SimState::process(void) const;
 * Get the owner process.
 * @return	Owner process.
 */


/**
 * @fn Inst *Simstate::execute(Inst *inst);
 * Simulate an instruction in the current process state.
 * @param inst	Instruction to simulate.
 * @return		Next instruction to execute.
 * @note This function must overload by the actual implement of simulation states.
 */


/**
 * Get the lower read address of the last executed instruction.
 * @return	Lower read address.
 */
Address SimState::lowerRead(void) {
		throw UnsupportedFeatureException(process(), MEMORY_ACCESSES);
}


/**
 * Get the upper read address of the last executed instruction.
 * @return	Upper read address.
 */
Address SimState::upperRead(void) {
		throw UnsupportedFeatureException(process(), MEMORY_ACCESSES);
}


/**
 * Get the lower written address of the last executed instruction.
 * @return	Lower written address.
 */
Address SimState::lowerWrite(void) {
		throw UnsupportedFeatureException(process(), MEMORY_ACCESSES);
}


/**
 * Get the upper written address of the last executed instruction.
 * @return	Lower written address.
 */
Address SimState::upperWrite(void) {
		throw UnsupportedFeatureException(process(), MEMORY_ACCESSES);
}


/**
 * Set the current value of the SP.
 */
void SimState::setSP(const Address& addr) {
  ASSERTP(false, "SimState::setSP() unsupported");
}


/**
 * Get register value from the state.
 * @param r	Looked register.
 */
t::uint32 SimState::getReg(hard::Register *r) {
	ASSERTP(false, "SimState::getReg() unsupported");
	return 0;
}


/**
 * Set register value in the state.
 * @param r		Register to change.
 * @param v		Value to set.
 */
void SimState::setReg(hard::Register *r, t::uint32 v) {
	ASSERTP(false, "SimState::setReg() unsupported");
}


/**
 * @class Process
 * A process is the realization of a program on a platform. It represents the
 * program and its implementation on the platform. A process may be formed
 * by many files in case of shared object for example. A process provides the
 * information needed for simulating, analyzing or transforming a program.
 *
 * @ingroup prog
 */


/**
 * Build a new empty process.
 * @param manager	Current manager.
 * @param props		Configuration properties to create this process.
 * @param program	The program file creating this process.
 */
Process::Process(Manager *manager, const PropList& props, File *program)
: prog(0), man(manager), smap(0) {
	addProps(props);
}


/**
 * @fn const elm::Collection<File *> *Process::files(void) const
 * Get the list of files used in this process.
 * @return	List of files.
 */


/**
 * @fn File *Process::createFile(void)
 * Build an empty file.
 * @return	The created file.
 */


/**
 * @fn	File *Process::loadFile(CString path)
 * Load an existing file.
 * @param path	Path to the file to load.
 * @return	The loaded file.
 * @exception LoadException							Error during the load.
 * @exception UnsupportedPlatformException	Platform of the file does
 * not match the platform of the process.
 */


/**
 * @fn Platform *Process::platform(void);
 * Get the platform of the process.
 * @return Process platform.
 */


/**
 * @fn Manager *Process::manager(void);
 * Get the manager owning this process.
 * @return Process manager.
 */


/**
 * @fn address_t Process::start(void) ;
  * Get the address of the first instruction of the program.
  * @return Address of the first instruction of the program or null if it unknown.
  */


/**
 * @fn Inst *Process::findInstAt(address_t addr);
 * Find the instruction at the given address.
 * @param addr	Address of instruction to retrieve.
 * @return		Found instruction or null if it cannot be found.
 */


/**
 * Find the address of the given label. For performing it, it looks iteratively
 * one each file of the process until finding it.
 * @param label		Label to find.
 * @return			Found address or null.
 */
address_t Process::findLabel(const string& label) {
	address_t result = Address::null;
	for(FileIter file(this); file(); file++) {
		result = file->findLabel(label);
		if(!result.isNull())
			break;
	}
	return result;
}


/**
 * find the instruction at the given label if the label matches a code
 * segment.
 * @param label		Label to look for.
 * @return			Matching instruction or null.
 */
Inst *Process::findInstAt(const string& label) {
	address_t addr = findLabel(label);
	if(addr.isNull())
		return nullptr;
	else
		return findInstAt(addr);
}


/**
 * Find the segment at the given address.
 * @param addr
 */
Segment *Process::findSegmentAt(Address addr) const {
	for(auto f: files())
		for(auto s: f->segments())
			if(s->contains(addr))
				return s;
	return nullptr;
}


/**
 * @fn File *Process::program(void) const;
 * Get the program file, that is, the startup executable of the process.
 * @return	Program file.
 */


/**
 * Load the program file
 */
File *Process::loadProgram(elm::CString path) {
	File *file = loadFile(path);
	ASSERT(file);
	if(!prog)
		prog = file;
	return file;
}


/**
 * Get a default address for the stack top.
 * @return	Default address of stack top.
 */
Address Process::defaultStack(void) {
	return 0x80000000;
}


/**
 * Provide semantic sequence to initialize the environment
 * before performing static analysis. As a default, do nothing.
 */
void Process::semInit(sem::Block& block) {
}


/**
 * Find the simulator used for the current process.
 * @return	A simulator for the current process or null if none is found.
 */
sim::Simulator *Process::simulator(void) {
	//cerr << "otawa::Process::simulator()\n";

	// Look just in configuration
	sim::Simulator *sim = SIMULATOR(this);
	if(sim)
		return sim;

	// Look for a name
	string name = SIMULATOR_NAME(this);
	if(!name)
		return 0;
	sim = manager()->findSimulator(name);
	if(!sim)
		throw LoadException(_ << "cannot get the simulator \"" << name << "\".");
	return sim;
}


/**
 * Add the given file to the list of files. The first added file is considered
 * as the program file.
 * @param file	Added file.
 * @note		After this call, the process is the owner of the file
 * 				and will released it at deletion time.
 */
void Process::addFile(File *file) {
	ASSERT(file);
	if(!prog)
		prog = file;
	_files.add(file);
}


/**
 */
Process::~Process(void) {
	for(FileIter file(this); file(); file++)
		delete *file;
	if(smap)
		delete smap;
}


/**
 * Find the instruction at the given address.
 * @param addr	Address to look at.
 * @return		Instruction at the given address or null if it cannot be found.
 */
Inst *Process::findInstAt(address_t addr) {
	for(FileIter file(this); file(); file++) {
		Inst *result = file->findInstAt(addr);
		if(result != nullptr)
			return result;
	}
	return nullptr;
}


/**
 * @fn int Process::instSize(void) const;
 * Get the instruction size.
 * @return	Instruction size or 0 for variable instruction size.
 */


/**
 * Get a decoder usuful to decode instructions. May be overriden to give
 * instruction set architecture dependent decoders.
 * @return	Instruction decoder or null if none is defined.
 */
Processor *Process::decoder(void) {
	return 0;
}

/**
 * Get a fresh startup state of the process for functional simulation.
 * @return	Startup state, possibly null if the process does not support
 * 			functional simulation/
 */
SimState *Process::newState(void) {
	return 0;
}


/**
 * Get the loader that has created this process. The result may be null.
 * @return	Loader that created this process.
 */
Loader *Process::loader(void) const {
	return 0;
}


/**
 * This method let provide feature from the loader / processors.
 */
void Process::provide(AbstractFeature& feature) {
	provided.add(&feature);
}


/**
 * Find the symbol matching the given name.
 * @param name	Symbol name to look for.
 * @return		Found symbol or null.
 */
Symbol *Process::findSymbol(const String& name) {
	Symbol *result = 0;
	for(FileIter file(this); file(); file++) {
		result = file->findSymbol(name);
		if(result)
			break;
	}
	return result;
}


/**
 * Find a symbol by its address.
 * @param address	Address of the looked symbol.
 * @return			Found symbol or null.
 */
Symbol *Process::findSymbolAt(const Address& address) {
	if(!smap) {
		stree::SegmentBuilder<Address::offset_t, Symbol *> builder(0);
		for(Process::FileIter file(this); file(); file++)
			for(File::SymIter sym(*file); sym(); sym++)
				if(sym->size())
					switch(sym->kind()) {
					case Symbol::DATA:
					case Symbol::FUNCTION:
						builder.add(sym->address().offset(), sym->address().offset() + sym->size(), *sym);
						break;
					default:
						break;
					}
		smap = new stree::Tree<Address::offset_t, Symbol *>();
		builder.make(*smap);
	}
	return smap->get(address.offset(), 0);
}


File::syms_t Process::symbols() {
	File::syms_t symbols;
	for(File *file : _files) {
		const File::syms_t s = file->symbols();
		for(auto it=s.begin() ; it != s.end() ; it++) {
			symbols.add(it.key(), it.item());
		}
	}
	return symbols;
}

/**
 * Get a signed byte value from the process.
 * @param at	Address of the value to get.
 * @param val	Got value.
 * @throws UnsupportedFeatureException if this method is called but the loader
 * 		does not provide this feature.
 * @throws OutOfMemStaticException if the given address does not point
 * 		to a static segment of the executable file.
 * @warning	To use this method, you must assert that the
 * 		@ref MEMORY_ACCESS_FEATURE is provided.
 */
void Process::get(Address at, t::int8& val) {
	throw UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
}


/**
 * Get an unsigned byte value from the process.
 * @param at	Address of the value to get.
 * @param val	Got value.
 * @throws UnsupportedFeatureException if this method is called but the loader
 * 		does not provide this feature.
 * @throws OutOfMemStaticException if the given address does not point
 * 		to a static segment of the executable file.
 * @warning	To use this method, you must assert that the
 * 		@ref MEMORY_ACCESS_FEATURE is provided.
 */
void Process::get(Address at, t::uint8& val) {
	throw UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
}


/**
 * Get a signed half-word value from the process.
 * @param at	Address of the value to get.
 * @param val	Got value.
 * @throws UnsupportedFeatureException if this method is called but the loader
 * 		does not provide this feature.
 * @throws OutOfMemStaticException if the given address does not point
 * 		to a static segment of the executable file.
 * @warning	To use this method, you must assert that the
 * 		@ref MEMORY_ACCESS_FEATURE is provided.
 */
void Process::get(Address at, t::int16& val) {
	throw UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
}


/**
 * Get a unsigned half-word value from the process.
 * @param at	Address of the value to get.
 * @param val	Got value.
 * @throws UnsupportedFeatureException if this method is called but the loader
 * 		does not provide this feature.
 * @throws OutOfMemStaticException if the given address does not point
 * 		to a static segment of the executable file.
 * @warning	To use this method, you must assert that the
 * 		@ref MEMORY_ACCESS_FEATURE is provided.
 */
void Process::get(Address at, t::uint16& val) {
	throw UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
}


/**
 * Get a signed word value from the process.
 * @param at	Address of the value to get.
 * @param val	Got value.
 * @throws UnsupportedFeatureException if this method is called but the loader
 * 		does not provide this feature.
 * @throws OutOfMemStaticException if the given address does not point
 * 		to a static segment of the executable file.
 * @warning	To use this method, you must assert that the
 * 		@ref MEMORY_ACCESS_FEATURE is provided.
 */
void Process::get(Address at, t::int32& val) {
	throw UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
}


/**
 * Get a unsigned word value from the process.
 * @param at	Address of the value to get.
 * @param val	Got value.
 * @throws UnsupportedFeatureException if this method is called but the loader
 * 		does not provide this feature.
 * @throws OutOfMemStaticException if the given address does not point
 * 		to a static segment of the executable file.
 * @warning	To use this method, you must assert that the
 * 		@ref MEMORY_ACCESS_FEATURE is provided.
 */
void Process::get(Address at, t::uint32& val) {
	throw UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
}


/**
 * Get a signed long word value from the process.
 * @param at	Address of the value to get.
 * @param val	Got value.
 * @throws UnsupportedFeatureException if this method is called but the loader
 * 		does not provide this feature.
 * @throws OutOfMemStaticException if the given address does not point
 * 		to a static segment of the executable file.
 * @warning	To use this method, you must assert that the
 * 		@ref MEMORY_ACCESS_FEATURE is provided.
 */
void Process::get(Address at, t::int64& val) {
	throw UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
}


/**
 * Get a unsigned long word value from the process.
 * @param at	Address of the value to get.
 * @param val	Got value.
 * @throws UnsupportedFeatureException if this method is called but the loader
 * 		does not provide this feature.
 * @throws OutOfMemStaticException if the given address does not point
 * 		to a static segment of the executable file.
 * @warning	To use this method, you must assert that the
 * 		@ref MEMORY_ACCESS_FEATURE is provided.
 */
void Process::get(Address at, t::uint64& val) {
	throw UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
}


/**
 * Get an address value from the process.
 * @param at	Address of the value to get.
 * @param val	Got value.
 * @throws UnsupportedFeatureException if this method is called but the loader
 * 		does not provide this feature.
 * @throws OutOfMemStaticException if the given address does not point
 * 		to a static segment of the executable file.
 * @warning	To use this method, you must assert that the
 * 		@ref MEMORY_ACCESS_FEATURE is provided.
 */
void Process::get(Address at, Address& val) {
	throw UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
}


/**
 * Get a float value from the process.
 * @param at	Address of the value to get.
 * @param val	Got value.
 * @throws UnsupportedFeatureException if this method is called but the loader
 * 		does not provide this feature.
 * @throws OutOfMemStaticException if the given address does not point
 * 		to a static segment of the executable file.
 * @warning	To use this method, you must assert that the
 * 		@ref MEMORY_ACCESS_FEATURE is provided.
 */
void Process::get(Address at, float& val) {
	throw UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
}


/**
 * Get a double value from the process.
 * @param at	Address of the value to get.
 * @param val	Got value.
 * @throws UnsupportedFeatureException if this method is called but the loader
 * 		does not provide this feature.
 * @throws OutOfMemStaticException if the given address does not point
 * 		to a static segment of the executable file.
 * @warning	To use this method, you must assert that the
 * 		@ref MEMORY_ACCESS_FEATURE is provided.
 */
void Process::get(Address at, double& val) {
	throw UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
}


/**
 * Get a long double value from the process.
 * @param at	Address of the value to get.
 * @param val	Got value.
 * @throws UnsupportedFeatureException if this method is called but the loader
 * 		does not provide this feature.
 * @throws OutOfMemStaticException if the given address does not point
 * 		to a static segment of the executable file.
 * @warning	To use this method, you must assert that the
 * 		@ref MEMORY_ACCESS_FEATURE is provided.
 */
void Process::get(Address at, long double& val) {
	throw UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
}


/**
 * Get a string value from the process.
 * @param at	Address of the value to get.
 * @param str	Got value.
 * @throws UnsupportedFeatureException if this method is called but the loader
 * 		does not provide this feature.
 * @throws OutOfMemStaticException if the given address does not point
 * 		to a static segment of the executable file.
 * @warning	To use this method, you must assert that the
 * 		@ref MEMORY_ACCESS_FEATURE is provided.
 */
void Process::get(Address at, String& str) {
	throw UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
}


/**
 * Get a byte block value from the process.
 * @param at	Address of the value to get.
 * @param buf	Buffer to store block to.
 * @param size	Size of block.
 * @throws UnsupportedFeatureException if this method is called but the loader
 * 		does not provide this feature.
 * @throws OutOfMemStaticException if the given address does not point
 * 		to a static segment of the executable file.
 * @warning	To use this method, you must assert that the
 * 		@ref MEMORY_ACCESS_FEATURE is provided.
 */
void Process::get(Address at, char *buf, int size) {
	throw UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
}


/**
 * Get the initial address of the stack pointer.
 * If the program is embedded, this call does not return any meaningful value as the
 * value is initialized at the program start. For program running in a rich OS environment,
 * this is the value of the stack pointer after the OS has filled environment and program
 * arguments.
 * @return	Initial stack pointer address.
 */
Address Process::initialSP(void) const {
	return Address::null;
}


/**
 */
elm::io::Output& operator<<(elm::io::Output& out, Process *proc) {
	out << "process";
	out << "[progam=\"" << proc->program_name() << "\"]";
	return out;
}


/**
 * Get the source file and the line matching the given address.
 * @param addr	Address to get source and line.
 * @return		Optional (file, line) pair if the address is found.
 * @throw UnsupportedFeatureException	If this function is called and
 * 				the feature @ref SOURCE_LINE_FEATURE is not implemented.
 */
Option<Pair<cstring, int> > Process::getSourceLine(Address addr) {
	throw UnsupportedFeatureException(this, SOURCE_LINE_FEATURE);
}


/**
 * Get the list of addresses implementing the given (source, line) pair.
 * @param file		Looked source file.
 * @param line		Looked source line.
 * @param addresses	Output parameter containing the list of address ranges
 * 					matching the given line. This vector is empty if the
 * 					source (file, line) cannot be found.
 * @throw UnsupportedFeatureException	If this function is called and
 * 				the feature @ref SOURCE_LINE_FEATURE is not implemented.
 */
void Process::getAddresses(cstring file, int line, Vector<Pair<Address, Address> >& addresses) {
	throw UnsupportedFeatureException(this, SOURCE_LINE_FEATURE);
}


/**
 * @class ProcessException
 * An exception generated from a process.
 * @ingroup proc
 */


/**
 * @fn ProcessException::ProcessException(Process *process);
 * Build a a simple exception for the given process.
 * @param process	Current process.
 */


/**
 * @fn ProcessException::ProcessException(Process *process, const string& message);
 * Build a process exception with a message.
 * @param process		Current process.
 * @param message		Exception message.
 */


/**
 * @fn Process *ProcessException::process(void) const;
 * Get the process where the exception was thrown.
 * @return	Exception process.
 */


/**
 */
String ProcessException::message(void) {
	if(process() == nullptr)
		return "";
	else
		return _ << process()->program_name() << ": ";
}


/**
 * @class UnsupportedFeatureException
 * This class is used to throw an exception when a process does not support
 * an invoked feature.
 * @ingroup prog
 */


/**
 */
String UnsupportedFeatureException::message(void) {
	return _ << ProcessException::message()
			<< f.name() << " is not supported";
}


/**
 * @class OutOfSegmentException
 * This exception is thrown when a memory access is performed on process
 * with an address that does not point in a segment of the executable file.
 * @ingroup prog
 */




/**
 */
String OutOfSegmentException::message(void) {
	return _ << "process " << process()
			<< " does not have defined segment at " << addr;
}


/**
 * This feature is usually asserted by processes that provides access to the
 * memory segment of the program.
 * @par Provided Methods
 * @li @ref Process::get() family of methods.
 * @ingroup prog
 */
p::feature MEMORY_ACCESS_FEATURE("otawa::MEMORY_ACCESS_FEATURE", new Maker<NoProcessor>());


/**
 * This feature is usually asserted by processes that provides access to the
 * memory segment of the program with float values.
 * @par Provided Methods
 * @li @ref Process::get(Address, float&),
 * @li @ref Process::get(Address, double&),
 * @li @ref Process::get(Address, long double&),
 * @ingroup prog
 */
p::feature FLOAT_MEMORY_ACCESS_FEATURE("otawa::FLOAT_MEMORY_ACCES_FEATURE", new Maker<NoProcessor>());


/**
 * This feature is usually asserted by processes that provides access to the
 * register usage information.
 * @par Provided Methods
 * @li @ref	Inst::readRegs(void);
 * @li @ref Inst::writtenRegs(void);
 * @ingroup prog
 */
p::feature REGISTER_USAGE_FEATURE("otawa::REGISTER_USAGE_FEATURE", new Maker<NoProcessor>());


/**
 * This feature is usually provided by the loader providing decoding facility
 * for control instructions.
 * @par Provided Methods
 * @li @ref Inst::target()
 * @ingroup prog
 */
p::feature CONTROL_DECODING_FEATURE("otawa::CONTROL_DECODING_FEATURE", new Maker<NoProcessor>());


/**
 * This feature asserts that a table matching program address and source
 * (file, lines) is available.
 * @par Provided Methods
 * @li @ref Process::getSourceLine()
 * @li @ref Process::getAddresses()
 * @ingroup prog
 */
p::feature SOURCE_LINE_FEATURE("otawa::SOURCE_LINE_FEATURE", new Maker<NoProcessor>());



/**
 * This feature is provided on Process objects by architecture implementing VLIW
 * facilities. This means mainly that Inst::IS_BUNDLE and Inst::semWriteBack() are used
 * in instructions.
 * @ingroup prog
 */
p::feature VLIW_SUPPORTED("otawa::VLIW_SUPPORTED", new Maker<NoProcessor>());



/**
 * This feature is provided by @ref Process which architecture provides conditional
 * instructions. A common example are the ARM microprocessors which instructions,
 * at least in normal mode, can be conditional. The architectures providing this
 * feature implement the method Inst::condition() and Inst::updateCondition().
 * @ingroup prog
 */
p::feature CONDITIONAL_INSTRUCTIONS_FEATURE("otawa::CONDITIONAL_INSTRUCTIONS_FEATURE", new Maker<NoProcessor>());

/**
 * This feature is put on the process object whose image supports Unix-like
 * argument passing. It contains the image address of the argv vector.
 * @p Hooks
 * @li @ref Process
 * @ingroup prog
 */
p::id<Address> ARGV_ADDRESS("otawa::ARGV_ADDRESS", Address::null);


/**
 * This feature is put on the process object whose image supports Unix-like
 * argument passing. It contains the image address of the envp vector.
 * @p Hooks
 * @li @ref Process
 * @ingroup prog
 */
p::id<Address> ENVP_ADDRESS("otawa::ENVP_ADDRESS", Address::null);


/**
 * This feature is put on the process object whose image supports Unix-like
 * argument passing. It contains the image address of the auxv vector.
 * @p Hooks
 * @li @ref Process
 * @ingroup prog
 */
p::id<Address> AUXV_ADDRESS("otawa::AUXV_ADDRESS", Address::null);


/**
 * This feature is put on the process to get information about the built image.
 * It contains the startup address of the stack pointer.
 * @p Hooks
 * @li @ref Process
 * @ingroup prog
 */
p::id<Address> SP_ADDRESS("otawa::SP_ADDRESS", Address::null);


/**
 * @class DecodingException
 * This exception is thrown by a loader to inform about problems during decoding.
 * @ingroup prog
 */


/**
 * Build the exception with the given message.
 * @param message	Message about the exception.
 */
DecodingException::DecodingException(const string& message):
	MessageException(message)
{
}


/**
 * This feature, put on a process, informs that the functional simulator
 * provides the ability to give information about memory accesses.
 *
 * @par Provider
 * @li @ref Process
 *
 * @par Activated Methods
 * @li @ref otawa::sim::State::lowerRead()
 * @li @ref otawa::sim::State::upperRead()
 * @li @ref otawa::sim::State::lowerWrite()
 * @li @ref otawa::sim::State::upperWrite()
 * @ingroup prog
 */
p::feature MEMORY_ACCESSES("otawa::MEMORY_ACCESSES", new Maker<NoProcessor>());


/**
 * This feature is present when the loader provides implementation
 * of semantics information.
 *
 * @par Provider
 * @li @ref Process
 *
 * @par Activated Methods
 * @li @ref otawa::Inst::semInsts() method.
 * @ingroup prog
 */
p::feature SEMANTICS_INFO("otawa::SEMANTICS_INFO", new Maker<NoProcessor>());


/**
 * This feature is present when the loader provides implementation
 * of extended semantics information. This feature ensures that
 * the following semantics instruction are generated:
 * @li NEG
 * @li NOT
 * @li AND
 * @li OR
 * @li XOR
 * @li MUL
 * @li MULU
 * @li DIV
 * @li DIVU
 * @li MOD
 * @li MODU
 * @li SPEC
 *
 * @par Provider
 * @li @ref Process
 *
 * @par Activated Methods
 * @li @ref otawa::Inst::semInsts() method.
 *
 * @ingroup prog
 */
p::feature SEMANTICS_INFO_EXTENDED("otawa::SEMANTICS_INFO_EXTENDED", new Maker<NoProcessor>());


/**
 * This feature is present when the loader provides implementation
 * of floating-point semantic instructions.
 *
 * @note This is a future feature as, currently, no floating-point instruction exists.
 *
 * @par Provider
 * @li @ref Process
 *
 * @par Activated Methods
 * @li @ref otawa::Inst::semInsts() method.
 *
 * @ingroup prog
 */
p::feature SEMANTICS_INFO_FLOAT("otawa::SEMANTICS_INFO_FLOAT", new Maker<NoProcessor>());


/**
 * @enum delayed_t
 * Enumeration giving the type of delayed modes used by control instruction.
 * @ingroup prog
 */

/**
 * @var delayed_t::DELAYED_None
 * No delayed instruction.
 */

/**
 * @var delayed_t::DELAYED_Always;
 * The delayed instruction is ever executed, branch taken or not.
 */

/**
 * @var delayed_t::DELAYED_Taken
 * The delayed instruction is only executed when the branch is taken.
 */


/**
 * This kind of property is put on control branch to know if the following
 * instruction is executed as part of a delayed branch.
 * @par Feature
 * @li @ref otawa::DELAYED_FEATURE
 * @par Hook
 * @li @ref otawa::Inst (control instruction)
 * @ingroup prog
 */
p::id<delayed_t> DELAYED("otawa::DELAYED", DELAYED_None);


/**
 */
template <>
void Identifier<delayed_t>::print (elm::io::Output &out, const Property *prop) const {
	switch(get(*prop)) {
	case DELAYED_None:
		out << "none";
		break;
	case DELAYED_Always:
		out << "always";
		break;
	case DELAYED_Taken:
		out << "taken";
		break;
	default:
		out << "unknown";
		break;
	}
}



/* Nop instruction */
class NopInst: public Inst {
public:
	typedef Array<hard::Register * > reg_t;
	NopInst(Address address, t::uint32 size): addr(address), _size(size) { }

	virtual void dump (io::Output &out) { out << "<nop>"; }
	virtual kind_t kind (void) { return IS_INT; }
	virtual Inst *toInst(void) { return this; }
	virtual Address address(void) const { return addr; }
	virtual t::uint32	size(void) const { return _size; }
	virtual const reg_t& readRegs(void) { return reg_t::null; }
	virtual const reg_t& writtenRegs(void) { return reg_t::null; }
private:
	Address addr;
	t::uint32 _size;
};

/**
 * Build a NOP instruction at the given address.
 * @param addr	Address of the instruction (default to null address).
 * @return		Built NOP instruction (must fried by deleteNop() call).
 */
Inst *Process::newNOp(Address addr) {
	t::uint32 size = this->instSize();
	if(size == 0)
		size = 1;
	return new NopInst(addr, size);
}


/**
 * Delete a NOP instruction previously allocated by NewOP().
 * @param inst	Instruction to delete.
 */
void Process::deleteNop(Inst *inst) {
	delete (NopInst *)inst;
}


/**
 * Get the maximum number of temporaries used in the semantic instruction
 * block.
 * @return	Maximum number of temporaries.
 */
int Process::maxTemp(void) const {
	return 16;
}


/**
 * @class DelayedInfo
 * Provide information on delayed branches.
 * @ingroup prog
 */


/**
 */
DelayedInfo::~DelayedInfo(void) {
}


/**
 * @fn delayed_t DelayedInfo::type(Inst *inst);
 * Get the type of the delayed control instruction.
 * @param inst	Branch instruction.
 * @return		Type of the delayed instruction (one of DELAYED_None, DELAYED_Taken or DELAYED_Always.
 */


/**
 * Get the count of instructions before the delayed branch becomes effective.
 * @param inst	Delayed control instruction (as a default, 1).
 * @return		Count of instructions.
 */
int DelayedInfo::count(Inst *inst) {
	return 1;
}


/**
 * This Process feature informs that the actual architecture
 * supports delayed branch.
 *
 * @par Properties
 * @li @ref DELAYED_INFO
 *
 * @ingroup prog
 */
p::feature DELAYED2_FEATURE("otawa::DELAYED2_FEATURE", new Maker<NoProcessor>());


/**
 * This property provides access to delayed branch information
 * of the current program.
 *
 * @par Hooks
 * @li @ref WorkSpace
 *
 * @par Features
 * @li @ref DELAYED2_FEATURE
 *
 * @ingroup prog
 */
Identifier<DelayedInfo *> DELAYED_INFO("otawa::DELAYED_INFO", 0);


/**
 * @class Condition
 * This type of object is returned by Inst::condition() method and is used
 * to manage conditional instructions (which conditional branches are only
 * a particular case). It provides information about the condition that
 * activates the instruction and the register containing the condition.
 *
 * In this representation, a condition is a boolean (true -- unsigned, false -- signed)
 * and a bit set representing the possible conditions (OR'ed composition
 * of @ref EQ, @ref LT or @ref GT).
 *
 * @seealso Inst, Inst::condition()
 * @ingroup prog
 */

/**
 * Build an empty condition.
 */
Condition::Condition(void): _unsigned(false), _cond(0), _reg(0) {
}

/**
 * Build a condition.
 * @param cond	Condition value as used in semantic instructions.
 * @param reg	Register containing the condition.
 */
Condition::Condition(sem::cond_t cond, hard::Register *reg): _reg(reg) {

	// set condition
	static cond_t set[] = {
		0,					// NO_COND	= 0
		EQ,				// EQ		= 1
		LT,				// LT		= 2
		LT | EQ,		// LE		= 3
		GT | EQ,		// GE		= 4
		GT,				// GT		= 5
		0,				//			= 6
		0,				//			= 7
		EQ | LT | GT,	// ANY_COND	= 8
		LT | GT,		// NE		= 9
		LT,				// ULT		= 10
		LT | EQ,		// ULE		= 11
		GT | EQ,		// UGE		= 12
		GT				// UGT		= 13
	};
	ASSERT(cond < sem::MAX_COND);
	_cond = set[cond];
	_unsigned = sem::ULT <= cond && cond <= sem::UGT;
}

/**
 * Build a condition.
 * @param unsigned_		True if the condition is unsigned, false else.
 * @param cond			Set of conditions.
 * @param reg			Register containing the condition.
 */
Condition::Condition(bool unsigned_, cond_t cond, hard::Register *reg)
: _unsigned(unsigned_), _cond(cond), _reg(reg) {
}

/**
 * Get the semantic condition.
 * @return	Semantic condition.
 */
sem::cond_t Condition::semCond(void) const {

	// signed case
	if(!_unsigned) {
		static sem::cond_t back[] = {
			sem::NO_COND,	// none
			sem::EQ,		// EQ
			sem::LT,		// LT
			sem::LE,		// LT | EQ
			sem::GT,		// GT
			sem::GE,		// GT | EQ
			sem::NE,		// GT | LT
			sem::ANY_COND	// GT | LT | EQ
		};
		return back[_cond];
	}

	// unsigned case
	else {
		static sem::cond_t back[] = {
			sem::NO_COND,	// none
			sem::EQ,		// EQ
			sem::ULT,		// LT
			sem::ULE,		// LT | EQ
			sem::UGT,		// GT
			sem::UGE,		// GT | EQ
			sem::NE,		// GT | LT
			sem::ANY_COND	// GT | LT | EQ
		};
		return back[_cond];
	}
}

/**
 * @fn bool Condition::isEmpty(void) const;
 * Test if the condition is empty.
 * @return	True if the condition is empty, false else.
 */

/**
 * @fn bool Condition::isAny(void) const;
 * Test if the condition is any condition.
 * @return	True if the condition is any, false else.
 */

/**
 * @fn hard::Register *Condition::reg(void) const;
 * Get the register containing the condition.
 * @return	Register containing the condition.
 */

/**
 * @fn bool Condition::isUnsigned(void) const;
 * Test if the condition is unsigned.
 * @return	True if the condition is unsigned, false else.
 */

/**
 * @fn bool Condition::isSigned(void) const;
 * Test if the condition is signed.
 * @return	True if the condition is signed, false else.
 */

/**
 * @fn cond_t Condition::cond(void) const;
 * Get the condition as a set of EQ, LT or GT.
 * @return	Condition as a set.
 */

/**
 * Test if the current condition is a subset of the given
 * condition.
 * @param c		Condition to be a superset of the current condition.
 * @return		True if the current condition is a subset of the condition c.
 */
bool Condition::subsetOf(const Condition& c) const {
	if(_reg != c._reg || _unsigned != c._unsigned)
		return false;
	else
		return (_cond | c._cond) == c._cond;
}

/**
 * Considering the current condition, return the complement
 * condition of the condition. The current condition must be
 * a superset of the condition c!
 *
 * For example, if the current condition GE =(Greater or Equal)
 * and c is EQ (Equal), this function returns GT (Greater-Than).
 *
 * @param c	Condition to get complement for.
 * @return	Complement condition.
 */
Condition Condition::complementOf(const Condition& c) const {
	ASSERT(_reg == c._reg);
	ASSERT(_unsigned == c._unsigned);
	return Condition(_unsigned, _cond & ~c._cond, _reg);
}

/**
 * Compute the intersection of the current condition with the given one.
 * Both conditions must match (same register, same domain)!
 * @param c		Condition to meet with.
 * @return		Meet of conditions.
 */
Condition Condition::meet(const Condition& c) const {
	ASSERT(_reg == c._reg);
	ASSERT(_unsigned == c._unsigned);
	return Condition(_unsigned, _cond & c._cond, _reg);
}

/**
 * Compute the union of the current condition with the given one.
 * Both conditions must match (same register, same domain)!
 * @param c		Condition to join with.
 * @return		Meet of conditions.
 */
Condition Condition::join(const Condition& c) const {
	ASSERT(_reg == c._reg);
	ASSERT(_unsigned == c._unsigned);
	return Condition(_unsigned, _cond | c._cond, _reg);
}

/**
 * Test if the current condition is equal to the given one.
 * @param c		Condition to compare with.
 * @return		True if conditions are equal, false else.
 */
bool Condition::equals(const Condition& c) const {
	return _unsigned == c._unsigned && _reg == c._reg && _cond == c._cond;
}

Condition Condition::inverse() const {
	return Condition(_unsigned, ANY & ~_cond, _reg);
}


/**
 */
io::Output& operator<<(io::Output& out, const Condition& c) {
	if(c.isEmpty())
		return out;
	if(c.reg())
		out << c.reg()->name();
	out << ':';
	if(c.isUnsigned())
		out << 'u';
	if(c.cond() & Condition::EQ)
		out << 'E';
	if(c.cond() & Condition::LT)
		out << 'L';
	if(c.cond() & Condition::GT)
		out << 'G';
	return out;
}


} // otawa
