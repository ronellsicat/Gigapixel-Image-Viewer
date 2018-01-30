#include "tiledimageexplorer/tiledimagedata.h"

TiledImageData::TiledImageData(QOpenGLWidget* parent, QSize tile_size)
    : parent_(parent),
      tile_size_(tile_size),
      tile_res_(QSize(-1, -1)) {}

TiledImageData::~TiledImageData() {
  CleanupGL();
}

TiledImageData& TiledImageData::operator=(const TiledImageData &buffer) {
  tile_res_ = buffer.tile_res_;
  tile_size_ = buffer.tile_size_;
  textures_ = buffer.textures_;  // TODO (ronell): Check if this is deep enough copy!
  parent_ = buffer.parent_;
  return *this;
}

void TiledImageData::ResizeTextures(QSize _tile_res) {
  ClearTextures();
  tile_res_ = _tile_res;
  textures_.resize(tile_res_.width() * tile_res_.height());
  std::fill(textures_.begin(), textures_.end(), nullptr);
}

void TiledImageData::ClearTextures() {
  // TODO (ronell): Program crashes when closing without opening an image.
  if (!TexturesIsEmpty()) {
    parent_->makeCurrent();
    for (size_t i = 0; i < textures_.size(); ++i) {
      if (textures_[i] != nullptr)
        textures_[i]->destroy();
      textures_[i] = nullptr;
    }
    textures_.clear();
  }
}

bool TiledImageData::TexturesIsEmpty() {
  return textures_.size() == 0;
}

void TiledImageData::RemoveTexture(int tx, int ty) {
  int index = ty * tile_res_.width() + tx;
  if (TexturesIsEmpty() || index < 0 || index >= int(textures_.size()) || textures_[index] == nullptr)
    return;
  parent_->makeCurrent();
  textures_[index]->destroy();
  textures_[index] = nullptr;
}

TexturePtr TiledImageData::ReplaceTexture(int tx, int ty, TexturePtr texture) {
  int index = ty * tile_res_.width() + tx;
  if (TexturesIsEmpty() || index < 0 || index >= int(textures_.size()))
    return nullptr;

  if (textures_[index] == nullptr) {
    textures_[index] = texture;
  } else {
    parent_->makeCurrent();
    textures_[index]->destroy();
    textures_[index] = texture;
    parent_->doneCurrent();
  }
  return textures_[index];
}

TexturePtr TiledImageData::GetTexture(int tx, int ty) {
  int index = ty * tile_res_.width() + tx;
  if (TexturesIsEmpty() || index < 0 || index >= int(textures_.size())) {
    return nullptr;
  }
  return textures_[index];
}

QRect TiledImageData::GetVisibleTileRange(QPointF view_offset, QSize view_size, 
                                          QPointF draw_scale) {
  float dimX = tile_size_.width() * draw_scale.x();
  float dimY = tile_size_.height() * draw_scale.y();

  int tx1 = -view_offset.x() / dimX;
  int tx2 = (view_size.width() - view_offset.x()) / dimX;
  int ty1 = -view_offset.y() / dimY;
  int ty2 = (view_size.height() - view_offset.y()) / dimY;

  tx1 = qMax(0, tx1);
  tx2 = qMin(tile_res_.width() - 1, tx2);
  ty1 = qMax(0, ty1);
  ty2 = qMin(tile_res_.height() - 1, ty2);

  return QRect(QPoint(tx1, ty1), QPoint(tx2, ty2));
}

void TiledImageData::CleanupGL() {
  ClearTextures();  
}
