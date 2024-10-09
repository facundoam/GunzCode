#include "stdafx.h"
#include "MCsvParser.h"


MCSVReader::MCSVReader()
{
	m_pBuffer = NULL;
	m_pOffset = NULL;
	m_nLine = 0;
}

MCSVReader::~MCSVReader()
{
	if (m_pBuffer)
		delete[] m_pBuffer;
	if (m_pOffset)
		delete[] m_pOffset;
}


char* MCSVReader::ReadFile(const char* fname)
{
	FILE* fp;
	char* ptr = nullptr;
	int len;

	// Use fopen_s for safer file opening in modern C++
	errno_t err = fopen_s(&fp, fname, "rb");
	if (err != 0 || fp == nullptr) {
		return nullptr; // Return nullptr if the file cannot be opened
	}

	// Go to the end of the file to determine its length
	fseek(fp, 0, SEEK_END);
	len = ftell(fp); // Get the file length
	if (len <= 0) {
		fclose(fp); // Handle empty or invalid files
		return nullptr;
	}

	// Rewind the file pointer to the beginning of the file
	fseek(fp, 0, SEEK_SET);

	// Allocate memory for file contents (+1 for null terminator)
	ptr = new (std::nothrow) char[len + 1];
	if (!ptr) {
		fclose(fp); // If memory allocation fails, close the file
		return nullptr;
	}

	// Read the file into the allocated buffer
	size_t bytesRead = fread(ptr, 1, len, fp);
	if (bytesRead != len) {
		delete[] ptr; // Free the allocated memory if reading fails
		fclose(fp);
		return nullptr;
	}

	// Null-terminate the string
	ptr[len] = '\0';

	fclose(fp); // Close the file
	return ptr; // Return the buffer
}

bool MCSVReader::Load(const char* fname)
{
	//	파일 전체를 읽는다
	//
	m_pBuffer = ReadFile(fname);

	//	라인수를 센 후 각 오프셋을 구한다
	//
	m_nLine = CountLine(m_pBuffer, NULL);
	m_pOffset = new int[m_nLine];
	CountLine(m_pBuffer, m_pOffset);

	return true;
}

int MCSVReader::CountLine(const char* buffer, int* offset)
{
	const char* p = buffer;
	int line;

	for (line = 0; p; line++)
	{
		if (line > 0) p++;

		if (offset)
			offset[line] = (int)(p - buffer);

		p = strchr(p, '\n');
	}

	return line;
}

int MCSVReader::PassToken(const char* str)
{
	int i = 0;

	if (str[0] == '\"')
	{
		for (i = 1; ;)
		{
			if (str[i] == '\"')
			{
				if (str[i + 1] == '\"') i += 2;
				else break;
			}
			else
			{
				i += 1;
			}
		}
		return i + 1;
	}
	else
	{
		for (i = 0; !strchr(",\n", str[i]); i++);
		return i;
	}
}

int MCSVReader::GetData(int col, int row, char* outptr, int outlen)
{
	int i, off;

	if (row >= m_nLine)
		return 0;

	for (i = 0, off = m_pOffset[row]; i < col; off++, i++)
	{
		off += PassToken(m_pBuffer + off);
		if (strchr("\n", m_pBuffer[off]))
		{
			outptr[0] = '\0';
			return 0;
		}
	}

	if (m_pBuffer[off] == '\"')
	{
		for (i = 0, off += 1; ; i++)
		{
			if (m_pBuffer[off] == '\"' && m_pBuffer[off + 1] == '\"')
			{
				if (i < outlen - 1)
					outptr[i] = m_pBuffer[off];
				off += 2;
			}
			else
			{
				if (m_pBuffer[off] != '\"')
				{
					outptr[i] = m_pBuffer[off];
					off += 1;
				}
				else
				{
					break;
				}
			}
		}
	}
	else
	{
		for (i = 0; !strchr(",\n", m_pBuffer[off]); i++, off++)
		{
			if (i < outlen - 1)
				outptr[i] = m_pBuffer[off];
		}
	}

	if (i >= outlen - 1) outptr[outlen - 1] = '\0';
	else outptr[i] = '\0';


	return i;
}
