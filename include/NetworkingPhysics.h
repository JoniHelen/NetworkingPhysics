#pragma once

namespace NetPhysics {

	struct TriangleData {
		vec3 SpatialData;
		vec3 PhysicsData;
	};

	struct Vertex {
		vec2 Position;
		vec3 Color;
	};

	// Triangle drawing data

	constexpr Vertex TriangleVertices[3] = {
		{ -0.4330127f, -0.25f, 1.0f, 1.0f, 0.0f },
		{  0.4330127f, -0.25f, 0.0f, 1.0f, 1.0f },
		{   0.0f,  0.5f, 1.0f, 0.0f, 1.0f }
	};

	constexpr uint32_t TriangleIndices[3] = { 0u, 1u, 2u };

	// World globals

	constexpr int COUNT_TRIANGLES = 30;
	inline b2Body* Walls[4u];

	inline std::atomic_flag ObjectsInitialized;
	inline std::atomic_flag ObjectsReceived;
	inline b2Body* Triangles[COUNT_TRIANGLES];
	inline mat4x4 TriangleTransforms[COUNT_TRIANGLES];

	inline std::mutex TriDataMutex;
	inline TriangleData TriData[COUNT_TRIANGLES];

	// Functions

	/// <summary>Creates walls around the scene with physics objects.</summary>
	///	<param name="world">The world in which the objects are instantiated</param>
	void CreateWorldBounds(const std::unique_ptr<b2World>& world);

	/// <summary>Creates triangular physics bodies into the world.</summary>
	/// <param name="world">The world in which the triangles are instantiated</param>
	void CreatePhysicsTriangles(const std::unique_ptr<b2World>& world);

	/// <summary>Resets the simulation to its starting state.</summary>
	void ResetSimulation();

	/// <summary>Key callback for GLFW.</summary>
	/// <param name="window">Caller GLFWwindow</param>
	/// <param name="key">GLFW key code</param>
	/// <param name="scancode">GLFW scancode</param>
	/// <param name="action">GLFW action code</param>
	/// <param name="mods">GLFW mods</param>
	void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

	/// <summary>Error callback for GLFW.</summary>
	/// <param name="error">Error code</param>
	///	<param name="description">Error description</param>
	void ErrorCallback(int error, const char* description);

	/// <summary>Initializes a GLFW window.</summary>
	GLFWwindow* InitWindow();

	/// <summary>Runs ImGui initialization functions.</summary>
	/// <param name="window">GLFWwindow to pass to init functions</param>
	void InitImGui(GLFWwindow* window);

	/// <summary>Reads text from file with provided filename. For use with shader files.</summary>
	/// <param name="filename">The name of the file to read</param>
	std::string ReadShaderFromFile(const std::string& filename);

	/// <summary>Generates a shader program using predefined file extensions and locations.</summary>
	/// <param name="name">The name of the shader to generate. Used in filename</param>
	GLuint GenerateShaderProgram(const std::string& name);

	/// <summary>Generates GPU buffers for drawing triangles.</summary>
	/// <param name="program">The shader program for triangles</param>
	/// <param name="vertexBuffer">The vertex buffer</param>
	/// <param name="transformBuffer">The transform buffer</param>
	/// <param name="indexBuffer">The index buffer</param>
	/// <param name="vertexArray">The vertex array</param>
	void GenerateTriangleBuffers(const GLuint& program, GLuint& vertexBuffer, GLuint& transformBuffer, GLuint& indexBuffer, GLuint& vertexArray);

	void CollectTriangleData();
}