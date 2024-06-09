#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <ncurses.h>
#include <math.h>
#include <time.h>
#include <pthread.h> // потоки
#define CHARACTER '#'
#define FPS 100
void RandFill(char** map, int y, int x){
	srand(time(NULL));
	int r;
	for (int i = 0; i < y; i++){
		for (int j = 0; j < x; j++){
		    r = rand() % 8;
			if((r == 1)){
				map[i][j] = CHARACTER;		
			}
			else map[i][j] = ' ';
		}
	}	

}

void FillNull(char** map, int y, int x){
	for (int i = 0; i < y; i++){
		for (int j = 0; j < x; j++){
		     map[i][j] = ' ';
		}
	}
}

void ClearMap(char** map, int y, int x){
	for (int i = 0; i < y; i++){
		for (int j = 0; j < x; j++){
		     if(map[i][j] != CHARACTER) map[i][j] = ' ';
		}
	}

}
int CountLife(int y, int x, char** map, int max_y, int max_x){
	int LifeNeib = 0;
	int x_1, y_1;
	for (int i = y-1; i <= y+1; i++){
		for (int j = x-1; j <= x+1; j++){
			y_1 = i;
			x_1 = j;
			if((y_1 == y) && (x_1 == x)){
				continue;
			}
			if((y_1 < 0) || (y_1 >= max_y-1)) continue;//y_1 = abs(abs(y_1) - max_y); // second for TOR canvas
			if((x_1 < 0) || (x_1 >= max_x-1)) continue;//x_1 = abs(abs(x_1) - max_x);
			if((map[y_1][x_1] == CHARACTER)){
				LifeNeib++;
			}
		}
	}
	return LifeNeib;
}

void PrintMap(char** map, int y, int x){
	move(0,0);
	clrtoeol();
	for(int i =0; i <= x; i++){ 
		if (i > 0) printw("_");
		else printw(" ");

	}
	printw("\n");
	for (int i = 0; i < y; i++){
		clrtoeol(); // Очистка текущей строки
        move(i+1, 0);
		printw("|");
		for (int j = 0; j < x; j++){
			printw("%c", map[i][j]);
		}
		printw("|");
		printw("\n");
	}
	move(y,0);
	clrtoeol();
	for(int i =0; i <= x; i++){ 
		if (i > 0) printw("-");
		else printw(" ");

	}
	printw("\n");
	refresh();
	
}

int EndGame(char** map, char** mapNext, int y, int x){
	for (int i = 0; i < y; i++){
		for (int j = 0; j < x; j++){
			if(map[i][j] != mapNext[i][j]) return 0;
		}
	}
	return 1;
}

int ReloadFlag = 0, ExitFlag = 0, HandleInitFlag = 0, ThreadFlag = 0;
char Change;

pthread_mutex_t handler = PTHREAD_MUTEX_INITIALIZER;;

void* thread_function1(void* arg) {
    while(1){
    	if(ThreadFlag) continue;
        //getch();
        Change = getch();
        pthread_mutex_lock(&handler);
        if(Change == 'R') ReloadFlag = 1; // flag to reload current game with random init
        else if(Change == 'H') HandleInitFlag =1; // flag to reload current game with handle init 
        else if(Change == 'Q'){ // flag to quit
        	ExitFlag = 1;
        	return NULL;
        }
        pthread_mutex_unlock(&handler);
    
   }
     return NULL;
}

