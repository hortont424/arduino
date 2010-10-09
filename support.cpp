extern "C" void __cxa_pure_virtual() { while (1); }

#include <WProgram.h>

int main(void)
{
	init();

	setup();

	for (;;)
		loop();

	return 0;
}