#include <cerrno>
#include <cstring>

#define INCBIN_PREFIX
#define INCBIN_STYLE INCBIN_STYLE_SNAKE

#include <GL/glew.h>
#include <SDL.h>
#include <incbin.h>

INCTXT(vshader_src, "../src/vertex_shader.glsl");
INCTXT(fshader_src, "../src/fragment_shader.glsl");

void printLog(GLuint resource) {
	GLsizei len, cap;
	enum {
		prog,
		shad,
		other
	} type = glIsProgram(resource)  ? prog
		 : glIsShader(resource) ? shad
					: other;
	switch (type) {
	case prog: glGetProgramiv(resource, GL_INFO_LOG_LENGTH, &cap); break;
	case shad: glGetShaderiv(resource, GL_INFO_LOG_LENGTH, &cap); break;
	default: return;
	}
	GLchar *log = (GLchar *)malloc(cap);
	if (!log) {
		SDL_Log("malloc(): %s", strerror(errno));
		return;
	}
	switch (type) {
	case prog: glGetProgramInfoLog(resource, cap, &len, log); break;
	case shad: glGetShaderInfoLog(resource, cap, &len, log); break;
	default: return;
	}
	if (len > 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: %s",
			     type == prog   ? "program"
			     : type == shad ? "shader"
					    : "other",
			     log);
	}
	free(log);
}

GLuint makeShader(GLint type, GLchar const *src) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);

	GLint ok = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
	if (ok == GL_FALSE) {
		printLog(shader);
		return -1;
	}

	return shader;
}

int main(int argc, char **argv) {
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window *win = SDL_CreateWindow(
	    "Main window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,  //
	    640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	if (!win) {
		SDL_Log("SDL_CreateWindow(): %s", SDL_GetError());
		return 1;
	}

	SDL_GLContext ctx = SDL_GL_CreateContext(win);
	if (!ctx) {
		SDL_Log("SDL_GL_CreateContext(): %s", SDL_GetError());
		return 1;
	}

	if (SDL_GL_SetSwapInterval(1) < 0) {
		SDL_Log("SDL_GL_SetSwapInterval(): %s", SDL_GetError());
	}

	GLenum err = glewInit();
	if (err != GLEW_OK) {
		SDL_Log("glewInit(): %s", glewGetErrorString(err));
		return 1;
	}

	GLuint vshader = makeShader(GL_VERTEX_SHADER, vshader_src_data);
	GLuint fshader = makeShader(GL_FRAGMENT_SHADER, fshader_src_data);
	if (vshader == -1 || fshader == -1) return 1;

	GLuint prog = glCreateProgram();
	glAttachShader(prog, vshader);
	glAttachShader(prog, fshader);
	glLinkProgram(prog);

	GLint ok = GL_FALSE;
	glGetProgramiv(prog, GL_LINK_STATUS, &ok);
	if (ok == GL_FALSE) {
		printLog(prog);
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
