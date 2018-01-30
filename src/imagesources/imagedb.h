#ifndef GIGAPATCHEXPLORER_IMAGE_DB_H_
#define GIGAPATCHEXPLORER_IMAGE_DB_H_

#include <string>

#include "common.h"
#include "imagesources/imagesource.h"

// Contains all information about an image database representation.
// Currently works with a set of images named with the image ids in a single directory.
struct ImageDBParams { 
  int start_index;
  int end_index;
  int total_num_images() {
    return (end_index - start_index) + 1;
  };						        // Total number of images.
  std::string source_dir;					          // Source directory for images.
  std::string file_format;                  // "jpg", "png"
};

// Encapsulates an out-of-core image data base that resides in a single source
// directory i.e. all images, and information are in source directory.
class ImageDBObject : public ImageSourceObject {

public:
  ImageDBObject();
  ~ImageDBObject();

  // Initializes to the image data found in sourceDir. Uses _info.txt inside sourceDir.
  // Returns true when successful.
  bool Init(std::string sourceDir);
  std::string GetImageFilename(int image_index);
  ImageDBParams GetParamsCopy() { return params_; }
  std::string source_dir() { return params_.source_dir; }
  int start_index() { return params_.start_index; }
  int end_index() { return params_.end_index; }
  int total_num_images() { return params_.total_num_images(); };
  
private:
  ImageDBParams params_;
};

#endif  // GIGAPATCHEXPLORER_IMAGE_DB_H_
