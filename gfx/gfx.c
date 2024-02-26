#define AUG_IMPLEMENTATION
#include <aug.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

typedef struct GfxWindow
{
	// TODO: move this to a Gfx object that scripts can manage on their own
	SDL_Window* window;
	SDL_Renderer* renderer;
} GfxWindow;

aug_value GfxStartup(int argc, aug_value* args)
{	
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
	return aug_create_bool(true);
}

aug_value GfxShutdown(int argc, aug_value* args)
{
	TTF_Quit();
	SDL_Quit(); 
	return aug_none();	
}

aug_value GfxTime(int argc, aug_value* args)
{
	return aug_create_int(SDL_GetPerformanceCounter());	
}

aug_value GfxRate(int argc, aug_value* args)
{
	return aug_create_int(SDL_GetPerformanceFrequency());	
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
				aug_value key = aug_create_string("window");
				aug_value value = aug_create_string("close");
				aug_value map = aug_create_map();
				aug_map_insert(map.map, &key, &value);
				return map;
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

aug_value GfxCreateWindow(int argc, aug_value* args)
{
	if (argc == 3)
	{
		GfxWindow* window = (GfxWindow*)malloc(sizeof(GfxWindow));

		const aug_string* title = (args++)->str;
		const int w = aug_to_int(args++);
		const int h = aug_to_int(args++);
		window->window = SDL_CreateWindow(title->buffer, 0, 0, w, h, SDL_WINDOW_SHOWN);
		window->renderer = SDL_CreateRenderer(window->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		return aug_create_user_data(window);
	}

	return aug_none();
}

aug_value GfxDestroyWindow(int argc, aug_value* args)
{
	if(argc != 1 || args->type != AUG_USERDATA)
		return aug_none();

	GfxWindow* window = (GfxWindow*)args->userdata;

	SDL_DestroyWindow(window->window);
	SDL_DestroyRenderer(window->renderer);
	free(window);
	return aug_none();
}

aug_value GfxWindowWidth(int argc, aug_value* args)
{
	if(argc != 1 || args->type != AUG_USERDATA)
		return aug_none();

	GfxWindow* window = (GfxWindow*)args->userdata;
	int x,y;
	SDL_GetWindowSize(window->window, &x, &y);
	return aug_create_int(x);
}

aug_value GfxWindowHeight(int argc, aug_value* args)
{
	if(argc != 1 || args->type != AUG_USERDATA)
		return aug_none();

	GfxWindow* window = (GfxWindow*)args->userdata;
	int x,y;
	SDL_GetWindowSize(window->window, &x, &y);
	return aug_create_int(y);
}

aug_value GfxClear(int argc, aug_value* args)
{
	if(argc == 0 || args[0].type != AUG_USERDATA)
		return aug_none();

	GfxWindow* window = (GfxWindow*)(args++)->userdata;

	if (argc == 5)
	{
		const int r = aug_to_int(args++);
		const int g = aug_to_int(args++); 
		const int b = aug_to_int(args++);
		const int a = aug_to_int(args++);
		SDL_SetRenderDrawColor(window->renderer, r, g, b, a);
	}
	else
	{
		SDL_SetRenderDrawColor(window->renderer, 0, 0, 0, 255);
	}
	
	SDL_RenderClear(window->renderer);
	SDL_SetRenderDrawColor(window->renderer, 0, 0, 0, 255);
	return aug_none();
}


aug_value GfxPresent(int argc, aug_value* args)
{
	if(argc != 1 || args->type != AUG_USERDATA)
		return aug_none();

	GfxWindow* window = (GfxWindow*)args->userdata;

	SDL_RenderPresent(window->renderer);
	return aug_none();
}

aug_value GfxDrawRect(int argc, aug_value* args)
{
	if(argc != 9 || args->type != AUG_USERDATA)
		return aug_none();

	GfxWindow* window = (GfxWindow*)(args++)->userdata;

	SDL_Rect rect;
	rect.x = aug_to_int(args++);
	rect.y = aug_to_int(args++);
	rect.w = aug_to_int(args++); 
	rect.h = aug_to_int(args++);
	const int r = aug_to_int(args++);
	const int g = aug_to_int(args++); 
	const int b = aug_to_int(args++);
	const int a = aug_to_int(args++);
	SDL_SetRenderDrawColor(window->renderer, r, g, b, a);
	SDL_RenderFillRect(window->renderer, &rect);
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
	    printf("TTF_OpenFont: %s\n", TTF_GetError());
	return aug_create_user_data(font);
}

aug_value GfxText(int argc, aug_value* args)
{
	// window, font, text, x,y, r g b a
	if(argc != 9)
		return aug_none();

	if(args[0].type != AUG_USERDATA && args[1].type != AUG_USERDATA && args[2].type != AUG_STRING)
		return aug_none();

	GfxWindow* window = (GfxWindow*)(args++)->userdata;
	TTF_Font* font = (TTF_Font*)((args++)->userdata);

	const char* text = (args++)->str->buffer;
	
	int x = aug_to_int(args++);
	int y = aug_to_int(args++);
	uint8_t r = aug_to_int(args++);
	uint8_t g = aug_to_int(args++);
	uint8_t b = aug_to_int(args++);
	uint8_t a = aug_to_int(args++);
	
	SDL_Rect rect;
	rect.x = x;    
	rect.y = y; 

	TTF_SizeText(font, text, &rect.w, &rect.h);
	
	SDL_Color color = {r, g, b, a};
	SDL_Surface* surface = TTF_RenderText_Blended(font, text, color); 
	SDL_Texture* texture = SDL_CreateTextureFromSurface(window->renderer, surface);
	SDL_RenderCopy(window->renderer, texture, NULL, &rect);
	return aug_none();
}

AUG_LIBCALL void aug_register_lib(aug_vm* vm)
{
	aug_register(vm, "GfxStartup", GfxStartup);
	aug_register(vm, "GfxShutdown", GfxShutdown);
	aug_register(vm, "GfxTime", GfxTime);
	aug_register(vm, "GfxRate", GfxRate);
	aug_register(vm, "GfxPollInput", GfxPollInput);
	aug_register(vm, "GfxCreateWindow", GfxCreateWindow);
	aug_register(vm, "GfxDestroyWindow", GfxDestroyWindow);
	aug_register(vm, "GfxWindowWidth", GfxWindowWidth);
	aug_register(vm, "GfxWindowHeight", GfxWindowHeight);
	aug_register(vm, "GfxClear", GfxClear);
	aug_register(vm, "GfxPresent", GfxPresent);
	aug_register(vm, "GfxDrawRect", GfxDrawRect);
	aug_register(vm, "GfxFont", GfxFont);
	aug_register(vm, "GfxText", GfxText);
}
