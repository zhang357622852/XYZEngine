#pragma once

#include "XYZ/Renderer/Texture.h"
#include <GL/glew.h>

namespace XYZ {
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(TextureWrap wrap, const std::string& path);
		OpenGLTexture2D(TextureFormat format, TextureWrap wrap, uint32_t width, uint32_t height);


		virtual ~OpenGLTexture2D();

		inline virtual uint32_t GetWidth() const override { return m_Specification.Width; }
		inline virtual uint32_t GetHeight() const override { return m_Specification.Height; }
		inline virtual uint32_t GetChannels() const override { return m_Specification.Channels; }
		inline virtual uint32_t GetRendererID() const override { return m_RendererID; }
		virtual void SetData(void* data, uint32_t size) override;
		virtual uint8_t* GetData() override;
		virtual const std::string& GetPath() const override { return m_Path; }
		virtual const TextureSpecs& GetSpecification() const { return m_Specification; };
		virtual void Bind(uint32_t slot = 0) const override;

		static void Bind(uint32_t rendererID, uint32_t slot);
	private:
		uint32_t m_RendererID;

		TextureSpecs m_Specification;
		std::string m_Path;
		GLenum m_DataFormat, m_InternalFormat;
	};

}