var window;
var font;
var pixel_size = 15;
var top_border = pixel_size * 2;
var font_size = 15;
var speed;
var w; var h;
var px; var py;
var dx; var dy;
var foodx; var foody;
var snake; var snake_len;
var waiting_start; var game_over; var victory;

func Startup(){
	w = 540; h = 540;
	window = GfxCreateWindow("Snake", w, h + top_border);
	font = GfxFont("font/OpenSans.ttf", font_size);
	snake = [];
	waiting_start = true;
}

func Shutdown(){
	GfxDestroyWindow(window);
}

func KeyUp(key){
	if(key == "Escape") { 
		Exit(); 
	}
	else if waiting_start { 
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
	if snake_len and snake_len > 1 {
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

	if ! waiting_start {		
		GfxDrawRect(window, foodx, foody, pixel_size, pixel_size, 255, 0, 0,255);
	}

	for pos in snake {
		GfxDrawRect(window, pos[0], pos[1], pixel_size, pixel_size, 255,255,255,255);
	}

	GfxText(window, font, concat("Score: ", to_string(snake_len)), 0, 0, 0, 255, 0, 255);

	if game_over {
		DrawText("Game Over\nHit any Key to Restart\nHit ESC to Exit", w/2, h/2, 180, 25, 15, 255);
	}
	else if waiting_start {
		DrawText("Hit any Key to Start\nHit ESC to Exit", w/2, h/2, 180, 25, 15, 255);
	}

	GfxPresent(window);
}

func Spawn(){
	waiting_start = false;
	game_over = false;
	victory = false;
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
		waiting_start = true;
		victory = true;
		return;
	}

	# avoid spawning in snake body. If did, move to next available food position
	if contains(snake, [foodx, foody]){
		var fx = foodx;
		var fy = foody;
		while contains(snake, [foodx, foody]) {
			foodx += 1;
			if foodx >= w {
				foodx = 0;
				foody += 1;
				if foody >= h {
					foody = 0;
				}
			}
		}
	}
}

func Move(delta){
	if waiting_start or game_over { return; }

	px += dx * delta * speed;
	py += dy * delta * speed;
	
	var x = snap(px, pixel_size); 
	var y = snap(py, pixel_size);

	# if has not moved since last frame
	var head = back(snake);
	if x == head[0] and y == head[1] { 
		return; 
	} 

	# check if snake hit border
	if x < 0 or x >= w or y < top_border or y >= h {
		game_over = true;
		waiting_start = true;
		return;
	}

	# check if the head hit the body
	var i = 0;
	while i < snake_len - 1 {
		if snake[i] == head {
			game_over = true;
			waiting_start = true;
			return;
		}
		i += 1;
	}

	if x == foodx and y == foody { 
		Eat(); 
	}

	append(snake, [x, y]);
	if length(snake) > snake_len {
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