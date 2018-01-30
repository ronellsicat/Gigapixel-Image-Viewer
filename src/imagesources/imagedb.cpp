#include <cassert>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "imagesources/imagedb.h"

ImageDBObject::ImageDBObject() {}

ImageDBObject::~ImageDBObject() {}

bool ImageDBObject::Init(std::string sourceDir) {
  params_.source_dir = sourceDir;

  std::stringstream ss;
  ss << params_.source_dir << "/_info.txt";
  std::ifstream f(ss.str().c_str());
  if (!f.is_open()) {
    printf("ERROR: Cannot find info file %s.", ss.str().c_str());
    return false;
  }

  std::string curLine;
  getline(f, curLine);
  std::stringstream s(curLine);
  std::string temp;
  s >> temp;
  params_.file_format = temp;
  
  getline(f, curLine);
  s = std::stringstream(curLine);
  s >> temp;
  params_.start_index = std::atoll(temp.c_str());

  getline(f, curLine);
  s = std::stringstream(curLine);
  s >> temp;
  params_.end_index = std::atoll(temp.c_str());

  assert(params_.end_index >= params_.start_index);
  f.close();

  printf("ImageDBObject created from %s with %d images of %s format.\n",
         params_.source_dir.c_str(), params_.total_num_images(), params_.file_format.c_str());
  return true;
}

std::string ImageDBObject::GetImageFilename(int image_index) {
  std::stringstream tilefname;
  tilefname << params_.source_dir << "\\";
  tilefname << std::setw(10) << std::setfill('0') << image_index << "." << params_.file_format;
  return tilefname.str();
}