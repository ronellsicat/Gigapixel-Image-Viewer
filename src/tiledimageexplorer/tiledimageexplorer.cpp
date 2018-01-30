#include <QMessageBox>
#include <QMouseEvent>

#include "tiledimageexplorer/tiledimageexplorer.h"

// For buffering the display, we use two TiledImageData levels, one for the currently viewed level
// and another for the previous level. We do this so that we can display the previous level's tile
// temporarily in place of missing tiles for the current level, while they are not loaded yet.
// 
// For figuring out the levels and scale factors, we keep track of view_params_.cur_level_exact 
// which the mouse basically changes when zooming in/out.
//
// IMPORTANT: In order to dock/undock this widget, the OpenGL resources for the context should be
// cleaned up appropriately upon deletion, and re-initialized in the constructor. Thus, during
// initialization, we make sure CleanupGL() is called when the OpenGL context is about to be 
// destroyed.

const int EXTRA_TILES_TO_LOAD = 4;

TiledImageExplorer::TiledImageExplorer(QWidget *parent)
    : QOpenGLWidget(parent),
      display_tile_debug_info_(true),
      extra_tiles_(EXTRA_TILES_TO_LOAD, EXTRA_TILES_TO_LOAD),
      display_patch_pointers_for_coarse_levels_(true),
      display_patch_pointers_for_current_level_(true),
      display_patch_pointers_for_fine_levels_(true),
      lookahead_depth_(0),
      opengl_functions_ptr_(nullptr),
      texture_placeholder_(nullptr),
      tiled_image_object_(nullptr),
      draw_on_window_(std::make_shared<DrawOnWindow>(this)),
      draw_focus_patch_on_window_(std::make_shared<DrawOnWindow>(this)),
      draw_tile_(std::make_shared<DrawTile>(this)),
      clear_color_(Qt::black),    
      single_patch_color_(Qt::magenta),
      single_patch_color_bigger_(Qt::cyan),
      coarse_patch_pointers_color_(QColor(44, 123,182, 220)),
      current_patch_pointers_color_(QColor(255, 255, 0, 220)),
      fine_patch_pointers_color_(QColor(215, 25, 28, 100)),
      draw_current_level_(true){

  patch_pointer_min_size_ = QSize(16, 16);
  patch_pointer_target_size_ = QSize(64, 64);
  view_params_.cur_level_exact = 0.0f;
  focus_patch_params_.enabled = false;
  image_selection_.rect = new QRubberBand(QRubberBand::Rectangle, this);
}

TiledImageExplorer::~TiledImageExplorer() {
  CleanupGL();
}

void TiledImageExplorer::keyPressEvent(QKeyEvent *event) {
  
  printf("Key pressed.\n");
  if (event->key() == Qt::Key_R) {
    ResetView();
  }

  if (event->key() == Qt::Key_M) {
    TestMouse();
  }
}

void TiledImageExplorer::ResetView() {
	InitViewParams();
	InitTiledImageData(); // Initialize empty tiled image data containers.
	initializeGL();
	image_selection_.rect->hide();
	update();
}

void TiledImageExplorer::TestMouse() {
  /*
  QMouseEvent* test = new QMouseEvent(QEvent::MouseMove, QPointF(0, 0), Qt::MouseButton::NoButton, Qt::MouseButton::NoButton,
                                      Qt::KeyboardModifier::ShiftModifier);
  mouseMoveEvent(test);
  update();
  */
  ZoomToPosition(0, 0, 0, 20, 10);
}

