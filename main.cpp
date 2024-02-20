
#include <vector>
#include <cstdio>

#include "aug_gfx.h"

int main(int argc, char* argv[])
{
	aug_gfx_init();
	aug_gfx_add_script("script.aug");
	aug_gfx_startup();
	while (aug_gfx_update()){}
	aug_gfx_shutdown();
	return 0;
}
