#ifndef GIGAPATCHEXPLORER_EXPLORER_TILEDIMAGEEXPLORER_H_
#define GIGAPATCHEXPLORER_EXPLORER_TILEDIMAGEEXPLORER_H_

#include <memory>

#include <QCoreApplication>
#include <QEventLoop>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLWidget>
#include <QPainter>
#include <QRubberBand>
#include <QTime>

#include "drawing/drawonwindow.h"
#include "drawing/drawtile.h"
#include "drawing/texturecache.h"
#include "tiledimageexplorer/tiledimagedata.h"
#include "imagesources/tiledimage.h"

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram);
QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)

// Contains parameters for the viewing window and interactions (panning and zooming).
// Encapsulates two resolution levels being viewed - current level, which is what we want to view,
// and previous level, which is what we were looking at before.
struct ViewParams {

  QPointF view_offset;    // Offset of upper left corner of image from window corner.
  float cur_level_exact;  // Exact resolution level being viewed - valid range: [0, num_levels-1]
  float cur_draw_scale;   // Scaling factor [0.5, 2.0] for current tiles.
  int prev_level;         // Resolution level of previous tiles.
  float prev_draw_scale;  // Scaling factor [0.5, 2.0] for previous tiles.
  int cur_level() {       // Current resolution level (round down exact current level).
    return int(cur_level_exact + 0.5f);
  }
};

struct ImageSelection {
  QRubberBand *rect;
  QPoint origin;
  int start_level;
  int end_level;
  std::vector<ROI> roi_per_level;
  int num_levels() {
    return int(roi_per_level.size());
  }
  void clear() {
    roi_per_level.clear();
  }
};

// QtOpenGLWidget where we display/interact with the "attached" TiledImageOBject and information
// associated with exploring it e.g. scale x space positions, etc.
// This widget is dockable. The minimum window size is 300 x 300 and the default is 1280 x 720.
class TiledImageExplorer : public QOpenGLWidget {
  Q_OBJECT

public:
  explicit TiledImageExplorer(QWidget *parent = 0);
  ~TiledImageExplorer();

  // Returns true when TiledImageObject is attached successfully.
  bool AttachTiledImageObject(std::shared_ptr<TiledImageObject> tiled_image_object);
  void SetClearColor(const QColor &color);
  QOpenGLTexture* GetPlaceholderTexture() { return texture_placeholder_.get(); }

  // Do not change the names of the following window size related functions.
  QSize minimumSizeHint() const Q_DECL_OVERRIDE;
  QSize sizeHint() const Q_DECL_OVERRIDE;
  void SetPatchCoordsToDraw(std::vector<PatchCoords>& patches_to_draw);
  void SetPatchPointersSize(int pointers_size);
  void SetLookaheadDepth(int depth);
  float GetPatchPointersSize();
  void SetCoarseLevelPatchPointersVisibility(bool value);
  void SetCurrentLevelPatchPointersVisibility(bool value);
  void SetFineLevelPatchPointersVisibility(bool value);
  void SetFocusPatchParams(FocusPatchParams params);
  void SetSelectionDepth(int depth);
  ImageSelection& GetCurrentSelection() {
    return image_selection_;
  };
  SourceDesc CurrentSource() {
    return tiled_image_object_->source_desc();
  }
  void ForceAllPatchPointersColorTo(QColor color) {
	  current_patch_pointers_color_ = color;
	  coarse_patch_pointers_color_ = color;
	  fine_patch_pointers_color_ = color;
	  update();
  }
  void ResetView();
  void TestMouse();
  void ZoomToPosition(int level, int global_x, int global_y, 
                      int num_steps = 20, int millisecs_delay_per_step = 50);
  void UseTextureCache(QTextureCache* texture_cache) {
    if (texture_cache == nullptr)
      return;

    texture_cache_ = texture_cache;
  }
  int GetCurrentSourceMaxResolutionLevel() {
    return tiled_image_object_->num_levels();
  }
  QColor GetCoarsePatchPointersColor() {
    return coarse_patch_pointers_color_;
  }
  QColor GetCurrentPatchPointersColor() {
    return current_patch_pointers_color_;
  }
  QColor GetFinePatchPointersColor() {
    return fine_patch_pointers_color_;
  }

signals:
  void SelectionSignalEmitted();

