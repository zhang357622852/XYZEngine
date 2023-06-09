#pragma once

#include "XYZ/Asset/Asset.h"
#include "XYZ/Renderer/Image.h"

#include <string>
#include <memory>


namespace XYZ {

	enum class BindImageType
	{
		Read,
		Write,
		ReadWrite
	};

	class XYZ_API Texture : public Asset
	{
	public:
		virtual ~Texture() = default;

		virtual ImageFormat GetFormat() const = 0;
		virtual uint32_t	GetWidth() const = 0;
		virtual uint32_t	GetHeight() const = 0;
		virtual uint32_t	GetMipLevelCount() const = 0;
		virtual std::pair<uint32_t, uint32_t> GetMipSize(uint32_t mip) const = 0;

		static uint32_t CalculateMipMapCount(uint32_t width, uint32_t height);

		virtual AssetType GetAssetType() const override { return AssetType::Texture; }
		static AssetType GetStaticType() { return AssetType::Texture; }
	};


	class XYZ_API Texture2D : public Texture
	{
	public:
		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual void Lock() = 0;
		virtual void Unlock() = 0;

		virtual bool Loaded() const = 0;

		virtual const std::string&		 GetPath() const = 0;
		virtual Ref<Image2D>			 GetImage() const = 0;
		virtual ByteBuffer				 GetWriteableBuffer() = 0;
		virtual const TextureProperties& GetProperties() const = 0;

		static Ref<Texture2D> Create(ImageFormat format, uint32_t width, uint32_t height, const void* data = nullptr, const TextureProperties& properties = TextureProperties());
		static Ref<Texture2D> Create(const std::string& path, const TextureProperties& properties = TextureProperties());
	};
}