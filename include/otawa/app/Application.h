/*
 *	Application class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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
#ifndef OTAWA_APP_APPLICATION_H_
#define OTAWA_APP_APPLICATION_H_

#include <elm/data/Vector.h>
#include <elm/options.h>
#include <elm/option/ListOption.h>
#include <otawa/proc/Processor.h>

namespace otawa {

using namespace elm;


// LogOption class
class LogOption: public option::AbstractValueOption {
public:
	LogOption(option::Manager *m);
	LogOption(option::Manager& m);
	inline operator Processor::log_level_t(void) const { return log_level; }
	inline Processor::log_level_t operator*(void) const { return log_level; }
	virtual void process(String arg);
private:
	Processor::log_level_t log_level;
};


// Application class
class Application: public option::Manager, public otawa::Monitor {
public:
	Application(const Make& make);
	Application(
		cstring _program,
		Version _version = Version::ZERO,
		cstring _description = "",
		cstring _author = "",
		cstring _copyright = ""
	);
	virtual ~Application(void);

protected:
	virtual void prepare(PropList& props);
	virtual void work(PropList& props);
	virtual void work(const string& entry, PropList &props);
	virtual void complete(PropList& props);

	inline WorkSpace *workspace(void) const { return ws; }
	void require(const AbstractFeature&  feature);
	void exit(int code = 0);
	void run(Processor *p);
	template <class T> T *perform()
		{ T *p = new T(); run(p); return p; }
	void fail(int code, string msg);
	void error(string msg);
	void warn(string msg);
	void info(string msg);
	void stats();
	void startTask(const string& entry);
	void completeTask();

	const Vector<string>& arguments(void) const { return _args; }
	Address parseAddress(const string& s);

	void process(string arg) override;
	void run() override;

	option::SwitchOption /*help,*/ verbose, dump;
	option::ListOption<string> sets;
	option::ListOption<string> params;
	option::ListOption<string> ff;
	option::Value<string> work_dir;
	option::Value<string> dump_to;
	option::SwitchOption record_stats;
	option::ListOption<string> log_for;
	option::ListOption<string> dump_for;
	option::SwitchOption view;

private:
	LogOption log_level;
	elm::sys::Path path;
	Vector<string> _args;
	PropList props;
	PropList *props2;
	WorkSpace *ws;
};

}	// otawa

// application macro
#define OTAWA_RUN(name) ELM_RUN(name)

#endif /* OTAWA_APP_APPLICATION_H_ */
