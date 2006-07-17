/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	test/props/test_systemc.cpp -- systemc integration test.
 */

#include <systemc.h>


// Half-adder module
SC_MODULE(half_adder) {
	sc_in<bool> a, b;
	sc_out<bool> sum, carry;
	
	void prc_half_adder() {
		sum = a ^ b;
		carry = a & b;
	}

	SC_CTOR(half_adder) {
		SC_METHOD(prc_half_adder);
		sensitive << a << b;
	}
};


// Full-adder module
SC_MODULE(full_adder) {
	sc_in<bool> a, b, carry_in;
	sc_out<bool> sum, carry_out;
	
	sc_signal<bool> c1, s1, c2;
	
	void prc_or() {
		carry_out = c1 | c2;
	}
	
	half_adder *ha1_ptr, *ha2_ptr;
	
	SC_CTOR(full_adder) {
		ha1_ptr = new half_adder("ha1");
		ha1_ptr->a(a);
		ha1_ptr->b(b);
		ha1_ptr->sum(s1);
		ha1_ptr->carry(c1);
		
		ha2_ptr = new half_adder("ha2");
		(*ha2_ptr)(s1, carry_in, sum, c2);
		
		SC_METHOD(prc_or);
		sensitive << c1 << c2;
	}
	
	~full_adder() {
		delete ha1_ptr;
		delete ha2_ptr;
	}
};


// Driver
SC_MODULE(driver) {
	sc_out<bool> d_a, d_b, d_cin;
	
	void prc_driver();
	
	SC_CTOR(driver) {
		SC_THREAD(prc_driver);
	}
};

void driver::prc_driver() {
	sc_uint<3> pattern;
	pattern = 0;
	
	while(1) {
		d_a = pattern[0];
		d_b = pattern[1];
		d_cin = pattern[2];
		wait(5, SC_NS);
		pattern++;
	}
}

// Monitor
SC_MODULE(monitor) {
	sc_in<bool> m_a, m_b, m_cin, m_sum, m_cout;
	
	void prc_monitor();
	
	SC_CTOR(monitor) {
		SC_METHOD(prc_monitor);
		sensitive << m_a << m_b << m_cin << m_sum << m_cout;
	}
};

void monitor::prc_monitor() {
	cout << "At time " << sc_time_stamp() << "::";
	cout << "(a, b, carry_in): ";
	cout << m_a << m_b << m_cin;
	cout << "  (sum, carry_out): " << m_sum << m_cout << endl;
}


// void sc_main
int sc_main(int argc, char *argv[]) {
	return 0;
}


// Main
//int sc_main(int argc, char *argv[]) {
int main(int argc, char *argv[]) {
	
	sc_core::sc_elab_and_sim(argc, argv);
	
	sc_signal<bool> t_a, t_b, t_cin, t_sum, t_cout;
	
	full_adder f1("blabla");
	f1 << t_a << t_b << t_cin << t_sum << t_cout;
	
	driver d1("driver");
	d1.d_a(t_a);
	d1.d_b(t_b);
	d1.d_cin(t_cin);
	
	monitor mo1("monitor");
	mo1 << t_a << t_b << t_cin << t_sum << t_cout;
	
	sc_start(100, SC_NS);
	
	return 0;
}

