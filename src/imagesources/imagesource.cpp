#include "imagesources/imagesource.h"

ImageSourceObject::ImageSourceObject() {
  source_desc_.type = SourceType_INVALID;
  source_desc_.id = 0;
}

ImageSourceObject::~ImageSourceObject() {}
