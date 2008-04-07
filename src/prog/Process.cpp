/*
 *	$Id$
 *	Process class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-7, IRIT UPS.
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

#include <elm/xom.h>
#include <otawa/prog/Process.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Platform.h>
#include <otawa/prog/Loader.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/prog/Manager.h>
#include <otawa/gensim/GenericSimulator.h>
#include <otawa/prog/FixedTextDecoder.h>
#include <elm/genstruct/DAGNode.h>
#include <otawa/proc/Feature.h>

using namespace elm;

namespace otawa {

/**
 * @class class SimState
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
 * @fn Process *process(void) const;
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
 * @class Process
 * A process is the realization of a program on a platform. It represents the
 * program and its implementation on the platform. A process may be formed
 * by many files in case of shared object for example. A process provides the
 * information needed for simulating, analyzing or transforming a program.
 */


/**
 * Build a new empty process.
 * @param manager	Current manager.
 * @param props		Configuration properties to create this process.
 * @param program	The program file creating this process.
 */
Process::Process(Manager *manager, const PropList& props, File *program)
: prog(0), man(manager) {
	addProps(props);
	if(prog)
		addFile(prog);
}


/**
 * Get the cache hierarchy of the current processor. The cachers are ordered
 * according the index in the vector.
 * @return	Cache hierarchy.
 */
