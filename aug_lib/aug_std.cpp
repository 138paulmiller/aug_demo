#include "aug_std.h"
#include <math.h>
namespace
{
	void print_value(const aug_value& value);

	void print_map_pair(const aug_value* key, aug_value* value, void* user_data)
	{
		printf("\n\t");
		print_value(*key);
		printf(" : ");
		print_value(*value);
	}

	void print_value(const aug_value& value)
	{
		switch (value.type)
		{
		case AUG_NONE:
			printf("none");
			break;
		case AUG_BOOL:
			printf("%s", value.b ? "true" : "false");
			break;
		case AUG_CHAR:
			printf("%c", value.c);
			break;
		case AUG_INT:
			printf("%d", value.i);
			break;
		case AUG_FLOAT:
			printf("%0.3f", value.f);
			break;
		case AUG_STRING:
			printf("%s", value.str->buffer);
			break;
		case AUG_OBJECT:
			printf("object");
			break;
		case AUG_FUNCTION:
			printf("function %d", value.i);
			break;
		case AUG_ARRAY:
		{
			printf("[");
			for( size_t i = 0; i < value.array->length; ++i)
			{
				printf(" ");
				const aug_value* entry = aug_array_at(value.array, i);
				print_value(*entry);
				if(entry->type == AUG_ARRAY) printf("\n");		
			}
			printf(" ]");
			break;
		}
		case AUG_MAP:
		{		
			printf("{");
			aug_map_foreach(value.map, print_map_pair, NULL);
			printf("\n}");

			break;
		}
		}
	}

	aug_value print(int argc, aug_value* args)
	{
		for( int i = 0; i < argc; ++i)
			print_value(args[i]);

		printf("\n");

		return aug_none();
	}


	aug_value random(int argc, aug_value* args)
	{
		int x;
		if(argc == 1)
			x = rand() % aug_to_int(args+0);
		else if(argc == 2)
		{
			const int lower = aug_to_int(args+0);
			const int upper = aug_to_int(args+1);
			x = rand() % (upper - lower + 1) + lower;
		}
		else
			x = rand();
		return aug_create_int(x);
	}

	aug_value append(int argc, aug_value* args)
	{
		if (argc == 0 || args[0].type != AUG_ARRAY)
			return aug_none();
 
		aug_value value = args[0];
		for (int i = 1; i < argc; ++i)
			aug_array_append(value.array, args+i);
		return aug_none();
	}

	aug_value remove(int argc, aug_value* args)
	{
		if (argc != 2 || args[0].type != AUG_ARRAY)
			return aug_none();

		aug_value value = args[0];
		aug_value index = args[1];
		aug_array_remove(value.array, aug_to_int(&index));
		return aug_none();
	}

	aug_value length(int argc, aug_value* args)
	{
		if(argc != 1)
			return aug_none();
		aug_value value = args[0];
		switch (value.type)
		{
		case AUG_STRING:
			return aug_create_int(value.str->length);
		case AUG_ARRAY:
			return aug_create_int(value.array->length);
		case AUG_MAP:
			return aug_create_int(value.map->count);
		default: break;
		}
		return aug_none();
	}

	aug_value contains(int argc, aug_value* args)
	{
		if (argc != 1 || args[0].type != AUG_ARRAY)
			return aug_none();
 
		aug_value value = args[0];
		aug_value arg = args[1];
		for (int i = 0; i < value.array->length; ++i){
			aug_value* element = aug_array_at(value.array, i);
			if(aug_compare(&arg, element))
				return aug_create_bool(true);
		}
		return aug_create_bool(false);
	}

	aug_value snap(int argc, aug_value* args)
	{
		int x = aug_to_int(args + 0);
		int grid = aug_to_int(args + 1);
		return aug_create_int(floor(x / grid) * grid);
	}
}

void aug_std_initialize(aug_vm* vm)
{
	aug_register(vm, "print", print);
	aug_register(vm, "random", random);
	aug_register(vm, "append", append);
	aug_register(vm, "remove", remove);
	aug_register(vm, "length", length);
	aug_register(vm, "contains", contains);
	aug_register(vm, "snap", snap);
}