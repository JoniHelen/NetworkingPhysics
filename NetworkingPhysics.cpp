#include "NetworkingPhysics.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using namespace std;

namespace {
	struct Vertex {
		vec2 pos;
		vec3 col;
	};

	constexpr Vertex vertices[3] = {
		{ -0.4330127f, -0.25f, 1.0f, 1.0f, 0.0f },
		{  0.4330127f, -0.25f, 0.0f, 1.0f, 1.0f },
		{   0.0f,  0.5f, 1.0f, 0.0f, 1.0f }
	};

	constexpr int COUNT_TRIANGLES = 50;

	constexpr uint32 indices[3] = { 0u, 1u, 2u };
	b2Body* walls[4u];
	b2Body* triangles[COUNT_TRIANGLES];
	mat4x4 triangleTransforms[COUNT_TRIANGLES];

	/**
	 * \brief Creates walls around the scene with physics objects.
	 * \param world The world in which the objects are instantiated
	 */
	void create_world_bounds(const unique_ptr<b2World>& world) {
		// Blank physics body template
		b2BodyDef bodyDef;

		// wall 1
		bodyDef.position.Set(0, -7);
		walls[0] = world->CreateBody(&bodyDef);

		// wall 2
		bodyDef.position.Set(0, 7);
		walls[1] = world->CreateBody(&bodyDef);

		// wall 3
		bodyDef.position.Set(7, 0);
		walls[2] = world->CreateBody(&bodyDef);

		// wall 4
		bodyDef.position.Set(-7, 0);
		walls[3] = world->CreateBody(&bodyDef);

		// Vertical and horizontal wall shapes
		b2PolygonShape polygonShapeV;
		polygonShapeV.SetAsBox(0.5f, 20);

		b2PolygonShape polygonShapeH;
		polygonShapeH.SetAsBox(20, 0.5f);

		// Attach shapes to bodies and make them have infinite mass
		walls[0]->CreateFixture(&polygonShapeH, 0.0f);
		walls[1]->CreateFixture(&polygonShapeH, 0.0f);
		walls[2]->CreateFixture(&polygonShapeV, 0.0f);
		walls[3]->CreateFixture(&polygonShapeV, 0.0f);
	}

	/**
	 * \brief Creates triangular physics bodies into the world.
	 * \param world The world in which the triangles are instantiated
	 */
	void create_physics_triangles(const unique_ptr<b2World>& world) {

		// Convert vertices into b2Vec2 array
		const b2Vec2 triangle[3] = {
			{ vertices[0].pos[0], vertices[0].pos[1] },
			{ vertices[1].pos[0], vertices[1].pos[1] },
			{ vertices[2].pos[0], vertices[2].pos[1] }
		};

		//  Define fixture
		b2PolygonShape triangleShape;
		triangleShape.Set(triangle, 3);

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &triangleShape;
		fixtureDef.density = 10.0f;
		fixtureDef.friction = 0.3f;
		fixtureDef.restitution = 1.1f;

		// Define physics body
		b2BodyDef dynamicBodyDef;
		dynamicBodyDef.type = b2_dynamicBody;

		// Create triangle objects
		for (int i = 0; i < COUNT_TRIANGLES; i++) {
			dynamicBodyDef.linearVelocity.Set(static_cast<float>((i % 10 - 4)), static_cast<float>(i / 10));
			dynamicBodyDef.position.Set(static_cast<float>((i % 10 - 4)), static_cast<float>(i / 10));

			triangles[i] = world->CreateBody(&dynamicBodyDef);
			triangles[i]->CreateFixture(&fixtureDef);
		}
	}

	/**
	 * \brief Resets the simulation to its starting state.
	 */
	void reset_simulation() {
		for (int i = 0; i < COUNT_TRIANGLES; i++) {
			triangles[i]->SetLinearVelocity(b2Vec2(static_cast<float>((i % 10 - 4)), static_cast<float>(i / 10)));
			triangles[i]->SetTransform(b2Vec2(static_cast<float>((i % 10 - 4)), static_cast<float>(i / 10)), 0);
			triangles[i]->SetAngularVelocity(0);
		}
	}

