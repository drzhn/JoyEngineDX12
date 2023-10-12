#include "TextureLoader.h"

#include <filesystem>

#include <fstream>
#include "combaseapi.h"
#include "winerror.h"

#include "Texconv.h"

const wchar_t* GetErrorDesc(HRESULT hr)
{
	static wchar_t desc[1024] = {};

	LPWSTR errorText = nullptr;

	const DWORD result = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		nullptr, static_cast<DWORD>(hr),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&errorText), 0, nullptr);

	*desc = 0;

	if (result > 0 && errorText)
	{
		swprintf_s(desc, L": %ls", errorText);

		size_t len = wcslen(desc);
		if (len >= 1)
		{
			desc[len - 1] = 0;
		}

		if (errorText)
			LocalFree(errorText);
	}

	return desc;
}

TextureLoader::TextureLoader()
{
	int result = AssetConversion::TextureConversionInit();
}

bool TextureLoader::LoadTexture(const std::string& filePath, std::string& errorMessage)
{
	const bool isHdr = AssetConversion::IsHDR(filePath.c_str());

	AssetConversion::TextureConversionParams conversionParams{
		.width = 0, // preserve original
		.height = 0,
		.mipLevels = 0,
		.format = DXGI_FORMAT_UNKNOWN
	};

	const uint64_t options =
		(1ull << OPT_MIPLEVELS) |
		(1ull << OPT_FORMAT) |
		(1ull << OPT_FIT_POWEROF2) |
		(1ull << OPT_USE_DX10);

	if (isHdr)
	{
		m_currentTextureData.m_header.format = BC6H_UF16;
		conversionParams.format = DXGI_FORMAT_BC6H_UF16;
	}
	else
	{
		m_currentTextureData.m_header.format = BC1_UNORM;
		conversionParams.format = DXGI_FORMAT_BC1_UNORM;
	}

	AssetConversion::TextureMetadata metadata;
	int result = AssetConversion::Convert(conversionParams, options, filePath.c_str(), metadata, m_currentTextureData.blob);

	m_currentTextureData.m_header.dataSize = m_currentTextureData.blob.GetBufferSize();
	m_currentTextureData.m_header.height = metadata.height;
	m_currentTextureData.m_header.width = metadata.width;
	m_currentTextureData.m_header.mipCount = metadata.mipLevels;

	return true;
}

bool TextureLoader::WriteData(const std::string& dataFilename, std::string& errorMessage) const
{
	std::ofstream modelFileStream(dataFilename, std::ofstream::binary | std::ofstream::trunc);

	modelFileStream.write(reinterpret_cast<const char*>(&m_currentTextureData.m_header), sizeof(TextureAssetHeader));
	modelFileStream.write(reinterpret_cast<const char*>(m_currentTextureData.blob.GetBufferPointer()), m_currentTextureData.blob.GetBufferSize());

	if (modelFileStream.is_open())
	{
		modelFileStream.close();
	}

	return true;
}
