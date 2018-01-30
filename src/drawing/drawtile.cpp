#include "drawing/drawtile.h"

DrawTile::DrawTile(QOpenGLWidget *parent)
    : parent_(parent),
      tile_size_(QPointF(-1.0f, -1.0f)),
      global_translation_(QPointF(0.0f, 0.0f)),
      global_scale_factor_(QPointF(1.0f, 1.0f)),
      default_shader_program_(nullptr), 
      border_percentage_(0.0f),
      border_color_(Qt::black) {}

DrawTile::~DrawTile() {
  CleanupGL();
}

void DrawTile::Init(QSize _tile_size) {
  if (_tile_size.width() <= 0 || _tile_size.height() <= 0) 
    return;

  parent_->makeCurrent();

  global_translation_ = QPointF(0.0f, 0.0f);
  global_scale_factor_ = QPointF(1.0f, 1.0f);
  UpdateGlobalTransformMatrix();

  tile_size_ = QPointF(float(_tile_size.width()), float(_tile_size.height()));

  // we use the same "quad" vertex coords as texture coords
  static const float coords[8] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f
  };

  std::vector<GLfloat> vertexData;
  for (int i = 0; i < 8; ++i) {

    vertexData.push_back(coords[i]);
  }

  if (vbo_.isCreated()) {
    vbo_.destroy();
  }

  vbo_.create();
  // We use StaticDraw because we only write this data once, and use it many times.
  vbo_.setUsagePattern(QOpenGLBuffer::StaticDraw);
  vbo_.bind();
  vbo_.allocate(&vertexData[0], int(vertexData.size() * sizeof(GLfloat)));

  // TODO (ronell): Put the shader codes in separate files for easy modification.
#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_TEXCOORD_ATTRIBUTE 1

  // Pass through vertex shader:
  QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, parent_);
  const char *vsrc =
    "#version 430 core\n"
    "layout (location=0) in vec2 vertexPosition;\n"
    "layout (location=1) in vec2 vertexTexCoords;\n"
    "uniform mat4 matrix;\n"
    "out vec2 vTexCoords;\n"

    "void main(void) {\n"
    "   gl_Position = matrix * vec4(vertexPosition, 0.0, 1.0);\n"
    "   vTexCoords = vertexTexCoords;\n"
    "}";
  vshader->compileSourceCode(vsrc);

  // Texture mapping fragment shader:
  QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, parent_);
  const char *fsrc =
    "#version 430 core\n"
    "in vec2 vTexCoords;\n"
    "uniform vec2 vTexCoordsScale;\n"
    "uniform vec2 vTexCoordsShift;\n"
    "uniform float borderPercentage;\n"
    "uniform vec4 borderColor;\n"
    "out vec4 fragColor;\n"
    "layout(binding = 0) uniform sampler2D tileImage;\n"
    "void main() {\n"
    "    vec2 texCoords = (vTexCoords*vTexCoordsScale) + vTexCoordsShift;\n"
    "    fragColor = texture(tileImage, texCoords);\n"
    "    if(vTexCoords.s < borderPercentage || vTexCoords.s > 1.0f - borderPercentage)\n" 
    "       {fragColor += borderColor;}\n"
    "    if(vTexCoords.t < borderPercentage || vTexCoords.t > 1.0f - borderPercentage )\n" 
    "       {fragColor += borderColor;}\n"
    "}\n";
  fshader->compileSourceCode(fsrc);

  default_shader_program_ = std::make_shared<QOpenGLShaderProgram>();
  default_shader_program_->addShader(vshader);
  default_shader_program_->addShader(fshader);
  default_shader_program_->bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
  default_shader_program_->bindAttributeLocation("texCoord", PROGRAM_TEXCOORD_ATTRIBUTE);
  default_shader_program_->link();

  default_shader_program_->bind();
  default_shader_program_->setUniformValue("texture", 0);
  default_shader_program_->setUniformValue("vTexCoordsScale", 1.0f, 1.0f);
  default_shader_program_->setUniformValue("vTexCoordsShift", 0.0f, 0.0f);
  default_shader_program_->setUniformValue("borderPercentage", border_percentage_);
  default_shader_program_->setUniformValue("borderColor", border_color_.redF(), 
                                           border_color_.greenF(), border_color_.blueF(),
                                           border_color_.alphaF());
  default_shader_program_->release();
  vbo_.release();
}

void DrawTile::SetGlobalTranslation(QPointF translation) {
  global_translation_ = translation;
  UpdateGlobalTransformMatrix();
}

void DrawTile::SetGlobalScaleFactor(QPointF scale_factor) {
  global_scale_factor_ = scale_factor;
  UpdateGlobalTransformMatrix();
}

void DrawTile::UpdateTextureCoordsScale(QPointF texcoords_scale_factor) {
  parent_->makeCurrent();
  default_shader_program_->bind();
  default_shader_program_->setUniformValue("vTexCoordsScale", texcoords_scale_factor.x(), 
                                           texcoords_scale_factor.y());
}

void DrawTile::UpdateTextureCoordsShift(QPointF texcoords_shift) {
  parent_->makeCurrent();
  default_shader_program_->bind();
  default_shader_program_->setUniformValue("vTexCoordsShift", texcoords_shift.x(),
                                           texcoords_shift.y());
}

void DrawTile::UpdateBorderPercentage(float border_percentage) {
  border_percentage_ = border_percentage;
  parent_->makeCurrent();
  default_shader_program_->bind();
  default_shader_program_->setUniformValue("borderPercentage", border_percentage_);
}

void DrawTile::UpdateBorderColor(QColor border_color) {
  border_color_ = border_color;
  parent_->makeCurrent();
  default_shader_program_->bind();
  default_shader_program_->setUniformValue("borderColor", border_color_.redF(),
                                           border_color_.greenF(), border_color_.blueF(),
                                           border_color_.alphaF());
}

void DrawTile::DrawTileAt(QPointF localTranslation, QOpenGLTexture *texture) {
  if (texture == nullptr || !default_shader_program_->isLinked() || !vbo_.isCreated()) 
    return;

  parent_->makeCurrent();
  default_shader_program_->bind();
  texture->bind();
  vbo_.bind();

  QMatrix4x4 localTransformMatrix;
  localTransformMatrix.setToIdentity();
  localTransformMatrix.translate(localTranslation.x(), localTranslation.y());
  localTransformMatrix.scale(tile_size_.x(), tile_size_.y());

  default_shader_program_->setUniformValue("matrix", 
                                           global_transform_matrix_ * localTransformMatrix);
  default_shader_program_->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
  default_shader_program_->enableAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE);
  default_shader_program_->setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 2);
  default_shader_program_->setAttributeBuffer(PROGRAM_TEXCOORD_ATTRIBUTE, GL_FLOAT, 0, 2);

  QOpenGLFunctions *QGL = QOpenGLContext::currentContext()->functions();
  QGL->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  default_shader_program_->release();
  texture->release();
  vbo_.release();
}

void DrawTile::CleanupGL() {
  parent_->makeCurrent();
  vbo_.destroy();
}

void DrawTile::UpdateGlobalTransformMatrix() {
  QSize windowSize = parent_->size();
  global_transform_matrix_.setToIdentity();
  global_transform_matrix_.ortho(0, float(windowSize.width()), float(windowSize.height()), 
                                 0, -1, 1);
  global_transform_matrix_.translate(global_translation_.x(), global_translation_.y());
  global_transform_matrix_.scale(global_scale_factor_.x(), global_scale_factor_.y());
}