#include "aug_gfx.h"
#include "aug_std.h"

#define AUG_IMPLEMENTATION
#include <aug.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <vector>

namespace
{
	struct GfxContext
	{
		SDL_Window* window;
		SDL_Renderer* renderer;
		SDL_Event event;

		aug_vm* vm;
		std::vector<aug_script*> scripts;


		Uint64 time_current;
		Uint64 time_prev = 0;
		float time_delta = 0;
	};

	static GfxContext s_gfx;

	aug_value Create(int argc, aug_value* args)
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

	aug_value Destroy(int argc, aug_value* args)
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

	void Error(const char* message)
	{
		fprintf(stderr, "[ERROR]\t%s\n", message);
	}
}

void aug_gfx_init()
{
	aug_vm* vm = aug_startup(Error);
	aug_std_initialize(vm);
	aug_register(vm, "GfxCreate", Create);
	aug_register(vm, "GfxDestroy", Destroy);	
	aug_register(vm, "GfxClear", Clear);
	aug_register(vm, "GfxPresent", Present);
	aug_register(vm, "GfxDrawRect", DrawRect);
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
		aug_call(s_gfx.vm, script, "Shutdown");

	for(aug_script* script : s_gfx.scripts)
		aug_unload(s_gfx.vm, script);

	aug_shutdown(s_gfx.vm);
}

void aug_gfx_add_script(const char* script_filepath)
{
	aug_script* script = aug_load(s_gfx.vm, script_filepath);
	s_gfx.scripts.push_back(script);
}

bool aug_gfx_update()
{
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
			for(aug_script* script : s_gfx.scripts)
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
	for(aug_script* script : s_gfx.scripts)
		aug_call_args(s_gfx.vm, script, "Update", 1, &arg);
	return true;
}