  public slots:
  void EmitSelectionSignal();

signals:
  void clicked();

  // The following protected functions are overridden so do not attempt to rename them.
protected:
  void initializeGL() Q_DECL_OVERRIDE;
  void paintGL() Q_DECL_OVERRIDE;
  void paintQt();
  void resizeGL(int width, int height) Q_DECL_OVERRIDE;
  void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
  void wheelEvent(QWheelEvent *event);
  
private:
  void AdjustGlobalTranslation(QPointF translation_delta);
  void AdjustSelectionTranslation(int dx, int dy);
  void AdjustGlobalZoom(int zoom_delta, QPoint zoom_center);
  void CleanupGL();
  // We initialize the view parameters so that the whole image fits into the window.
  void InitViewParams();
  void UpdateViewParams(float level_delta, QPoint zoom_center);
  bool InitTiledImageData();
  void RefreshTiledImageData();
  void DrawTiles();
  void DrawCurrentTilesGlobal();
  void DrawPreviousTilesGlobal();
  void UpdateSingleTileGlobal(int level, int tx, int ty);
  void ToggleDisplayTileDebugInfo();
  void DrawPatchPointers();
  void AssignPatchPointersColors();
  void PaintSinglePatchPointer(QPainter *painter, PatchCoords patch_coords);
  void DrawSinglePatchPointer(PatchCoords patch_coords, QSize size, QColor color);
  void DrawFocusPatchPointer();
  void UpdateSelectionRange();
  void UpdateSelectionROIs();
  QPoint ConvertToImagePosInLevel(QPoint window_pos, int level);
  QSize ConvertToImageSizeInLevel(QSize window_size, int level);
  void PaintFPS(QPainter *painter);
  void PaintResolutionLevel(QPainter *painter);

  static std::shared_ptr<QImage> LoadTileJPG(const std::string& filename);
  static QImage CreateCheckerboardPattern(QSize size);

  bool display_tile_debug_info_;
  bool display_patch_pointers_for_coarse_levels_;
  bool display_patch_pointers_for_current_level_;
  bool display_patch_pointers_for_fine_levels_;
  int lookahead_depth_;
  QColor clear_color_;
  QColor coarse_patch_pointers_color_;
  QColor current_patch_pointers_color_;
  QColor fine_patch_pointers_color_;
  QColor single_patch_color_;
  QColor single_patch_color_bigger_;
  QSize patch_pointer_min_size_;
  QSize patch_pointer_target_size_;
  std::vector<QColor> patch_pointers_color_per_level_;
  QPoint last_mouse_pos_;
  std::shared_ptr<TiledImageObject> tiled_image_object_;
  ViewParams view_params_;
  std::shared_ptr<DrawOnWindow> draw_on_window_;
  std::shared_ptr<DrawOnWindow> draw_focus_patch_on_window_;
  std::shared_ptr<DrawTile> draw_tile_;
  TiledImageData previous_tiles_;
  TiledImageData current_tiles;
  std::shared_ptr<QOpenGLTexture> texture_placeholder_;
  Size2DInt extra_tiles_;                 // Extra tiles to load from each TiledImageData.
  QOpenGLFunctions *opengl_functions_ptr_; // Use this to call raw OpenGL functions.
  std::vector<PatchCoords> patches_to_draw_;
  FocusPatchParams focus_patch_params_;

  ImageSelection image_selection_;
  QTextureCache* texture_cache_;
  bool draw_current_level_;
};

inline void QTDelay(int millisecondsToWait) {
  QTime dieTime = QTime::currentTime().addMSecs(millisecondsToWait);
  while (QTime::currentTime() < dieTime) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
  }
}

#endif  // GIGAPATCHEXPLORER_EXPLORER_TILEDIMAGEEXPLORER_H_
