/*
 *	$Id$
 *	ParamFeature and ActualFeature classes interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2010, IRIT UPS <casse@irit.fr>
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

#include <otawa/proc/ParamFeature.h>
#include <otawa/proc/ParamProcessor.h>

namespace otawa {

/**
 * @defgroup parametric Parametric Features and Processor
 *
 * Usual features and associated processors are defined for computing exactly one feature.
 * Yet, several features in the WCET computation may have the same behaviour but with different configurations.
 * For example, in several microprocessors, we may have several levels of cache (L1, L2, ...) exhibiting the same
 * behaviour but with different configurations.
 *
 * @section parametric_declare Declaring and requiring
 *
 * Parametric and processors allows to use the same feature/processors system for different facilities in a WCET
 * computation. Basically, a parametric feature is a feature generator. The feature are identified by specific parameters
 * passed to the parametric feature. For the example of a cache, the parameter may be the description of the cache.
 * Once the parameter is passed, the actual feature may be generated by the parametric feature. The code belows
 * continue with the example of the cache and show the declaration of the parametric feature in a header file.
 * @code
 * #include <otawa/hard/Cache.h>
 * #include <otawa/proc/ParamFeature.h>
 *
 * extern Identifier<hard::Cache *> CACHE;
 * extern ParamFeature CACHE_SUPPORT;
 *
 * @endcode
 *
 * A processor that has to require the feature with a given cache has to obtain the actual feature from the paramtric feature
 * and to require it:
 * @code
 * UserProcessor::UserProcessor(hard::Cache *cache) {
 *	PropList params;
 *	CACHE(params) = cache;
 *	ActualFeature *cache_feature = CACHE_SUPPORT.get(params);
 *	require(cache_feature);
 * }
 * @endcode
 *
 * @section parametric_using Using the parametric feature
 *
 * Getting an actual feature is not enough to use it. Indeed, each actual feature has its own set of identifier
 * to use to hook annotation on the program representation. When a parametric feature is instantiated into an actual
 * feature, it also instantiates a set of parametric identifiers. The instantiated identifiers are then hooked to
 * be used by the requirer processor.
 *
 * The code below show how to declare parametric identifiers:
 * @code
 * #include <otawa/hard/Cache.h>
 * #include <otawa/proc/ParamFeature.h>
 *
 * extern Identifier<hard::Cache *> CACHE;
 * extern ParamFeature CACHE_SUPPORT;
 * extern ParamIdentifier<category_t> CATEGORY;
 * @endcode
 *
 *  The requirer processor then may use the instantiated identifier to retrieve annotation
 *  put on the program representation:
 *  @code
 *  class UserProcessor: public BBprocessor {
 *  public:
 * 		UserProcessor::UserProcessor(hard::Cache *cache) {
 *			PropList params;
 *			CACHE(params) = cache;
 *			ActualFeature *cache_feature = CACHE_SUPPORT.get(params);
 *			require(cache_feature);
 *			CAT = &CATEGORY.instantiate(cache_feature);
 *		}
 *	protected:
 *		void processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {
 *			category_t *cat = (*CAT)[bb];
 *			...
 *		}
 *	private:
 *		Identifier<category_t> *CAT;
 *  };
 *  @endcode
 *
 * @section parametric_implementing Implementing the parametric processor
 *
 * A parametric processor is processor implementing a parametric feature.
 * Usually, a default parametric processor is passed as parameter to a default
 * feature and used to generate a processor customized with an actual feature.
 *
 * From the example below, we get below the source file of the parametric feature
 * and its parametric processor:
 * @code
 *	class CacheProcessor {
 *	};
 *
 *	ParamProcessor CACHE_PROCESSOR("CACHE_PROCESSOR", Version(1, 0, 0), new ParamProcessor::Maker<CacheProcessor>());
 *
 *	Identifier<hard::Cache *> CACHE("CACHE");
 *	ParamFeature CACHE_SUPPORT("CACHE_SUPPORT", CACHE_PROCESSOR);
 *	ParamIdentifier<category_t> CATEGORY(CACHE_SUPPORT, "CATEGORY");
 * @endcode
 *
 * In this example, the default processor implementation is given by the CacheProcessor class. This cache must have
 * a constructor taking as parameter the name, the version and the actual feature. This actual feature is used by
 * the processor to get instantiation parameters and instantiated parametric identified associated with the feature.
 * The code below continues the example:
 * @code
 * class CacheProcessor: public BBProcessor {
 * public:
 *	CacheProcessor(cstring name, Version version, AcutalFeature *feature)
 *		: BBProcessor(name, version), cache(CACHE(feature)), CAT(CATEGORY(feature))
 *		{ }
 *
 *	protected:
 *		void processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {
 *			CAT(bb) = computeCategory(bb);
 *		}
 *
 *	private:
 *		hard::Cache *cache;
 *		Identifier<category_t>& CAT;
 * };
 * @endcode
 *
 * In fact, the actual features contains two sets of annotations:
 * @li parameters of the parametric feature instantiation (identifier passed to instantiations) (CACHE in our example)
 * @li identifier from the parametric identifier instantiation (hooked with parametric identifiers) (CAT in our example)
 */


/**
 * @class ActualFeature
 * An actual feature is the result of the instantiation of a parametric feature.
 * Although this is a feature, it is also a property list containing:
 * @li configuration properties for the parametric feature
 * @li actual identifier matching parametric identifier (identified by the parametric identifier).
 * @ingroup parametric
 */


