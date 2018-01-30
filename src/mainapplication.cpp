#include <QtWidgets>

#include "mainapplication.h"

MainApplication::MainApplication() {
  // Create the default TiledImageExplorer for the central widget.
  central_tiled_image_explorer_ = std::make_shared<TiledImageExplorer>();
  const bool display_tile_filenames = false;
  texture_cache_ = std::make_shared<QTextureCache>(display_tile_filenames);  // TODO: Change this initial limit.
  central_tiled_image_explorer_->UseTextureCache(texture_cache_.get());
  central_tiled_image_explorer_->setFocusPolicy(Qt::StrongFocus);

  setCentralWidget(central_tiled_image_explorer_.get());
  centralWidget()->setObjectName(tr("GigaPixelExplorer"));

  // Create other interface stuff.
  CreateActions();
  CreateToolBars();
  CreateDockWindows();

  setWindowTitle(tr("GigaPatchExplorer"));
  setObjectName(QString("MainWindow"));
  setUnifiedTitleAndToolBarOnMac(true);
  
  DisplayOpenPrompt();
  
  LoadSettings();
}

MainApplication::~MainApplication() {
  
}

void MainApplication::DisplayOpenPrompt() {
  QMessageBox::about(this, tr("GigapatchExplorer"),
                     tr("Press <b> o </b> to open an image."));
}

void MainApplication::closeEvent(QCloseEvent * event) {
  QSettings settings("KAUST", "GigaPatchExplorer");
  settings.setValue("geometry", saveGeometry());
  settings.setValue("windowState", saveState());
  QMainWindow::closeEvent(event);
}

void MainApplication::LoadSettings() {
  QSettings settings("KAUST", "GigaPatchExplorer");
  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("windowState").toByteArray());
}

void MainApplication::ShowHelp() {
  QMessageBox::about(this, tr("GigapatchExplorer"),
                     tr("The <b>GigapatchExplorer</b> application ... "));
}

void MainApplication::CreateActions() {
  quit_action = new QAction(tr("&Quit"), this);
  quit_action->setShortcuts(QKeySequence::Quit);
  quit_action->setStatusTip(tr("Quit the application"));
  connect(quit_action, SIGNAL(triggered()), this, SLOT(close()));

  show_help_action_ = new QAction(tr("&Help"), this);
  show_help_action_->setShortcut(Qt::Key_0);
  show_help_action_->setStatusTip(tr("Show the application's help box"));
  connect(show_help_action_, SIGNAL(triggered()), this, SLOT(ShowHelp()));
  addAction(show_help_action_);

  open_action_ = new QAction(tr("&Open"), this);
  open_action_->setShortcut(Qt::Key_O);
  connect(open_action_, SIGNAL(triggered()), this, SLOT(OpenTiledImage()));
  addAction(open_action_);
}

void MainApplication::CreateToolBars() {}

void MainApplication::CreateDockWindows() {

}

void MainApplication::OpenTiledImage() {
  QString directory_name = QFileDialog::getExistingDirectory(this,
                                                             tr("Choose an image directory"), ".",
                                                             QFileDialog::DontResolveSymlinks);
  if (directory_name.isEmpty())
    return;

  std::shared_ptr<TiledImageObject> tio = std::make_shared<TiledImageObject>();
  if (!tio->Init(directory_name.toStdString())) {
    return;
  }
  if (!central_tiled_image_explorer_->AttachTiledImageObject(tio))
    return;
}