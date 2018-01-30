/*
 The MIT License
 
 Copyright (c) 2012 IVDA & GMSV Group @ KAUST
  
 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
 */

/**
 \author    Jens Krueger, IVDA, SCI, Intel VCI 
 */


#ifndef _FILEBACKEDDATA_H
#define _FILEBACKEDDATA_H

#include "StdInclude.h"
#include "LargeRAWFile.h"


class FileBackedData {
public:

  enum DSType {
    DS_OTHER = 0,
    DS_INPUT_IMAGE = 1,
    DS_INPUT_HIST_VOL = 2,
    DS_IMAGE_MIP = 3,
    DS_SHM_MIP = 4,
    DS_RBF_LEVEL = 5,
    DS_RBF_LEVEL_SORTED = 6,
	  DS_RBF_LEVEL_SORTED_SINGLE_TILE = 7,
    DS_VSHM_MIP = 8,
  };

  FileBackedData(const std::string& id);
  FileBackedData(const std::string& id, uint64_t iSize);
  
  static const std::string AddExtension(const std::string& strFilename) {
    return strFilename + "." + Extension();
  }

  static const std::string Extension() {
    return "tmp";
  }

  static void ClearCaches(const std::string& strDirectory);

  bool IsEmpty() const {return m_bIsEmpty;}
  
  template<class T> size_t Read(const T* pData, size_t iCount, uint64_t iPos) {
    return m_LargeRawFile.Read<T>(pData, iCount, iPos, 0);
  }

  template<class T> size_t Write(const T* pData, size_t iCount, uint64_t iPos) {
    return m_LargeRawFile.Write<T>(pData, iCount, iPos, 0);
  }

  template<class T> void Read(std::vector<T>& vData) {
    m_LargeRawFile.ReadData<T>(vData, IVDA::EndianConvert::IsBigEndian());
  }

  template<class T> void Write(const std::vector<T>& vData) {
    m_LargeRawFile.WriteData<T>(vData, IVDA::EndianConvert::IsBigEndian());
  }

  template<class T> size_t Read(const T* pData, size_t iCount=1) {
    return m_LargeRawFile.Read<T>(pData, iCount);
  }

  template<class T> size_t Write(const T* pData, size_t iCount=1) {
    return m_LargeRawFile.Write<T>(pData, iCount);
  }

  uint64_t GetPos() {
    return m_LargeRawFile.GetPos();
  }

  void Seek(uint64_t iPos) {
    m_LargeRawFile.SeekPos(iPos);
  }

  void Skip(uint64_t iPos) {
    // TODO: optimize
    m_LargeRawFile.SeekPos(iPos+m_LargeRawFile.GetPos());
  }

  void Close() {
    m_LargeRawFile.Close();
    m_id = "";
  }

  void Delete() {
    m_LargeRawFile.Delete();
    m_id = "";
  }

  std::string GetFilename() const {return m_id;}

private:
  IVDA::LargeRAWFile m_LargeRawFile;
  bool m_bIsEmpty;
  std::string m_id;

};


#endif // _FILEBACKEDDATA_H
