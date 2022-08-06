/*
 *	TaskInfoSerice class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2022, IRIT UPS.
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

#include <elm/sys/System.h>
#include <otawa/prog/features.h>
#include <otawa/prog/File.h>
#include <otawa/prog/Process.h>
#include <otawa/prog/Symbol.h>
#include <otawa/prog/WorkSpace.h>

namespace otawa {

/**
 * @class TaskInfo
 * Interface providing information about the current task.
 * @ingroup prog
 */

///
TaskInfo::~TaskInfo() {}

/**
 * @fn Path TaskInfo::workDirectory();
 * Get the path of a working directory for this task, i.e. a place to save
 * data, statistics, etc. If it not already created, it is created at this
 * call. Usual name path is EXEDIR/EXE-otawa/TASK where EXEDIR is the directory
 * containing the executable, EXE is the executable name without extension and
 * TASK is the task name.
 * @return	Path to working directory.
 */

/**
 * @fn Address TaskInfo::entryAddress();
 * Get the task entry address.
 * @return Task entry address.
 */

/**
 * @fn string TaskInfo::entryName();
 * Get the task entry name.
 * @return Task entry name.
 */
	

///
class TaskInfoService: public Processor, public TaskInfo {
public:
	static p::declare reg;
	TaskInfoService(p::declare& r = reg) {}

	void *interfaceFor(const AbstractFeature &feature) override {
		if(&feature == &TASK_INFO_FEATURE)
			return this;
		else
			return nullptr;
	}
	
	Path workDirectory() override {

		// if needed, compute the working directory name
		if(work_dir.isEmpty()) {
			task_name = LABEL_FEATURE.get(workspace())->labelFor(entry_inst);
			Path p = workspace()->process()->program()->name();
			auto pn = p.withoutExt().namePart();
			auto pd = p.dirPart();
			work_dir = pd / (pn + "-otawa") / task_name;
		}
		
		// if needed, make the directory
		if(work_dir.exists()) {
			if(!work_dir.isDir())
				throw otawa::Exception(_ << "file " << work_dir << " and is not a directory!");
		}
		else 
			try {
				sys::System::makeDirs(work_dir);
			}
			catch(sys::SystemException& e)
				{ throw otawa::Exception(_ << "cannot create " << work_dir << ": " << e.message()); }

		// return it at end
		return work_dir;
	}
	
	Inst *entryInst() override {
		return entry_inst;
	}
	
	string entryName() override {
		return entry_name;
	}

protected:
	
	void configure(const PropList& props) override {
		Processor::configure(props);
		entry_addr = TASK_ADDRESS(props);
		entry_name = TASK_ENTRY(props);
	}
	
	void processWorkSpace(WorkSpace *ws) override {

		// address case
		if(!entry_addr.isNull()) {
			entry_inst = ws->process()->findInstAt(entry_addr);
			if(entry_inst == nullptr)
				throw ProcessorException(*this,
					_ << "entry instruction at " << entry_addr << "cannot be found!");
			entry_name = LABEL_FEATURE.get(workspace())->labelFor(entry_inst);
		}
		
		// name case
		else {
			entry_inst = workspace()->process()->findInstAt(entry_name);
			if(entry_inst == nullptr)
				throw ProcessorException(*this,
					_ << "entry instruction at " << entry_name << "cannot be found!");
			entry_addr = entry_inst->address();
		}
	}
	
private:
	Address entry_addr;
	string entry_name;
	Inst *entry_inst = nullptr;
	string task_name;
	Path work_dir;
};	


///
p::declare TaskInfoService::reg =
	p::init("otawa::TaskInfoSerice", Version(1, 0, 0))
	.require(LABEL_FEATURE)
	.provide(TASK_INFO_FEATURE)
	.make<TaskInfoService>();

	
/**
 * Feature providing several information about the current task including
 * task entry address, task entry name, working directory, etc.
 * 
 * Configuration properties:
 * * @ref TASK_ENTRY
 * * @ref TASK_ADDRESS
 * 
 * @ingroup prog
 */
p::interfaced_feature<TaskInfo> TASK_INFO_FEATURE(
	"otawa::TASK_INFO_FEATURE",
	p::make<TaskInfoService>());
	
}	// otawa


