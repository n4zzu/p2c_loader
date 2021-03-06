#pragma once
#include <iostream>
#include <windows.h>
#include <WinUser.h>
#include <WinBase.h>
#include <tchar.h>
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib,"wininet.lib")
#pragma comment(lib, "Advapi32.lib")
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <WinInet.h>
#include <iomanip>
#include <cstdio>
using namespace std;

#define HOST ("site")
#define HOST_VK ("group")

static const string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

class CLicense
{
private:
	void ToClipboard2(const char* text)
	{
		if (OpenClipboard(0))
		{
			EmptyClipboard();
			char* clip_data = (char*)(GlobalAlloc(GMEM_FIXED, MAX_PATH));
			lstrcpy(clip_data, text);
			SetClipboardData(CF_TEXT, (HANDLE)(clip_data));
			LCID* lcid = (DWORD*)(GlobalAlloc(GMEM_FIXED, sizeof(DWORD)));
			*lcid = MAKELCID(MAKELANGID(LANG_RUSSIAN, SUBLANG_NEUTRAL), SORT_DEFAULT);
			SetClipboardData(CF_LOCALE, (HANDLE)(lcid));
			CloseClipboard();
		}
	}

	string GetHwUID()
	{
		HW_PROFILE_INFO hwProfileInfo;
		string szHwProfileGuid = "";

		if (GetCurrentHwProfileA(&hwProfileInfo) != NULL)
			szHwProfileGuid = hwProfileInfo.szHwProfileGuid;

		return szHwProfileGuid;
	}

	string GetCompUserName(bool User)
	{
		string CompUserName = "";

		char szCompName[MAX_COMPUTERNAME_LENGTH + 1];
		char szUserName[MAX_COMPUTERNAME_LENGTH + 1];

		DWORD dwCompSize = sizeof(szCompName);
		DWORD dwUserSize = sizeof(szUserName);

		if (GetComputerNameA(szCompName, &dwCompSize))
		{
			CompUserName = szCompName;

			if (User && GetUserNameA(szUserName, &dwUserSize))
			{
				CompUserName = szUserName;
			}
		}

		return CompUserName;
	}

	string StringToHex(const string input)
	{
		const char* lut = "0123456789ABCDEF";
		size_t len = input.length();
		string output = "";

		output.reserve(2 * len);

		for (size_t i = 0; i < len; i++)
		{
			const unsigned char c = input[i];
			output.push_back(lut[c >> 4]);
			output.push_back(lut[c & 15]);
		}

		return output;
	}

	DWORD GetVolumeID()
	{
		DWORD VolumeSerialNumber;

		BOOL GetVolumeInformationFlag = GetVolumeInformationA(
			"c:\\",
			0,
			0,
			&VolumeSerialNumber,
			0,
			0,
			0,
			0
		);

		if (GetVolumeInformationFlag)
			return VolumeSerialNumber;

		return 0;
	}

	string GetSerialKey()
	{
		string SerialKey = "61A345B5496B2";
		string CompName = GetCompUserName(false);
		string UserName = GetCompUserName(true);

		SerialKey.append(StringToHex(GetHwUID()));
		SerialKey.append("-");
		SerialKey.append(StringToHex(to_string(GetVolumeID())));
		SerialKey.append("-");
		SerialKey.append(StringToHex(CompName));
		SerialKey.append("-");
		SerialKey.append(StringToHex(UserName));

		return SerialKey;
	}

	string GetHashText(const void * data, const size_t data_size)
	{
		HCRYPTPROV hProv = NULL;

		if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
		{
			return "";
		}

		BOOL hash_ok = FALSE;
		HCRYPTPROV hHash = NULL;

		hash_ok = CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash);

		if (!hash_ok)
		{
			CryptReleaseContext(hProv, 0);
			return "";
		}

		if (!CryptHashData(hHash, static_cast<const BYTE *>(data), data_size, 0))
		{
			CryptDestroyHash(hHash);
			CryptReleaseContext(hProv, 0);
			return "";
		}