	/**
	* \brief Key callback for GLFW.
	*/
	void key_callback(GLFWwindow* const window, const int key, const int scancode, const int action, const int mods) {
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			reset_simulation();
		}
	}

	/**
	* \brief Error callback for GLFW.
	*/
	void error_callback(const int error, const char* const description)
	{
		cerr << "Error: " << description << endl;
	}

	/**
	* \brief Initializes a GLFW window.
	*/
	GLFWwindow* init_window()
	{
		if (!glfwInit()) exit(-1);
		glfwSetErrorCallback(error_callback);

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

		GLFWwindow* window = glfwCreateWindow(1280, 720, "Synced Physics", nullptr, nullptr);

		if (!window) {
			error_callback(-1, "Window creation failed.");
			glfwTerminate();
			exit(-1);
		}

		glfwMakeContextCurrent(window);
		glfwSwapInterval(1);

		glfwSetKeyCallback(window, key_callback);

		return window;
	}

	/**
	* \brief Runs ImGui initialization functions.
	*/
	void init_ImGui(GLFWwindow* const window)
	{
		ImGui::CreateContext();

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 330");

		ImGui::StyleColorsDark();
	}

	/**
	* \brief Reads text from file with provided filename. For use with shader files.
	* \param filename The name of the file to read
	*/
	const string read_shader_from_file(const string& filename) {
		ifstream file(filename);

		if (file.fail()) {
			cout << "File " << filename << " not found." << endl;
			return "";
		}

		stringstream ss;
		ss << file.rdbuf();

		return ss.str();
	}

	/*
	* \brief Genrates a shader program using predefined file extensions and locations.
	* \param name The name of the shader to generate. Used in filename
	*/
	GLuint generate_shader_program(const string& name)
	{
		auto vertex_text = read_shader_from_file(name + ".vert.glsl");
		auto fragment_text = read_shader_from_file(name + ".frag.glsl");

		auto vt = vertex_text.c_str();
		auto ft = fragment_text.c_str();

		const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, &vt, nullptr);
		glCompileShader(vertex_shader);

		const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, &ft, nullptr);
		glCompileShader(fragment_shader);

		const GLuint program = glCreateProgram();
		glAttachShader(program, vertex_shader);
		glAttachShader(program, fragment_shader);
		glLinkProgram(program);

		return program;
	}

	void generate_triangle_buffers(const GLuint& program, GLuint& vertex_buffer, GLuint& transform_buffer, GLuint& index_buffer, GLuint& vertex_array)
	{
		// Generate and bind VAO
		glGenVertexArrays(1, &vertex_array);
		glBindVertexArray(vertex_array);

		// Generate and bind VBO
		glGenBuffers(1, &vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		// Specify vertex attributes
		const GLint vpos_location = glGetAttribLocation(program, "PositionOS");

		glEnableVertexAttribArray(vpos_location);
		glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
			sizeof(vertices[0]), static_cast<void*>(nullptr));

		const GLint vcol_location = glGetAttribLocation(program, "Color");

		glEnableVertexAttribArray(vcol_location);
		glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
			sizeof(vertices[0]), reinterpret_cast<void*>(sizeof(float) * 2));

		// Generate and bind instancing transform buffer
		glGenBuffers(1, &transform_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, transform_buffer);
		glBufferData(GL_ARRAY_BUFFER, COUNT_TRIANGLES * sizeof(mat4x4), triangleTransforms, GL_STATIC_DRAW);

		// Specify transform matrix attribute for 4 attribute slots
		const GLint model_location = glGetAttribLocation(program, "ModelMatrix");

		glEnableVertexAttribArray(model_location);
		glVertexAttribPointer(model_location, 4, GL_FLOAT, GL_FALSE,
			sizeof(mat4x4), static_cast<void*>(nullptr));

		glEnableVertexAttribArray(model_location + 1);
		glVertexAttribPointer(model_location + 1, 4, GL_FLOAT, GL_FALSE,
			sizeof(mat4x4), reinterpret_cast<void*>(1 * sizeof(vec4)));

		glEnableVertexAttribArray(model_location + 2);
		glVertexAttribPointer(model_location + 2, 4, GL_FLOAT, GL_FALSE,
			sizeof(mat4x4), reinterpret_cast<void*>(2 * sizeof(vec4)));

		glEnableVertexAttribArray(model_location + 3);
		glVertexAttribPointer(model_location + 3, 4, GL_FLOAT, GL_FALSE,
			sizeof(mat4x4), reinterpret_cast<void*>(3 * sizeof(vec4)));

		// Set divisors for instancing
		glVertexAttribDivisor(model_location, 1);
		glVertexAttribDivisor(model_location + 1, 1);
		glVertexAttribDivisor(model_location + 2, 1);
		glVertexAttribDivisor(model_location + 3, 1);

		// Generate and bind index buffer
		glGenBuffers(1, &index_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		// Clear gl state
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}

