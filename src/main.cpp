#include <SDL.h>

int main(int argc, char **argv) {
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window *win = SDL_CreateWindow(
	    "Main window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,  //
	    640, 480, SDL_WINDOW_SHOWN);
	if (!win) {
		SDL_Log("SDL_CreateWindow(): %s", SDL_GetError());
		return 1;
	}

	SDL_Event evt;
	bool running = true;
	while (running) {
		if (!SDL_WaitEvent(&evt)) {
			SDL_Log("SDL_WaitEvent(): %s", SDL_GetError());
			return 1;
		}

		switch (evt.type) {
		case SDL_QUIT: running = false; break;
		case SDL_KEYDOWN:
			switch (evt.key.keysym.sym) {
			case SDLK_q: running = false; break;
			}
			break;
		}
	}

	SDL_DestroyWindow(win);
	SDL_Quit();
}
