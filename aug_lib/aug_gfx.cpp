#include "aug_gfx.h"
#include "aug_std.h"

#define AUG_IMPLEMENTATION
#include <aug.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <string>
#include <vector>

namespace
{
	struct GfxContext
	{
		// TODO: move this to a Gfx object that scripts can manage on their own
		SDL_Window* window;
		SDL_Renderer* renderer;
		SDL_Event event;

		aug_vm* vm;
		std::string script_path;
		std::string asset_path;
		std::vector<aug_script*> scripts;

		std::string script_pending_load;

		Uint64 time_current;
		Uint64 time_prev = 0;
		float time_delta = 0;
	};

	static GfxContext s_gfx;

	aug_value CreateWindow(int argc, aug_value* args)
	{
		SDL_Init(SDL_INIT_VIDEO);
		if (argc == 3)
		{
			const aug_string* title = args[0].str;
			const int w = aug_to_int(&args[1]);
			const int h = aug_to_int(&args[2]);
			s_gfx.window = SDL_CreateWindow(title->buffer, 0, 0, w, h, SDL_WINDOW_SHOWN);
			s_gfx.renderer = SDL_CreateRenderer(s_gfx.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		}

		return aug_none();
	}

	aug_value DestroyWindow(int argc, aug_value* args)
	{
		SDL_DestroyWindow(s_gfx.window);
		SDL_DestroyRenderer(s_gfx.renderer);
		SDL_Quit(); 
		s_gfx.renderer = NULL;
		s_gfx.window = NULL;
		return aug_none();
	}

	aug_value Clear(int argc, aug_value* args)
	{
		if (argc == 0)
			return aug_none();

		if (argc == 4)
		{
			const int r = aug_to_int(&args[0]);
			const int g = aug_to_int(&args[1]); 
			const int b = aug_to_int(&args[2]);
			const int a = aug_to_int(&args[3]);
			SDL_SetRenderDrawColor(s_gfx.renderer, r, g, b, a);
		}
		else
		{
			SDL_SetRenderDrawColor(s_gfx.renderer, 0, 0, 0, 255);
		}
		
		SDL_RenderClear(s_gfx.renderer);
		SDL_SetRenderDrawColor(s_gfx.renderer, 0, 0, 0, 255);
		return aug_none();
	}


	aug_value Present(int argc, aug_value* args)
	{
		SDL_RenderPresent(s_gfx.renderer);
		return aug_none();
	}

	aug_value DrawRect(int argc, aug_value* args)
	{
		if (argc == 8)
		{
			SDL_Rect rect;
			rect.x = aug_to_int(&args[0]);
			rect.y = aug_to_int(&args[1]);
			rect.w = aug_to_int(&args[2]); 
			rect.h = aug_to_int(&args[3]);
			const int r = aug_to_int(&args[4]);
			const int g = aug_to_int(&args[5]); 
			const int b = aug_to_int(&args[6]);
			const int a = aug_to_int(&args[7]);
			SDL_SetRenderDrawColor(s_gfx.renderer, r, g, b, a);
			SDL_RenderFillRect(s_gfx.renderer, &rect);
		}
		return aug_none();
	}

	aug_value LoadFont(int argc, aug_value* args)
	{
		if(argc != 2 || args[0].type != AUG_STRING)
			return aug_none();

		const std::string font_path = s_gfx.asset_path + args[0].str->buffer;
		const int size = aug_to_int(args+1);
		TTF_Font* font = TTF_OpenFont(font_path.c_str(), size);
		return aug_create_user_data(font);
	}

	aug_value DrawText(int argc, aug_value* args)
	{
		// font, text, x,y, r g b a
		if(argc != 1 || args[0].type != AUG_STRING)
			return aug_none();

		TTF_Font* font = (TTF_Font*)(args[0].userdata);
		const char * text = args[1].str->buffer;
		int x = aug_to_int(args + 2);
		int y = aug_to_int(args + 3);
		char r = aug_to_int(args + 4);
		char g = aug_to_int(args + 5);
		char b = aug_to_int(args + 6);
		char a = aug_to_int(args + 6);
		
		SDL_Rect rect;
		rect.x = x;    
		rect.y = y; 
		TTF_SizeText(font, text, &rect.w, &rect.h);
		
		SDL_Color color = {r, g, b, a};
		SDL_Surface* surface = TTF_RenderText_Solid(font, text, color); 
		SDL_Texture* texture = SDL_CreateTextureFromSurface(s_gfx.renderer, surface);
		SDL_RenderCopy(s_gfx.renderer, texture, NULL, &rect);
	}

	aug_value LoadScript(int argc, aug_value* args)
	{
		if(argc == 1 && args[0].type == AUG_STRING)
			s_gfx.script_pending_load = args[0].str->buffer;
	}

	aug_value ExitScript(int argc, aug_value* args)
	{
		aug_gfx_pop_script();
	}
}

void aug_gfx_init(aug_vm* vm)
{
	aug_std_initialize(vm);
	aug_register(vm, "Load", LoadScript);
	aug_register(vm, "Exit", ExitScript);

	aug_register(vm, "GfxCreateWindow", CreateWindow);
	aug_register(vm, "GfxDestroyWindow", DestroyWindow);	
	aug_register(vm, "GfxClear", Clear);
	aug_register(vm, "GfxPresent", Present);
	aug_register(vm, "GfxDrawRect", DrawRect);
	aug_register(vm, "GfxFont", LoadScript);
	aug_register(vm, "GfxText", LoadScript);
	s_gfx.vm = vm;
}

void aug_gfx_startup()
{
	s_gfx.time_current = SDL_GetPerformanceCounter();
	s_gfx.time_prev = 0;
	s_gfx.time_delta = 0;
	for(aug_script* script : s_gfx.scripts)
		aug_call(s_gfx.vm, script, "Startup");
}

void aug_gfx_shutdown()
{
	for(aug_script* script : s_gfx.scripts)
		aug_unload(s_gfx.vm, script);

	aug_shutdown(s_gfx.vm);
}

void aug_gfx_push_script(const char* script_filepath)
{
	const std::string& full_script_path = s_gfx.script_path + script_filepath;
	aug_script* script = aug_load(s_gfx.vm, full_script_path.c_str());
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
	if(s_gfx.script_pending_load.size() > 0)
	{
		aug_gfx_push_script(s_gfx.script_pending_load.c_str());
		s_gfx.script_pending_load = "";
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