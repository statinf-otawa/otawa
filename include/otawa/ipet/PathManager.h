/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	otawa/ipet/PathManager.h -- PathManager class interface.
 */
#ifndef OTAWA_IPET_PATHMANAGER_H
#define OTAWA_IPET_PATHMANAGER_H

#include <otawa/ipet/BBPath.h>
#include <otawa/cfg.h>
#include <otawa/ipet.h>
#include <elm/genstruct/Vector.h>
#include <otawa/ipet/TreePath.h>

namespace otawa { namespace ipet {

class BBPath;

class PathManager: public PropList {
public:
	virtual BBPath* getBBPath(BasicBlock *start);
	virtual BBPath* getBBPath(elm::genstruct::Vector<BasicBlock*> *path);
	inline bool explicitNames();
	
	static GenericIdentifier<TreePath<BasicBlock*,BBPath*>*> ID_Tree;
};

inline bool PathManager::explicitNames(){return IPET::ID_Explicit(this);}

} } // otawa::ipet

#endif /*OTAWA_IPET_PATHMANAGER_H*/