void HandleInit(char** map,int y, int x){
	ThreadFlag = 1;
	char key;
	int coord[2] = {y/2, x/2};
	int DrawFlag = 1; // flag to draw
	mvprintw(y/2, 0, "DRAW USE WASD, PRESED (\"SHIFT + D\") TO SWITCH DRAW MODE!\nPRESS ENTER TO END DRAW!\n \"SHIFT + H\" TO DRAW AGAIN!\n \"SHIFT + R\" TO RANDOM INIT!\n \"SHIFT + Q\" TO QUIT");
	getch();
	FillNull(map, y ,x); // init map with ' '
	PrintMap(map, y, x);
	do{ // move
		key = getch();
		if(key == 'D'){
			DrawFlag++;
		}
		if(key == 'w'){
			if(coord[0] < 1){
				continue;
			}
            coord[0]--;
        }
        else if(key == 's'){
            coord[0]++;
            if(coord[0] > y-1){
			coord[0]--;
			continue;
			}
        }
        else if(key == 'a'){
            if(coord[1] < 1){
				continue;
			}
            coord[1]--;
        }
        else if(key == 'd'){
            if(coord[1] > x-2){
				continue;
			}
            coord[1]++;
        }

        if((DrawFlag%2) == 0){
			map[coord[0]][coord[1]] = CHARACTER;
		}
		else{
			ClearMap(map, y, x);
			if((map[coord[0]][coord[1]] == CHARACTER) && (key != '\n')){
				map[coord[0]][coord[1]] = ' ';
			}
			map[coord[0]][coord[1]] = '.';
		}
		PrintMap(map, y, x);
		
    }
	while(key != '\n');
	ThreadFlag = 0;

	
}

void SwapMapNextToMap(char** map, char** mapNext, int y, int x){
	for (int i = 0; i < y; i++){
		for (int j = 0; j < x; j++){
			mapNext[i][j] = map[i][j];
		}
	}
}

int main(){
	initscr();
	noecho();
	raw();
	//nodelay(stdscr, TRUE);

	pthread_t keyboarding_handler;
	
	int x = COLS-3; // Получение количества столбцов (ширины)
    int y = LINES-2; // Получение количества строк (высоты)
	char** map = (char**)malloc(sizeof(char*) * y);
	for(int i =0; i < y; i++){
		map[i] = (char*)malloc(sizeof(char) * x);
	}
	char** mapNext = (char**)malloc(sizeof(char*) * y);
	for(int i =0; i < y; i++){
		mapNext[i] = (char*)malloc(sizeof(char) * x);
	}
	char** mapPrevPerFour = (char**)malloc(sizeof(char*) * y);
	for(int i =0; i < y; i++){
		mapPrevPerFour[i] = (char*)malloc(sizeof(char) * x);
	}
	int neighbors, count = 0;

	HandleInit(map, y, x);
	PrintMap(map, y, x);
	
    // Создайте и запустите поток обработки клавиш after hadnle init!!!
    if (pthread_create(&keyboarding_handler, NULL, thread_function1, NULL) != 0) {
        perror("pthread_create");
        return 1;
    }
    SwapMapNextToMap(map, mapNext, y, x);
	while(!ExitFlag){
		ReloadFlag = 0;
		HandleInitFlag = 0;
		pthread_mutex_lock(&handler);
		PrintMap(map, y, x);
		pthread_mutex_unlock(&handler);
		usleep(1000000/FPS); // delay
	    //if(ExitFlag) break;

		if((count % 4) == 0){ // если игра зациклилась
			count = 0;
			for (int i = 0; i < y; i++){
				for (int j = 0; j < x; j++){
					mapPrevPerFour[i][j] = map[i][j];
				}
			}
		}
	
		for (int i = 1; i < y-1; i++){ // изменение карты по правилам
			for (int j = 1; j < x-1; j++){
				neighbors = CountLife(i,j,map, y, x);
				if(map[i][j] == CHARACTER){
					if((neighbors < 2) || (neighbors > 3)){ // rule 1
						mapNext[i][j] = ' ';
					}
				}
				else{
					if(neighbors  == 3 ){ // rule 2
						mapNext[i][j] = CHARACTER;
					}
					else continue;
				}

			}
		}
		count++;

		if(HandleInitFlag){
			count = 0;
			HandleInit(map, y, x);
			SwapMapNextToMap(map, mapNext, y, x);
			continue;
		}
		if((EndGame(mapPrevPerFour, mapNext, y, x)) || ReloadFlag){
			count = 0;
			usleep(1000000/FPS);
			//FillNull(map, y, x);
			RandFill(map, y, x);
			SwapMapNextToMap(map, mapNext, y, x);
				//continue;
		}
		for (int i = 0; i < y; i++){
			for (int j = 0; j < x; j++){
				map[i][j] = mapNext[i][j];
			}
		}
	}
	pthread_join(keyboarding_handler, NULL);
	endwin();
	return 0;
}