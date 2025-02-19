/*
 *	CFGApplication class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2019, IRIT UPS.
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
#ifndef OTAWA_APP_CFGAPPLICATION_H_
#define OTAWA_APP_CFGAPPLICATION_H_

#include <otawa/app/Application.h>
#include <otawa/cfg/features.h>
#include <otawa/flowfact/features.h>

namespace otawa {

class CFGApplication: public Application {
public:
	CFGApplication(const Make& make);

protected:
	virtual void processTask(const CFGCollection& coll, PropList& props);

	void work(const string& entry, PropList &props) override;

	void prepareCFG(const string& entry, PropList& props);
	
	option::SwitchOption
		cfg_raw,
		cfg_virtualize,
		cfg_unroll,
		no_cfg_tune,
		cfg_reduceloop;
};

} // otawa

#endif /* OTAWA_APP_CFGAPPLICATION_H_ */
