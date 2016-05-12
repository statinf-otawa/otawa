#ifndef __OTAWA_OSLICE_SLICER_H__
#define __OTAWA_OSLICE_SLICER_H__

#include <elm/util/Option.h>
#include <otawa/proc/Processor.h>
#include <otawa/proc/Feature.h>
#include <otawa/cfg/features.h>

namespace otawa { namespace oslice {

class DummySlicer: public otawa::Processor {
public:
	static p::declare reg;
	DummySlicer(AbstractRegistration& _reg = reg);

protected:
	virtual void configure(const PropList &props);
	virtual void processWorkSpace(WorkSpace *fw);

};

} } // otawa::oslice

#endif
