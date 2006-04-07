/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	gliss/Process.cpp -- gliss::Process class implementation.
 */

#include <otawa/gliss.h>
#include <otawa/hardware/CacheConfiguration.h>

// Trace
#ifndef NDEBUG
//#define GLISS_PROCESS_TRACE
#ifdef GLISS_PROCESS_TRACE
#	define TRACE(str) cerr << __FILE__ << ':' << __LINE__ << ": " << str << '\n';
#else
#	define TRACE(str)
#endif
#endif

// Elf Header information
extern Elf32_Ehdr Ehdr;

namespace otawa { namespace gliss {

/**
 * @class Process
 * Process implementation for GLISS PowerPC.
 */

/**
 * Process constructor.
 * @param _man	Caller manager.
 */
Process::Process(Manager *_man, PropList& props): man(_man) {
	TRACE(this << ".Process::Process(" << _man << ", " << &props << ')');
	static char *default_argv[] = { "", 0 };
	static char *default_envp[] = { 0 };
	argc = props.get<int>(Loader::ID_Argc, 1);
	argv = props.get<char **>(Loader::ID_Argv, default_argv);
	envp = props.get<char **>(Loader::ID_Envp, default_envp);
	_platform = new Platform(props);
}

/**
 * Clear the loaded file.
 */
void Process::clear(void) {
	for(int i = 0; i < _files.count(); i++)
		delete _files[i];
	_files.clear();
}

/**
 * Get the list of loaded files.
 * @return	Loader files.
 */
elm::Collection<otawa::File *> *Process::files(void) {
	return &_files;
}


/**
 * Not implemented. Ever fails.
 * @return Created file or null.
 */
::otawa::File *Process::createFile(void) {
	return 0;
}


/**
 * Load the given file.
 * @return Loaded file.
 * @note GLISS loader can only load one file. Load a new file delete the old one.
 */
::otawa::File *Process::loadFile(CString path) {
	clear();
	File *file = new otawa::gliss::File(path, argc, argv, envp);
	if(!file->isOK()) {
		delete file;
		return 0;
	}
	else {
		_files.add(file);
		start_addr = (address_t)Ehdr.e_entry;
		return file;
	}
}


/**
 * Get the GLISS platform.
 * @return GLISS platform.
 */
::otawa::Platform *Process::platform(void) {
	return _platform;
};


/**
 * Get the current manager.
 * @return Manager.
 */
Manager *Process::manager(void) {
	return man;
}


/**
 * Free all files.
 */
Process::~Process(void) {
	TRACE(this << ".Process::~Process()");
	for(Iterator<otawa::File *> file(_files); file; file++)
		delete (File *)*file;
	delete _platform;
}


// otawa::Process overload
otawa::Inst *Process::start(void) {
	if(!start_addr)
		return 0;
	else  if(!_files.count())
		return 0;
	else {
		File *file = (File *)_files[0];
		return file->findByAddress(start_addr);
	}
}


// otawa::Process overload
otawa::Inst *Process::findInstAt(address_t addr) {
	if(!_files.count())
		return 0;
	else {
		File *file = (File *)_files[0];
		return file->findByAddress(addr);
	}
}


} } // otawa::gliss