void TiledImageExplorer::ZoomToPosition(int level, int global_x, int global_y, 
                                        int num_steps, int millisecs_delay_per_step) {

  QPoint current_image_pos_centered = ConvertToImagePosInLevel(
    QPoint(width() / 2, height() / 2), view_params_.cur_level());

  QPointF target_image_pos_centered(
    float(global_x) * std::pow(2, view_params_.cur_level_exact - float(level)), 
    float(global_y) * std::pow(2, view_params_.cur_level_exact - float(level)));

  // Compute step size in zoom level.
  float current_level = view_params_.cur_level_exact;
  float target_level = float(level);
  float total_zoom_diff = target_level - current_level;
  float zoom_delta = total_zoom_diff / float(num_steps);

  // Compute step size in image space at current level.
  float x_delta = (target_image_pos_centered.x() - float(current_image_pos_centered.x())) / 
    float(num_steps);
  float y_delta = float(target_image_pos_centered.y() - float(current_image_pos_centered.y())) /
    float(num_steps);

  // Do steps.
  float prev_level_exact = view_params_.cur_level_exact;
  for (int step = 0; step < num_steps; ++step) {

    // Translate. To move the center point to one direction, we need to add offsets with the opposite
    // sign.
    AdjustGlobalTranslation(QPointF(-x_delta, -y_delta));

    // Zoom. Always use the center of the window as zoom center pos.
    UpdateViewParams(zoom_delta, QPoint(width()/2, height()/2));

    // Adjust x and y delta based on new level
    x_delta = x_delta * std::pow(2, view_params_.cur_level_exact - prev_level_exact);
    y_delta = y_delta * std::pow(2, view_params_.cur_level_exact - prev_level_exact);
    prev_level_exact = view_params_.cur_level_exact;

    QTDelay(millisecs_delay_per_step);
  }
}

bool TiledImageExplorer::AttachTiledImageObject(std::shared_ptr<TiledImageObject> tiled_image_object) {
  if (tiled_image_object == nullptr)
    return false;

  tiled_image_object_ = tiled_image_object;
  InitViewParams();     // Compute initial view parameters so image fits in window.
  InitTiledImageData(); // Initialize empty tiled image data containers.
  initializeGL();       // Initialize GL with correct tiled image parameters.
  image_selection_.rect->hide();
  return true;
}

void TiledImageExplorer::SetClearColor(const QColor &color) {
  clear_color_ = color;
  update();
}

QSize TiledImageExplorer::minimumSizeHint() const {
  return QSize(300, 300);
}

QSize TiledImageExplorer::sizeHint() const {
  return QSize(64 * 21, 64 * 10);
}

void TiledImageExplorer::SetPatchCoordsToDraw(std::vector<PatchCoords>& patches_to_draw) {
  patches_to_draw_ = patches_to_draw;
  update();
}

void TiledImageExplorer::SetPatchPointersSize(int pointers_size) {
  if (draw_on_window_ == nullptr)
    return;
  draw_on_window_->SetPatchPointSize(float(pointers_size));
  update();
}

void TiledImageExplorer::SetLookaheadDepth(int depth) {
  lookahead_depth_ = depth;
  update();
}

float TiledImageExplorer::GetPatchPointersSize() {
  return (draw_on_window_ == nullptr) ? 0.0f : draw_on_window_->point_size();
}

void TiledImageExplorer::SetCoarseLevelPatchPointersVisibility(bool value) {
  display_patch_pointers_for_coarse_levels_ = value;
  update();
}

void TiledImageExplorer::SetCurrentLevelPatchPointersVisibility(bool value) {
  display_patch_pointers_for_current_level_ = value;
  update();
}

void TiledImageExplorer::SetFineLevelPatchPointersVisibility(bool value) {
  display_patch_pointers_for_fine_levels_ = value;
  update();
}

void TiledImageExplorer::SetFocusPatchParams(FocusPatchParams params) {
  focus_patch_params_ = params;
}

void TiledImageExplorer::SetSelectionDepth(int depth) {
  image_selection_.end_level = depth;
  UpdateSelectionRange();
}

void TiledImageExplorer::EmitSelectionSignal() {
  emit SelectionSignalEmitted();
}

void TiledImageExplorer::initializeGL() {
  connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &TiledImageExplorer::CleanupGL);
  if (opengl_functions_ptr_ == nullptr) {
    opengl_functions_ptr_ = new QOpenGLFunctions();
    opengl_functions_ptr_->initializeOpenGLFunctions();
  }

  if (tiled_image_object_ == nullptr)
    return;

  QSize tile_size(tiled_image_object_->tile_size().width, tiled_image_object_->tile_size().height);
  draw_tile_->Init(tile_size);
  draw_on_window_->Init(size());
  draw_focus_patch_on_window_->Init(size());
  AssignPatchPointersColors();
  if (texture_placeholder_ == nullptr) {
    texture_placeholder_ = std::make_shared<QOpenGLTexture>(CreateCheckerboardPattern(tile_size));
  }


  InitTiledImageData(); // We also call this here in case of docking/undocking so that tiles 
                        // will be refreshed.
  
  patches_to_draw_.clear();
}

