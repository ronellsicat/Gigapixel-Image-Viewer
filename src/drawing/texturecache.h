#ifndef GIGAPATCHEXPLORER_EXPLORER_TEXTURECACHE_H_
#define GIGAPATCHEXPLORER_EXPLORER_TEXTURECACHE_H_

#include <memory>
#include <string>

#include <QCache>
#include <QFileInfo>
#include <QOpenGLContext>
#include <QOpenGLTexture>

#include "external/ivda/timer.h"

class QTextureCache {
public:
  QTextureCache(bool display_texture_basefilename)
    : texture_cache_(std::make_shared<QCache<QString, QOpenGLTexture>>(10000)) {
    opengl_widget_ = nullptr;
    display_texture_basefilename_ = display_texture_basefilename;
  };
  ~QTextureCache() {};

  void SetOpenGLWidget(QOpenGLWidget* opengl_widget) {
    opengl_widget_ = opengl_widget;
  }

  bool Contains(std::string image_filename) {
    return texture_cache_->contains(QString(image_filename.c_str()));
  }

  QOpenGLTexture* GetTexture(std::string image_filename, 
                             QOpenGLTexture::WrapMode mode = QOpenGLTexture::ClampToEdge) {

    if (opengl_widget_ != nullptr && opengl_widget_->context()->isValid()) {
      opengl_widget_->makeCurrent();
    }
    
    QString image_filename_qstring = QString(image_filename.c_str());
//    IVDA::Timer find_timer; find_timer.Start();
    QOpenGLTexture* texture = texture_cache_->object(image_filename_qstring);
//    printf("Find took %f seconds.\n", find_timer.Elapsed() / 1000.0f);
    if (texture == 0) {

//      IVDA::Timer load_timer; load_timer.Start();
      std::shared_ptr<QImage> content = std::make_shared<QImage>(image_filename_qstring);
      if (content->isNull()) {
        printf("Warning! Cannot load image %s.\n", image_filename_qstring.toStdString().c_str());
        return nullptr;
      } else if (display_texture_basefilename_) {
        QFileInfo info(image_filename_qstring);
        WriteTextureDebugInfo(content, info.baseName());
      }
//      printf("Load took %f seconds.\n", load_timer.Elapsed() / 1000.0f);

      texture = new QOpenGLTexture(*content);
      texture_cache_->insert(image_filename_qstring, texture);
      texture->setWrapMode(mode);

    }
    return texture;
  }

private:
  QOpenGLWidget* opengl_widget_;
  std::shared_ptr<QCache<QString, QOpenGLTexture>> texture_cache_;
  bool display_texture_basefilename_;


  void WriteTextureDebugInfo(std::shared_ptr<QImage> content, QString display) {
    
      QPainter debugPainter(&*content);
      debugPainter.setRenderHint(QPainter::Antialiasing, true);
      debugPainter.setPen(QColor(0, 255, 0));
      debugPainter.drawRect(3, 3, content->width() - 6, content->height() - 6);
      debugPainter.drawText(4, 16, QString("tile %1").arg(display));
      debugPainter.end();
  }
};

#endif  // GIGAPATCHEXPLORER_EXPLORER_TEXTURECACHE_H_
