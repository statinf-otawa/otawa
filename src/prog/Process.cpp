/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	otawa/prog/Process.cpp -- implementation for Process class.
 */

#include <otawa/prog/Process.h>

namespace otawa {

/**
 * @class Process
 * A process is the realization of a program on a platform. It represents the
 * program and its implementation on the platform. A process may be formed
 * by many files in case of shared object for example. A process provides the
 * information needed for simulating, analyzing or transforming a program.
 */


/**
 * @fn const Collection<File *> *Process::files(void) const
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

} // otawa
