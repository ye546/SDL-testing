#include "SDL.h"
#include "SDL_image.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <cstdlib>


#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720


SDL_Texture* texture(const char* path, SDL_Renderer* renderer);
bool inside_bounds_check(SDL_Rect &box);
void stretchW(SDL_Rect& rect, SDL_Event &event);
void stretchH(SDL_Rect& rect, SDL_Event &event);
bool win_condition_met(SDL_Rect* player, SDL_Rect* goal);
bool has_intersection(SDL_Rect* player, SDL_Rect* wall);
bool collision(SDL_Rect* rect1, SDL_Rect& rect2);
void draw_lines(SDL_Renderer* rend, int x1, int y1, int x2, int y2, SDL_Rect* box1);

class Block {
public:
	SDL_Rect box;
	SDL_Texture* texture;
	//int id;
};

class Goal {
public:
	SDL_Rect goal;
	SDL_Texture * texture;
};


//store the blocks
std::vector<Block*> blocks;



SDL_Texture* texture(const char* path, SDL_Renderer* renderer) {
	SDL_Surface* surf = nullptr;
	SDL_Texture* texture = nullptr;

	surf = IMG_Load(path);

	if (!surf)
		printf("%s", SDL_GetError());

	texture = SDL_CreateTextureFromSurface(renderer, surf);
	if (!texture)
		printf("%s", SDL_GetError());

	SDL_FreeSurface(surf);
	return texture;
}

//checks if the mouse is inside the bounds
bool inside_bounds_check(SDL_Rect &box) {
	int x(0), y(0);
	SDL_GetMouseState(&x, &y);
	if ((x > box.x) && (x < box.x + box.w) && (y > box.y) && (y < box.y + box.h)) {
		//printf("I am inside the bounds!\n");
		return true;
	}
	return false;
}


void stretchW(SDL_Rect& rect, SDL_Event &event) {
	if (inside_bounds_check(rect)) {
		if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
			if (event.type == SDL_MOUSEWHEEL)
			{
				if (event.wheel.y > 0)
					rect.w+=5;
				if (event.wheel.y < 0)
					rect.w-=5;
				if (event.wheel.y = 0)
					rect.w = rect.w;
			}
		}
	}

	if (rect.w < 5)
		rect.w = 5;
}


void stretchH(SDL_Rect& rect, SDL_Event &event) {
	if (inside_bounds_check(rect)) {
		if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
			if (event.type == SDL_MOUSEWHEEL)
			{
				if (event.wheel.y > 0)
					rect.h += 10;
				if (event.wheel.y < 0)
					rect.h -= 10;
				if (event.wheel.y = 0)
					rect.h = rect.h;
			}
		}
	}

	if (rect.h < 5)
		rect.h = 5;
}

bool win_condition_met(SDL_Rect* player, SDL_Rect* goal) {
	if (has_intersection(player, goal))
		return true;
	
	return false;
}

bool has_intersection(SDL_Rect* player, SDL_Rect* wall) {
	if (player->x < wall->x + wall->w &&
		player->x + player->w > wall->x &&
		player->y < wall->y + wall->h &&
		player->y + player->h > wall->y)
	{
		return true;
	}

	return false;
}


bool collision(SDL_Rect* rect1,SDL_Rect& rect2) {
	if (rect1->x >= (rect2.x + rect2.w))
		return false;
	if ((rect1->x + rect1->w) <= rect2.x)
		return false;
	if (rect1->y >= (rect2.y + rect2.h))
		return false;
	if ((rect1->y + rect1->h) <= rect2.y)
		return false;

	return true;
}

 void draw_lines(SDL_Renderer* rend, int x1, int y1, int x2, int y2, SDL_Rect* box1) {
	 //midpoint for movable box
	 x1 = (box1->x + box1->w / 2);
	 y1 = (box1->y + box1->h / 2);

	 //calculate midpoint for all boxes
	 
	 for (Block* i : blocks) {
		 x2 = (i->box.x + i->box.w / 2);
		 y2 = (i->box.y + i->box.h / 2);
		 //set color for line and draw to respective x and y
		 SDL_SetRenderDrawColor(rend, 255, 255, 0, 255);
		 SDL_RenderDrawLine(rend, x2, y2, x1, y1);
	 }
}

