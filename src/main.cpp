#include <GL/glew.h>
#include <SDL.h>

#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>

struct GlShader {
	GLuint m_handle;

	GlShader(GLint type, std::string_view src) {
		m_handle = glCreateShader(type);

		char const *srcs[] = {src.data()};
		GLint sizes[] = {
		    static_cast<GLint>(src.size()),
		};
		glShaderSource(m_handle, 1, srcs, sizes);

		glCompileShader(m_handle);
		if (getParam(GL_COMPILE_STATUS) == GL_FALSE) {
			throw std::runtime_error{getInfoLog()};
		}
	}

	~GlShader() { glDeleteShader(m_handle); }

	GLint getParam(GLenum pname) const {
		GLint param;
		glGetShaderiv(m_handle, pname, &param);
		return param;
	}

	std::string getInfoLog() const {
		GLsizei cap = getParam(GL_INFO_LOG_LENGTH);
		auto buf = std::make_unique<char[]>(cap);
		GLsizei len;
		glGetShaderInfoLog(m_handle, cap, &len, buf.get());
		return std::string{buf.get(), static_cast<size_t>(len)};
	}
};

struct GlProgram {
	GLuint m_handle;

	GlProgram() { m_handle = glCreateProgram(); }

	~GlProgram() { glDeleteProgram(m_handle); }

	void attachShader(GlShader const &shader) {
		glAttachShader(m_handle, shader.m_handle);
	}

	void link() {
		glLinkProgram(m_handle);

		if (getParam(GL_LINK_STATUS) == GL_FALSE) {
			throw std::runtime_error{getInfoLog()};
		}
	}

	GLint getParam(GLenum pname) const {
		GLint param;
		glGetProgramiv(m_handle, pname, &param);
		return param;
	}

	std::string getInfoLog() const {
		GLsizei cap = getParam(GL_INFO_LOG_LENGTH);
		auto buf = std::make_unique<char[]>(cap);
		GLsizei len;
		glGetProgramInfoLog(m_handle, cap, &len, buf.get());
		return std::string{buf.get(), static_cast<size_t>(len)};
	}

	// Mark the program as current.
	// The following computations will be made by this program.
	void use() { glUseProgram(m_handle); }
};

// Represents an OpenGL buffer.
struct GlBuffer {
	GLenum m_buffer_type, m_usage, m_element_type;
	GLuint m_handle;
	size_t m_size, m_nmemb;

	template <typename ForwardIterator>
	GlBuffer(GLenum buffer_type, GLenum usage, GLenum element_type,
		 ForwardIterator begin, ForwardIterator end)
	    : m_buffer_type{buffer_type},
	      m_usage{usage},
	      m_element_type{element_type} {
		glGenBuffers(1, &m_handle);
		set(begin, end);
	}

	// Marks this buffer as current.
	// Further method calls will operate on this buffer.
	void bind() { glBindBuffer(m_buffer_type, m_handle); }

	// Copies data to the buffer.
	// sizeof(@data) must be equal to @size.
	template <typename ForwardIterator>
	void set(ForwardIterator begin, ForwardIterator end) {
		m_size = sizeof *begin;
		m_nmemb = end - begin;
		bind();
		glBufferData(m_buffer_type, m_size * m_nmemb, begin, m_usage);
	}

	void drawElements(GLenum mode) {
		bind();
		glDrawElements(mode, m_nmemb, m_element_type, nullptr);
	}
};

enum GlVariableType {
	GlVarUniform1ui,
	GlVarAttribute,
};

template <GlVariableType T>
struct GlVariable;

template <>
struct GlVariable<GlVarUniform1ui> {
	GLint m_handle;

	GlVariable(GlProgram const &program, GLchar const *name) {
		m_handle = glGetUniformLocation(program.m_handle, name);
		if (m_handle == -1) {
			throw std::runtime_error{
			    std::format("could not find uniform variable `{}' "
					"in the shader",
					name)};
		}
	}

	void set(GLuint value) { glUniform1ui(m_handle, value); }
};

template <>
struct GlVariable<GlVarAttribute> {
	GLint m_handle;
	GLenum m_type;

	GlVariable(GlProgram const &program, GLchar const *name, GLenum type)
	    : m_type{type} {
		m_handle = glGetAttribLocation(program.m_handle, name);
		if (m_handle == -1) {
			throw std::runtime_error{
			    std::format("could not find uniform variable `{}' "
					"in the shader",
					name)};
		}
	}

	void enable() { glEnableVertexAttribArray(m_handle); }

	void set(GlBuffer &buf, GLboolean normalized, GLint size,
		 GLsizei stride = 0) {
		if (stride == 0) {
			// by default, stride == size
			stride = size;
		}
		buf.bind();
		glVertexAttribPointer(m_handle, size, m_type, normalized,
				      stride * buf.m_size, nullptr);
	}
};

std::string read_file(std::string_view filename) {
	std::ifstream file{filename.data()};
	if (!file) {
		throw std::runtime_error{
		    std::format("could not open file {}", filename)};
	}
	return std::string{std::istreambuf_iterator<char>(file),
			   std::istreambuf_iterator<char>()};
}

