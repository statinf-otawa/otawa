/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	test/lp_solve/test.cpp -- test file for integration of lp_solve in OTAWA.
 */

#include <assert.h>
#include <elm/io.h>
#include <otawa/lp_solve/System.h>

using namespace elm;
using namespace otawa;
using namespace ilp;

void check(char *msg, int req, int fnd) {
	cout << msg << ": " << req << " = " << fnd;
	if(req == fnd)
		cout << " [OK]\n";
	else
		cout << "[FAIL]\n";
			
}

int main(void) {
	Var v1, v2, v3, v4;
	lp_solve::System sys(false);
	
	// Build the system
	Constraint *cons = sys.newConstraint(ilp::Constraint::LE, 4);
	cons->add(3, &v1);
	cons->add(2, &v2);
	cons->add(2, &v3);
	cons->add(1, &v4);
	cons = sys.newConstraint(ilp::Constraint::GE, 3);
	cons->add(0, &v1);
	cons->add(4, &v2);
	cons->add(3, &v3);
	cons->add(1, &v4);
	sys.addObjectFunction(2,&v1);
	sys.addObjectFunction(3,&v2);
	sys.addObjectFunction(-2,&v3);
	sys.addObjectFunction(3,&v4);
	
	// Solve it
	assert(sys.solve());
	
	// Display result
	check("Result", -4, (int)sys.value());
	check("v1", 0, (int)sys.valueOf(&v1));
	check("v2", 0, (int)sys.valueOf(&v2));
	check("v3", 2, (int)sys.valueOf(&v3));
	check("v4", 0, (int)sys.valueOf(&v4));
	
	return 0;
}

