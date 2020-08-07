#include "stdafx.h"
#include "AssetManager.h"

#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/Texture.h"

#include <yaml-cpp/yaml.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace YAML {

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::quat>
	{
		static Node encode(const glm::quat& rhs)
		{
			Node node;
			node.push_back(rhs.w);
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::quat& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.w = node[0].as<float>();
			rhs.x = node[1].as<float>();
			rhs.y = node[2].as<float>();
			rhs.z = node[3].as<float>();
			return true;
		}
	};
}


namespace XYZ {
	static enum class FieldType
	{
		Float,
		Float2,
		Float3,
		Float4,
		Int,
		None
	};

	static FieldType FindType(const std::string& str)
	{
		char tokenComma = ',';
		size_t numCommas = std::count(str.begin(), str.end(), tokenComma);
		char tokenDot = '.';
		size_t numDots = std::count(str.begin(), str.end(), tokenDot);

		switch (numCommas)
		{
		case 0:
			if (numDots)
				return FieldType::Float;
			else
				return FieldType::Int;
		case 1:
			return FieldType::Float2;
		case 2:
			return FieldType::Float3;
		case 3:
			return FieldType::Float4;
		}

		XYZ_ASSERT(false, "Wrong type");
		return FieldType::None;
	}

	static TextureWrap StringToTextureWrap(const std::string& str)
	{
		
		if (str == "Clamp")
			return TextureWrap::Clamp;
		if (str == "Repeat")
			return TextureWrap::Repeat;
		

		return TextureWrap::None;
	}


	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}


	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::quat& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.w << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}
	static std::tuple<glm::vec3, glm::quat, glm::vec3> GetTransformDecomposition(const glm::mat4& transform)
	{
		glm::vec3 scale, translation, skew;
		glm::vec4 perspective;
		glm::quat orientation;
		glm::decompose(transform, scale, orientation, translation, skew, perspective);

		return { translation, orientation, scale };
	}

	template<>
	void Asset<Texture2D>::Serialize()
	{
		XYZ_LOG_INFO("Serializing texture ", Filepath);
		auto& texture = Handle;

		YAML::Emitter out;
		out << YAML::BeginMap; // Texture
		out << YAML::Key << "Texture" << YAML::Value << "texture name";

		out << YAML::Key << "Wrap" << YAML::Value << ToUnderlying(texture->GetSpecification().Wrap);

		out << YAML::EndMap; // Texture

		std::ofstream fout(Filepath + ".meta");
		fout << out.c_str();
	}


	template<>
	void Asset<Texture2D>::Deserialize(AssetManager& manager)
	{
		XYZ_LOG_INFO("Deserializing texture ", Filepath);
		TextureWrap wrap = TextureWrap::None;

		std::ifstream stream(Filepath + ".meta");
		if (stream.is_open())
		{
			std::stringstream strStream;
			strStream << stream.rdbuf();

			YAML::Node data = YAML::Load(strStream.str());

			XYZ_ASSERT(data["Texture"], "Incorrect file format");
			wrap = StringToTextureWrap(data["Wrap"].as<std::string>());
		}
		else
		{
			XYZ_LOG_WARN("Missing texture meta data, setting default");
		}

		Handle = Texture2D::Create(wrap, Filepath);
	}


	template <>
	void Asset<Material>::Serialize()
	{
		auto& material = Handle;
		XYZ_LOG_INFO("Serializing material ", Filepath);
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Material" << YAML::Value << "material name";
		out << YAML::Key << "ShaderAssetPath" << YAML::Value << material->GetShader()->GetPath();


		out << YAML::Key << "Textures";
		out << YAML::Value << YAML::BeginSeq;
		uint32_t counter = 0;
		for (auto& texture : material->GetTextures())
		{
			out << YAML::BeginMap;
			out << YAML::Key << "TextureAssetPath";
			out << YAML::Value << texture->GetPath();
			out << YAML::Key << "TextureIndex";
			out << YAML::Value << counter++;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		out << YAML::Key << "Values";
		out << YAML::Value << YAML::BeginSeq;
		for (auto& uniform : material->GetShader()->GetUniforms())
		{
			switch (uniform.Type)
			{
			case UniformDataType::FLOAT:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(float*)&material->GetBuffer()[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::FLOAT_VEC2:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(glm::vec2*) & material->GetBuffer()[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::FLOAT_VEC3:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(glm::vec3*) & material->GetBuffer()[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::FLOAT_VEC4:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(glm::vec4*) & material->GetBuffer()[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::INT:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(int*)&material->GetBuffer()[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::FLOAT_MAT4:
				break;
			};
		}


		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(Filepath);
		fout << out.c_str();
	}

	template <>
	void Asset<Material>::Deserialize(AssetManager& manager)
	{
		std::ifstream stream(Filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());

		XYZ_ASSERT(data["Material"], "Incorrect file format");

		XYZ_LOG_INFO("Deserializing material ", Filepath);


		auto shader = Shader::Create(data["ShaderAssetPath"].as<std::string>());
		Ref<Material> material = Material::Create(shader);

		for (auto& seq : data["Textures"])
		{
			std::string path = seq["TextureAssetPath"].as<std::string>();
			uint32_t index = seq["TextureIndex"].as<uint32_t>();
			manager.LoadAsset<Texture2D>(path);
			material->Set("u_Texture", manager.GetAsset<Texture2D>(path), index);
		}

		for (auto& seq : data["Values"])
		{
			for (auto& val : seq)
			{
				std::stringstream ss;
				ss << val.second;
				auto type = FindType(ss.str());
				switch (type)
				{
				case FieldType::Float:
					material->Set(val.first.as<std::string>(), val.second.as<float>());
					break;
				case FieldType::Float2:
					material->Set(val.first.as<std::string>(), val.second.as<glm::vec2>());
					break;
				case FieldType::Float3:
					material->Set(val.first.as<std::string>(), val.second.as<glm::vec3>());
					break;
				case FieldType::Float4:
					material->Set(val.first.as<std::string>(), val.second.as<glm::vec4>());
					break;
				case FieldType::None:
					break;
				}

			}
		}
		Handle = material;
	}
	AssetManager::~AssetManager()
	{
		for (auto storage : m_AssetStorages)
		{
			storage.second->Serialize();
			delete storage.second;
		}
	}
	void AssetManager::Serialize()
	{
		for (auto storage : m_AssetStorages)
		{
			storage.second->Serialize();
		}
	}
}