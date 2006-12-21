/*
 *	$Id$
 *	Copyright (c) 2003-06, IRIT UPS.
 *
 *	gliss/Process.cpp -- gliss::Process class implementation.
 */

#include <otawa/gliss.h>
#include <otawa/hard/CacheConfiguration.h>
#include <gel.h>

// Trace
//#define GLISS_PROCESS_TRACE
#if defined(NDEBUG) || !defined(GLISS_PROCESS_TRACE)
#	define TRACE(str)
#else
#	define TRACE(str) cerr << __FILE__ << ':' << __LINE__ << ": " << str << '\n';
#endif


extern "C" gel_file_t *loader_file(memory_t* memory);

namespace otawa { namespace gliss {

/**
 * @class Process
 * Process implementation for GLISS PowerPC.
 */

/**
 * Process constructor.
 * @param _man	Caller manager.
 */
Process::Process(Manager *_man, PropList& props): otawa::Process(props), man(_man) {
	TRACE(this << ".Process::Process(" << _man << ", " << &props << ')');
	no_sys = NO_SYSTEM(props);
	default_argv[0] = "";
	default_argv[1] = 0;
	default_envp[0] = 0;
	argc = ARGC(props);
	if(argc < 0)
		argc = 1;
	argv = ARGV(props);
	if(!argv)
		argv = default_argv;
	envp = ENVP(props);
	if(!envp)
		envp = default_envp;
	_platform = new Platform(props);
}

/**
 * Load the given file.
 * @return Loaded file.
 * @note GLISS loader can only load one file. Load a new file delete the old one.
 */
::otawa::File *Process::loadFile(CString path) {
	File *file = new otawa::gliss::File(path, argc, argv, envp, no_sys);
	addFile(file);
	gel_file_info_t infos;
	gel_file_infos(loader_file(file->state()->M), &infos);
	start_addr = (address_t)infos.entry;
	GLISS_STATE(this) = file->state();
	return file;
}


/**
 * Get the GLISS platform.
 * @return GLISS platform.
 */
hard::Platform *Process::platform(void) {
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
	delete _platform;
}


// otawa::Process overload
otawa::Inst *Process::start(void) {
	if(!start_addr)
		return 0;
	else  if(!program())
		return 0;
	else {
		File *file = (File *)program();
		return file->findByAddress(start_addr);
	}
}


// otawa::Process overload
otawa::Inst *Process::findInstAt(address_t addr) {
	if(!program())
		return 0;
	else {
		File *file = (File *)program();
		return file->findByAddress(addr);
	}
}


} } // otawa::gliss
