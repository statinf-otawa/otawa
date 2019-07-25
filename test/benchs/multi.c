
int s = 0;
 
void sum(int n) {
	int i;
	for(i = 0; i < n; i++)
		s += i;
}

int main(void) {
	int i;
	sum(0);
	for(i = 0; i < 100; i++)
		sum(i);
	return s;
}
 
