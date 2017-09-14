#include "WorkerThread.hpp"
#include <cassert>
#include <cups/cups.h>
#include <QDebug>
#include <QPainter>
#include <QPrinter>
#include <QtConcurrent>
#include <QString>
#include <time.h>
#include <iostream>

WorkerThread::WorkerThread(std::shared_ptr<QPrinter> picturePrinter,
			   double duration_1er,
               double duration_4er,
               int numberOfImages,
               int numberOfExtraImages,
               QString pathOfExtraImages,
			   QObject* parent)
    : QThread(parent),
      _workerQueue(),
      _picturePrinter(picturePrinter),
      _jobsActive(0),
      _shouldExit(false),
      _captureInProgress(false),
      _duration_1er(duration_1er),
      _duration_4er(duration_4er),
      _numberOfImages(numberOfImages),
      _numberOfExtraImages(numberOfExtraImages),
      _pathOfExtraImages(pathOfExtraImages)
       {
     

}

void WorkerThread::startCapture() {
    _workerQueueMutex.lock();
    _workerQueue.push_back(CAPTURE_IMAGES);
    _workerQueueEmpty.wakeAll();
    _workerQueueMutex.unlock();
}

void WorkerThread::stopLoop() {
    _workerQueueMutex.lock();
    _workerQueue.push_back(DO_EXIT);
    _workerQueueEmpty.wakeAll();
    _workerQueueMutex.unlock();
}

void WorkerThread::captureLiveImage() {
    _workerQueueMutex.lock();
    _workerQueue.push_back(CAPTURE_LIVE_IMAGE);
    _workerQueueEmpty.wakeAll();
    _workerQueueMutex.unlock();
}

void WorkerThread::printImage(std::shared_ptr<QImage> image) {
    _workerQueueMutex.lock();
    _imageToPrint = image;
    _workerQueue.push_back(PRINT_IMAGE);
    _workerQueueEmpty.wakeAll();
    _workerQueueMutex.unlock();
}

void WorkerThread::run() {
    _camInterface.reset(new CameraInterface);
    if (!_camInterface->ok()) {
        qDebug() << "<WorkerThread> Initialization of CameraInterface failed, exiting!";
        emit workerErrorOccured();
        return;
    }

    while (true) {
        _workerQueueMutex.lock();

        if (_workerQueue.empty()) {
            bool waitOk = _workerQueueEmpty.wait(&_workerQueueMutex);
            if (!waitOk)
                return;
        }
        WorkObjective obj = _workerQueue.front();

        _workerQueue.pop_front();
        _workerQueueMutex.unlock();

        switch (obj) {
            case DO_EXIT:
                _shouldExit = true;
                while (_jobsActive != 0) {
                }
                return;
            case CAPTURE_LIVE_IMAGE:
                QtConcurrent::run([&]() {
                    _jobsActive++;
                    if (_camInterface->ok() && !_shouldExit && !_captureInProgress && _liveImageMutex.tryLock()) {
                        auto liveImage = _camInterface->captureLiveViewImage();
                        emit liveImageCaptured(liveImage);
                        _liveImageMutex.unlock();
                    }
                    _jobsActive--;
                });
                break;
            case CAPTURE_IMAGES:
                assert(_captureInProgress == false);
                QtConcurrent::run([&]() {
                    _jobsActive++;
                    assert(_camInterface->ok());

                    auto testAndExit = [&]() {
                        if (_shouldExit)
                            return true;
                        else
                            return false;
                    };

                    for (int i = 0; i < _numberOfImages; ++i) {
                        emit startCountdown(5);

                        this->msleep(4900);
                        if (testAndExit())
                            break;
                        _captureInProgress = true;
                        emit disableVideoUpdate();

                        this->msleep(100);
                        if (testAndExit())
                            break;
                                             
                        auto img = _camInterface->captureImage();
                        
                        emit singleCaptureDone(img);
                        if (testAndExit())
                            break;

                        this->msleep(_duration_1er);
                        if (testAndExit())
                            break;
                        if (i < _numberOfImages-1 )  // grap check the condition!
                        {
                            _captureInProgress = false;
                            emit enableVideoUpdate();
                        }    
                    }
                    //_captureInProgress = false;
                    //emit enableVideoUpdate();
                    for (int i=0;i< _numberOfExtraImages; i++)
                    {
                       std::shared_ptr<QImage> new_img(new QImage(_pathOfExtraImages));
                       emit extraImageDone(new_img);
                       printf("extra image is emited\n");
                    }
	             //this->msleep(1000);
                    _captureInProgress = false;
                    //emit disableVideoUpdate();
                    emit capturingFinished();
                    this->msleep(_duration_4er);

                    
                    //
                    
                    emit enableVideoUpdate();
                    _jobsActive--;
                });
                break;
            case PRINT_IMAGE:
                QtConcurrent::run([&]() {
                    _jobsActive++;
		    
		    /*cups_dest_t *dests;
		    const char *value;

		    int num_dests = cupsGetDests(&dests);
		    cups_dest_t *dest = cupsGetDest(NULL, NULL, num_dests, dests);
		    value = cupsGetOption( "printer-state-reasons", dest->num_options, dest->options);
		    
		    for(int i = 0; i < dest->num_options; i++){

			qDebug() << "printer options" << dest->options[i].name << dest->options[i].value;
		    }

		    qDebug() << "printer state" << dest->name << dest->num_options << value;*/

                    if (static_cast<bool>(_picturePrinter) == true) {
                        float paperWidth = 71.0f;
                        float paperHeight = 210.0f;
                        float wMargin = 9.0f;
                        QSizeF pageSize(paperWidth + wMargin, paperHeight);
                        _picturePrinter->setPageSizeMM(pageSize);
                        _picturePrinter->setPageMargins(0.0f, 0.0f, 0.0f, 0.0f, QPrinter::Millimeter);
                        _picturePrinter->setResolution(300);

                        QPainter painter;
                        painter.begin(_picturePrinter.get());
                        painter.drawImage(0, 0, _imageToPrint->scaledToWidth(_picturePrinter->width()));
                        painter.end();

                        bool jobsInSystem = true;
                        while (jobsInSystem && !_shouldExit) {
                            cups_job_t* jobs;
                            int myjobs = 1;
                            int num_jobs = cupsGetJobs(&jobs, cupsGetDefault(), myjobs, CUPS_WHICHJOBS_ACTIVE);
                            jobsInSystem = num_jobs > 0;
                            cupsFreeJobs(num_jobs, jobs);

                            qDebug() << "<WorkerThread> Number of print jobs: " << num_jobs;

                            int ms = 250;
                            struct timespec ts = {ms / 1000, (ms % 1000) * 1000 * 1000};
                            nanosleep(&ts, NULL);
                        }

                        emit printingFinished();
                    } else {
                        QSizeF pageSize(71.0f, 210.0f);

                        QPrinter printer(QPrinter::HighResolution);
                        printer.setOutputFileName("output.pdf");
                        printer.setPageSizeMM(pageSize);
                        printer.setPageMargins(0.0f, 0.0f, 0.0f, 0.0f, QPrinter::Millimeter);

                        QPainter painter;
                        painter.begin(&printer);
                        painter.drawImage(0, 0, _imageToPrint->scaledToWidth(printer.width()));
                        painter.end();

                        emit printingFinished();
                    }

                    _jobsActive--;
                });
                break;
        }
    }
}
