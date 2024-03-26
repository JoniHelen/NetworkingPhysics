#include <pch.h>
#include <NetworkingPhysics.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace NetPhysics {
	void CreateWorldBounds(const std::unique_ptr<b2World>& world) {
		// Blank physics body template
		b2BodyDef bodyDef;

		// wall 1
		bodyDef.position.Set(0, -7);
		Walls[0] = world->CreateBody(&bodyDef);

		// wall 2
		bodyDef.position.Set(0, 7);
		Walls[1] = world->CreateBody(&bodyDef);

		// wall 3
		bodyDef.position.Set(7, 0);
		Walls[2] = world->CreateBody(&bodyDef);

		// wall 4
		bodyDef.position.Set(-7, 0);
		Walls[3] = world->CreateBody(&bodyDef);

		// Vertical and horizontal wall shapes
		b2PolygonShape polygonShapeV;
		polygonShapeV.SetAsBox(0.5f, 20);

		b2PolygonShape polygonShapeH;
		polygonShapeH.SetAsBox(20, 0.5f);

		// Attach shapes to bodies and make them have infinite mass
		Walls[0]->CreateFixture(&polygonShapeH, 0.0f);
		Walls[1]->CreateFixture(&polygonShapeH, 0.0f);
		Walls[2]->CreateFixture(&polygonShapeV, 0.0f);
		Walls[3]->CreateFixture(&polygonShapeV, 0.0f);
	}

	void CreatePhysicsTriangles(const std::unique_ptr<b2World>& world) {

		// Convert vertices into b2Vec2 array
		const b2Vec2 triangle[3] {
			{ TriangleVertices[0].Position[0], TriangleVertices[0].Position[1] },
			{ TriangleVertices[1].Position[0], TriangleVertices[1].Position[1] },
			{ TriangleVertices[2].Position[0], TriangleVertices[2].Position[1] }
		};

		//  Define fixture
		b2PolygonShape triangleShape;
		triangleShape.Set(triangle, 3);

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &triangleShape;
		fixtureDef.density = 10.0f;
		fixtureDef.friction = 0.3f;
		fixtureDef.restitution = 1.0f;

		// Define physics body
		b2BodyDef dynamicBodyDef;
		dynamicBodyDef.type = b2_dynamicBody;

		// Create triangle objects
		for (int i = 0; i < COUNT_TRIANGLES; i++) {
			dynamicBodyDef.linearVelocity.Set(static_cast<float>((i % 10) - 5), static_cast<float>(i / 10) - 5);
			dynamicBodyDef.position.Set(static_cast<float>((i % 10) - 5), static_cast<float>(i / 10) - 5);

			Triangles[i] = world->CreateBody(&dynamicBodyDef);
			Triangles[i]->CreateFixture(&fixtureDef);
		}
	}

	void ResetSimulation() {
		for (int i = 0; i < COUNT_TRIANGLES; i++) {
			Triangles[i]->SetLinearVelocity(b2Vec2(static_cast<float>((i % 10 - 4)), static_cast<float>(i / 10)));
			Triangles[i]->SetTransform(b2Vec2(static_cast<float>((i % 10 - 4)), static_cast<float>(i / 10)), 0);
			Triangles[i]->SetAngularVelocity(0);
		}
	}

	void KeyCallback(GLFWwindow* const window, const int key, const int scancode, const int action, const int mods) {
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			ResetSimulation();
		}
	}

	void ErrorCallback(const int error, const char* const description) {
		std::cerr << "Error " << error << ": " << description << std::endl;
	}

	GLFWwindow* InitWindow() {
		if (!glfwInit()) exit(-1);
		glfwSetErrorCallback(ErrorCallback);

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

		GLFWwindow* window = glfwCreateWindow(1280, 720, "Synced Physics", nullptr, nullptr);

		if (!window) {
			ErrorCallback(-1, "Window creation failed.");
			glfwTerminate();
			exit(-1);
		}

		glfwMakeContextCurrent(window);
		glfwSwapInterval(1);

		glfwSetKeyCallback(window, KeyCallback);

		return window;
	}

	void InitImGui(GLFWwindow* const window) {
		ImGui::CreateContext();

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 330");

		ImGui::StyleColorsDark();
	}

	std::string ReadShaderFromFile(const std::string& filename) {
		const std::ifstream file(filename);

		if (file.fail()) {
			ErrorCallback(-1, ("File " + filename + " not found.").c_str());
			return "";
		}

		std::stringstream ss;
		ss << file.rdbuf();

		return ss.str();
	}

	GLuint GenerateShaderProgram(const std::string& name) {
		const auto vertex_text = ReadShaderFromFile(name + ".vert.glsl");
		const auto fragment_text = ReadShaderFromFile(name + ".frag.glsl");

		const auto vt = vertex_text.c_str();
		const auto ft = fragment_text.c_str();

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

	void GenerateTriangleBuffers(const GLuint& program, GLuint& vertexBuffer, GLuint& transformBuffer, GLuint& indexBuffer, GLuint& vertexArray)
	{
		// Generate and bind VAO
		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);

		// Generate and bind VBO
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(TriangleVertices), TriangleVertices, GL_STATIC_DRAW);

		// Specify vertex attributes
		const GLint posLocation = glGetAttribLocation(program, "PositionOS");

		glEnableVertexAttribArray(posLocation);
		glVertexAttribPointer(posLocation, 2, GL_FLOAT, GL_FALSE,
			sizeof(Vertex), static_cast<void*>(nullptr));

		const GLint colLocation = glGetAttribLocation(program, "Color");

		glEnableVertexAttribArray(colLocation);
		glVertexAttribPointer(colLocation, 3, GL_FLOAT, GL_FALSE,
			sizeof(Vertex), reinterpret_cast<void*>(sizeof(vec2)));

		// Generate and bind instancing transform buffer
		glGenBuffers(1, &transformBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, transformBuffer);
		glBufferData(GL_ARRAY_BUFFER, COUNT_TRIANGLES * sizeof(mat4x4), TriangleTransforms, GL_DYNAMIC_DRAW);

		// Specify transform matrix attribute for 4 attribute slots
		const GLint modelLocation = glGetAttribLocation(program, "ModelMatrix");

		glEnableVertexAttribArray(modelLocation);
		glVertexAttribPointer(modelLocation, 4, GL_FLOAT, GL_FALSE,
			sizeof(mat4x4), static_cast<void*>(nullptr));

		glEnableVertexAttribArray(modelLocation + 1);
		glVertexAttribPointer(modelLocation + 1, 4, GL_FLOAT, GL_FALSE,
			sizeof(mat4x4), reinterpret_cast<void*>(1 * sizeof(vec4)));

		glEnableVertexAttribArray(modelLocation + 2);
		glVertexAttribPointer(modelLocation + 2, 4, GL_FLOAT, GL_FALSE,
			sizeof(mat4x4), reinterpret_cast<void*>(2 * sizeof(vec4)));

		glEnableVertexAttribArray(modelLocation + 3);
		glVertexAttribPointer(modelLocation + 3, 4, GL_FLOAT, GL_FALSE,
			sizeof(mat4x4), reinterpret_cast<void*>(3 * sizeof(vec4)));

		// Set divisors for instancing
		glVertexAttribDivisor(modelLocation, 1);
		glVertexAttribDivisor(modelLocation + 1, 1);
		glVertexAttribDivisor(modelLocation + 2, 1);
		glVertexAttribDivisor(modelLocation + 3, 1);

		// Generate and bind index buffer
		glGenBuffers(1, &indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(TriangleIndices), TriangleIndices, GL_STATIC_DRAW);

		// Clear gl state
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void CollectTriangleData() {
		if (TriDataMutex.try_lock()) {
			for(int i = 0; i < COUNT_TRIANGLES; i++) {
				const auto pos = Triangles[i]->GetPosition();
				const auto vel = Triangles[i]->GetLinearVelocity();
				const auto angle = Triangles[i]->GetAngle();
				const auto angularVel = Triangles[i]->GetAngularVelocity();

				TriData[i].SpatialData[0] = pos.x;
				TriData[i].SpatialData[1] = pos.y;
				TriData[i].SpatialData[2] = angle;

				TriData[i].PhysicsData[0] = vel.x;
				TriData[i].PhysicsData[1] = vel.y;
				TriData[i].PhysicsData[2] = angularVel;
			}
			TriDataMutex.unlock();
		}
	}
}