var width = 540;
var height = 540;
var pixel_size = 15;
var speed = 100;
var px = 0;
var py = 0;
var dx = 1;
var dy = 0;
var foodx;
var foody;
var snake;
var snake_len;
var game_over;
var victory;

func Startup(){
	GfxCreateWindow("Snake", width, height);

	Spawn();
}

func Shutdown(){
	GfxDestroyWindow();
}

func KeyUp(key){
	if(key == "Escape"){
		Exit();
	}

	if game_over {
		Spawn();
	}
}

func KeyDown(key){
	var pdx = dx;
	var pdy = dy;

	if(key == "Left"){
		dx = -1;
		dy = 0;
	}
	else if(key == "Right"){
		dx = 1;
		dy = 0;
	}
	else if(key == "Up"){
		dy = -1;
		dx = 0;
	}
	else if(key == "Down"){
		dy = 1;
		dx = 0;
	}

	# disallow movement backwards
	if snake_len > 1 {
		var x = snap(px, pixel_size) + dx * pixel_size; 
		var y = snap(py, pixel_size) + dy * pixel_size;
		if [x,y] == snake[snake_len-2] {
			dx = pdx;
			dy =  pdy;
		}
	}
}

func Update(delta){
	if game_over {
		GameOver();
		return;
	}

	if ! Move(delta) { 
		return; 
	}

	# draw food and snake
	GfxClear(0,0,0,255);
	GfxDrawRect(foodx, foody, pixel_size, pixel_size, 0,255,0,255);
	for pos in snake {
		GfxDrawRect(pos[0], pos[1], pixel_size, pixel_size, 255,255,255,255);
	}
	GfxPresent();
}

func Spawn(){
	px = width / 2;
	py = height / 2;
	game_over = false;
	victory = false;

	snake = [];
	append(snake, [px, py]);
	snake_len = 0;
	speed = 100;
	Eat();

	# start in random direction
	var c = random(0, 3);
	if c == 0 { 
		dx = 1; dy = 0;
	} else if c == 1 {
		dx = -1; dy = 0;
	} else if c == 2 {
		dx = 0; dy = 1;
	} else if c == 3 {
		dx = 0; dy = -1;
	}
}

func Eat(){
	speed += 10;
	snake_len += 1;

	foodx = random(0, width);
	foody = random(0, height);

	foodx = snap(foodx, pixel_size);
	foody = snap(foody, pixel_size);

	if contains(snake, [foodx, foody]){
		var fx = foodx;
		var fy = foody;
		while contains(snake, [foodx, foody]) {
			foodx += 1;
			if foodx >= width {
				foodx = 0;
				foody += 1;
				if foody >= height {
					foody = 0;
				}
			}

			if foodx == fx and foody == fy {
				game_over = true;
				victory = true;
				return;
			}
		}
	}
}

func GameOver(){
	GfxClear(32,0,0,255);
	GfxDrawRect(foodx, foody, pixel_size, pixel_size, 0,255,0,255);
	for pos in snake {
		GfxDrawRect(pos[0], pos[1], pixel_size, pixel_size, 255,255,255,255);
	}
	GfxPresent();
}

func Move(delta){
	px += dx * delta * speed;
	py += dy * delta * speed;
	
	var x = snap(px, pixel_size); 
	var y = snap(py, pixel_size);

	# if has not moved since last frame
	var head = snake[length(snake) - 1];
	if x == head[0] and y == head[1] {
		return true;
	} 

	# check border
	if x < 0 or x >= width or y < 0 or y >= height{
		game_over = true;
		return false;
	}

	# if the head hit the body
	var i = 0;
	while i < snake_len - 1 {
		if snake[i] == head {
			game_over = true;
			return false;
		}
		i += 1;
	}

	if x == foodx and y == foody {
		Eat();
	}

	# otherwise, grow snake, shrink tail if exceeding length
	append(snake, [x, y]);
	if length(snake) > snake_len {
		remove(snake, 0);
	}

	return true;
}
