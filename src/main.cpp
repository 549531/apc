#define INCBIN_PREFIX
#define INCBIN_STYLE INCBIN_STYLE_SNAKE

#include <GL/glew.h>
#include <SDL.h>
#include <incbin.h>

#include <cerrno>

// include shaders as strings
INCTXT(vshader_src, "../src/vertex_shader.glsl");
INCTXT(fshader_src, "../src/fragment_shader.glsl");

/// Prints compilation log of a resource (program or shader).
/// @param resource The resource's handle.
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

/// Creates and compiles a shader.
/// @param type The type of the shader.
/// @param src The shader's source code.
/// @returns the shader's handle.
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

/// Create an OpenGL buffer.
/// sizeof(@param data) must be equal to @param len.
GLuint makeBuffer(GLenum type, GLenum usage, void *data, size_t len) {
	GLuint buf;
	glGenBuffers(1, &buf);
	glBindBuffer(type, buf);
	glBufferData(type, len, data, usage);
	return buf;
}

int main(int argc, char **argv) {
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window *win = SDL_CreateWindow(
	    "Main window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,  //
	    500, 500, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	if (!win) {
		SDL_Log("SDL_CreateWindow(): %s", SDL_GetError());
		return 1;
	}

	SDL_GLContext ctx = SDL_GL_CreateContext(win);
	if (!ctx) {
		SDL_Log("SDL_GL_CreateContext(): %s", SDL_GetError());
		return 1;
	}

	// Enable double buffering AKA vsync
	if (SDL_GL_SetSwapInterval(1) < 0) {
		SDL_Log("SDL_GL_SetSwapInterval(): %s", SDL_GetError());
	}

	// Initialize OpenGL functions
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

	err = glGetError();
	if (err != GL_NO_ERROR) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			     "while binding opengl program: %s",
			     gluErrorString(err));
		return 1;
	}

	// Get a handle to the in_pos variable in the vertex shader.
	// This handle will be used to pass current pixel's coordinates
	// to the vertex shader.
	GLint position = glGetAttribLocation(prog, "in_pos");
	if (position == -1) {
		SDL_LogError(
		    SDL_LOG_CATEGORY_APPLICATION,
		    "could not find variable `in_pos' in the vertex shader");
		return 1;
	}

	// This rectangle will be drawn on screen,
	// then the fragment shader will draw on this rectangle.
	GLfloat rectangle[][2] = {
	    {-1, -1},
	    {+1, -1},
	    {+1, +1},
	    {-1, +1},
	};
	GLuint indices[] = {0, 1, 2, 3};

	GLuint rectBuf = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, rectangle,
				    sizeof rectangle);

	GLuint indexBuf = makeBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW,
				     indices, sizeof indices);

	// Set background color
	glClearColor(0, 0, 0, 1);

	// Main event loop
	SDL_Event evt;
	bool running = true;
	while (running) {
		while (SDL_PollEvent(&evt)) {
			switch (evt.type) {
			case SDL_QUIT: running = false; break;
			case SDL_KEYDOWN:
				switch (evt.key.keysym.sym) {
				case SDLK_q: running = false; break;
				}
				break;
			}
		}

		// Begin rendering

		// FIXME: I have no idea what this does, but it works

		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(prog);
		glEnableVertexAttribArray(position);

		glBindBuffer(GL_ARRAY_BUFFER, rectBuf);
		glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE,
				      sizeof *rectangle, nullptr);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
		glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, nullptr);

		glDisableVertexAttribArray(position);
		glUseProgram(0);

		SDL_GL_SwapWindow(win);

		// Wait 16ms to get approx. 60 FPS
		// Actual FPS will be lower,
		// because event processing + rendering takes time
		SDL_Delay(16);
	}

	// TODO: clean up OpenGL resources

	SDL_DestroyWindow(win);
	SDL_Quit();
}
