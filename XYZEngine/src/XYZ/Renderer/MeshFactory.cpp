#include "stdafx.h"
#include "MeshFactory.h"

namespace XYZ {
	

	Ref<MeshSource> MeshFactory::CreateQuad(const glm::vec2& size)
	{   
		const std::vector<Vertex> vertices{
			Vertex{glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(0.0f, 0.0f)},
			Vertex{glm::vec3( 0.5f, -0.5f, 0.0f), glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(1.0f, 0.0f)},
			Vertex{glm::vec3( 0.5f,  0.5f, 0.0f), glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(1.0f, 1.0f)},
			Vertex{glm::vec3(-0.5f,  0.5f, 0.0f), glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(0.0f, 1.0f)}
		};
		const std::vector<uint32_t> indices { 0, 1, 2, 2, 3, 0 };
		return Ref<MeshSource>::Create(vertices, indices);
	}
	Ref<StaticMesh> MeshFactory::CreateInstancedQuad(const glm::vec2& size, const BufferLayout& layout, const BufferLayout& instanceLayout, uint32_t count)
	{
		Ref<StaticMesh> result;
		return result;
	}



	Ref<StaticMesh> MeshFactory::CreateBox(const glm::vec3& size)
	{
		std::vector<Vertex> vertices = {
			Vertex{{ -size.x / 2.0f, -size.y / 2.0f,  size.z / 2.0f }, {}},  // Front Down Left
			Vertex{{  size.x / 2.0f, -size.y / 2.0f,  size.z / 2.0f }, {}},  // Front Down Right 
			Vertex{{  size.x / 2.0f,  size.y / 2.0f,  size.z / 2.0f }, {}},  // Front Up   Right
			Vertex{{ -size.x / 2.0f,  size.y / 2.0f,  size.z / 2.0f }, {}},	 // Front Up   Left

			Vertex{{ -size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f }, {}},	// Back  Down Left
			Vertex{{  size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f }, {}},	// Back  Down Right
			Vertex{{  size.x / 2.0f,  size.y / 2.0f, -size.z / 2.0f }, {}},	// Back  Up	  Right
			Vertex{{ -size.x / 2.0f,  size.y / 2.0f, -size.z / 2.0f }, {}}	// Back  Up	  Left
		};

		std::vector<uint32_t> indices = {
			0, 1, 2,
			2, 3, 0,

			1, 5, 6,
			6, 2, 1,

			7, 6, 5,
			5, 4, 7,

			4, 0, 3,
			3, 7, 4,

			4, 5, 1,
			1, 0, 4,

			3, 2, 6,
			6, 7, 3
		};
		Ref<StaticMesh> result = Ref<StaticMesh>::Create(Ref<MeshSource>::Create(vertices, indices));
		return result;
	}
	Ref<StaticMesh> MeshFactory::CreateInstancedBox(const glm::vec3& size, const BufferLayout& layout, const BufferLayout& instanceLayout, uint32_t count)
	{
		Ref<StaticMesh> result;
		return result;
	}
	Ref<StaticMesh> MeshFactory::CreateCube(const glm::vec3& size, const BufferLayout& layout)
	{
		const Vertex vertices[24] = {
			// Front face
			Vertex{{ -size.x / 2.0f, -size.y / 2.0f,  size.z / 2.0f },glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(0.0f, 0.0f)},
			Vertex{{  size.x / 2.0f, -size.y / 2.0f,  size.z / 2.0f },glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(1.0f, 0.0f)},
			Vertex{{  size.x / 2.0f,  size.y / 2.0f,  size.z / 2.0f },glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(1.0f, 1.0f)},
			Vertex{{ -size.x / 2.0f,  size.y / 2.0f,  size.z / 2.0f },glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(0.0f, 1.0f)},

			// Top face
			Vertex{{ -size.x / 2.0f,  size.y / 2.0f,  size.z / 2.0f },glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(0.0f, 0.0f)},
			Vertex{{  size.x / 2.0f,  size.y / 2.0f,  size.z / 2.0f },glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(1.0f, 0.0f)},
			Vertex{{  size.x / 2.0f,  size.y / 2.0f, -size.z / 2.0f },glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(1.0f, 1.0f)},
			Vertex{{ -size.x / 2.0f,  size.y / 2.0f, -size.z / 2.0f },glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(0.0f, 1.0f)},

			// Bottom face
			Vertex{{ -size.x / 2.0f, -size.y / 2.0f,  size.z / 2.0f },glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(0.0f, 0.0f)},
			Vertex{{  size.x / 2.0f, -size.y / 2.0f,  size.z / 2.0f },glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(1.0f, 0.0f)},
			Vertex{{  size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f },glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(1.0f, 1.0f)},
			Vertex{{ -size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f },glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(0.0f, 1.0f)},

			// Left face
			Vertex{{ -size.x / 2.0f, -size.y / 2.0f,  size.z / 2.0f },glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(0.0f, 0.0f)},
			Vertex{{ -size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f },glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(1.0f, 0.0f)},
			Vertex{{ -size.x / 2.0f,  size.y / 2.0f, -size.z / 2.0f },glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(1.0f, 1.0f)},
			Vertex{{ -size.x / 2.0f,  size.y / 2.0f,  size.z / 2.0f },glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(0.0f, 1.0f)},

			// Right face 
			Vertex{{  size.x / 2.0f, -size.y / 2.0f,  size.z / 2.0f },glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(0.0f, 0.0f)},
			Vertex{{  size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f },glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(1.0f, 0.0f)},
			Vertex{{  size.x / 2.0f,  size.y / 2.0f, -size.z / 2.0f },glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(1.0f, 1.0f)},
			Vertex{{  size.x / 2.0f,  size.y / 2.0f,  size.z / 2.0f },glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(0.0f, 1.0f)},

			// Back face
			Vertex{{ -size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f },glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(0.0f, 0.0f)},
			Vertex{{  size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f },glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(1.0f, 0.0f)},
			Vertex{{  size.x / 2.0f,  size.y / 2.0f, -size.z / 2.0f },glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(1.0f, 1.0f)},
			Vertex{{ -size.x / 2.0f,  size.y / 2.0f, -size.z / 2.0f },glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec2(0.0f, 1.0f)}
		};

		

		//std::vector<uint32_t> indices;
		//uint32_t offset = 0;
		//for (uint32_t i = 0; i < 12; i += 2)
		//{
		//	triangles[i]	 = Triangle{ offset, offset + 1, offset + 2 };
		//	triangles[i + 1] = Triangle{ offset + 2, offset + 3, offset };
		//	offset += 4;
		//}
		Ref<StaticMesh> result;
		return result;
	}
	Ref<StaticMesh> MeshFactory::CreateInstancedCube(const glm::vec3& size, const BufferLayout& layout, const BufferLayout& instanceLayout, uint32_t count)
	{
		Ref<StaticMesh> result;
		return result;
	}
}
