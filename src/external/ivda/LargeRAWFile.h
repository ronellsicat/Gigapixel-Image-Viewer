#pragma once

#ifndef IVDA_LARGERAWFILE_H
#define IVDA_LARGERAWFILE_H

#define _LARGEFILE64_SOURCE 1
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE 1

#define BLOCK_COPY_SIZE (1024*1024*1024)

#include "StdInclude.h"

#include <string>
#include <vector>
#include "EndianConvert.h"

#ifdef _MSC_VER
  #include <windows.h>

  #include <io.h>
  #include <fcntl.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <share.h>
  // undef stupid windows defines to max, min and LoadImage
  #ifdef max
  #undef max
  #endif

  #ifdef min
  #undef min
  #endif

  #ifdef LoadImage
  #undef LoadImage
  #endif

#endif

namespace IVDA
{
#ifdef _MSC_VER
	typedef HANDLE FILETYPE;
#else
	typedef FILE* FILETYPE;
#endif

	class LargeRAWFile {
	public:
	  LargeRAWFile(const std::string& strFilename, uint64_t iHeaderSize=0);
	  LargeRAWFile(const std::wstring& wstrFilename, uint64_t iHeaderSize=0);
	  LargeRAWFile(const LargeRAWFile &other);
	  virtual ~LargeRAWFile() {Close();}

	  bool Open(bool bReadWrite=false);
	  bool IsOpen() const { return m_bIsOpen;}
	  bool IsWritable() const { return m_bWritable;}
	  bool Create(uint64_t iInitialSize=0);
	  bool Exists() const;
    bool Append();
	  void Close();
	  void Delete();
	  bool Truncate();
	  bool Truncate(uint64_t iPos);
	  uint64_t GetCurrentSize();
	  std::string GetFilename() const { return m_strFilename;}

	  void SeekStart();
	  uint64_t SeekEnd();
	  uint64_t GetPos();
	  void SeekPos(uint64_t iPos);
	  size_t ReadRAW(unsigned char* pData, uint64_t iCount);
	  size_t WriteRAW(const unsigned char* pData, uint64_t iCount);
	  bool CopyRAW(uint64_t iCount, uint64_t iSourcePos, uint64_t iTargetPos, 
				   unsigned char* pBuffer, uint64_t iBufferSize);


	  template<class T> size_t Read(const T* pData, size_t iCount) {
		  return ReadRAW((unsigned char*)pData, sizeof(T)*iCount)/sizeof(T);
	  }

	  template<class T> size_t Write(const T* pData, size_t iCount) {
		  return WriteRAW((unsigned char*)pData, sizeof(T)*iCount)/sizeof(T);
	  }

	  template<class T> size_t Read(const T* pData, size_t iCount, uint64_t iPos,
								  uint64_t iOffset) {
		  SeekPos(iOffset+sizeof(T)*iPos);
		  return ReadRAW((unsigned char*)pData, sizeof(T)*iCount)/sizeof(T);
	  }

	  template<class T> size_t Write(const T* pData, size_t iCount, uint64_t iPos,
								   uint64_t iOffset) {
		  SeekPos(iOffset+sizeof(T)*iPos);
		  return WriteRAW((unsigned char*)pData, sizeof(T)*iCount)/sizeof(T);
	  }

	  template<class T> void ReadData(T& value, bool bIsBigEndian) {
		  ReadRAW((unsigned char*)&value, sizeof(T));
		  if (EndianConvert::IsBigEndian() != bIsBigEndian)EndianConvert::Swap<T>(value);
	  }

	  template<class T> void WriteData(const T& value, bool bIsBigEndian) {
		if (EndianConvert::IsBigEndian() != bIsBigEndian)
		  EndianConvert::Swap<T>(value);
		WriteRAW((unsigned char*)&value, sizeof(T));
		if (EndianConvert::IsBigEndian() != bIsBigEndian)
		  EndianConvert::Swap<T>(value);
	  }

    template<class T> void ReadData(std::vector<T> &value, bool bIsBigEndian) {
        uint64_t count=0;
        ReadRAW((unsigned char*)&count, sizeof(uint64_t));
        if (EndianConvert::IsBigEndian() != bIsBigEndian) {
          EndianConvert::Swap(count);
        }
        value.resize(size_t(count));
        if (!count) return;

        ReadRAW( (unsigned char*)&value[0], sizeof(T)*size_t(count));
        if (EndianConvert::IsBigEndian() != bIsBigEndian) {
          for (size_t i = 0; i < count; i++) {
            EndianConvert::Swap<T>(value[i]);
          }
        }
    }

    template<class T> void WriteData(const std::vector<T> &value, bool bIsBigEndian) {
        uint64_t count = value.size();

        if (EndianConvert::IsBigEndian() != bIsBigEndian) {
          for (size_t i = 0; i < count; i++) {
            EndianConvert::Swap<T>(value[i]);
          }
          EndianConvert::Swap(count);
        }
        WriteRAW((unsigned char*)&count, sizeof(uint64_t));
        if (count == 0) return;
        WriteRAW((unsigned char*)&value[0], sizeof(T)*size_t(count));
        if (EndianConvert::IsBigEndian() != bIsBigEndian) {
          for (size_t i = 0; i < count; i++) {
            EndianConvert::Swap<T>(value[i]);
          }
        }
    }

	  void ReadData(std::string &value, uint64_t count) {
		if (count == 0) return;
		value.resize(size_t(count));
		ReadRAW((unsigned char*)&value[0], sizeof(char)*size_t(count));
	  }

	  void WriteData(const std::string &value) {
		if (value.empty()) return;
		WriteRAW((unsigned char*)&value[0], sizeof(char)*size_t(value.length()));
	  }

	  static bool Copy(const std::string& strSource, const std::string& strTarget,
					   uint64_t iSourceHeaderSkip=0, std::string* strMessage=NULL);
	  static bool Copy(const std::wstring& wstrSource,
					   const std::wstring& wstrTarget, uint64_t iSourceHeaderSkip=0,
					   std::wstring* wstrMessage=NULL);
	  static bool Compare(const std::string& strFirstFile,
						  const std::string& strSecondFile,
						  std::string* strMessage=NULL);
	  static bool Compare(const std::wstring& wstrFirstFile,
						  const std::wstring& wstrSecondFile,
						  std::wstring* wstrMessage=NULL);
	protected:
	  FILETYPE      m_StreamFile;
	  std::string   m_strFilename;
	  bool          m_bIsOpen;
	  bool          m_bWritable;
	  uint64_t        m_iHeaderSize;
	};
}

#endif // LARGERAWFILE_H
