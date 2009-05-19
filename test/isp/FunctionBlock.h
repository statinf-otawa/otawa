/*
 *	$Id$
 *	Copyright (c) 2009, IRIT UPS.
 *
 *	FunctionBlock.h -- interface of FunctionBlock class.
 */
#ifndef OTAWA_ISP_FUNCTIONBLOCK_H
#define OTAWA_ISP_FUNCTIONBLOCK_H

#include <elm/string.h>
#include <elm/genstruct/SLList.h>
#include <elm/genstruct/HashTable.h>
#include <elm/inhstruct/DLList.h>
#include <elm/Iterator.h>
#include <otawa/instruction.h>
#include <otawa/cache/ccg/CCGNode.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/ilp/Var.h>
#include <otawa/cache/categorisation/CATNode.h>
#include <otawa/hard/Cache.h>

namespace otawa {

  // FunctionBlock class
  class FunctionBlock: public elm::inhstruct::DLNode, public PropList {
    size_t _size;
    BasicBlock *_entry_bb;
     // Private methods
    ~FunctionBlock(void) { delete this; };

  public:
    FunctionBlock(BasicBlock *entry_bb, size_t size);
    inline BasicBlock *entryBB(void);
    inline size_t size(void) const;
  };

  // Inlines
  inline size_t FunctionBlock::size(void) const {
    return _size;
  }

  inline BasicBlock *FunctionBlock::entryBB(void) {
    return _entry_bb;
  }


} // otawa

#endif //  OTAWA_ISP_FUNCTIONBLOCK_H
