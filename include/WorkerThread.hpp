#ifndef WORKER_THREAD_HPP
#define WORKER_THREAD_HPP

#include "CameraInterface.hpp"
#include "ImageSaver.hpp"
#include <atomic>
#include <QMutex>
#include <QPrinter>
#include <QQueue>
#include <QThread>
#include <QWaitCondition>

class WorkerThread : public QThread {
    Q_OBJECT
   public:
    explicit WorkerThread(std::shared_ptr<QPrinter> picturePrinter, 
			  double duration_1er,
              double duration_4er,
              int numberOfImages,
              int numberOfExtraImages,
              QString pathOfExtraImages,
			  QObject* parent = 0);
    void stopLoop();

signals:
    void startWork();
    void liveImageCaptured(std::shared_ptr<QImage> image);
    void singleCaptureDone(std::shared_ptr<QImage> image);
    void extraImageDone(std::shared_ptr<QImage> image);
    void capturingFinished();
    void printingFinished();
    void startCountdown(int seconds);
    void enableVideoUpdate();
    void disableVideoUpdate();

    void workerErrorOccured();

   public slots:
    void startCapture();
    void captureLiveImage();
    void printImage(std::shared_ptr<QImage> image);

   private:
    void run();

    std::shared_ptr<CameraInterface> _camInterface;
    bool _finished;

    enum WorkObjective { CAPTURE_LIVE_IMAGE, CAPTURE_IMAGES, PRINT_IMAGE, DO_EXIT };
    QMutex _workerQueueMutex;
    QWaitCondition _workerQueueEmpty;
    QQueue<WorkObjective> _workerQueue;
    std::shared_ptr<QImage> _imageToPrint;
    std::shared_ptr<QPrinter> _picturePrinter;
    std::atomic_int _jobsActive;
    std::atomic_bool _shouldExit;
    std::atomic_bool _captureInProgress;
    QMutex _liveImageMutex;
    double _duration_1er;	
    double _duration_4er;
    int _numberOfImages;
    int _numberOfExtraImages;
    QString  _pathOfExtraImages;
};

#endif  // WORKER_THREAD_HPP
