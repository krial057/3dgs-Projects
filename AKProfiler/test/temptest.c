#include "../AKProfiler.c"
#include <acknex.h>
function slow_function(int count) {
	int i;
	for(i = 0; i < count; i ++)
	{
		BMAP* temp = bmap_createblack(1024, 1024, 32);
		bmap_fill(temp, COLOR_RED, 100);
		ptr_remove(temp);
	}
}

function paralel_func_1() {
	int i;
	for(i = 0; i < 3; i++) {
		wait(1);
	}
}

function paralel_func_2() {
	int i;
	for(i = 0; i < 3; i++) {
		wait(5);
	}
}

int main() {
	
	level_load(NULL);
	
	slow_function(100);
	
	paralel_func_1();
	paralel_func_2();
	
	return 0;
}