void TiledImageExplorer::paintGL() {
  QPainter painter(this);
  painter.beginNativePainting();

  opengl_functions_ptr_->glClearColor(clear_color_.redF(), clear_color_.greenF(),
                                      clear_color_.blueF(), clear_color_.alphaF());
  opengl_functions_ptr_->glDisable(GL_DEPTH_TEST);
  opengl_functions_ptr_->glClear(GL_COLOR_BUFFER_BIT);
  opengl_functions_ptr_->glDisable(GL_BLEND);
  
  DrawTiles();

  opengl_functions_ptr_->glEnable(GL_BLEND);
  DrawPatchPointers();
//  DrawFocusPatchPointer();

  painter.endNativePainting();
  
  // Call Qt related draws:
  paintQt();
}

void TiledImageExplorer::paintQt() {

  QPainter painter(this);
  //PaintFPS(painter);
  if (draw_current_level_) {
    PaintResolutionLevel(&painter);
  }
  if (focus_patch_params_.enabled) {
    PaintSinglePatchPointer(&painter, focus_patch_params_.coords);
  }
}

void TiledImageExplorer::PaintFPS(QPainter *painter) {
  QColor white_transparent = QColor(255, 255, 255, 100);
  const int fontSize = 15;
  painter->setPen(white_transparent);
  painter->setFont(QFont("helvetica", fontSize));
  painter->drawText(20, painter->font().pointSize() + 20,
                   QString(QString::number(0.01) + " sec last frame"));
}

void TiledImageExplorer::PaintResolutionLevel(QPainter *painter) {
  QColor white_transparent = QColor(255, 255, 255, 100);
  const int fontSize = 15;
  painter->setPen(white_transparent);
  painter->setFont(QFont("helvetica", fontSize));
  painter->drawText(20, painter->font().pointSize() + 20,
                    QString("level " + QString::number(view_params_.cur_level())));
}

void TiledImageExplorer::resizeGL(int width, int height) {
  int side = qMin(width, height);
  opengl_functions_ptr_->glViewport((width - side) / 2, (height - side) / 2, side, side);
}

void TiledImageExplorer::mousePressEvent(QMouseEvent *event) {
  last_mouse_pos_ = event->pos();

  // Create new selection when SHIFT key is held.
  if (event->modifiers() & Qt::ShiftModifier) {
    image_selection_.clear();
    image_selection_.origin = event->pos();
    UpdateSelectionRange();
    image_selection_.rect->hide();
    image_selection_.rect->setGeometry(QRect(image_selection_.origin, QSize()));
    image_selection_.rect->show();
    printf("Image selection level range %d - %d.\n", image_selection_.start_level,
           image_selection_.end_level);
  }
}

void TiledImageExplorer::mouseMoveEvent(QMouseEvent *event) {
  int dx = event->x() - last_mouse_pos_.x();
  int dy = event->y() - last_mouse_pos_.y();

  // Only allow drawing selection from top left to lower right:
  if (event->modifiers() & Qt::ShiftModifier) { 
    image_selection_.rect->setGeometry(QRect(image_selection_.origin, event->pos()).normalized());
  } else {
    if (event->buttons() & Qt::LeftButton) {
      AdjustSelectionTranslation(dx, dy);
      AdjustGlobalTranslation(QPointF(float(dx), float(dy)));
    }
  }

  last_mouse_pos_ = event->pos();
}

void TiledImageExplorer::mouseReleaseEvent(QMouseEvent * event) {
  if (image_selection_.rect->isVisible() && event->modifiers() & Qt::ShiftModifier) {
    if ((image_selection_.origin.x() < last_mouse_pos_.x()) &&
        (image_selection_.origin.y() < last_mouse_pos_.y())) {
      UpdateSelectionROIs();
      EmitSelectionSignal();
    } else {
      QMessageBox warning;
      warning.setText("Draw selection from top left to bottom right.");
      warning.exec();
    }
  }
}

