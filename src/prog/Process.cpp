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

} // otawa
