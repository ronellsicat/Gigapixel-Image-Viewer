#ifndef GIGAPATCHEXPLORER_IMAGE_IMAGESOURCE_H_
#define GIGAPATCHEXPLORER_IMAGE_IMAGESOURCE_H_

#include "common.h"

class ImageSourceObject {
public:
  ImageSourceObject();
  ~ImageSourceObject();

  SourceDesc source_desc() {
    return source_desc_;
  }

  void SetSourceDesc(SourceDesc source_desc) {
    source_desc_ = source_desc;
  }

  virtual int total_num_images() = 0;

private:
  SourceDesc source_desc_;
};

#endif  // GIGAPATCHEXPLORER_IMAGE_IMAGESOURCE_H_