void TiledImageExplorer::wheelEvent(QWheelEvent *event) {
  if (tiled_image_object_ == nullptr)
    return;
  // TODO (ronell): Adjust selection. For now hide it.
  image_selection_.rect->hide();

  AdjustGlobalZoom(event->delta(), event->pos());
  event->accept();
}

void TiledImageExplorer::AdjustGlobalTranslation(QPointF translation_delta) {
  view_params_.view_offset += translation_delta;
  update();
}

void TiledImageExplorer::AdjustSelectionTranslation(int dx, int dy) {
  if (image_selection_.rect == nullptr)
    return;
  image_selection_.origin.setX(image_selection_.origin.x() + dx);
  image_selection_.origin.setY(image_selection_.origin.y() + dy);
  image_selection_.rect->move(image_selection_.origin);
}

void TiledImageExplorer::AdjustGlobalZoom(int zoom_delta, QPoint zoom_center) {
  // TODO (ronell): This section of code is from the SVG viewer ("backing store"). I'm not really
  // sure what king of scaling is used here, but it seems to work ok.
  // Lower the zoom sensitivity for more sensitive mouse (default: kZoomSensitivity = 0.6f).
  const float kZoomSensitivity = 0.6f;
  float level_delta = (qreal(zoom_delta * (view_params_.cur_level() + 1)) * 0.25) /
    (1200.0f * kZoomSensitivity);
  UpdateViewParams(level_delta, zoom_center);
}

void TiledImageExplorer::CleanupGL() {
  if (!context()->isValid()) {
    return;
  }
  makeCurrent();
  texture_placeholder_ = nullptr;
  draw_tile_->CleanupGL();
  draw_on_window_->CleanupGL();
  draw_focus_patch_on_window_->CleanupGL();
  current_tiles.CleanupGL();
  previous_tiles_.CleanupGL();
  delete(opengl_functions_ptr_);
  opengl_functions_ptr_ = nullptr;
  doneCurrent();
}

void TiledImageExplorer::InitViewParams() {
  // Compute refLevel is resolution level of image that fits in display window:
  int ref_level = tiled_image_object_->num_levels() - 1;
  for (int l = 1; l < tiled_image_object_->num_levels(); ++l) {
    if (tiled_image_object_->imgres_for_level(l).width >= width() ||
        tiled_image_object_->imgres_for_level(l).height >= height()) {

      ref_level = l - 1;
      break;
    }
  }

  view_params_.cur_level_exact = float(ref_level);
  view_params_.prev_level = view_params_.cur_level();
  view_params_.cur_draw_scale = view_params_.prev_draw_scale = 1.0f;
  view_params_.view_offset.setX((float(width()) / 2.0f) - 
                                (float(tiled_image_object_->imgres_for_level(ref_level).width) /
                                2.0f));
  view_params_.view_offset.setY((float(height()) / 2.0f) -
                                (float(tiled_image_object_->imgres_for_level(ref_level).height) /
                                2.0f));
}

void TiledImageExplorer::UpdateViewParams(float level_delta, QPoint zoom_center) {
  int prev_level = view_params_.cur_level();
  float prev_cur_level_exact = view_params_.cur_level_exact;
  float prev_draw_scale = view_params_.cur_draw_scale;

  // Update exact level and drawing scale factor for current level:
  view_params_.cur_level_exact = qBound(0.0f, view_params_.cur_level_exact + level_delta,
                                       float(tiled_image_object_->num_levels() - 1));
  view_params_.cur_draw_scale = pow(2.0f,
                                   view_params_.cur_level_exact - float(view_params_.cur_level()));

  // Update view offset: we center the zooming relative to the mouse position,
  // hence the translation before and after the scaling.
  QPointF pos(float(zoom_center.x()), float(zoom_center.y()));
  QPointF center = pos - view_params_.view_offset;
  center *= (pow(2.0f, view_params_.cur_level_exact) / pow(2.0f, prev_cur_level_exact));
  view_params_.view_offset = pos - center;

  // Update resolution levels if we switch from one level to another:
  if (prev_level != view_params_.cur_level()) {

    view_params_.prev_level = prev_level;
    // If there is a level switch, update the TiledImageData parameters and contents.
    RefreshTiledImageData();
    printf("Switched from level %d to %d.\n", view_params_.prev_level, view_params_.cur_level());
  }

  // Update drawing scale factor for previous level.
  view_params_.prev_draw_scale = pow(2.0f,
                                     view_params_.cur_level_exact - float(view_params_.prev_level));
  update();
}