int main(int argc, char **argv) {
	SDL_Init(SDL_INIT_EVERYTHING);

	try {
		auto win_xy = SDL_WINDOWPOS_CENTERED;
		auto win_wh = 500;
		auto win_attrs =
		    SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
		SDL_Window *win = SDL_CreateWindow(
		    "Main window", win_xy, win_xy, win_wh, win_wh, win_attrs);
		if (!win) {
			throw std::runtime_error{std::format(
			    "SDL_CreateWindow(): {}", SDL_GetError())};
		}

		SDL_GLContext ctx = SDL_GL_CreateContext(win);
		if (!ctx) {
			throw std::runtime_error{std::format(
			    "SDL_GL_CreateContext(): {}", SDL_GetError())};
		}

		// Enable double buffering AKA vsync
		if (SDL_GL_SetSwapInterval(1) < 0) {
			throw std::runtime_error{std::format(
			    "SDL_GL_SetSwapInterval(): {}", SDL_GetError())};
		}

		// Initialize OpenGL functions
		GLenum err = glewInit();
		if (err != GLEW_OK) {
			auto msg = reinterpret_cast<char const *>(
			    glewGetErrorString(err));
			throw std::runtime_error{
			    std::format("glewInit(): {}", msg)};
		}

		GlProgram program;

		{
			auto vertex_shader = GlShader{
			    GL_VERTEX_SHADER, read_file("src/shader.vert")};

			auto fragment_shader = GlShader{
			    GL_FRAGMENT_SHADER, read_file("src/shader.frag")};

			program.attachShader(vertex_shader);
			program.attachShader(fragment_shader);
			program.link();
			// shaders are not needed anymore, so they get deleted
		}

		// Get a handle to the in_pos variable in the vertex shader.
		// This handle will be used to pass current pixel's coordinates
		// to the vertex shader.
		GlVariable<GlVarAttribute> positionAttr{program, "in_pos",
							GL_FLOAT};

		// Get a handle to the time variable in the fragment shader.
		// This handle will be used to animate things.
		GlVariable<GlVarUniform1ui> timeUf{program, "time"};

		constexpr size_t point_count = 4;

		// This rectangle will be drawn on screen,
		// then the fragment shader will color this rectangle.
		std::array<std::array<GLfloat, 3>, point_count> rectangle{{
		    {-1, -1, 0},
		    {+1, -1, 0},
		    {+1, +1, 0},
		    {-1, +1, 0},
		}};

		std::array<GLuint, point_count> indices{{0, 1, 2, 3}};

		GlBuffer rectangleBuf{GL_ARRAY_BUFFER, GL_STATIC_DRAW, GL_FLOAT,
				      rectangle.begin(), rectangle.end()};

		GlBuffer indexBuf{GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW,
				  GL_UNSIGNED_INT, indices.begin(),
				  indices.end()};

		program.use();
		positionAttr.enable();

		// Set background color
		glClearColor(0, 0, 0, 1);

		// Main event loop
		SDL_Event evt;
		bool running = true;
		bool paused = false;
		Uint32 prev_ticks = 0, total_ticks = 0;

		// time between two consecutive frames
		const auto period =
		    SDL_GetPerformanceFrequency() / 60;  // 60fps

		while (running) {
			auto start = SDL_GetPerformanceCounter();

			// Process events (e.g. key presses)
			while (SDL_PollEvent(&evt)) {
				switch (evt.type) {
				case SDL_QUIT: running = false; break;
				case SDL_KEYDOWN:
					switch (evt.key.keysym.sym) {
					case SDLK_q: running = false; break;
					case SDLK_SPACE:
						paused = !paused;
						break;
					}
					break;
				case SDL_WINDOWEVENT:
					switch (evt.window.event) {
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						auto w = evt.window.data1;
						auto h = evt.window.data2;
						auto size = w < h ? w : h;
						glViewport((w - size) / 2,
							   (h - size) / 2, size,
							   size);
						break;
					}
					break;
				}
			}

			// Timekeeping: compute how much time passed
			// since last render
			auto ticks = SDL_GetTicks();
			auto delta = ticks - prev_ticks;
			prev_ticks = ticks;
			if (!paused) { total_ticks += delta; }

			// Begin rendering
			if (!paused) {
				// Send current time to the shader
				timeUf.set(total_ticks);

				// Send coordinates to the shader
				positionAttr.set(rectangleBuf, GL_FALSE, 2, 1);

				// Clear the buffer
				glClear(GL_COLOR_BUFFER_BIT);

				// Render to the buffer
				indexBuf.drawElements(GL_TRIANGLE_FAN);

				// Send the buffer to the screen
				SDL_GL_SwapWindow(win);

				std::clog << std::format(
				    "delta: {:3} fps: {:>6.03f}\n", delta,
				    1000. / delta);
			}

			auto timeSpent = SDL_GetPerformanceCounter() - start;

			// how much time till next frame?
			SDL_Delay((period - timeSpent % period) * 1000 /
				  SDL_GetPerformanceFrequency());
		}

		SDL_DestroyWindow(win);
	} catch (std::exception &e) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", e.what());
	}

	SDL_Quit();
}
