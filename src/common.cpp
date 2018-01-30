#include "common.h"

QSize Size2DIntToQSize(Size2DInt in_size) {
  QSize out_size(in_size.width, in_size.height);
  return out_size;
}