#include "gfx.h"

#define AUG_IMPLEMENTATION
#include <aug.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <string>
#include <vector>

namespace
{
	struct GfxWindow
	{
		// TODO: move this to a Gfx object that scripts can manage on their own
		SDL_Window* window;
		SDL_Renderer* renderer;
	};

	struct GfxContext
	{
		aug_vm* vm;
		SDL_Event event;

		std::string script_path;
		std::string asset_path;
		std::vector<aug_script*> scripts;
		std::string script_pending_load;
		bool script_pending_unload;

		Uint64 time_current;
		Uint64 time_prev = 0;
		float time_delta = 0;
	};

	static GfxContext s_gfx;

	aug_value Startup(int argc, aug_value* args)
	{
		return aug_none();	
	}
	aug_value Shutdown(int argc, aug_value* args)
	{
		return aug_none();	
	}
	aug_value Update(int argc, aug_value* args)
	{
		return aug_none();	
	}


	aug_value CreateWindow(int argc, aug_value* args)
	{
		if (argc == 3)
		{
			GfxWindow* window = new GfxWindow();

			const aug_string* title = (args++)->str;
			const int w = aug_to_int(args++);
			const int h = aug_to_int(args++);
			window->window = SDL_CreateWindow(title->buffer, 0, 0, w, h, SDL_WINDOW_SHOWN);
			window->renderer = SDL_CreateRenderer(window->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			return aug_create_user_data(window);
		}

		return aug_none();
	}

	aug_value DestroyWindow(int argc, aug_value* args)
	{
		if(argc != 1 || args->type != AUG_USERDATA)
			return aug_none();

		GfxWindow* window = (GfxWindow*)args->userdata;

		SDL_DestroyWindow(window->window);
		SDL_DestroyRenderer(window->renderer);
		delete window;

		return aug_none();
	}

	aug_value WindowWidth(int argc, aug_value* args)
	{
		if(argc != 1 || args->type != AUG_USERDATA)
			return aug_none();

		GfxWindow* window = (GfxWindow*)args->userdata;
		int x,y;
		SDL_GetWindowSize(window->window, &x, &y);
		return aug_create_int(x);
	}

	aug_value WindowHeight(int argc, aug_value* args)
	{
		if(argc != 1 || args->type != AUG_USERDATA)
			return aug_none();

		GfxWindow* window = (GfxWindow*)args->userdata;
		int x,y;
		SDL_GetWindowSize(window->window, &x, &y);
		return aug_create_int(y);
	}

	aug_value Clear(int argc, aug_value* args)
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


	aug_value Present(int argc, aug_value* args)
	{
		if(argc != 1 || args->type != AUG_USERDATA)
			return aug_none();

		GfxWindow* window = (GfxWindow*)args->userdata;

		SDL_RenderPresent(window->renderer);
		return aug_none();
	}

	aug_value DrawRect(int argc, aug_value* args)
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

	aug_value LoadFont(int argc, aug_value* args)
	{
		if(argc != 2 || args[0].type != AUG_STRING)
			return aug_none();

		const std::string font_path = s_gfx.asset_path + (args++)->str->buffer;

		const int size = aug_to_int(args++);
		TTF_Font* font = TTF_OpenFont(font_path.c_str(), size);
		if(font == nullptr) {
		    printf("TTF_OpenFont: %s\n", TTF_GetError());
		}

		return aug_create_user_data(font);
	}

	aug_value DrawText(int argc, aug_value* args)
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

	aug_value LoadScript(int argc, aug_value* args)
	{
		if(argc == 1 && args[0].type == AUG_STRING)
			s_gfx.script_pending_load = args[0].str->buffer;
		return aug_none();
	}

	aug_value ExitScript(int argc, aug_value* args)
	{
		s_gfx.script_pending_unload = true;
		return aug_none();
	}
}

void aug_gfx_init(aug_vm* vm)
{
	aug_register(vm, "GfxStartup", Startup);
	aug_register(vm, "GfxShutdown", Shutdown);
	aug_register(vm, "GfxUpate", Update);

	aug_register(vm, "Load", LoadScript);
	aug_register(vm, "Exit", ExitScript);

	aug_register(vm, "GfxCreateWindow", CreateWindow);
	aug_register(vm, "GfxDestroyWindow", DestroyWindow);
	aug_register(vm, "GfxWindowWidth", WindowWidth);
	aug_register(vm, "GfxWindowHeight", WindowHeight);
	aug_register(vm, "GfxClear", Clear);
	aug_register(vm, "GfxPresent", Present);
	aug_register(vm, "GfxDrawRect", DrawRect);
	aug_register(vm, "GfxFont", LoadFont);
	aug_register(vm, "GfxText", DrawText);
	s_gfx.vm = vm;
}

void aug_gfx_startup()
{
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
	    printf("Could not initialize SDL %s\n", SDL_GetError());
	    return;
	}

