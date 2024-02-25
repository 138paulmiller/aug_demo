use std;

var game_over; 
var window;
var font;
var font_size = 15;
var pixel_size = 15;
var top_border = pixel_size * 2;
var speed;
var w; var h;
var px; var py;
var dx; var dy;
var foodx; var foody;
var snake; var snake_len;

func Startup(){
	w = 540; h = 540;
	window = GfxCreateWindow("Snake", w, h + top_border);
	font = GfxFont("font/OpenSans.ttf", font_size);
	snake = [];
	snake_len = 0;
	game_over = true;
}

func Shutdown(){
	GfxDestroyWindow(window);
}

func KeyUp(key){
	if(key == "Escape") { 
		Exit(); 
	}
	else if game_over { 
		Spawn();
	}
}

func KeyDown(key){
	var pdx = dx;
	var pdy = dy;

	if(key == "Left"){
		dx = -1; dy = 0;
	}
	else if(key == "Right"){
		dx = 1; dy = 0;
	}
	else if(key == "Up"){
		dy = -1; dx = 0;
	}
	else if(key == "Down"){
		dy = 1; dx = 0;
	}

	# disallow movement backwards
	if snake_len > 1 {
		var x = snap(px, pixel_size) + dx * pixel_size; 
		var y = snap(py, pixel_size) + dy * pixel_size;
		if [x,y] == snake[snake_len-2] {
			dx = pdx; dy = pdy;
		}
	}
}

func Update(delta){
	w = GfxWindowWidth(window);
	h = GfxWindowHeight(window) - top_border;	
	Move(delta);

	GfxClear(window);

	GfxDrawRect(window, 0, 0, w, top_border, 32, 32, 32, 255);

	if ! game_over {		
		GfxDrawRect(window, foodx, foody, pixel_size, pixel_size, 255, 0, 0,255);
	}

	for pos in snake {
		GfxDrawRect(window, pos[0], pos[1], pixel_size, pixel_size, 255,255,255,255);
	}

	GfxText(window, font, concat("Score: ", to_string(snake_len)), 0, 0, 0, 255, 0, 255);

	if game_over {
		DrawText("Hit any Key to Start\nHit ESC to Exit", w/2, h/2, 180, 25, 15, 255);
	}

	GfxPresent(window);
}

func Spawn(){
	game_over = false;
	snake = [];
	snake_len = 0;
	speed = 100;
	dx = 1; dy = 0;
	px = w / 2;
	py = h / 2;
	append(snake, [px, py]);

	Eat();
}

func Eat(){
	speed += 10;
	snake_len += 1;

	# respawn food
	foodx = snap(random(0, w), pixel_size);
	foody = snap(random(0, h), pixel_size);

	# if snake encompasses entire game board, victory
	if snake_len == (w / pixel_size) * (h / pixel_size) {
		game_over = true;
		return;
	}

	# avoid spawning in snake body. If did, move to next available food position
	if contains(snake, [foodx, foody]){
		var xdir = 1;
		if random(0, 1) { 
			xdir = -1; 
		}
		var ydir = 1;
		if random(0, 1) { 
			ydir = -1; 
		}

		var fx = foodx;
		var fy = foody;
		while contains(snake, [foodx, foody]) {
			foodx += pixel_size * xdir;
			if foodx >= w {
				foodx = 0;
				foody += pixel_size * ydir;
				if foody >= h {
					foody = 0;
				}
			}
		}
	}
}

func Move(delta){
	if game_over { return; }

	px += dx * delta * speed;
	py += dy * delta * speed;
	
	var x = snap(px, pixel_size); 
	var y = snap(py, pixel_size);

	var head = back(snake);

	# if has not moved since last frame
	if [x, y] == head { 
		return; 
	} 

	# check if snake hit border
	if x < 0 or x > w or y < top_border or y >= h + top_border {
		game_over = true;
		return;
	}

	# check if the head hit the body. if the head position is within remaining body 
	if contains(snake, head, 0, snake_len - 1){
		game_over = true;
		return;
	}

	append(snake, [x, y]);
	if x == foodx and y == foody { 
		Eat(); 
	}
	else{
		remove(snake, 0);
	}

	return;
}

func DrawText(msg, ox, oy, r, g, b, a){
	var lines = split(msg, "\n");
	var dy = -1 * font_size * length(lines) / 2;
	for line in lines {
		var dx = -1 * (length(line) * font_size) / 4;
		GfxText(window, font, line, ox + dx, oy + dy, r,g,b,a);
		dy += font_size + 2;
	}
}