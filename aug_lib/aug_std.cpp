#include "aug_std.h"
#include <math.h>

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

aug_value concat(int argc, aug_value* args)
{
	if (argc == 0)
		return aug_none();

	aug_value value = aug_create_string("");
	for (int i = 0; i < argc; ++i)
	{
		aug_value* arg = args + i;
		switch(arg->type)
		{
			case AUG_CHAR:
				aug_string_push(value.str, arg->c);
				break;
			case AUG_STRING:
				aug_string_append(value.str, arg->str);
				break;
		}
	}
	return value;
}

aug_value split(int argc, aug_value* args)
{
	if (argc != 2 && args[0].type != AUG_STRING && args[1].type != AUG_STRING)
		return aug_none();

	aug_value value = aug_create_array();
	aug_string* str = args[0].str;
	aug_string* delim = args[1].str;
	aug_value line = aug_create_string("");
	for (int i = 0; i < str->length; ++i)
	{
		char c = str->buffer[i];
		if( c == delim->buffer[0])
		{
			int j = 0;
			while(j < str->length &&  j < delim->length)
			{
				if(str->buffer[i+j] != delim->buffer[j])
					break;
				++j;
			}

			if(j == delim->length)
			{				
				aug_array_append(value.array, &line);
				line = aug_create_string("");
				i += (j - 1); // -1 to account for the ++i in the for loop 
			}
		}
		else
		{
			aug_string_push(line.str, c);
		}
	}
	
	aug_array_append(value.array, &line);
	return value;
}

aug_value append(int argc, aug_value* args)
{
	if (argc == 0)
		return aug_none();

	aug_value value = args[0];
	for (int i = 1; i < argc; ++i)
	{
		aug_value* arg = args + i;

		switch(value.type)
		{
		case AUG_ARRAY:
			aug_array_append(value.array, arg);
			break;
		case AUG_STRING:
		{
			switch(arg->type)
			{
				case AUG_CHAR:
					aug_string_push(value.str, arg->c);
					break;
				case AUG_STRING:
					aug_string_append(value.str, arg->str);
					break;
			}
			break;
		}
		} 
	}
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

aug_value front(int argc, aug_value* args)
{
	if (argc == 0 || args[0].type != AUG_ARRAY)
		return aug_none();
	aug_value value = args[0];
	aug_value* element = aug_array_at(value.array, 0);
	aug_incref(element);
	if(element != NULL)
		return *element;
	return aug_none();
}

aug_value back(int argc, aug_value* args)
{
	if (argc == 0 || args[0].type != AUG_ARRAY)
		return aug_none(); 
	aug_value value = args[0];
	aug_value* element = aug_array_at(value.array, value.array->length - 1);
	aug_incref(element);
	if(element != NULL)
		return *element;
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

aug_value to_string(int argc, aug_value* args)
{
	if(argc != 1)
		return aug_none();

	aug_value value = args[0];
   	char out[1024];
    int len = 0;
    switch (value.type)
    {
    case AUG_NONE:
    	return aug_none();
    case AUG_BOOL:
        len = snprintf(out, sizeof(out), "%s", value.b ? "true" : "false");
        break;
	case AUG_CHAR:
        len = snprintf(out, sizeof(out), "%c", value.c);
        break;
    case AUG_INT:
        len = snprintf(out, sizeof(out), "%d", value.i);
        break;
    case AUG_FLOAT:
        len = snprintf(out, sizeof(out), "%f", value.f);
        break;
    case AUG_STRING:
        len = snprintf(out, sizeof(out), "%s", value.str->buffer);
        break;
    default:
    	return aug_none();
    }
	return aug_create_string(out);
}

void aug_std_initialize(aug_vm* vm)
{
	aug_register(vm, "random", random);
	aug_register(vm, "print", print);
	aug_register(vm, "to_string", to_string);
	aug_register(vm, "concat", concat);
	aug_register(vm, "split", split);
	aug_register(vm, "append", append);
	aug_register(vm, "remove", remove);
	aug_register(vm, "front", front);
	aug_register(vm, "back", back);
	aug_register(vm, "length", length);
	aug_register(vm, "contains", contains);
	aug_register(vm, "snap", snap);
}