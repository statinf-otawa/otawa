/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS.
 *
 *	Registry class interface.
 */
#ifndef OTAWA_PROC_PROC_REGISTER_H
#define OTAWA_PROC_PROC_REGISTER_H

#include <elm/genstruct/HashTable.h>
#include <elm/util/Initializer.h>
#include <otawa/proc/Processor.h>

namespace otawa {

using namespace elm;

// Pre-declaration
class Registration;
class Configuration;

// Registry class
class Registry: public Initializer<Registration> {
	friend class Registration;
	typedef genstruct::HashTable<String, const Registration *> htab_t; 

public:
	static const Registration *find(CString name);

	// Iter class
	class Iter: public htab_t::ItemIterator {
	public:
		inline Iter(void): htab_t::ItemIterator(_.procs) { }
	};

private:
	htab_t procs;
	static Registry _;
	Registry(void);
};


// Registration class
class Registration {
	friend class ProcessorConfig;

public:
	Registration(Processor& processor, CString help = "");
	inline String name(void) const { return _processor.name(); }
	inline Version version(void) const { return _processor.version(); }
	inline Processor *processor(void) const { return &_processor; }
	inline CString help(void) const { return _help; }

	// ConfigIter class
	class ConfigIter: public genstruct::Vector<Configuration *>::Iterator {
	public:
		inline ConfigIter(const Registration& reg):
			genstruct::Vector<Configuration *>::Iterator(reg._processor.configs)
			{ }
		inline ConfigIter(const Registration *reg):
			genstruct::Vector<Configuration *>::Iterator(reg->_processor.configs)
			{ }
	};

	// Private use only
	void initialize(void);

private:
	Processor& _processor;
	CString _help;
};


// Configuration class
class Configuration {
public:
	inline Configuration(AbstractIdentifier& id, CString help = "")
		: _id(id), _help(help) { }
	inline AbstractIdentifier& id(void) const { return _id; }
	inline CString help(void) const { return _help; }
		
private:
	AbstractIdentifier &_id;
	CString _help;
};

} // otawa

#endif	// OTAWA_PROC_PROC_REGISTER_H 
