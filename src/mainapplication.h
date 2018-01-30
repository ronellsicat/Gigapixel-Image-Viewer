#ifndef GIGAPATCHEXPLORER_MAINWINDOW_H_
#define GIGAPATCHEXPLORER_MAINWINDOW_H_

#include <memory>

#include <QMainWindow>

#include "tiledimageexplorer/tiledimageexplorer.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QScrollArea;
class QStackedWidget;
QT_END_NAMESPACE

// Handles user inputs via the menu and manages display windows.
// Menu provides, opening, saving, closing files (images, histograms, etc.).
class MainApplication : public QMainWindow {
  Q_OBJECT

public:
  MainApplication();
  ~MainApplication();
  void closeEvent(QCloseEvent * event) Q_DECL_OVERRIDE;
  TiledImageExplorer* GetCentralTiledImageExplorer() {
    return central_tiled_image_explorer_.get();
  }

private slots:
  void ShowHelp(); 
  void OpenTiledImage();

private:
  void CreateActions();
  void CreateToolBars();
  void CreateDockWindows();
  void LoadSettings();
  void DisplayOpenPrompt();


  QToolBar *main_tool_bar_;
  QAction *open_action_;
  QAction *show_help_action_;
  QAction *quit_action;
  
  std::shared_ptr<TiledImageExplorer> central_tiled_image_explorer_;
  std::shared_ptr<QTextureCache> texture_cache_;
};

#endif  // GIGAPATCHEXPLORER_MAINWINDOW_H_