int main(int argc, char** argv) {
	srand(time(NULL));
	SDL_Init(SDL_INIT_VIDEO); //SDL_INIT_AUDIO 

	SDL_Window* window = SDL_CreateWindow("lol", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_Surface* surf = SDL_GetWindowSurface(window);
	
	//movable box with wasd
	SDL_Rect box1 = { 0, 0, 100, 100 };

	//ptr to rect
	SDL_Rect* pBox1 = &box1;

	//texture for movable box
	SDL_Texture* t_box1 = texture("purplebox.png", renderer);

	//array for random textures to apply to spawnable boxes
	std::string rand_texture[] = { "redbox.png", "bluebox.png" };

	Goal* goal = new Goal;
	goal->texture = texture("greenbox.png", renderer);
	goal->goal = {(1280-50), (720-50), 50, 50 };

	SDL_Rect* pGoal = &goal->goal;

	//´time calculation
	Uint32 start;
	//set fps
	const int fps = 144;

	//toggles between adjusting width and height of rect
	bool toggle = false;

	//toggle to draw lines between rects
	bool render_lines = false;

	//flag for movement, if 1 then walk
	bool flags[4] = { 0, 0, 0, 0 };

	bool isRunning = true;

	while (isRunning){
		//mouse
		SDL_PumpEvents();
		//event handling, etc keyboard
		SDL_Event event;
		
		start = SDL_GetTicks();

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		
		//handle events
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) isRunning = 0;

			//toggle render
			if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_r && !render_lines)
					render_lines = true;
				else if (event.key.keysym.sym == SDLK_r && render_lines)
					render_lines = false;
			}

			if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_1 && !toggle) {
					toggle = true;
					printf("change height\n");
				}
				else if (event.key.keysym.sym == SDLK_1 && toggle) {
					toggle = false;
					printf("change width\n");
				}
			}

			switch (event.type) 
			{
			case SDL_MOUSEBUTTONUP:
				if (event.button.button == SDL_BUTTON_LEFT) {
					int x(0), y(0);
					SDL_GetMouseState(&x, &y);

					Block* new_block = new Block;
					new_block->box = { x, y, 100, 100 };
					new_block->texture = texture(rand_texture[rand() % sizeof(rand_texture) / sizeof(rand_texture[0])].c_str(), renderer);
					//new_block->id = i++;

					blocks.push_back(new_block);

					printf("left clicked\n");
				}
				break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym) {
						case SDLK_w:
							flags[0] = 1;
							break;
						case SDLK_a:
							flags[1] = 1;
							break;
						case SDLK_s:
							flags[2] = 1;
							break;
						case SDLK_d:
							flags[3] = 1;
							break;
					}
				break;
				case SDL_KEYUP:
					switch (event.key.keysym.sym) {
						case SDLK_w :
						flags[0] = 0;
						break;
					case SDLK_a:
						flags[1] = 0;
						break;
					case SDLK_s:
						flags[2] = 0;
						break;
					case SDLK_d:
						flags[3] = 0;
						break;
					}
					break;
			}
		}

		if (flags[0])
		{
			pBox1->y-=5;
			//this method allows for multidetection collision
			for(Block* i : blocks)
				if (collision(pBox1, i->box))
				pBox1->y+=5;
		}
		if (flags[1])
		{
			pBox1->x-=5;
			for (Block* i : blocks)
			if (collision(pBox1, i->box))
				pBox1->x+=5;
		}
		if (flags[2])
		{
			pBox1->y+=5;
			for (Block* i : blocks)
			if (collision(pBox1, i->box))
				pBox1->y-=5;
		}
		if (flags[3])
		{
			pBox1->x+=5;
			for (Block* i : blocks)
			if (collision(pBox1, i->box))
				pBox1->x-= 5;
		}

		if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
			if (!blocks.empty()) {
				int x(0), y(0);
				
				SDL_GetMouseState(&x, &y);
				for (Block* i : blocks) {
					if(inside_bounds_check(i->box)) {
						i->box.x = x - i->box.w / 2;
						i->box.y = y - i->box.h / 2;
					}
				}
			}
		}

		//player
		SDL_RenderCopy(renderer, t_box1, NULL, &box1);
		//goal
		SDL_RenderCopy(renderer, goal->texture, NULL, &goal->goal);

		//if we created a box, render it
		if (!blocks.empty()) {
			//render all the rects with respective texture
			for (Block* i : blocks) {
				SDL_RenderCopy(renderer, i->texture, NULL, &i->box);
				
				//toggle rendering lines to main block
				if (render_lines)
					draw_lines(renderer, i->box.x, i->box.y, pBox1->x, pBox1->y, pBox1);
				
				//scaling blocks
				if (toggle)
					stretchH(i->box, event);
				if(!toggle)
					stretchW(i->box, event);			
			}
		}

		SDL_RenderPresent(renderer);

		//check if wincondition is met
		if (win_condition_met(pBox1, pGoal))
			isRunning = false;

		if (1000 / fps > SDL_GetTicks() - start)
			SDL_Delay(1000 / fps - (SDL_GetTicks() - start));
	
	}

	for (Block* i : blocks) {
		SDL_DestroyTexture(i->texture);
	}
	
	SDL_DestroyTexture(t_box1);
	SDL_FreeSurface(surf);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
}
