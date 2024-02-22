#include "NetworkingPhysics.h"

using namespace std;

static const char* vertex_shader_text =
"#version 110\n"
"uniform mat4 MVP;\n"
"attribute vec3 vCol;\n"
"attribute vec2 vPos;\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    color = vCol;\n"
"}\n";

static const char* fragment_shader_text =
"#version 110\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(color, 1.0);\n"
"}\n";

static const struct
{
	float x, y;
	float r, g, b;
} vertices[3] =
{
	{ -0.6f, -0.4f, 1.f, 1.f, 0.f },
	{  0.6f, -0.4f, 0.f, 1.f, 1.f },
	{   0.f,  0.6f, 1.f, 0.f, 1.f }
};

void error_callback(int error, const char* description)
{
	cerr << "Error: " << description << endl;
}

int main()
{
	if (!glfwInit()) return -1;
	glfwSetErrorCallback(error_callback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
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

	GLuint vertex_buffer, vertex_shader, fragment_shader, program;

	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

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
	b2BodyDef bodyDef;
	bodyDef.position.Set(0, -7);
	b2Body* body = world->CreateBody(&bodyDef);

	b2PolygonShape polygonShape;
	polygonShape.SetAsBox(5, 5);

	body->CreateFixture(&polygonShape, 0.0f);

	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.type = b2_dynamicBody;
	dynamicBodyDef.position.Set(0.0f, 0.0f);
	dynamicBodyDef.angle = 1;
	b2Body* dynamicBody = world->CreateBody(&dynamicBodyDef);

	const b2Vec2 triangle[3] = {
		{ -0.6f, -0.4f },
		{  0.6f, -0.4f },
		{   0.f,  0.6f }
	};

	b2PolygonShape dynamicShape;
	dynamicShape.Set(triangle, 3);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &dynamicShape;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.3f;

	dynamicBody->CreateFixture(&fixtureDef);

	while (!glfwWindowShouldClose(window))
	{
		world->Step(1 / 60.0f, 6, 2);

		float ratio;
		int width, height;
		mat4x4 m, v, p, mvp;

		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;

		glViewport(0, 0, width, height);
		glClearColor(0.2f, 0.2f, 0.2f, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		// Move triangle
		mat4x4_identity(m);
		mat4x4_rotate_Z(m, m, dynamicBody->GetAngle());
		auto pos = dynamicBody->GetPosition();
		mat4x4_translate(m, pos.x, pos.y, 0);

		// Set camera
		mat4x4_identity(v);
		mat4x4_translate(v, 0, 0, 2);
		mat4x4_invert(v, v);

		// Projection
		mat4x4_perspective(p, b2_pi / 2, ratio, 0.003f, 1000.0f);

		// mvp
		mat4x4_mul(v, v, m);
		mat4x4_mul(mvp, p, v);

		glUseProgram(program);
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}