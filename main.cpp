#define AUG_IMPLEMENTATION
#include <aug.h>
#include <test/dump.inl>

#if AUG_DEBUG
void on_aug_post_instruction_debug(aug_vm* vm, int opcode)
{
    printf("%ld:   %s\n", vm->instruction - vm->bytecode, aug_opcode_label(opcode));
    int i;
    for(i = vm->stack_index-1; i > 0; --i)
    {
        aug_value val = vm->stack[i];
        printf("%s %d: %s ", (vm->stack_index-1) == i ? ">" : " ", i, aug_type_label(&val));
        switch(val.type)
        {
            case AUG_INT:      printf("%d", val.i); break;
            case AUG_FLOAT:    printf("%f", val.f); break;
            case AUG_STRING:   printf("%s", val.str->buffer); break;
            case AUG_BOOL:     printf("%s", val.b ? "true" : "false"); break;
            case AUG_CHAR:     printf("%c", val.c); break;
            default: break;
        }
        printf("\n");
    }
    getchar();
}
#endif

bool error = false;
void handle_error(const char* message)
{
	fprintf(stderr, "[ERR]\t%s\n", message);
	error = true;;
}

int main(int argc, char* argv[])
{
	aug_vm* vm = aug_startup(handle_error);
#if AUG_DEBUG
    vm->debug_post_instruction = on_aug_post_instruction_debug;
	aug_dump_file(vm, "scripts/snake.aug");
#endif
	aug_execute(vm, "scripts/snake.aug");
	aug_shutdown(vm);
	return 0;
}
