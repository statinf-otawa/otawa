#include <otawa/oslice_reg/DummySlicer.h>


namespace otawa { namespace oslice_reg {

p::feature DUMMY_SLICER_FEATURE("otawa::oslice_reg::DummySlicerFeature", new Maker<DummySlicer>());

p::declare DummySlicer::reg = p::init("otawa::oslice_reg::DummySlicer", Version(1, 0, 0))
	.maker<DummySlicer>()
	.require(COLLECTED_CFG_FEATURE)
	.provide(DUMMY_SLICER_FEATURE);

/**
 */
DummySlicer::DummySlicer(AbstractRegistration& _reg) : otawa::Processor(_reg) {
}

/**
 */
void DummySlicer::configure(const PropList &props) {
	Processor::configure(props);
}


void DummySlicer::processWorkSpace(WorkSpace *fw) {

} // end of function Slicer::work

} }