bool TiledImageExplorer::InitTiledImageData() {
  if (tiled_image_object_->tile_size().width <= 0 || tiled_image_object_->tile_size().height <= 0)
    return false;

  previous_tiles_ = TiledImageData(this, Size2DIntToQSize(tiled_image_object_->tile_size()));
  current_tiles = TiledImageData(this, Size2DIntToQSize(tiled_image_object_->tile_size()));

  if (tiled_image_object_->num_levels() > 0) {
    current_tiles.ResizeTextures(Size2DIntToQSize(
      tiled_image_object_->tileres_for_level(view_params_.cur_level())));
  }
  return true;
}


void TiledImageExplorer::RefreshTiledImageData() {

  previous_tiles_.ClearTextures();
  previous_tiles_ = current_tiles;

  current_tiles = TiledImageData(this, Size2DIntToQSize(tiled_image_object_->tile_size()));
  current_tiles.ResizeTextures(Size2DIntToQSize(
    tiled_image_object_->tileres_for_level(view_params_.cur_level())));
}

void TiledImageExplorer::DrawTiles() {
  if (tiled_image_object_ == nullptr) 
    return;   // don't draw anything if not initialized

  DrawPreviousTilesGlobal();
  DrawCurrentTilesGlobal();
}

void TiledImageExplorer::DrawCurrentTilesGlobal() {
  if (tiled_image_object_ == nullptr)
    return; // Don't draw anything if there is no object attached

  draw_tile_->SetGlobalTranslation(view_params_.view_offset);
  draw_tile_->SetGlobalScaleFactor(QPointF(view_params_.cur_draw_scale,
    view_params_.cur_draw_scale));
  QRect tile_range = current_tiles.GetVisibleTileRange(view_params_.view_offset, size(),
                                                       QPointF(view_params_.cur_draw_scale,
                                                       view_params_.cur_draw_scale));

  int next_tile_tx = -1;
  int next_tile_ty = -1;
  
  for (int ty = tile_range.top(); ty <= tile_range.bottom(); ++ty) {
    for (int tx = tile_range.left(); tx <= tile_range.right(); ++tx) {

      QPointF tileTranslation(tx * tiled_image_object_->tile_size().width,
                              ty * tiled_image_object_->tile_size().height);


      std::string tilename = tiled_image_object_->GetTileFilename(view_params_.cur_level(), tx, ty);
      if (texture_cache_->Contains(tilename)) {

        // Draw textured quad for this tile at given location (local translation).
        draw_tile_->DrawTileAt(tileTranslation, texture_cache_->GetTexture(tilename));

      } else if (next_tile_tx == -1) {  // Mark for loading next time
          next_tile_tx = tx;
          next_tile_ty = ty;
      }
    }
  }

  // Load needed tile
  if (next_tile_tx > -1) {
    UpdateSingleTileGlobal(view_params_.cur_level(), next_tile_tx, next_tile_ty);
    update();
  }
}

void TiledImageExplorer::DrawPreviousTilesGlobal() {
  if (tiled_image_object_ == nullptr)
    return; // Don't draw anything if there is no object attached

  draw_tile_->SetGlobalTranslation(view_params_.view_offset);
  
  draw_tile_->SetGlobalScaleFactor(QPointF(view_params_.prev_draw_scale,
       view_params_.prev_draw_scale));
  QRect tile_range = previous_tiles_.GetVisibleTileRange(view_params_.view_offset, size(),
                                                           QPointF(view_params_.prev_draw_scale,
                                                           view_params_.prev_draw_scale));

  for (int ty = tile_range.top(); ty <= tile_range.bottom(); ++ty) {
    for (int tx = tile_range.left(); tx <= tile_range.right(); ++tx) {
    
        QPointF tileTranslation(tx * tiled_image_object_->tile_size().width,
                                ty * tiled_image_object_->tile_size().height);
                
        std::string tilename = tiled_image_object_->GetTileFilename(view_params_.prev_level, tx, ty);
        if (texture_cache_->Contains(tilename)) {

          // Draw textured quad for this tile at given location (local translation).
          draw_tile_->DrawTileAt(tileTranslation, texture_cache_->GetTexture(tilename));

        } else {

          draw_tile_->DrawTileAt(tileTranslation, texture_placeholder_.get());
        }
    }
  }
}

