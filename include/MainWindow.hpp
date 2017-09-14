#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "CameraInterface.hpp"
#include "ImageSaver.hpp"
#include "OverlayWidget.hpp"
#include "VideoWidget.hpp"
#include <QApplication>
#include <QJsonObject>
#include <QMainWindow>
#include <QPrinter>
#include <QTimer>
#include <QVBoxLayout>

#include <cups/sidechannel.h>

class MainWindow : public QMainWindow {
    Q_OBJECT
   public:
    MainWindow(std::shared_ptr<QApplication>& fotomat,
               std::shared_ptr<QJsonObject>& jsonConfig,
               std::shared_ptr<ImageSaver>& imageSaver,
               int number_of_extra_images = 0,
               QWidget* parent = 0);

    virtual ~MainWindow();

    // Unix signal handlers.
    static void hupSignalHandler(int unused);
    static void termSignalHandler(int unused);
    static void intSignalHandler(int unused);

signals:
    void startCapture();
    void captureLiveImage();
    void printImage(std::shared_ptr<QImage> image);
    void startCountdown(int seconds);
    void showQRCode(QString& s, int seconds);
    // if papertray is empty
    void paperEmpty();

   public slots:
    void singleCaptureDone(std::shared_ptr<QImage> image);
    void extraImageDone(std::shared_ptr<QImage> image);
    void capturingFinished();
    void liveImageCaptured(std::shared_ptr<QImage> image);
    void printingFinished();
    void enableVideoUpdate();
    void disableVideoUpdate();
//void setOverlayText(QString s);


    void workerErrorOccured();

    // Qt signal handlers.
    void handleSigHup();
    void handleSigTerm();
    void handleSigInt();

   protected:
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void keyReleaseEvent(QKeyEvent* event);
    virtual void resizeEvent(QResizeEvent* event);
    virtual void closeEvent(QCloseEvent* event);

   protected slots:
    void updateVideoFrame();

   private:
    std::shared_ptr<QApplication> _fotomatApplication;
    std::shared_ptr<QJsonObject> _jsonConfig;
    std::shared_ptr<ImageSaver> _imageSaver;

    QTimer _videoUpdateTimer;

    VideoWidget _videoWidget;
    OverlayWidget _overlayWidget;

    QVector<std::shared_ptr<QImage>> _lastImages;

    bool _captureInProgress;

    static constexpr double _FPS = 10.0;
    static constexpr double _TIME_PER_FRAME_MS = 1000.0 / _FPS;

    // signal handling
    static int sighupFd[2];
    static int sigtermFd[2];
    static int sigintFd[2];

    // printer empty
    bool _paperTrayEmpty = false;

    QSocketNotifier* _snHup;
    QSocketNotifier* _snTerm;
    QSocketNotifier* _snInt;

    int _number_of_extra_images;
};

#endif  // MAINWINDOW_HPP
