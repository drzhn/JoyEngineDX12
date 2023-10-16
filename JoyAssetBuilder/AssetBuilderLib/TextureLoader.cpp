#include "TextureLoader.h"

#include "objbase.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include <mutex>


#include "Texconv.h"
std::condition_variable cv2;
std::mutex m2;

bool ready2 = false;
bool processed2 = false;

struct ThreadPayload
{
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

	AssetConversion::TextureMetadata metadata;
	const char* filePath = nullptr;
	Blob* blob = nullptr;
} threadPayload;

void TextureConversionCycle2()
{
	int result = AssetConversion::TextureConversionInit(COINIT_MULTITHREADED);
	if (result != 0)
	{
		int a = 5;
	}
	while (true)
	{
		std::unique_lock lk(m2);
		cv2.wait(lk, [] { return ready2; });

		int result = AssetConversion::Convert(
			threadPayload.conversionParams,
			threadPayload.options,
			threadPayload.filePath,
			threadPayload.metadata,
			*threadPayload.blob);

		ready2 = false;
		processed2 = true;
		lk.unlock();
		cv2.notify_one();
	}
}

TextureLoader::TextureLoader()
{
	m_workerThread = std::thread(TextureConversionCycle2);
}

bool TextureLoader::LoadTexture(const std::string& filePath, std::string& errorMessage)
{
	const bool isHdr = AssetConversion::IsHDR(filePath.c_str());

	threadPayload.filePath = filePath.c_str();
	if (isHdr)
	{
		m_currentTextureData.m_header.format = BC6H_UF16;
		threadPayload.conversionParams.format = DXGI_FORMAT_BC6H_UF16;
	}
	else
	{
		m_currentTextureData.m_header.format = BC1_UNORM;
		threadPayload.conversionParams.format = DXGI_FORMAT_BC1_UNORM;
	}
	threadPayload.blob = &m_currentTextureData.blob;

	// send data to the worker thread
	{
		std::lock_guard lk(m2);
		ready2 = true;
		processed2 = false;
	}
	cv2.notify_one();

	// wait for the worker
	{
		std::unique_lock lk(m2);
		cv2.wait(lk, [] { return processed2; });
	}

	m_currentTextureData.m_header.dataSize = m_currentTextureData.blob.GetBufferSize();
	m_currentTextureData.m_header.height = threadPayload.metadata.height;
	m_currentTextureData.m_header.width = threadPayload.metadata.width;
	m_currentTextureData.m_header.mipCount = threadPayload.metadata.mipLevels;

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
