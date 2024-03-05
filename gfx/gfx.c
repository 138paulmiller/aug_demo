#define AUG_IMPLEMENTATION
#include <aug.h>

#define SDL_MAIN_HANDLED
#if _WIN32
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#elif __linux
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GfxWindow
{
	SDL_Window* s_window;
	SDL_Renderer* renderer;
} GfxWindow;
static GfxWindow* s_window;

bool GfxInt3(aug_value* arg, int* x, int* y, int* z)
{
	if (arg != NULL && arg->type == AUG_ARRAY && arg->array->length == 3)
	{
		*x = aug_to_int(aug_array_at(arg->array, 0));
		*y = aug_to_int(aug_array_at(arg->array, 1)); 
		*z = aug_to_int(aug_array_at(arg->array, 2));
		return true;	
	}
	return false;
}

bool GfxInt4(aug_value* arg, int* x, int* y, int* z, int* w)
{
	if (arg != NULL && arg->type == AUG_ARRAY && arg->array->length == 4)
	{
		*x = aug_to_int(aug_array_at(arg->array, 0));
		*y = aug_to_int(aug_array_at(arg->array, 1)); 
		*z = aug_to_int(aug_array_at(arg->array, 2));
		*w = aug_to_int(aug_array_at(arg->array, 3));
		return true;	
	}
	return false;
}

int GfxColor(aug_value* args, int* r, int* g, int* b, int* a)
{
	*r = *g = *b = 0;
	*a = 255;

	if (GfxInt3(args, r, g, b))
		return 3;
	if (GfxInt4(args, r, g, b, a))
		return 4;

	int i = 0;
	if(args) *r = aug_to_int(&args[i++]);
	if(args) *g = aug_to_int(&args[i++]);
	if(args) *b = aug_to_int(&args[i++]);
	if(args) *a = aug_to_int(&args[i++]);
	return i;
}

aug_value GfxStartup(int argc, aug_value* args)
{	
	if (s_window != NULL)
		return aug_create_bool(false);

	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) 
	{
	    printf("Could not initialize SDL %s\n", SDL_GetError());
		return aug_create_bool(false);
	}

	if(TTF_Init() < 0) 
	{
	    printf("Could not initialize TTF %s\n", TTF_GetError());
		return aug_create_bool(false);
	}

	if (argc == 3)
	{
		s_window = (GfxWindow*)malloc(sizeof(GfxWindow));

		const aug_string* title = (args++)->str;
		const int w = aug_to_int(args++);
		const int h = aug_to_int(args++);
		s_window->s_window = SDL_CreateWindow(title->buffer, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_SHOWN);
		s_window->renderer = SDL_CreateRenderer(s_window->s_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		return aug_create_bool(true);
	}

	return aug_create_bool(false);
}

aug_value GfxShutdown(int argc, aug_value* args)
{
	SDL_DestroyRenderer(s_window->renderer);
	SDL_DestroyWindow(s_window->s_window);
	SDL_Quit(); // Causes win32 crash ? 
	free(s_window);
	return aug_none();	
}

aug_value GfxTime(int argc, aug_value* args)
{
	return aug_create_int((int)SDL_GetPerformanceCounter());	
}

aug_value GfxRate(int argc, aug_value* args)
{
	return aug_create_int((int)SDL_GetPerformanceFrequency());
}

aug_value GfxPollInput(int argc, aug_value* args)
{
	// TODO: avoid using string, use enums instead
	// see https://wiki.libsdl.org/SDL2/SDLKeycodeLookup for int defs in aug script
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_KEYDOWN:
		{
			aug_value key = aug_create_string("key_down");
			aug_value value = aug_create_string(SDL_GetKeyName(event.key.keysym.sym));
			aug_value map = aug_create_map();
			aug_map_insert(map.map, &key, &value);
			return map;
		}
		case SDL_KEYUP:
		{
			aug_value key = aug_create_string("key_up");
			aug_value value = aug_create_string(SDL_GetKeyName(event.key.keysym.sym));
			aug_value map = aug_create_map();
			aug_map_insert(map.map, &key, &value);
			return map;
		}
		case SDL_WINDOWEVENT:
		{
			switch (event.window.event) 
			{
			case SDL_WINDOWEVENT_CLOSE:
			{
				aug_value key = aug_create_string("window");
				aug_value value = aug_create_string("close");
				aug_value map = aug_create_map();
				aug_map_insert(map.map, &key, &value);
				return map;
			}
			default:
				break;
			}
			break;
		}
		default:
			break;
		}
	}
	return aug_none();
}


