import std;
import gfx;

var fg_color = [32, 32, 32, 255]
var bg_color = [100, 120, 94, 255] 

var game_over = true; 
var cell_size = 15;
var top_border = cell_size * 2;
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

var window = GfxStartup("Snake", w, h + top_border);
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

	w = GfxWindowWidth();
	h = GfxWindowHeight() - top_border;	

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
			snap(pos[0], cell_size) + velocity[0] * cell_size,
			snap(pos[1], cell_size) + velocity[1] * cell_size
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
	food = [ snap(random(0, w), cell_size), snap(random(top_border, h), cell_size)];

	# if snake encompasses entire game board, victory
	if snake_len == (w / cell_size) * (h / cell_size) {
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
			food[0] += cell_size * xdir;
			if food[0] >= w {
				food[0] = 0;
				food[1] += cell_size * ydir;
				if food[1] >= h {
					food[1] = 0;
				}
			}
		}
	}
}

func Move(delta){
	if game_over 
		return

	pos = [ 
		pos[0] + velocity[0] * delta * speed, 
		pos[1] + velocity[1] * delta * speed 
	];
	
	var cell_pos = [ 
		snap(pos[0], cell_size),
		snap(pos[1], cell_size)
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
	GfxClear(bg_color);
	GfxDrawRect(0, 0, w, top_border, fg_color);
	if ! game_over {		
		var fx = food[0];	
		var fy = food[1];
		var fsz = cell_size / 3;
		GfxDrawRect(fx+fsz,   fy,       fsz, fsz, fg_color);
		GfxDrawRect(fx,       fy+fsz,   fsz, fsz, fg_color);
		GfxDrawRect(fx+fsz*2, fy+fsz,   fsz, fsz, fg_color);
		GfxDrawRect(fx+fsz,   fy+fsz*2, fsz, fsz, fg_color);
	}
	for pos in snake {
		GfxDrawRect(pos[0], pos[1], cell_size-2, cell_size-2, fg_color);
	}

	if game_over {
		DrawTextLines("Hit Space to Start\nHit ESC to Exit", w/2, h/2, fg_color);
	}

	GfxText(font, concat("Score: ", to_string(snake_len)), 0, 0, bg_color);
	GfxText(font, concat("FPS: ", to_string(fps)), w - 74, 0, bg_color);
	GfxPresent();
}


func DrawTextLines(msg, ox, oy, color){
	var lines = split(msg, "\n");
	var dy = -1 * font_size * length(lines) / 2;
	for line in lines {
		var dx = -1 * (length(line) * font_size) / 4;
		GfxText(font, line, ox + dx, oy + dy, color);
		dy += font_size + 2;
	}
}

