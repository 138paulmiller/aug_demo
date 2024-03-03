import std;
import gfx;

GfxStartup();

var window = GfxCreateWindow("AUG Demo", 540, 540);
var font = GfxFont("font/OpenSans.ttf", 12);

var time_curr = GfxTime();
var time_prev;
var time_delta;

var running = true;

while running {	
	time_prev = time_curr;
	time_curr = GfxTime();
	time_delta = (time_curr - time_prev) / GfxRate();

	var input = GfxPollInput();
	if get(input, "window") == "close" {
		running = false;
	} 

	GfxClear(window);

	var x = 128;
	var y = 128;
	GfxText(window, font, "Hello", x,y, 255, 255, 255, 255);
	GfxPresent(window);
}

GfxShutdown();
