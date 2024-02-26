#define AUG_IMPLEMENTATION
#include <aug.h>

bool error = false;
void handle_error(const char* message)
{
	fprintf(stderr, "[ERR]\t%s\n", message);
	error = true;;
}

int main(int argc, char* argv[])
{
	aug_vm* vm = aug_startup(handle_error);
	aug_execute(vm, "scripts/snake.aug");
	aug_shutdown(vm);
	return 0;
}
