/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	prog/Process.cpp -- implementation for Process class.
 */

#include <elm/xom.h>
#include <otawa/prog/Process.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Platform.h>
#include <otawa/prog/Loader.h>
#include <otawa/prog/FrameWork.h>
#include <otawa/prog/Manager.h>
#include <otawa/gensim/GenericSimulator.h>

using namespace elm;

namespace otawa {

/**
 * @class Process
 * A process is the realization of a program on a platform. It represents the
 * program and its implementation on the platform. A process may be formed
 * by many files in case of shared object for example. A process provides the
 * information needed for simulating, analyzing or transforming a program.
 */


/**
 * Build a new empty process.
 * @param props		Configuration properties to create this process.
 * @param program	The program file creating this process.
 */
Process::Process(const PropList& props, File *program): prog(program) {
	addProps(props);
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
address_t Process::findLabel(String& label) {
	address_t result = 0;
	for(Iterator<File *> file(files()->visit()); file; file++) {
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
Inst *Process::findInstAt(String& label) {
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
		throw LoadException("cannot get the simulator \"%s\".", &name);
	return sim;
}


/**
 * Load the given configuration in the process.
 * @param path				Path to the XML configuration file.
 * @throw LoadException		If the file cannot be found or if it does not match
 * the OTAWA XML type.
 */
void Process::loadConfig(const elm::system::Path& path) {
	xom::Builder builder;
	xom::Document *doc = builder.build(&path);
	if(!doc)
		throw LoadException("cannot load \"%s\".", &path);
	xom::Element *conf = doc->getRootElement();
	if(conf->getLocalName() != "otawa"
	|| conf->getNamespaceURI() != "")
		throw LoadException("bad file type in \"%s\".", &path);
	CONFIG_ELEMENT(this) = conf;
}


/**
 * Get the current configuration, if any, as an XML XOM element.
 * @return	Configuration XML element or null.
 */
xom::Element *Process::config(void) {
	xom::Element *conf = CONFIG_ELEMENT(this);
	if(!conf) {
		elm::system::Path path = CONFIG_PATH(this);
		if(path) {
			loadConfig(path);
			conf = CONFIG_ELEMENT(this);
		}
	}
	return conf;
}

} // otawa
