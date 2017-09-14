#include "MainWindow.hpp"
#include <QDebug>
#include <QDesktopWidget>
#include <QtConcurrentRun>

#include <cassert>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int MainWindow::sighupFd[2];
int MainWindow::sigtermFd[2];
int MainWindow::sigintFd[2];

void MainWindow::hupSignalHandler(int) {
    char a = 1;
    ssize_t n = write(sighupFd[0], &a, sizeof(a));
    assert(n == sizeof(a));
}

void MainWindow::termSignalHandler(int) {
    char a = 1;
    ssize_t n = write(sigtermFd[0], &a, sizeof(a));
    assert(n == sizeof(a));
}

void MainWindow::intSignalHandler(int) {
    char a = 1;
    ssize_t n = write(sigintFd[0], &a, sizeof(a));
    assert(n == sizeof(a));
}

void MainWindow::handleSigHup() {
    _snHup->setEnabled(false);
    char tmp;
    ssize_t n = read(sighupFd[1], &tmp, sizeof(tmp));
    assert(n == sizeof(tmp));

    qDebug() << "<MainWindow> SIGHUP catched";

    _snHup->setEnabled(true);
}

void MainWindow::handleSigTerm() {
    _snTerm->setEnabled(false);
    char tmp;
    ssize_t n = read(sigtermFd[1], &tmp, sizeof(tmp));
    assert(n == sizeof(tmp));

    qDebug() << "<MainWindow> SIGTERM catched";

    _snTerm->setEnabled(true);
}

void MainWindow::handleSigInt() {
    _snInt->setEnabled(false);
    char tmp;
    ssize_t n = read(sigintFd[1], &tmp, sizeof(tmp));
    assert(n == sizeof(tmp));

    qDebug() << "<MainWindow> SIGINT catched";
    _fotomatApplication->quit();

    _snInt->setEnabled(true);
}

void MainWindow::workerErrorOccured() {
    _fotomatApplication->quit();
}

MainWindow::MainWindow(std::shared_ptr<QApplication>& fotomat,
                       std::shared_ptr<QJsonObject>& jsonConfig,
                       std::shared_ptr<ImageSaver>& imageSaver,
                       int number_of_extra_images,
                       QWidget* parent)
    : QMainWindow(parent),
      _fotomatApplication(fotomat),
      _jsonConfig(jsonConfig),
      _imageSaver(imageSaver),
      _videoUpdateTimer(),
      _videoWidget(this),
      _overlayWidget(jsonConfig, &_videoWidget),
      _captureInProgress(false),
      _number_of_extra_images(number_of_extra_images) {
    setStyleSheet("background-color:black;");
    _overlayWidget.setPalette(Qt::transparent);
    _overlayWidget.setAttribute(Qt::WA_TransparentForMouseEvents);
    setCentralWidget(&_videoWidget);

    int initialWidth = 400;
    int initialHeight = 400;

    _videoWidget.resize(initialWidth, initialHeight);
    _overlayWidget.resize(initialWidth, initialHeight);
    _videoUpdateTimer.setInterval(static_cast<int>(_TIME_PER_FRAME_MS));
    connect(&_videoUpdateTimer, SIGNAL(timeout()), this, SLOT(updateVideoFrame()));
    connect(this, SIGNAL(startCountdown(int)), &_overlayWidget, SLOT(startCountdown(int)));
    connect(this, SIGNAL(showQRCode(QString&, int)), &_overlayWidget, SLOT(showQRCode(QString&, int)));
    _videoUpdateTimer.start();

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sighupFd))
        qFatal("Couldn't create HUP socketpair");
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sigtermFd))
        qFatal("Couldn't create TERM socketpair");
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sigintFd))
        qFatal("Couldn't create INT socketpair");
    _snHup = new QSocketNotifier(sighupFd[1], QSocketNotifier::Read, this);
    connect(_snHup, SIGNAL(activated(int)), this, SLOT(handleSigHup()));
    _snTerm = new QSocketNotifier(sigtermFd[1], QSocketNotifier::Read, this);
    connect(_snTerm, SIGNAL(activated(int)), this, SLOT(handleSigTerm()));
    _snInt = new QSocketNotifier(sigintFd[1], QSocketNotifier::Read, this);
    connect(_snInt, SIGNAL(activated(int)), this, SLOT(handleSigInt()));
}

void MainWindow::singleCaptureDone(std::shared_ptr<QImage> image) {
    if (!_videoWidget.updateImage(image)) {
        _fotomatApplication->quit();
    }
    QtConcurrent::run([&](std::shared_ptr<QImage> imageToSave) { _imageSaver->saveSingleImage(imageToSave); }, image);
    _lastImages.push_back(image);
}

void MainWindow::extraImageDone(std::shared_ptr<QImage> image){
    //if (!_videoWidget.updateImage(image, VideoWidget::NOT_MIRRORED)) {
    //    _fotomatApplication->quit();
    //}
    QtConcurrent::run([&](std::shared_ptr<QImage> imageToSave) { _imageSaver->saveSingleImage(imageToSave); }, image);
    _lastImages.push_back(image);
}

void MainWindow::liveImageCaptured(std::shared_ptr<QImage> image) {
    _videoWidget.updateImage(image, VideoWidget::MIRRORED);
}

void MainWindow::printingFinished() {
    _captureInProgress = false;
    _overlayWidget.disable();
}

void MainWindow::capturingFinished() {
    auto merged = ImageSaver::mergeImagesVertically(_lastImages,4,0,QString("/none.png"));
    auto quadMerged = ImageSaver::mergeImagesToQuad(_lastImages, _number_of_extra_images);
    _videoWidget.updateImage(quadMerged, VideoWidget::NOT_MIRRORED);
    QtConcurrent::run([&](std::shared_ptr<QImage> imageToSave) { _imageSaver->saveGroupedImage(imageToSave); }, merged);
    _lastImages.clear();

    // QString qrString("Test");
    // emit showQRCode(qrString, 3);

    emit printImage(merged);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {

    // check the papertray of the printer
/*    if ( cups_sc_state_e == 32){

        _paperTrayEmpty = true;
        emit paperEmpty();
    }*/

//    else{
        auto startPrinting = [&]() {
            _captureInProgress = true;
            _paperTrayEmpty = false;
            _overlayWidget.enable();
            emit startCapture();
        };
//    }

    if (!_captureInProgress) {
        switch (event->key()) {
            case Qt::Key_C:
            case Qt::Key_1:
                startPrinting();
                break;

            case Qt::Key_Escape:
                _fotomatApplication->quit();
                break;
            default:
                qDebug() << "Key has no effect";
        }
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent*) {
}

MainWindow::~MainWindow() {
    delete _snHup;
    _snHup = nullptr;

    delete _snTerm;
    _snTerm = nullptr;

    delete _snInt;
    _snInt = nullptr;
}

void MainWindow::updateVideoFrame() {
    emit captureLiveImage();
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QSize size = event->size();
    int width = size.width();
    int height = size.height();
    _videoWidget.resize(width, height);
    _overlayWidget.resize(width, height);
}

void MainWindow::enableVideoUpdate() {
    setStyleSheet("background-color:black;");
    _videoUpdateTimer.start();
}

void MainWindow::disableVideoUpdate() {
    setStyleSheet("background-color:white;");
    _videoUpdateTimer.stop();
}

void MainWindow::closeEvent(QCloseEvent*) {
    qDebug() << "<MainWindow> Exit clicked";
    _fotomatApplication->quit();
}