void TiledImageExplorer::UpdateSingleTileGlobal(int level, int tx, int ty) {
  
  std::string tilename = tiled_image_object_->GetTileFilename(level, tx, ty);
  texture_cache_->GetTexture(tilename);
}

void TiledImageExplorer::ToggleDisplayTileDebugInfo() {
  display_tile_debug_info_ = !display_tile_debug_info_;
  update();
}

void TiledImageExplorer::DrawPatchPointers() {
  if (patches_to_draw_.size() == 0 || tiled_image_object_ == nullptr || draw_on_window_ == nullptr)
    return;

  // This variable indicates how many levels ahead of the current one do we look ahead for pointers
  int lookahead_depth = std::min( 
    tiled_image_object_->num_levels() - 1 - view_params_.cur_level(), lookahead_depth_);

  int first_level_to_draw = (display_patch_pointers_for_fine_levels_) ? 
    view_params_.cur_level() + lookahead_depth : view_params_.cur_level();
  int last_level_to_draw = (display_patch_pointers_for_coarse_levels_) ? 
    0 : view_params_.cur_level();

  // Draw patch pointers (sprites) for each patch on visible window
  for (int level = first_level_to_draw; level >= last_level_to_draw; --level) {
    
    if (level >= tiled_image_object_->num_levels()) {
      continue;
    }

    if (!display_patch_pointers_for_current_level_ && level == view_params_.cur_level()) {
      printf("Skipping patch pointers for current level.\n");
      continue; // Skip current level if specified.
    }

    float level_draw_scale = pow(2.0f, view_params_.cur_level_exact - float(level));
    draw_on_window_->SetGlobalScaleFactor(QPointF(level_draw_scale, level_draw_scale));
    draw_on_window_->SetGlobalTranslation(view_params_.view_offset);

    QColor color_to_use = (level == view_params_.cur_level()) ? current_patch_pointers_color_ :
      (level < view_params_.cur_level()) ? coarse_patch_pointers_color_ : fine_patch_pointers_color_;
    draw_on_window_->DrawPatchPointers(level, color_to_use, patches_to_draw_);
  }
}

void TiledImageExplorer::AssignPatchPointersColors() {
  if (tiled_image_object_ == nullptr)
    return;
  patch_pointers_color_per_level_.clear();
  patch_pointers_color_per_level_.resize(tiled_image_object_->num_levels(),
                                         coarse_patch_pointers_color_);
}

void TiledImageExplorer::PaintSinglePatchPointer(QPainter *painter, PatchCoords patch_coords) {
  if (painter == nullptr)
    return;

  int level = int(patch_coords.level);
  float level_draw_scale = pow(2.0f, view_params_.cur_level_exact - float(level));
  QSize patch_display_size(int(float(focus_patch_params_.patch_size.width) * level_draw_scale),
                           int(float(focus_patch_params_.patch_size.height) * level_draw_scale));

  patch_coords.x = int((float(patch_coords.x) * level_draw_scale) + view_params_.view_offset.x());
  patch_coords.y = int((float(patch_coords.y) * level_draw_scale) + view_params_.view_offset.y());

  QPen pen = painter->pen();
  pen.setBrush(single_patch_color_);
  pen.setWidth(2);
  painter->setPen(pen);
  painter->drawRect(patch_coords.x - (patch_display_size.width() / 2),
                    patch_coords.y - (patch_display_size.height() / 2),
                    patch_display_size.width(), patch_display_size.height());

  // Draw bigger rectangle
  if (patch_display_size.width() < patch_pointer_min_size_.width() ||
      patch_display_size.height() < patch_pointer_min_size_.height()) {

    QPen pen = painter->pen();
    pen.setBrush(single_patch_color_bigger_);
    pen.setWidth(2);
    painter->setPen(pen);
    painter->drawRect(patch_coords.x - (patch_pointer_target_size_.width() / 2),
                      patch_coords.y - (patch_pointer_target_size_.height() / 2),
                      patch_pointer_target_size_.width(), patch_pointer_target_size_.height());
  }
}

