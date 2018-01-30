#include "drawing/drawonwindow.h"

DrawOnWindow::DrawOnWindow(QOpenGLWidget *parent)
    : parent_(parent),
      tile_size_(QPointF(-1.0f, -1.0f)),
      global_translation_(QPointF(0.0f, 0.0f)),
      global_scale_factor_(QPointF(1.0f, 1.0f)),
      default_shader_program_(nullptr), 
      border_percentage_(0.0f),
      border_color_(Qt::black),
      point_size_(50.0f) {}

DrawOnWindow::~DrawOnWindow() {
  CleanupGL();
}

void DrawOnWindow::Init(QSize _tile_size) {
  if (_tile_size.width() <= 0 || _tile_size.height() <= 0) 
    return;

  parent_->makeCurrent();

  global_translation_ = QPointF(0.0f, 0.0f);
  global_scale_factor_ = QPointF(100.0f, 100.0f);
  UpdateGlobalTransformMatrix();

  tile_size_ = QPointF(float(_tile_size.width()), float(_tile_size.height()));

  // TODO (ronell): Put the shader codes in separate files for easy modification.
#define PROGRAM_VERTEX_ATTRIBUTE_ 0
#define PROGRAM_TEXCOORD_ATTRIBUTE_ 1

  // Pass through vertex shader:
  QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, parent_);
  const char *vsrc =
    "#version 430 core\n"
    "layout (location=0) in vec2 vertexPosition;\n"
    "uniform mat4 matrix;\n"
    "uniform float pointSize;\n"
    "void main(void) {\n"
    "gl_PointSize = pointSize;\n"
    "   gl_Position = matrix * vec4(vertexPosition, 0.0, 1.0);\n"
    "}";
  vshader->compileSourceCode(vsrc);

  // Draw circle fragment shader:
  QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, parent_);
  const char *fsrc =
    "#version 430 core\n"
    "uniform vec4 spriteColor;\n"
    "out vec4 fragColor;\n"
    "void main() {\n"
    
    "    vec2 centerVec = gl_PointCoord-0.5;\n"
    "    float d = 1.0 - 2*length(centerVec);\n"
    "    fragColor = spriteColor*d;\n"
    "}\n";
    fshader->compileSourceCode(fsrc);

  default_shader_program_ = std::make_shared<QOpenGLShaderProgram>();
  default_shader_program_->addShader(vshader);
  default_shader_program_->addShader(fshader);
  default_shader_program_->link();

  default_shader_program_->bind();
  default_shader_program_->setUniformValue("pointSize", point_size_);
  default_shader_program_->setUniformValue("spriteColor", 1.0f, 0.0f, 0.0f, 1.0f);
  default_shader_program_->release();
  vbo_.release();
}

void DrawOnWindow::SetPatchPointSize(float point_size) {
  if (default_shader_program_ == nullptr)
    return;
  default_shader_program_->bind();
  point_size_ = point_size;
  default_shader_program_->setUniformValue("pointSize", point_size_);
  default_shader_program_->release();
}

void DrawOnWindow::SetGlobalTranslation(QPointF translation) {
  global_translation_ = translation;
  UpdateGlobalTransformMatrix();
}

void DrawOnWindow::SetGlobalScaleFactor(QPointF scale_factor) {
  global_scale_factor_ = scale_factor;
  UpdateGlobalTransformMatrix();
}

void DrawOnWindow::UpdateTextureCoordsScale(QPointF texcoords_scale_factor) {
  parent_->makeCurrent();
  default_shader_program_->bind();
  default_shader_program_->setUniformValue("vTexCoordsScale", texcoords_scale_factor.x(), 
                                           texcoords_scale_factor.y());
  default_shader_program_->release();
}

void DrawOnWindow::UpdateTextureCoordsShift(QPointF texcoords_shift) {
  parent_->makeCurrent();
  default_shader_program_->bind();
  default_shader_program_->setUniformValue("vTexCoordsShift", texcoords_shift.x(),
                                           texcoords_shift.y());
  default_shader_program_->release();
}

void DrawOnWindow::UpdateBorderPercentage(float border_percentage) {
  border_percentage_ = border_percentage;
  parent_->makeCurrent();
  default_shader_program_->bind();
  default_shader_program_->setUniformValue("borderPercentage", border_percentage_);
  default_shader_program_->release();
}

void DrawOnWindow::UpdateBorderColor(QColor border_color) {
  border_color_ = border_color;
  parent_->makeCurrent();
  default_shader_program_->bind();
  default_shader_program_->setUniformValue("borderColor", border_color_.redF(),
                                           border_color_.greenF(), border_color_.blueF(),
                                           border_color_.alphaF());
  default_shader_program_->release();
}

void DrawOnWindow::DrawPatchPointers(int level, QColor pointers_color,
                                     std::vector<PatchCoords>& patches_to_draw) {
  parent_->makeCurrent();
  QOpenGLFunctions *QGL = QOpenGLContext::currentContext()->functions();
  QGL->glEnable(GL_POINT_SPRITE);
  QGL->glEnable(GL_PROGRAM_POINT_SIZE);
  QGL->glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);

  default_shader_program_->bind();
  vbo_.bind();

  // Filter the patch coords to get only those from specified level.
  std::vector<GLfloat> vertexData;
  for (int i = 0; i < int(patches_to_draw.size()); ++i) {
    const PatchCoords cur_patch_coords = patches_to_draw[i];
    if (patches_to_draw[i].level == level) {
      // Compute correct patch coordinates in full image space:
      float patch_coord_x = float(cur_patch_coords.x);
      vertexData.push_back(patch_coord_x);
      float patch_coord_y = float(cur_patch_coords.y);
      vertexData.push_back(patch_coord_y);
    }
  }

  if (vertexData.size() > 0) {
    if (vbo_.isCreated()) {
      vbo_.destroy();
    }

    vbo_.create();
    vbo_.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    vbo_.bind();
    vbo_.allocate(&vertexData[0], int(vertexData.size() * sizeof(GLfloat)));

    default_shader_program_->setUniformValue("spriteColor", pointers_color.redF(), 
                                             pointers_color.greenF(), pointers_color.blueF(), 
                                             pointers_color.alphaF());
    default_shader_program_->setUniformValue("matrix", global_transform_matrix_);
    default_shader_program_->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE_);
    default_shader_program_->setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE_, GL_FLOAT, 0, 2);

    QGL->glDrawArrays(GL_POINTS, 0, GLsizei(vertexData.size() / 2)); // Draw points.
  }
    
  QGL->glDisable(GL_POINT_SPRITE);
  QGL->glDisable(GL_PROGRAM_POINT_SIZE);
  default_shader_program_->release();
  vbo_.release();
}

void DrawOnWindow::CleanupGL() {
  parent_->makeCurrent();
  vbo_.destroy();
}

void DrawOnWindow::UpdateGlobalTransformMatrix() {
  QSize windowSize = parent_->size();
  global_transform_matrix_.setToIdentity();
  global_transform_matrix_.ortho(0, float(windowSize.width()), float(windowSize.height()), 
                                 0, -1, 1);
  global_transform_matrix_.translate(global_translation_.x(), global_translation_.y());
  global_transform_matrix_.scale(global_scale_factor_.x(), global_scale_factor_.y());
}