		DWORD cbHashSize = 0, dwCount = sizeof(DWORD);
		if (!CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE *)&cbHashSize, &dwCount, 0))
		{
			CryptDestroyHash(hHash);
			CryptReleaseContext(hProv, 0);
			return "";
		}

		vector<BYTE> buffer(cbHashSize);

		if (!CryptGetHashParam(hHash, HP_HASHVAL, reinterpret_cast<BYTE*>(&buffer[0]), &cbHashSize, 0))
		{
			CryptDestroyHash(hHash);
			CryptReleaseContext(hProv, 0);
			return "";
		}

		ostringstream oss;

		for (vector<BYTE>::const_iterator iter = buffer.begin(); iter != buffer.end(); ++iter)
		{
			oss.fill('0');
			oss.width(2);
			oss << hex << static_cast<const int>(*iter);
		}

		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		return oss.str();
	}

	string GetHashSerialKey()
	{
		string SerialKey = GetSerialKey();
		const void* pData = SerialKey.c_str();
		size_t Size = SerialKey.size();
		string Hash = GetHashText(pData, Size);

		for (auto& c : Hash)
		{
			if (c >= 'a' && c <= 'f')
			{
				c = '4';
			}
			else if (c == 'b')
			{
				c = '5';
			}
			else if (c == 'c')
			{
				c = '6';
			}
			else if (c == 'd')
			{
				c = '7';
			}
			else if (c == 'e')
			{
				c = '8';
			}
			else if (c == 'f')
			{
				c = '9';
			}

			c = toupper(c);
		}

		return Hash;
	}

	string base64_encode(char const* bytes_to_encode, unsigned int in_len)
	{
		string ret;
		int i = 0;
		int j = 0;
		unsigned char char_array_3[3];
		unsigned char char_array_4[4];

		while (in_len--)
		{
			char_array_3[i++] = *(bytes_to_encode++);
			if (i == 3)
			{
				char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
				char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
				char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
				char_array_4[3] = char_array_3[2] & 0x3f;

				for (i = 0; (i < 4); i++)
					ret += base64_chars[char_array_4[i]];
				i = 0;
			}
		}

		if (i)
		{
			for (j = i; j < 3; j++)
				char_array_3[j] = '\0';

			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (j = 0; (j < i + 1); j++)
				ret += base64_chars[char_array_4[j]];

			while ((i++ < 3))
				ret += '=';

		}

		return ret;
	}

	string GetUrlData(string url, LPCSTR host)
	{
		string request_data = "";

		HINTERNET hIntSession = InternetOpenA("", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);

		if (!hIntSession)
		{
			return request_data;
		}

		HINTERNET hHttpSession = InternetConnectA(hIntSession, host, 80, 0, 0, INTERNET_SERVICE_HTTP, 0, NULL);

		if (!hHttpSession)
		{
			return request_data;
		}

		HINTERNET hHttpRequest = HttpOpenRequestA(hHttpSession, "GET", url.c_str()
			, 0, 0, 0, INTERNET_FLAG_RELOAD, 0);

		if (!hHttpSession)
		{
			return request_data;
		}

		char* szHeaders = ("Content-Type: text/html\r\nUser-Agent: License");
		char szRequest[1024] = { 0 };

		if (!HttpSendRequestA(hHttpRequest, szHeaders, strlen(szHeaders), szRequest, strlen(szRequest)))
		{
			return request_data;
		}

		CHAR szBuffer[1024] = { 0 };
		DWORD dwRead = 0;

		while (InternetReadFile(hHttpRequest, szBuffer, sizeof(szBuffer) - 1, &dwRead) && dwRead)
		{
			request_data.append(szBuffer, dwRead);
		}

		InternetCloseHandle(hHttpRequest);
		InternetCloseHandle(hHttpSession);
		InternetCloseHandle(hIntSession);

		return request_data;
	}

public:

	string GetSerial()
	{
		string Serial = "";
		string HashSerialKey = GetHashSerialKey();

		string Serial1 = HashSerialKey.substr(0, 4);
		string Serial2 = HashSerialKey.substr(4, 4);
		string Serial3 = HashSerialKey.substr(8, 4);
		string Serial4 = HashSerialKey.substr(12, 4);

		Serial += Serial1;
		Serial += '-';
		Serial += Serial2;
		Serial += '-';
		Serial += Serial3;
		Serial += '-';
		Serial += Serial4;

		return Serial;
	}

	string GetSerial64()
	{
		return base64_encode(GetSerial().c_str(), GetSerial().size());
	}

	bool CheckLicense()
	{
		string Serial = GetSerial64();
		string UrlRequest = "";
		UrlRequest.append(("/gate.php?serial=") + Serial);

		string ReciveHash = GetUrlData(UrlRequest, HOST);
		if (ReciveHash.size() > 0)
		{
			string LicenseOK = ("license-success-ok-") + Serial + "-";
			for (int RandomMd5 = 1; RandomMd5 <= 10; RandomMd5++)
			{
				string LicenseCheck = LicenseOK + to_string(RandomMd5);
				string LicenseOKHash = GetHashText(LicenseCheck.c_str(), LicenseCheck.size());
				if (ReciveHash == LicenseOKHash)
				{
					return true;
				}
			}
		}
		return false;
	}

	bool GetOnlineAdmin(string name)
	{
		return GetUrlData("/" + name, HOST_VK).find("<span class=\"pp_last_activity_text\">Online</span>") != string::npos;
	}

	//string Username()
	//{
	//	return GetUrlData("/gate.php?GetUserByHwid=" + GetSerial64(), HOST);
	//}

	string GetDays()
	{
		return GetUrlData("/gate.php?day=" + GetSerial64(), HOST);
	}

	string GetVer()
	{
		return GetUrlData("/gate.php?version=ok", HOST);
	}

	bool CheatEnabled()
	{
		return GetUrlData("/gate.php?act=1", HOST) == "1";
	}

};

CLicense* Lic;