	if(TTF_Init() < 0) {
	    printf("Could not initialize TTF %s\n", TTF_GetError());
	    return;
	}

	s_gfx.time_current = SDL_GetPerformanceCounter();
	s_gfx.time_prev = 0;
	s_gfx.time_delta = 0;
	s_gfx.script_pending_unload = false;
	for(aug_script* script : s_gfx.scripts)
		aug_call(s_gfx.vm, script, "Startup");
}

void aug_gfx_shutdown()
{
	for(aug_script* script : s_gfx.scripts)
		aug_unload(s_gfx.vm, script);

	aug_shutdown(s_gfx.vm);

	SDL_Quit(); 
}

void aug_gfx_push_script(const char* script_filepath)
{
	const std::string& full_script_path = s_gfx.script_path + script_filepath;
	aug_script* script = aug_load(s_gfx.vm, full_script_path.c_str());
	if(script == NULL)
		return;
	s_gfx.scripts.push_back(script);
	aug_call(s_gfx.vm, script, "Startup");
}

void aug_gfx_pop_script()
{
	if(s_gfx.scripts.size())
		aug_call(s_gfx.vm, s_gfx.scripts.back(), "Shutdown");
	s_gfx.scripts.pop_back();

	if(s_gfx.scripts.size())
		aug_call(s_gfx.vm, s_gfx.scripts.back(), "Startup");
}

void aug_gfx_set_working_paths(const char* asset_path, const char* script_path)
{
	s_gfx.script_path = script_path;
	s_gfx.asset_path = asset_path;
}

bool aug_gfx_update()
{
	if(s_gfx.vm == NULL || !s_gfx.vm->valid)
		return false;

	if(s_gfx.script_pending_load.size() > 0)
	{
		aug_gfx_push_script(s_gfx.script_pending_load.c_str());
		s_gfx.script_pending_load = "";
		return true;
	}

	if(s_gfx.script_pending_unload)
	{
		aug_gfx_pop_script();
		s_gfx.script_pending_unload = false;
		return true;
	}

	if(s_gfx.scripts.size() == 0)
		return false;

	// Current executing script
	aug_script* script = s_gfx.scripts.back(); 

	s_gfx.time_prev = s_gfx.time_current;
	s_gfx.time_current = SDL_GetPerformanceCounter();
	s_gfx.time_delta = (s_gfx.time_current - s_gfx.time_prev) / (float)SDL_GetPerformanceFrequency();

	while (SDL_PollEvent(&s_gfx.event))
	{
		switch (s_gfx.event.type)
		{
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		{
			// see https://wiki.libsdl.org/SDL2/SDLKeycodeLookup for int defs in aug script
			const bool is_up = (s_gfx.event.type == SDL_KEYUP); 
			const char* key = SDL_GetKeyName(s_gfx.event.key.keysym.sym);
			aug_value arg = aug_create_string(key);
			aug_call_args(s_gfx.vm, script, is_up ? "KeyUp" : "KeyDown", 1, &arg);
			break;
		}
		case SDL_WINDOWEVENT:
		{
			switch (s_gfx.event.window.event) 
			{
			case SDL_WINDOWEVENT_CLOSE:
				return false;
			default:
				break;
			}
			break;
		}
		default:
			break;
		}
	}

	aug_value arg = aug_create_float(s_gfx.time_delta);
	aug_call_args(s_gfx.vm, script, "Update", 1, &arg);
	return true;
}