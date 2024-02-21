func Startup(){
	Load("snake.aug");
}
func Shutdown(){}

func KeyUp(key){
	if(key == "Escape"){
		Exit();
	}
}

func KeyDown(key){
	if(key == "0"){
		Load("../snake.aug");
	}
}

func Update(delta){}