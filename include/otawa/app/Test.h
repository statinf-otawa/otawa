/*
 *	Test class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2018, IRIT UPS.
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
#ifndef OTAWA_APP_TEST_H_
#define OTAWA_APP_TEST_H_

#include "Application.h"

namespace otawa {

class Test: public Application {
public:
	Test(cstring name);

protected:
	virtual void generate(io::Output& out) = 0;

	void work(const string& entry, PropList &props) override;

private:
	option::Switch ref;
};

} // otawa

#endif /* OTAWA_APP_TEST_H_ */
