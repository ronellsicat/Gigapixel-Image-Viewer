#ifndef GIGAPATCHEXPLORER_EXPLORER_DRAWTILE_H_
#define GIGAPATCHEXPLORER_EXPLORER_DRAWTILE_H_

#include <memory>

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLWidget>

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram);
QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)

// Handles drawing stuff per tile (textures, points, etc.) on the parent QOpenGLWidget.
// The global translation and scale can be set, while the local translation is specified
// during the per tile draw call.
class DrawTile {

public:
  DrawTile(QOpenGLWidget *parent = 0);
  virtual ~DrawTile();

  // Initialize resources for drawing (quad and texture coordinates, shaders, VBOs, etc.)
  void Init(QSize _tile_size);
  void SetGlobalTranslation(QPointF translation);
  void SetGlobalScaleFactor(QPointF scale_factor);
  void UpdateTextureCoordsScale(QPointF texcoords_scale_factor);
  void UpdateTextureCoordsShift(QPointF texcoords_shift);
  void UpdateBorderPercentage(float border_percentage);
  void UpdateBorderColor(QColor border_color);
  void DrawTileAt(QPointF local_translation, QOpenGLTexture *texture);
  // We make the clean up function public so that the parent can call it anytime.
  void CleanupGL();

private:
  void UpdateGlobalTransformMatrix();

  std::shared_ptr<QOpenGLShaderProgram> default_shader_program_;
  QPointF tile_size_;
  QOpenGLWidget *parent_;               // Contains active OpenGL context where to draw.
  QOpenGLBuffer vbo_;                   // Contains tile's quad vertices and texture coords.
  QMatrix4x4 global_transform_matrix_;  // Transform applied to all tiles drawn.
  QPointF global_translation_;
  QPointF global_scale_factor_;
  float border_percentage_;              // Percentage of quad size for border.
  QColor border_color_;
};

#endif  // GIGAPATCHEXPLORER_EXPLORER_DRAWTILE_H_
