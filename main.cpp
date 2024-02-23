#include <gfx.h>

bool error = false;
void handle_error(const char* message)
{
	fprintf(stderr, "[ERR]\t%s\n", message);
	error = true;;
}

int main(int argc, char* argv[])
{
	aug_vm* vm = aug_startup(handle_error);
	aug_gfx_init(vm);
	aug_gfx_set_working_paths("../assets/", "../scripts/");

	aug_gfx_push_script("main.aug");

	// TODO move to main script
	aug_gfx_startup();
	while (!error && aug_gfx_update()){}
	aug_gfx_shutdown();
	return 0;
}
