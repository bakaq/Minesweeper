#include <iostream>
#include <SDL2/SDL.h>
#include <vector>
#include <stdlib.h>

const int SCREEN_W = 640;
const int SCREEN_H = 480;

void set_minefield(std::vector<std::vector<int>> *minefield){
	int num_bombs = 10;

	for(int i = 0; i < num_bombs; ++i){
		int y = rand()%8;
		int x = rand()%8;

		if((*minefield)[y][x] == -1){
			--i;
		}else{
			(*minefield)[y][x] = -1;
		}
	}

	// Calculate the numbers
	for(int i = 0; i < 8; ++i){
		for(int j = 0; j < 8; ++j){
			if((*minefield)[i][j] == -1) continue;

			int bombs = 0;

			bool top    = (i == 0); 
			bool bottom = (i == 7); 
			bool left   = (j == 0); 
			bool right  = (j == 7);

			if(!top)    if((*minefield)[i-1][j] == -1) ++bombs; 
			if(!bottom) if((*minefield)[i+1][j] == -1) ++bombs; 
			if(!left)   if((*minefield)[i][j-1] == -1) ++bombs; 
			if(!right)  if((*minefield)[i][j+1] == -1) ++bombs;
				
			if(!top    && !left)  if((*minefield)[i-1][j-1] == -1) ++bombs;
			if(!top    && !right) if((*minefield)[i-1][j+1] == -1) ++bombs;
			if(!bottom && !left)  if((*minefield)[i+1][j-1] == -1) ++bombs;
			if(!bottom && !right) if((*minefield)[i+1][j+1] == -1) ++bombs;
		
			
			(*minefield)[i][j] = bombs;
		}
	}
}

void open_mask(std::vector<std::vector<bool>> *mask, std::vector<std::vector<int>> *minefield, int y, int x){
	bool iz = ((*minefield)[y][x] == 0) && !(*mask)[y][x];
	
	(*mask)[y][x] = true;	

	bool top    = (y == 0); 
	bool bottom = (y == 7); 
	bool left   = (x == 0); 
	bool right  = (x == 7);
	

	if(!top)    if(((*minefield)[y-1][x] == 0 && !(*mask)[y-1][x]) || iz) open_mask(mask, minefield, y-1, x);
	if(!bottom) if(((*minefield)[y+1][x] == 0 && !(*mask)[y+1][x]) || iz) open_mask(mask, minefield, y+1, x); 
	if(!left)   if(((*minefield)[y][x-1] == 0 && !(*mask)[y][x-1]) || iz) open_mask(mask, minefield, y, x-1); 
	if(!right)  if(((*minefield)[y][x+1] == 0 && !(*mask)[y][x+1]) || iz) open_mask(mask, minefield, y, x+1); 
		
	if(!top    && !left)  if(((*minefield)[y-1][x-1] == 0 && !(*mask)[y-1][x-1]) || iz) open_mask(mask, minefield, y-1, x-1);
	if(!top    && !right) if(((*minefield)[y-1][x+1] == 0 && !(*mask)[y-1][x+1]) || iz) open_mask(mask, minefield, y-1, x+1);
	if(!bottom && !left)  if(((*minefield)[y+1][x-1] == 0 && !(*mask)[y+1][x-1]) || iz) open_mask(mask, minefield, y+1, x-1);
	if(!bottom && !right) if(((*minefield)[y+1][x+1] == 0 && !(*mask)[y+1][x+1]) || iz) open_mask(mask, minefield, y+1, x+1);
}

