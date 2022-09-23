#ifndef GUID_UTILS_H
#define GUID_UTILS_H

#include <string>
#include "combaseapi.h"
#include "Assert.h"

namespace JoyEngine
{
	struct GUID
	{
		uint32_t Data1 = 0;
		uint16_t Data2 = 0;
		uint16_t Data3 = 0;
		uint8_t Data4[8] = {0, 0, 0, 0, 0, 0, 0, 0};

		[[nodiscard]] bool IsNull() const
		{
			return Data1 == 0 &&
				Data2 == 0 &&
				Data3 == 0 &&
				Data4[0] == 0 &&
				Data4[1] == 0 &&
				Data4[2] == 0 &&
				Data4[3] == 0 &&
				Data4[4] == 0 &&
				Data4[5] == 0 &&
				Data4[6] == 0 &&
				Data4[7] == 0;
		}

		bool operator<(const GUID& guid) const
		{
			if (Data1 != guid.Data1)
			{
				return Data1 < guid.Data1;
			}
			if (Data2 != guid.Data2)
			{
				return Data2 < guid.Data2;
			}
			if (Data3 != guid.Data3)
			{
				return Data3 < guid.Data3;
			}
			for (int i = 0; i < 8; i++)
			{
				if (Data4[i] != guid.Data4[i])
				{
					return Data4[i] < guid.Data4[i];
				}
			}
			return false;
		}

		static GUID StringToGuid(const std::string& str)
		{
			GUID guid;
			sscanf_s(str.c_str(),
			         "%8x-%4hx-%4hx-%2hhx%2hhx-%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
			         &guid.Data1, &guid.Data2, &guid.Data3,
			         &guid.Data4[0], &guid.Data4[1], &guid.Data4[2], &guid.Data4[3],
			         &guid.Data4[4], &guid.Data4[5], &guid.Data4[6], &guid.Data4[7]);

			return guid;
		}

		static GUID StringToGuid(const char* str)
		{
			GUID guid;
			sscanf_s(str,
				"%8x-%4hx-%4hx-%2hhx%2hhx-%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
				&guid.Data1, &guid.Data2, &guid.Data3,
				&guid.Data4[0], &guid.Data4[1], &guid.Data4[2], &guid.Data4[3],
				&guid.Data4[4], &guid.Data4[5], &guid.Data4[6], &guid.Data4[7]);

			return guid;
		}

		static std::string GuidToString(GUID guid)
		{
			char guid_cstr[37];
			snprintf(guid_cstr, sizeof(guid_cstr),
			         "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			         guid.Data1, guid.Data2, guid.Data3,
			         guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
			         guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

			return std::string{guid_cstr};
		}

		static GUID Random()
		{
			GUID guid;
			ASSERT_SUCC(CoCreateGuid(reinterpret_cast<::GUID*>(&guid)));
			return guid;
		}
	};
}

#endif //GUID_UTILS_H