int main()
{
	GLFWwindow* window = init_window();
	gladLoadGL(glfwGetProcAddress);
	init_ImGui(window);
	const ImGuiIO& io = ImGui::GetIO(); (void)io;

	const GLuint program = generate_shader_program("triangle");

	const GLint v_location = glGetUniformLocation(program, "ViewMatrix");
	const GLint p_location = glGetUniformLocation(program, "ProjMatrix");

	GLuint vertex_buffer, transform_buffer, index_buffer, vertex_array;
	generate_triangle_buffers(program, vertex_buffer, transform_buffer, index_buffer, vertex_array);

	const auto world = make_unique<b2World>(b2Vec2(0, -9.81f));

	create_world_bounds(world);

	create_physics_triangles(world);

	const auto rate = glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate;

	float gravityModifier = 0;
	float clearColor[3] = { 0.2f, 0.2f, 0.2f };

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Test Window");

		ImGui::SliderFloat("Gravity factor", &gravityModifier, 0, 1);
		ImGui::ColorPicker3("Clear Color", clearColor);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

		ImGui::End();

		world->SetGravity({ 0, -9.81f * gravityModifier });

		int width, height;
		mat4x4 m, v, p;

		glfwGetFramebufferSize(window, &width, &height);
		const float ratio = static_cast<float>(width) / static_cast<float>(height);

		glViewport(0, 0, width, height);
		glClearColor(clearColor[0], clearColor[1], clearColor[2], 1);
		glClear(GL_COLOR_BUFFER_BIT);

		world->Step(1.0f / static_cast<float>(rate), 20, 10);

		// Set camera
		mat4x4_identity(v);
		mat4x4_translate_in_place(v, 0, 0, 0);
		mat4x4_invert(v, v);

		constexpr float zoom = 7;

		// Projection
		mat4x4_ortho(p, -ratio * zoom, ratio* zoom, -zoom, zoom, 1.0f, -1.0f);

		for (int i = 0; i < COUNT_TRIANGLES; i++) {
			mat4x4_identity(m);
			auto& pos = triangles[i]->GetPosition();
			mat4x4_translate_in_place(m, pos.x, pos.y, 0);
			mat4x4_rotate_Z(m, m, triangles[i]->GetAngle());
			mat4x4_dup(triangleTransforms[i], m);
		}

		glUseProgram(program);
		glBindVertexArray(vertex_array);

		glBindBuffer(GL_ARRAY_BUFFER, transform_buffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, COUNT_TRIANGLES * sizeof(mat4x4), triangleTransforms);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

		glUniformMatrix4fv(v_location, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(v));
		glUniformMatrix4fv(p_location, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(p));

		glDrawElementsInstanced(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr, COUNT_TRIANGLES);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}