
#define AUG_IMPLEMENTATION
#include <aug.h>
#include <vector>
#include <cstdio>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

aug_value AUG_SDL_Init(int argc, aug_value* args)
{
	return aug_create_int(SDL_Init(SDL_INIT_EVERYTHING)); 
}

aug_value AUG_SDL_Quit(int argc, aug_value* args)
{
	SDL_Quit(); 
	return aug_none();
}

aug_value AUG_SDL_CreateWindow(int argc, aug_value* args)
{
	if (argc == 5)
	{
		const aug_string* title = args[0].str;
		const int x = aug_to_int(&args[1]);
		const int y = aug_to_int(&args[2]);
		const int w = aug_to_int(&args[3]);
		const int h = aug_to_int(&args[4]);
		return aug_create_user_data(SDL_CreateWindow(title->buffer, x, y, w, h, SDL_WINDOW_SHOWN)); 
	}

	return aug_none();
}

aug_value AUG_SDL_DestroyWindow(int argc, aug_value* args)
{
	if (argc == 1)
	{
		SDL_Window* renderer = (SDL_Window*)args[0].userdata;
		SDL_DestroyWindow(renderer);
	}
	return aug_none();
}

aug_value AUG_SDL_CreateRenderer(int argc, aug_value* args)
{
	if (argc == 1)
	{
		SDL_Window* window = (SDL_Window*)args[0].userdata;
		aug_create_user_data(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
	}
	return aug_none();
}


aug_value AUG_SDL_DestroyRenderer(int argc, aug_value* args)
{
	if (argc == 1)
	{
		SDL_Renderer* renderer = (SDL_Renderer*)args[0].userdata;
		SDL_DestroyRenderer(renderer);
	}
	return aug_none();
}

aug_value AUG_SDL_SetRenderDrawColor(int argc, aug_value* args)
{
	if (argc == 5)
	{
		SDL_Renderer* renderer = (SDL_Renderer*)args[0].userdata;
		const int r = aug_to_int(&args[1]);
		const int g = aug_to_int(&args[2]);
		const int b = aug_to_int(&args[3]);
		const int a = aug_to_int(&args[4]);
		SDL_SetRenderDrawColor(renderer, r, g, b, a);
	}
	return aug_none();
}

aug_value AUG_SDL_RenderFillRect(int argc, aug_value* args)
{
	if (argc == 5)
	{
		SDL_Renderer* renderer = (SDL_Renderer*)args[0].userdata;

		SDL_Rect rect;
		rect.x = aug_to_int(&args[1]);
		rect.y = aug_to_int(&args[2]);
		rect.w = aug_to_int(&args[3]); 
		rect.h = aug_to_int(&args[4]);
		SDL_RenderFillRect(renderer, &rect);
	}
	return aug_none();
}

aug_value AUG_SDL_RenderClear(int argc, aug_value* args)
{
	if (argc == 1)
	{
		SDL_Renderer* renderer = (SDL_Renderer*)args[0].userdata;
		SDL_RenderClear(renderer);
	}
	return aug_none();
}


aug_value AUG_SDL_RenderPresent(int argc, aug_value* args)
{
	if (argc == 1)
	{
		SDL_Renderer* renderer = (SDL_Renderer*)args[0].userdata;
		SDL_RenderPresent(renderer);
	}
	return aug_none();
}

void AUG_Error(const char* message)
{
	fprintf(stderr, "[ERROR]\t%s\n", message);
}

int main(int argc, char* argv[])
{
	aug_vm* vm = aug_startup(AUG_Error);

	aug_register(vm, "SDL_Init", AUG_SDL_Init);	
	aug_register(vm, "SDL_Quit", AUG_SDL_Quit);
	aug_register(vm, "SDL_CreateWindow", AUG_SDL_CreateWindow);
	aug_register(vm, "SDL_DestroyWindow", AUG_SDL_DestroyWindow);	
	aug_register(vm, "SDL_CreateRenderer", AUG_SDL_CreateRenderer);
	aug_register(vm, "SDL_DestroyRenderer", AUG_SDL_DestroyRenderer);
	aug_register(vm, "SDL_SetRenderDrawColor", AUG_SDL_SetRenderDrawColor);
	aug_register(vm, "SDL_RenderFillRect", AUG_SDL_RenderFillRect);
	aug_register(vm, "SDL_RenderFillRect", AUG_SDL_RenderFillRect);
	aug_register(vm, "SDL_RenderClear", AUG_SDL_RenderClear);
	aug_register(vm, "SDL_RenderPresent", AUG_SDL_RenderPresent);
	
	// Load all the scripts
	aug_script* script = aug_load(vm, "script.aug");
	aug_call(vm, script, "Startup");

	Uint64 time_current = SDL_GetPerformanceCounter();
	Uint64 time_prev = 0;
	float time_delta = 0;

	bool running = true;
	SDL_Event event;
	while (running)
	{
		time_prev = time_current;
		time_current = SDL_GetPerformanceCounter();
		time_delta = (time_current - time_prev) / (float)SDL_GetPerformanceFrequency();

		//printf("\033[01;1H %.3f             ", 1 / time_delta);

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_q)
					running = false;
				break;
			case SDL_KEYUP:
				break;
			case SDL_WINDOWEVENT:
				switch (event.window.event) 
				{
				case SDL_WINDOWEVENT_CLOSE:
					running = false;
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}
		}

		aug_value args[] = { aug_create_float(time_delta) };
		aug_call_args(vm, script, "Update", 1, &args[0]);

	}

	aug_call(vm, script, "Shutdown");
	aug_unload(vm, script);
	aug_shutdown(vm);

	return 0;
}
