#ifndef ISP_CATBUILDER_H
#define ISP_CATBUILDER_H

#include <otawa/proc/CFGProcessor.h>
#include "FunctionBlockBuilder.h"

using namespace otawa;

namespace otawa {

  typedef enum isp_category_t {
    ISP_INVALID,
    ISP_ALWAYS_HIT,
    ISP_ALWAYS_MISS,
    ISP_NOT_CLASSIFIED,
    ISP_PERSISTENT
  } isp_category_t;
  
  class ISPCATBuilder: public CFGProcessor {
  public:
    virtual void configure (const PropList &props);
    ISPCATBuilder();
  protected:
    virtual void processCFG(WorkSpace *ws, CFG *cfg);
  };
  
  extern Identifier<isp_category_t> ISP_CATEGORY;
  extern Feature<ISPCATBuilder> ISP_CAT_FEATURE;
} // otawa

#endif // ISP_CATBUILDER_H