void TiledImageExplorer::DrawSinglePatchPointer(PatchCoords patch_coords, 
                                                QSize size, QColor color) {
  if (draw_focus_patch_on_window_ == nullptr)
    return;
  int level = int(patch_coords.level);
  float level_draw_scale = pow(2.0f, view_params_.cur_level_exact - float(level));
  draw_focus_patch_on_window_->SetPatchPointSize(float(size.width()));  // TODO (ronell): make 2D.
  draw_focus_patch_on_window_->SetGlobalScaleFactor(QPointF(level_draw_scale, level_draw_scale));
  draw_focus_patch_on_window_->SetGlobalTranslation(view_params_.view_offset);
  std::vector<PatchCoords> temp_patch_list;
  temp_patch_list.push_back(patch_coords);
  draw_focus_patch_on_window_->DrawPatchPointers(level, color, temp_patch_list);
}

void TiledImageExplorer::DrawFocusPatchPointer() {
  if (!focus_patch_params_.enabled)
    return;
  // TODO (ronell): Figure out visualization parameters to indicate level.
  DrawSinglePatchPointer(focus_patch_params_.coords, QSize(60, 60), single_patch_color_);
}

void TiledImageExplorer::UpdateSelectionRange() {
  if (tiled_image_object_ == nullptr)
    return;
  image_selection_.start_level = view_params_.cur_level();
  image_selection_.end_level = std::max(view_params_.cur_level(), image_selection_.end_level);
  image_selection_.end_level = std::min(tiled_image_object_->num_levels() - 1,
                                        image_selection_.end_level);
}

void TiledImageExplorer::UpdateSelectionROIs() {
  printf("Origin %d, %d.\n", image_selection_.origin.x(), image_selection_.origin.y());
  printf("Size %d, %d.\n", image_selection_.rect->width(), image_selection_.rect->height());

  image_selection_.clear();
  for (int level = image_selection_.start_level; level <= image_selection_.end_level; ++level) {

    QPoint top_left_in_image_coords = ConvertToImagePosInLevel(image_selection_.origin, level);   
    QSize roi_size = ConvertToImageSizeInLevel(image_selection_.rect->size(), level);
    ROI roi_for_level(top_left_in_image_coords.x(), top_left_in_image_coords.y(),
                      roi_size.width(), roi_size.height());
    image_selection_.roi_per_level.push_back(roi_for_level);
  }
}

QPoint TiledImageExplorer::ConvertToImagePosInLevel(QPoint window_pos, int level) {
  float level_draw_scale = pow(2.0f, view_params_.cur_level_exact - float(level));
  QPoint image_pos;
  image_pos.setX(int(((float(window_pos.x()) -
    view_params_.view_offset.x()) / level_draw_scale) + 0.5f));
  image_pos.setY(int(((float(window_pos.y()) -
    view_params_.view_offset.y()) / level_draw_scale) + 0.5f));
  return image_pos;
}

QSize TiledImageExplorer::ConvertToImageSizeInLevel(QSize window_size, int level) {
  float level_draw_scale = pow(2.0f, view_params_.cur_level_exact - float(level));
  QSize image_size;
  image_size.setWidth(int((float(window_size.width() / level_draw_scale) + 0.5f)));
  image_size.setHeight(int((float(window_size.height() / level_draw_scale) + 0.5f)));
  return image_size;
}

std::shared_ptr<QImage> TiledImageExplorer::LoadTileJPG(const std::string& filename) {
  return std::make_shared<QImage>(QString(filename.c_str()));
}

QImage TiledImageExplorer::CreateCheckerboardPattern(QSize size) {
  QImage pattern(size.width(), size.height(), QImage::Format_ARGB32);
  QRgb color1 = qRgb(0, 0, 0);
  QRgb color2 = qRgb(35, 35, 35);
  for (int y = 0; y < size.height(); ++y)
  for (int x = 0; x < size.width(); ++x)
    pattern.setPixel(x, y, (((x >> 4) + (y >> 4)) & 1) ? color1 : color2);
  return pattern;
}