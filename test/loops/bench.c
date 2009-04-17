
int sum(void) {
	int sum = 0, i, j;
	for(i = 0; i < 10; i++)
		for(j = 0; j < i; j++)
			sum += j;
	return sum;
}

int zero(int n) {
	int i, s = 0;
	if(n < 10)
		for(i = 0; i < n; i++)
			s += i;
	else
		for(i = 0; i < n * 100; i++)
			s +=  i;
	return s;
}

int main(int argc, char **argv) {
	sum();
	zero(10);
}