int main(){
	// Minefield
	std::vector<std::vector<int>> minefield(8, std::vector<int>(8, 0));

	std::vector<std::vector<bool>> field_mask(8, std::vector<bool>(8, false));
	std::vector<std::vector<bool>> flag_mask(8, std::vector<bool>(8, false));

	std::vector<SDL_Rect> rects;
	
	set_minefield(&minefield);

	flag_mask[4][4] = true;

	// Fill rects
	for(int i = 0; i < 8; ++i){
		for(int j = 0; j < 8; ++j){
			rects.push_back(SDL_Rect{20+50*j, 20+50*i, 40, 40});
		}
	}

	// Declare and init stuff
	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;

	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow("Minesweeper",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			SCREEN_W, SCREEN_H, SDL_WINDOW_SHOWN);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_SetRenderDrawColor(renderer, 0xFF,0xFF,0xFF,0xFF);

	// Main loop
	bool running = true;
	bool lost = false;
	SDL_Event e;
	while(running){
		bool mouse_pressed = false;
		bool flagging = false;
		// Events
		while(SDL_PollEvent(&e) != 0){
			if(e.type == SDL_QUIT){
				running = false;
			}
			if(e.type == SDL_MOUSEBUTTONDOWN){
				if(e.button.button == SDL_BUTTON_LEFT){
					mouse_pressed = true;
				}
				if(e.button.button == SDL_BUTTON_RIGHT){
					flagging = true;
				}
			}
		}
		
		// Open tile
		if(mouse_pressed){
			int mouse_x, mouse_y;
			SDL_GetMouseState(&mouse_x, &mouse_y);
			SDL_Point mouse{mouse_x, mouse_y};

			for(int i = 0; i < 8; ++i){
				for(int j = 0; j < 8; ++j){
					if(SDL_PointInRect(&mouse, &rects[j+8*i])){
						open_mask(&field_mask, &minefield, i, j);
						if(minefield[i][j] == -1){
							lost = true;
							running = false;
						}
					}
				}
			}			
		}

		// Flag tile
		if(flagging){
			int mouse_x, mouse_y;
			SDL_GetMouseState(&mouse_x, &mouse_y);
			SDL_Point mouse{mouse_x, mouse_y};

			for(int i = 0; i < 8; ++i){
				for(int j = 0; j < 8; ++j){
					if(SDL_PointInRect(&mouse, &rects[j+8*i])){
						flag_mask[i][j] = !flag_mask[i][j];
					}
				}
			}			
		}

		// Clear renderer
		SDL_SetRenderDrawColor(renderer, 0xFF,0xFF,0xFF,0xFF);
		SDL_RenderClear(renderer);
		
		// Draw minefield
		for(int i = 0; i < 8; ++i){
			for(int j = 0; j < 8; ++j){
				if(field_mask[i][j]){
					// Open
					if(minefield[i][j] == -1){
						// If bomb, draw red
						SDL_SetRenderDrawColor(renderer, 0xFF,0x00,0x00,0xFF);
					}else{
						SDL_SetRenderDrawColor(renderer, 0xD0,0xD0,0xD0,0xFF);
					}
					SDL_RenderFillRect(renderer, &rects[j+8*i]);
					
					// "Write" number 
					if(minefield[i][j] > 0){
						SDL_SetRenderDrawColor(renderer, 0x00,0x00,0x00,0x00);
						int line_x = rects[j+8*i].x + 6;
						int line_y = rects[j+8*i].y + 5;
						for(int k = 0; k < minefield[i][j]; ++k){
							SDL_RenderDrawLine(renderer, line_x+4*k, line_y, line_x+4*k, line_y+30);
						}
					}
				}else{
					// Closed
					SDL_SetRenderDrawColor(renderer, 0x70,0x70,0x70,0xFF);
					SDL_RenderFillRect(renderer, &rects[j+8*i]);
					// Draw flag
					if(flag_mask[i][j]){
						SDL_SetRenderDrawColor(renderer, 0x00,0x00,0xFF,0xFF);
						int rect_x = rects[j+8*i].x;
						int rect_y = rects[j+8*i].y;
						SDL_RenderDrawLine(renderer, rect_x, rect_y, rect_x + 39, rect_y + 39);
						SDL_RenderDrawLine(renderer, rect_x + 39, rect_y, rect_x, rect_y + 39);
					}
				}
			}
		}

		// Update screen
		SDL_RenderPresent(renderer);

		// Slowdown
		SDL_Delay(1000/600);
	}
	
	if(lost){
		SDL_Delay(750);
	}

	// Free stuff
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