aug_value GfxWindowWidth(int argc, aug_value* args)
{
	int x, y;
	SDL_GetWindowSize(s_window->s_window, &x, &y);
	return aug_create_int(x);
}

aug_value GfxWindowHeight(int argc, aug_value* args)
{
	int x,y;
	SDL_GetWindowSize(s_window->s_window, &x, &y);
	return aug_create_int(y);
}

aug_value GfxClear(int argc, aug_value* args)
{
	int r,g,b,a;
	GfxColor(args, &r, &g, &b, &a);
	SDL_SetRenderDrawColor(s_window->renderer, r, g, b, a);
	SDL_RenderClear(s_window->renderer);
	SDL_SetRenderDrawColor(s_window->renderer, 0, 0, 0, 255);
	return aug_none();
}

aug_value GfxPresent(int argc, aug_value* args)
{
	SDL_RenderPresent(s_window->renderer);
	return aug_none();
}

aug_value GfxDrawRect(int argc, aug_value* args)
{
	// x,y,w,h, color
	if(!(argc == 5 || argc == 8))
		return aug_none();

	SDL_Rect rect;
	rect.x = aug_to_int(args++);
	rect.y = aug_to_int(args++);
	rect.w = aug_to_int(args++); 
	rect.h = aug_to_int(args++);

	int r,g,b,a;
	GfxColor(args, &r, &g, &b, &a);
	SDL_SetRenderDrawColor(s_window->renderer, r, g, b, a);
	SDL_RenderFillRect(s_window->renderer, &rect);
	return aug_none();
}

aug_value GfxFont(int argc, aug_value* args)
{
	if(argc != 2 || args[0].type != AUG_STRING)
		return aug_none();

	const char* font_path = (args++)->str->buffer;
	const int size = aug_to_int(args++);
	TTF_Font* font = TTF_OpenFont(font_path, size);
	if(font == NULL)
	    printf("Failed to load font: %s\n", TTF_GetError());
	return aug_create_user_data(font);
}

aug_value GfxText(int argc, aug_value* args)
{
	// font, text, x,y, r g b a
	if(!(argc == 5 || argc == 8))
		return aug_none();

	if(args[0].type != AUG_USERDATA && args[1].type != AUG_STRING)
		return aug_none();

	TTF_Font* font = (TTF_Font*)((args++)->userdata);

	const char* text = (args++)->str->buffer;
	
	int x = aug_to_int(args++);
	int y = aug_to_int(args++);

	int r,g,b,a;
	args+=GfxColor(args, &r, &g, &b, &a);
	
	SDL_Rect rect;
	rect.x = x;    
	rect.y = y; 

	TTF_SizeText(font, text, &rect.w, &rect.h);
	
	SDL_Color color = {r, g, b, a};
	SDL_Surface* surface = TTF_RenderText_Blended(font, text, color); 
	SDL_Texture* texture = SDL_CreateTextureFromSurface(s_window->renderer, surface);
	SDL_RenderCopy(s_window->renderer, texture, NULL, &rect);
	return aug_none();
}

AUG_LIB aug_register_lib(aug_vm* vm)
{
	aug_register(vm, "GfxStartup", GfxStartup);
	aug_register(vm, "GfxShutdown", GfxShutdown);
	aug_register(vm, "GfxTime", GfxTime);
	aug_register(vm, "GfxRate", GfxRate);
	aug_register(vm, "GfxPollInput", GfxPollInput);
	aug_register(vm, "GfxWindowWidth", GfxWindowWidth);
	aug_register(vm, "GfxWindowHeight", GfxWindowHeight);
	aug_register(vm, "GfxClear", GfxClear);
	aug_register(vm, "GfxPresent", GfxPresent);
	aug_register(vm, "GfxDrawRect", GfxDrawRect);
	aug_register(vm, "GfxFont", GfxFont);
	aug_register(vm, "GfxText", GfxText);
}

#ifdef __cplusplus
}
#endif