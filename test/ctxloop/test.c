

int f(int n) {
	int s = 0, i;
	for(i = 0; i < n; i++)
		s += i;
	return s;
}

int g1(void) {
	return f(10);
}

int g2(void) {
	return f(20);
}

int main(void) {
	return g1() + g2();
}
