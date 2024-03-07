#include "NetworkingPhysics.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using namespace std;

static const char* vertex_shader_text =
"#version 330\n"
"in vec2 vPos;\n"
"in vec3 vCol;\n"
"uniform mat4 MVP;\n"
"out vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    color = vCol;\n"
"}\n";

static const char* fragment_shader_text =
"#version 330\n"
"in vec3 color;\n"
"out vec4 fragColor;\n"
"void main()\n"
"{\n"
"    fragColor = vec4(color, 1.0);\n"
"}\n";

static const struct Vertex
{
	float x, y;
	float r, g, b;
} vertices[3] =
{
	{ -0.8660254f, -0.5f, 1.f, 1.f, 0.f },
	{  0.8660254f, -0.5f, 0.f, 1.f, 1.f },
	{   0.0f,  1.0f, 1.f, 0.f, 1.f }
};

constexpr auto COUNT_TRIANGLES = 10;

static const uint32 indices[3] = { 0, 1, 2 };
static b2Body* walls[4];
static b2Body* triangles[COUNT_TRIANGLES];


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_E && action == GLFW_PRESS);
}

void error_callback(int error, const char* description)
{
	cerr << "Error: " << description << endl;
}

void create_world_bounds(const unique_ptr<b2World>& world) {
	b2BodyDef bodyDef;

	// wall 1
	bodyDef.position.Set(0, -5);
	walls[0] = world->CreateBody(&bodyDef);

	// wall 2
	bodyDef.position.Set(0, 5);
	walls[1] = world->CreateBody(&bodyDef);

	// wall 3
	bodyDef.position.Set(5, 0);
	walls[2] = world->CreateBody(&bodyDef);

	// wall 4
	bodyDef.position.Set(-5, 0);
	walls[3] = world->CreateBody(&bodyDef);

	b2PolygonShape polygonShapeV;
	polygonShapeV.SetAsBox(0.5f, 5);

	b2PolygonShape polygonShapeH;
	polygonShapeH.SetAsBox(5, 0.5f);

	walls[0]->CreateFixture(&polygonShapeH, 0.0f);
	walls[1]->CreateFixture(&polygonShapeH, 0.0f);
	walls[2]->CreateFixture(&polygonShapeV, 0.0f);
	walls[3]->CreateFixture(&polygonShapeV, 0.0f);
}

void create_physics_triangles(const unique_ptr<b2World>& world) {
	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.type = b2_dynamicBody;
	dynamicBodyDef.angle = 0;
	dynamicBodyDef.linearVelocity.Set(0, 15);

	const b2Vec2 triangle[3] = {
		{ -0.8660254f, -0.5f },
		{  0.8660254f, -0.5f },
		{   0.0f,  1.0f }
	};

	b2PolygonShape dynamicShape;
	dynamicShape.Set(triangle, 3);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &dynamicShape;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.3f;
	fixtureDef.restitution = 1;

	for (int i = 0; i < COUNT_TRIANGLES; i++) {
		dynamicBodyDef.position.Set(static_cast<float>((i % 2 - 2) * 2), static_cast<float>(i / 2));
		
		triangles[i] = world->CreateBody(&dynamicBodyDef);
		triangles[i]->CreateFixture(&fixtureDef);
	}
}

int main()
{
	if (!glfwInit()) return -1;
	glfwSetErrorCallback(error_callback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	GLFWwindow* window = glfwCreateWindow(1280, 720, "Synced Physics", nullptr, nullptr);

	if (!window) {
		error_callback(-1, "Window creation failed.");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	gladLoadGL(glfwGetProcAddress);
	glfwSwapInterval(1);

	glfwSetKeyCallback(window, key_callback);

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	ImGui::StyleColorsDark();

	GLuint vertex_buffer, index_buffer, vertex_array, vertex_shader, fragment_shader, program;

	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);

	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
	glCompileShader(vertex_shader);

	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
	glCompileShader(fragment_shader);

	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	GLint mvp_location, vpos_location, vcol_location;

	mvp_location = glGetUniformLocation(program, "MVP");
	vpos_location = glGetAttribLocation(program, "vPos");
	vcol_location = glGetAttribLocation(program, "vCol");

	glEnableVertexAttribArray(vpos_location);
	glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
		sizeof(vertices[0]), (void*)0);

	glEnableVertexAttribArray(vcol_location);
	glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
		sizeof(vertices[0]), (void*)(sizeof(float) * 2));

	unique_ptr<b2World> world = make_unique<b2World>(b2Vec2(0, -9.81f));

	create_world_bounds(world);

	create_physics_triangles(world);

	float gravityModifier = 0;

	while (!glfwWindowShouldClose(window))
	{
		float clearColor[3] = { 0.2f, 0.2f, 0.2f };

		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Test Window");

		ImGui::SliderFloat("Test Slider", &gravityModifier, 0, 1);
		ImGui::ColorPicker3("Clear Color", clearColor);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

		ImGui::End();

		world->SetGravity({ 0, -9.81f * gravityModifier });

		float ratio;
		int width, height;
		mat4x4 m, v, p, mv, mvp;

		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;

		glViewport(0, 0, width, height);
		glClearColor(clearColor[0], clearColor[1], clearColor[2], 1);
		glClear(GL_COLOR_BUFFER_BIT);

		world->Step(1 / 60.0f, 6, 2);

		// Set camera
		mat4x4_identity(v);
		mat4x4_translate(v, 0, 0, 0);
		mat4x4_invert(v, v);

		float zoom = 5;

		// Projection
		mat4x4_ortho(p, -ratio * zoom, ratio* zoom, -zoom, zoom, 1.f, -1.f);

		glUseProgram(program);
		glBindVertexArray(vertex_array);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

		for (b2Body* triangle : triangles) {
			mat4x4_identity(m);
			auto& pos = triangle->GetPosition();
			mat4x4_translate_in_place(m, pos.x, pos.y, 0);
			mat4x4_rotate_Z(m, m, triangle->GetAngle());

			mat4x4_mul(mv, v, m);
			mat4x4_mul(mvp, p, mv);

			glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
			//glDrawElementsInstanced(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr, COUNT_TRIANGLES);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
		}

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