const hard::CacheConfiguration& Process::cache(void) {
	return platform()->cache();
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
	address_t result = 0;
	for(FileIter file(this); file; file++) {
		//cerr << "Looking at " << file->name() << io::endl;
		result = file->findLabel(label);
		if(result)
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
	if(!addr)
		return 0;
	else
		return findInstAt(addr);
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
	assert(!prog);
	File *file = loadFile(path);
	if(file)
		prog = file;
	return file;
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
	CString name = SIMULATOR_NAME(this);
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
	assert(file);
	if(!files)
		prog = file;
	files.add(file);
}


/**
 */
Process::~Process(void) {
	for(FileIter file(this); file; file++)
		delete file;
}


/**
 * Find the instruction at the given address.
 * @param addr	Address to look at.
 * @return		Instruction at the given address or null if it cannot be found.
 */
Inst *Process::findInstAt(address_t addr) {
	for(FileIter file(this); file; file++) {
		Inst *result = file->findByAddress(addr);
		if(result)
			return result;
	}
	return 0;
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
 * This method is called each a workspace using the current process is
 * created. It may be used to perform some workspace initialization like
 * feature providing.
 */
void Process::link(WorkSpace *ws) {
	ASSERT(ws);
	for(int i = 0; i < provided.length(); i++)
		ws->provide(*provided[i]);
}


/**
 * This method is called each a workspace using the current process is
 * deleted.
 */
void Process::unlink(WorkSpace *ws) {
	ASSERT(ws);
}


/**
 * This method let provide feature from the loader / processors.
 */
void Process::provide(AbstractFeature& feature) {
	provided.add(&feature);
}


/**
 * This feature ensure that stack information are available, that is, that
 * Inst::stackSet() and Inst::stackUse() methods returns meaningful results.
 * This feature is usually provided by the program loader.
 */
Feature<NoProcessor> STACK_USAGE_FEATURE("otawa::stack_usage");


/**
 * Find the symbol matching the given name.
 * @param name	Symbol name to look for.
 * @return		Found symbol or null.
 */
Symbol *Process::findSymbol(const String& name) {
	Symbol *result = 0;
	for(FileIter file(this); file; file++) {
		result = file->findSymbol(name);
		if(result)
			break;
	}
	return result;
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
void Process::get(Address at, signed char& val) {
	throw new UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
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
void Process::get(Address at, unsigned char& val) {
	throw new UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
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
void Process::get(Address at, signed short& val) {
	throw new UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
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
void Process::get(Address at, unsigned short& val) {
	throw new UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
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
void Process::get(Address at, signed long& val) {
	throw new UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
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
void Process::get(Address at, unsigned long& val) {
	throw new UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
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
void Process::get(Address at, signed long long& val) {
	throw new UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
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
void Process::get(Address at, unsigned long long& val) {
	throw new UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
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
	throw new UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
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
	throw new UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
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
	throw new UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
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
	throw new UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
}


/**
 * Get a string value from the process.
 * @param at	Address of the value to get.
 * @param val	Got value.
 * @throws UnsupportedFeatureException if this method is called but the loader
 * 		does not provide this feature.
 * @throws OutOfMemStaticException if the given address does not point
 * 		to a static segment of the executable file. 
 * @warning	To use this method, you must assert that the
 * 		@ref MEMORY_ACCESS_FEATURE is provided.
 */
void Process::get(Address at, String& val) {
	throw new UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
}


/**
 * Get a byte block value from the process.
 * @param at	Address of the value to get.
 * @param val	Got value.
 * @throws UnsupportedFeatureException if this method is called but the loader
 * 		does not provide this feature.
 * @throws OutOfMemStaticException if the given address does not point
 * 		to a static segment of the executable file. 
 * @warning	To use this method, you must assert that the
 * 		@ref MEMORY_ACCESS_FEATURE is provided.
 */
void Process::get(Address at, char *buf, int size) {
	throw new UnsupportedFeatureException(this, MEMORY_ACCESS_FEATURE);
}


/**
 */
elm::io::Output& operator<<(elm::io::Output& out, Process *proc) {
	out << "process(" << (void *)proc << ")";
	File *file = proc->program();
	if(file)
		out << "[progam=\"" << file->name() << "\"]";
	return out;
}


/**
 * @class ProcessException
 * An exception generated from a process.
 */


/**
 * @class UnsupportedFeatureException
 * This class is used to throw an exception when a process does not support
 * an invoked feature.
 */


/**
 */
String UnsupportedFeatureException::message(void) {
	return _ << "process " << process()
			<< " does not support \"" << f.name() << "\"";
}


/**
 * @class OutOfSegmentException
 * This exception is thrown when a memory access is performed on process
 * with an address that does not point in a segment of the executable file. 
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
 */
Feature<NoProcessor> MEMORY_ACCESS_FEATURE("memory_access_feature");


/**
 * This feature is usually asserted by processes that provides access to the
 * memory segment of the program with float values.
 * @par Provided Methods
 * @li @ref Process::get(Address, float&), 
 * @li @ref Process::get(Address, double&), 
 * @li @ref Process::get(Address, long double&), 
 */
Feature<NoProcessor> FLOAT_MEMORY_ACCESS_FEATURE("float_memory_access_feature");


/**
 * This feature is usually asserted by processes that provides access to the
 * register usage information.
 * @par Provided Methods
 * @li @ref	Inst::readRegs(void);
 * @li @ref writtenRegs(void);
 */
Feature<NoProcessor> REGISTER_USAGE_FEATURE("register_usage_feature");


/**
 * This feature is put on the process object whose image supports Unix-like
 * argument passing. It contains the image address of the argv vector.
 * @p Hooks
 * @li @ref Process
 */
Identifier<Address> ARGV_ADDRESS("otawa::argv_address", Address::null);


/**
 * This feature is put on the process object whose image supports Unix-like
 * argument passing. It contains the image address of the envp vector.
 * @p Hooks
 * @li @ref Process
 */
Identifier<Address> ENVP_ADDRESS("otawa::envp_address", Address::null);


/**
 * This feature is put on the process object whose image supports Unix-like
 * argument passing. It contains the image address of the auxv vector.
 * @p Hooks
 * @li @ref Process
 */
Identifier<Address> AUXV_ADDRESS("otawa::auxv_address", Address::null);


/**
 * This feature is put on the process to get information about the built image.
 * It contains the startup address of the stack pointer.
 * @p Hooks
 * @li @ref Process
 */
Identifier<Address> SP_ADDRESS("otawa::sp_address", Address::null);

} // otawa
