/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	otawa/prog/FrameWork.h -- interface for FrameWork class.
 */
#ifndef OTAWA_PROG_FRAMEWORK_H
#define OTAWA_PROG_FRAMEWORK_H

#include <elm/datastruct/Collection.h>
#include <otawa/prog/Process.h>

namespace otawa {

// Classes
class CFG;
class CFGInfo;
class File;
class Inst;
class Manager;
class Platform;

// FrameWork class
class FrameWork: public Process, public PropList {
	Process *proc;
public:
	FrameWork(Process *_proc);
	~FrameWork(void);
	inline Process *getProcess(void) const { return proc; };
	
	// Process overload
	virtual const Collection<File *> *files(void) const { return proc->files(); };
	virtual File *createFile(void) { return proc->createFile(); };
	virtual File *loadFile(elm::CString path) { return proc->loadFile(path); };
	virtual Platform *platform(void) { return proc->platform(); };
	virtual Manager *manager(void) { return proc->manager(); };
	virtual Inst *start(void) { return proc->start(); };
	
	// CFG Management
	void buildCFG(void);
	CFGInfo *getCFGInfo(void);
	CFG *getStartCFG(void);
};

};	// otawa

#endif	// OTAWA_PROG_FRAMEWORK_H
