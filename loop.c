#include <stdio.h>

int main() {
	int i = 0;
	while(1) {
		i++;
		if (i % 100000000000 == 0) {
			printf("%d\n", i);
		}
	}
	return 0;
}

