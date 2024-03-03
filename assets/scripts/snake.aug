import std;
import gfx;

var game_over = true; 
var pixel_size = 15;
var top_border = pixel_size * 2;
var w = 540; 
var h = 540;
var font_size = 15;
var speed = 0;
var pos = [0,0]; 
var velocity = [0,0]; 
var food =[0,0]; 
var snake = []; 
var snake_len = 0;
var input_map = {
	"Right" : [ 1, 0 ], 
	"Left"  : [-1, 0 ],
	"Up"    : [0, -1 ], 
	"Down"  : [0,  1 ],
};

GfxStartup();
var window = GfxCreateWindow("Snake", w, h + top_border);
var font = GfxFont("font/OpenSans.ttf", font_size);

var time_curr = GfxTime();
var time_prev;
var time_delta;
var running = true;

var fps_accum = 0;
var fps_count = 0;
var fps = 0; # rolling avg

while running {	
	time_prev = time_curr;
	time_curr = GfxTime();
	time_delta = (time_curr - time_prev) / GfxRate();

	fps_count += 1;
	fps_accum += 1/time_delta;
	if(fps_count > 100){
		fps = fps_accum / fps_count;
		fps_count = 0;
		fps_accum = 0;
	}

	w = GfxWindowWidth(window);
	h = GfxWindowHeight(window) - top_border;	

	# handle input
	var input = GfxPollInput();
	running = !( get(input, "window") == "close");
	KeyDown(get(input, "key_down"));
	Move(time_delta);

	Draw();
}

GfxShutdown();

func KeyDown(key){
	if key == none { return; }

	if key == "Escape" { 
		running = false; 
	}
	else if game_over and key == "Space"  { 
		Spawn();
	}

	var prev_vel = velocity;

	if exists(input_map, key) {
		velocity = input_map[key];
	}

	# disallow movement backwards. if next position will be at the previous body segment
	if snake_len > 1 {
		var next_pos = [ 
			snap(pos[0], pixel_size) + velocity[0] * pixel_size,
			snap(pos[1], pixel_size) + velocity[1] * pixel_size
		];

		if next_pos == snake[snake_len-2] {
			velocity = prev_vel; 
		}
	}
}

func Spawn(){
	game_over = false;
	speed = 100;
	velocity = [ 1, 0 ];
	pos = [ w / 2, h / 2];
	snake = [pos];
	snake_len = 1;
	SpawnFood();
}

func SpawnFood(){

	# respawn food
	food = [ snap(random(0, w), pixel_size), snap(random(top_border, h), pixel_size)];

	# if snake encompasses entire game board, victory
	if snake_len == (w / pixel_size) * (h / pixel_size) {
		game_over = true;
		return;
	}

	# avoid spawning in snake body. If did, move to next available food position
	if contains(snake, food){
		var xdir = 1;
		if random(0, 1) { 
			xdir = -1; 
		}
		var ydir = 1;
		if random(0, 1) { 
			ydir = -1; 
		}

		while contains(snake, food) {
			food[0] += pixel_size * xdir;
			if food[0] >= w {
				food[0] = 0;
				food[1] += pixel_size * ydir;
				if food[1] >= h {
					food[1] = 0;
				}
			}
		}
	}
}

func Move(delta){
	if game_over { 
		return; 
	}

	pos = [ 
		pos[0] + velocity[0] * delta * speed, 
		pos[1] + velocity[1] * delta * speed 
	];
	
	var cell_pos = [ 
		snap(pos[0], pixel_size),
		snap(pos[1], pixel_size)
	]; 

	# if has not moved since last frame
	if cell_pos == back(snake) { 
		return; 
	} 

	if  cell_pos[0] < 0 or cell_pos[0] > w or 
		cell_pos[1] < top_border or cell_pos[1] >= h + top_border {
		game_over = true;
		return;
	}

	# check if the head (back) is hitting body
	if contains(snake, back(snake), 0, snake_len - 1){
		game_over = true;
		return;
	}

	append(snake, cell_pos);

	if cell_pos == food { 
		speed += 10;
		snake_len += 1;
		SpawnFood(); 
	}else{
		remove(snake, 0);
	}
}


func Draw(){
	GfxClear(window);
	GfxDrawRect(window, 0, 0, w, top_border, 32, 32, 32, 255);
	if ! game_over {		
		GfxDrawRect(window, food[0], food[1], pixel_size, pixel_size, 255, 0, 0,255);
	}
	for pos in snake {
		GfxDrawRect(window, pos[0], pos[1], pixel_size, pixel_size, 255,255,255,255);
	}
	GfxText(window, font, concat("Score: ", to_string(snake_len)), 0, 0, 0, 255, 0, 255);

	if game_over {
		DrawTextLines("Hit Space to Start\nHit ESC to Exit", w/2, h/2, 180, 25, 15, 255);
	}

	GfxText(window, font, concat("FPS: ", to_string(fps)), w - 128, 0, 0, 255, 0, 255);
	GfxPresent(window);
}


func DrawTextLines(msg, ox, oy, r, g, b, a){
	var lines = split(msg, "\n");
	var dy = -1 * font_size * length(lines) / 2;
	for line in lines {
		var dx = -1 * (length(line) * font_size) / 4;
		GfxText(window, font, line, ox + dx, oy + dy, r,g,b,a);
		dy += font_size + 2;
	}
}

