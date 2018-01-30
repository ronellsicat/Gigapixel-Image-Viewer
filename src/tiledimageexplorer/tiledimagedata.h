#ifndef GIGAPATCHEXPLORER_EXPLORER_TILEDIMAGEDATA_H_
#define GIGAPATCHEXPLORER_EXPLORER_TILEDIMAGEDATA_H_

#include <memory>

#include <QOpenGLTexture>
#include <QOpenGLWidget>

QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)

typedef std::shared_ptr<QOpenGLTexture> TexturePtr;

// Contains the data needed for a single 2D array of tiles e.g. textures, etc. for the OpenGL 
// context of its parent QOpenGLWidget.
class TiledImageData {

public:
  TiledImageData() {}
  TiledImageData(QOpenGLWidget* parent, QSize tile_size);
  ~TiledImageData();
  TiledImageData& operator=(const TiledImageData&);

  QSize tile_res() { return tile_res_; }
  // Deletes the old textures and creates a new empty one with the new resolution.
  void ResizeTextures(QSize _tile_res);
  // Deletes all textures (keeps resolution).
  void ClearTextures();
  bool TexturesIsEmpty();
  void RemoveTexture(int tx, int ty);
  TexturePtr ReplaceTexture(int tx, int ty, TexturePtr texture);
  TexturePtr GetTexture(int tx, int ty);
  QRect GetVisibleTileRange(QPointF view_offset, QSize view_size, QPointF draw_scale);
  // We make the clean up function public so that the parent can call it anytime.
  void CleanupGL();

private:
  QOpenGLWidget* parent_;
  QSize tile_size_;
  QSize tile_res_;
  std::vector<TexturePtr> textures_;
};

#endif  // GIGAPATCHEXPLORER_EXPLORER_TILEDIMAGEDATA_H_