/**
 * Build the actual feature.
 * @param feature	Instantiated parametric feature.
 * @param def		Default parametric processor.
 */
ActualFeature::ActualFeature(const ParamFeature& feature, const AbstractParamProcessor& def)
	: feat(feature), proc(def), cnt(1) { }


/**
 */
ActualFeature::~ActualFeature(void) {
	for(PropList::Iter prop(this); prop; prop++)
		if(prop->id()->hasProp(AbstractParamIdentifier::ACTUAL_ID))
			delete ((GenericProperty<AbstractIdentifier *> *)*prop)->value();
}


/**
 */
void ActualFeature::process(WorkSpace *ws, const PropList &props) const {
	const AbstractParamProcessor *p = feat.get(props, 0);
	if(!p)
		p = &proc;
	return p->instantiate(*this)->process(ws, props);
}


/**
 * @fn const ParamFeature& ActualFeature::feature(void) const;
 * Get the instantiated parametric feature.
 * @return	Instantiated parametric feature.
 */


/**
 * @fn const AbstractParamProcessor& ActualFeature::processor(void);
 * Get the default parametric processor.
 * @return	Default parametric processor.
 */


/**
 * Lock the actual feature (prevent deletion).
 */
void ActualFeature::lock(void) {
	cnt++;
}


/**
 * Unlock the current feature, possibly deleting it.
 */
void ActualFeature::unlock(void) {
	cnt--;
	if(cnt == 0)
		delete this;
}


/**
 * @class ParamFeature
 * A parametric feature is a feature that may be instantiated according to
 * some parameters passed in a property list.
 *
 * @param id	Param feature identifier.
 * @param def	Default processor.
 * @author H. Cassé <casse@irit.fr>
 * @ingroup parametric
 */

ParamFeature::ParamFeature(cstring id, AbstractParamProcessor& def)
	: Identifier<AbstractParamProcessor *>(&id), proc(def) { }


/**
 * Test if the given feature matches the given properties.
 * @param feat	Feature to test.
 * @param props	Property list to test.
 * @return		True if both matches, false else.
 */
bool ParamFeature::matches(ActualFeature *feat, const PropList& props) {
	for(PropList::Iter prop(props); prop; prop++) {
		Property *propf = feat->getProp(prop->id());
		if(!propf)
			return false;
		if(!prop->id()->equals(prop, propf))
			return false;
	}
	return true;
}

/**
 * Instantiate the feature.
 * @param props		Properties to instantiate the feature.
 * @return			Instantiated feature.
 */
ActualFeature *ParamFeature::instantiate(const PropList& props) {

	// look for an existing one
	for(int i = 0; i < feats.count(); i++)
		if(matches(feats[i], props))
			return feats[i];

	// create it
	ActualFeature *feat = new ActualFeature(*this, proc);
	feat->addProps(props);
	feats.add(feat);

	// instantiate the identifier
	for(Identifier<AbstractParamIdentifier *>::Getter id(this, PARAM_ID); id; id++)
		id->add(feat, id->instantiate(*feat));

	return feat;
}

/**
 * @class AbstractParamIdentifier
 * A parametric identifier allows to instantiate identifier depending on a parametric feature.
 * This class is the abstract form.
 * @ingroup parametric
 */

/**
 * This identifier marks all parametric identifier.
 */
AbstractIdentifier AbstractParamIdentifier::ACTUAL_ID("otawa::AbstractIdentifier::ACTUAL_ID");


/**
 * Build an abstract parametric identifier.
 * @param feature	Owner parametric feature.
 * @param name		Name of the parametric identifier.
 */
AbstractParamIdentifier::AbstractParamIdentifier(ParamFeature& feature, cstring name)
: Identifier<AbstractIdentifier *>(name) {
	ParamFeature::PARAM_ID(feature) = this;
}


/**
 * @fn AbstractIdentifier *AbstractParamIdentifier::instantiate(ActualFeature& feature) const;
 * Instantiate the identifier. Must be overload by subclasses.
 */


/**
 * @class ParamIdentifier
 * Implementation of @ref AbstractParamIdentifier for an @ref Identifier.
 * @par T	Data type of the @ref Identifier.
 * @ingroup parametric
 */


/**
 * @fn ParamIdentifier::ParamIdentifier(ParamFeature& feature, cstring name);
 * @param feature	Owner parametric feature.
 * @param name		Param identifier name.
 */


/**
 * @fn ParamIdentifier::ParamIdentifier(ParamFeature& feature, cstring name, const T& def);
 * @param feature	Owner parametric feature.
 * @param name		Param identifier name.
 * @param def		Default value.
 */


/**
 * @fn Identifier<T>& ParamIdentifier::operator[](ActualFeature *feat) const;
 * This function implements a fast way to get a parametric identifier instance
 * from an actual feature instance.
 * @param feat	Feature to look in.
 * @return		Identifier, instance of the parametric identifier.
 */


/**
 * This identifier is used to hook parametric identifiers associated with a parametric feature.
 * This identifier is automatically used at parametric identifier creation.
 * @par Hooks
 * @li @ref otawa::ParamFeature
 */
Identifier<AbstractParamIdentifier *> ParamFeature::PARAM_ID("otawa::ParamFeature::PARAM_ID");

}	// otawa
