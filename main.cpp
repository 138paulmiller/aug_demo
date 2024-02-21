#include <aug_gfx.h>

void handle_error(const char* message)
{
	fprintf(stderr, "[ERROR]\t%s\n", message);
}

int main(int argc, char* argv[])
{
	aug_vm* vm = aug_startup(handle_error);
	aug_gfx_init(vm);
	aug_gfx_set_working_paths("assets/", "scripts/");
	aug_gfx_push_script("main.aug");
	aug_gfx_startup();
	while (aug_gfx_update()){}
	aug_gfx_shutdown();
	return 0;
}
