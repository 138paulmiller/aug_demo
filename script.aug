var renderer;
var window;
var width = 720;
var height = 540;
 
func Startup(){
	SDL_Init();
	window = SDL_CreateWindow("Test", 0, 0, width, height); 
	renderer = SDL_CreateRenderer(window);
}

func Shutdown(){
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

func BeginDraw(){
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
}

func EndDraw(){
	SDL_RenderPresent(renderer);
}

func DrawRect(x, y, w, h, r, g, b){
	SDL_SetRenderDrawColor(renderer, r, g, b, 255);
	SDL_RenderFillRect(renderer, x, y, w, h);
}

var x = 0;
var y = 0;
var w = 30;  
var h = 20;

func Update(delta){

	x += (delta * 20) % (width - w); 
	y += (delta * 30) % (height - h); 
	
	BeginDraw();
	
	var r = 255;
	var g = delta * 100;
	var b = 0; 
	DrawRect(x, y, w, h, r,g,b);
	
	EndDraw();
}
