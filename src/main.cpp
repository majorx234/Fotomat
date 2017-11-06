#include "ImageSaver.hpp"
#include "MainWindow.hpp"
#include "VideoWidget.hpp"
#include "WorkerThread.hpp"
#include "QRCodeGenerator.hpp"
#include "GpioThread.hpp"

#include <QCommandLineParser>
#include <QFile>
#include <QJsonDocument>
#include <QPrinter>
#include <QPrinterInfo>
#include <QVBoxLayout>
#include <QtConcurrent>

#include <cassert>
#include <memory>
#include <signal.h>

static const char* APPLICATION_NAME = "fotomat";
static const char* APPLICATION_DISPLAY_NAME = APPLICATION_NAME;
static const char* APPLICATION_VERSION = "v0.1";

static int setupUnixSignalHandlers() {
    struct sigaction hup, term, intA;

    hup.sa_handler = MainWindow::hupSignalHandler;
    sigemptyset(&hup.sa_mask);
    hup.sa_flags = 0;
    hup.sa_flags |= SA_RESTART;

    if (sigaction(SIGHUP, &hup, 0) > 0)
        return 1;

    term.sa_handler = MainWindow::termSignalHandler;
    sigemptyset(&term.sa_mask);
    term.sa_flags |= SA_RESTART;

    if (sigaction(SIGTERM, &term, 0) > 0)
        return 2;

    intA.sa_handler = MainWindow::intSignalHandler;
    sigemptyset(&intA.sa_mask);
    intA.sa_flags = 0;
    intA.sa_flags |= SA_RESTART;

    if (sigaction(SIGINT, &intA, 0) > 0)
        return 3;

    return 0;
}

// returns true if file parsing was ok and file contains minimal set of needed
// keys
bool readConfigFile(QString& configFilePath, QJsonObject& jsonConfig) {
    QFile configFile(configFilePath);
    if (!configFile.open(QIODevice::ReadOnly)) {
        qWarning("<main.cpp> Couldn't open config file.");
        return false;
    }

    QByteArray configData = configFile.readAll();
    QJsonDocument configDoc(QJsonDocument::fromJson(configData));
    if (!configDoc.isObject())
        return false;

    jsonConfig = configDoc.object();

    // some sanity checks
    // //////////////////
    if (!(jsonConfig.contains("paths") && jsonConfig["paths"].isObject()))
        return false;

    if (!jsonConfig["paths"].isObject()) {
        return false;
    }


    auto paths = jsonConfig["paths"].toObject();
    if (!paths.contains("base") || !paths["base"].isString())
        return false;

    if (!paths.contains("single_pictures") || !paths["single_pictures"].isString())
        return false;

    if (!paths.contains("grouped_pictures") || !paths["grouped_pictures"].isString())
        return false;

    return true;
}

std::shared_ptr<ImageSaver> createImageSaver(QJsonObject& jsonConfig) {
    auto basePath = jsonConfig["paths"].toObject()["base"].toString();
    auto singlePicturePath = jsonConfig["paths"].toObject()["single_pictures"].toString();
    auto groupedPicturePath = jsonConfig["paths"].toObject()["grouped_pictures"].toString();
    //int number_of_images        =  jsonConfig["number_of_images"].toInt();
    return std::shared_ptr<ImageSaver>(new ImageSaver(basePath, singlePicturePath, groupedPicturePath,4));
}

int main(int argc, char** argv) {
    std::shared_ptr<QApplication> fotomat{new QApplication(argc, argv)};
    fotomat->setApplicationName(APPLICATION_NAME);
    fotomat->setApplicationDisplayName(APPLICATION_DISPLAY_NAME);
    fotomat->setApplicationVersion(APPLICATION_VERSION);

    setupUnixSignalHandlers();

    std::shared_ptr<QCommandLineParser> commandLineParser{new QCommandLineParser};
    commandLineParser->setApplicationDescription("a photo booth tool");
    commandLineParser->addHelpOption();
    commandLineParser->addVersionOption();

    // own options
    QCommandLineOption configFilePathOption("config",                    // argument name
                                            "path to JSON config file",  // description
                                            "configFilePath",            // name of variable
                                            "config.json");              // default value
    commandLineParser->addOption(configFilePathOption);

    QCommandLineOption printingOption("enable-printing", "option to enable printing subsystem");
    commandLineParser->addOption(printingOption);

    QCommandLineOption fullscreenOption("fullscreen", "option to start in fullscreen");
    commandLineParser->addOption(fullscreenOption);

    commandLineParser->process(*fotomat);

    QString configFilePath = commandLineParser->value(configFilePathOption);
    qDebug() << "<main.cpp> Reading config from " + configFilePath;
    std::shared_ptr<QJsonObject> jsonConfig{new QJsonObject};
    bool configParsable = readConfigFile(configFilePath, *jsonConfig);

    if (!configParsable) {
        return EXIT_FAILURE;
    }

    // Printing
    std::shared_ptr<QPrinter> picturePrinter;
    if (commandLineParser->isSet(printingOption)) {
        QPrinterInfo defaultPrinterInfo = QPrinterInfo::defaultPrinter();
        assert(defaultPrinterInfo.isNull() == false);
        qDebug() << "<main.cpp> Use default printer: " << defaultPrinterInfo.makeAndModel();
        picturePrinter.reset(new QPrinter(defaultPrinterInfo));
    }

    // Image Saving
    std::shared_ptr<ImageSaver> imageSaver = createImageSaver(*jsonConfig);

    int number_of_images        =  (*jsonConfig)["number_of_images"].toInt();
    double duration_1er         =  2000.0; // default value
    duration_1er                =        (*jsonConfig)["duration_1er"].toDouble();
    double duration_4er         =  8000.0; // default value
    duration_4er                =  (*jsonConfig)["duration_4er"].toDouble();
    int number_of_extra_images   =  (*jsonConfig)["number_of_extra_images"].toInt();
    QString path_to_extra_images             =  (*jsonConfig)["extra_images"].toString();
    std::shared_ptr<WorkerThread> workerThread{new WorkerThread(picturePrinter, duration_1er, duration_4er, number_of_images, number_of_extra_images, path_to_extra_images)};
    std::shared_ptr<GpioThread>   gpioThread(new GpioThread(204, 199));

    workerThread->start();
    gpioThread->start();
    std::shared_ptr<MainWindow> mainWindow(new MainWindow(fotomat, jsonConfig, imageSaver, number_of_extra_images));

    qRegisterMetaType<std::shared_ptr<QImage>>("std::shared_ptr<QImage>");

    // live image
    QObject::connect(mainWindow.get(), SIGNAL(captureLiveImage()), workerThread.get(), SLOT(captureLiveImage()),
                     Qt::QueuedConnection);
    QObject::connect(workerThread.get(), SIGNAL(liveImageCaptured(std::shared_ptr<QImage>)), mainWindow.get(),
                     SLOT(liveImageCaptured(std::shared_ptr<QImage>)), Qt::QueuedConnection);

    // image capture
    QObject::connect(mainWindow.get(), SIGNAL(startCapture()), workerThread.get(), SLOT(startCapture()),
                     Qt::QueuedConnection);
    /* button is pressed so startCapture */
    QObject::connect(gpioThread.get(), SIGNAL(buttonPressed()), mainWindow.get(), SLOT(startCaptureProcess()),
                     Qt::QueuedConnection);

    /* capture started so unset LED lock button*/
    QObject::connect(mainWindow.get(), SIGNAL(startCapture()), gpioThread.get(), SLOT(lockButton()),
                     Qt::QueuedConnection);
    QObject::connect(workerThread.get(), SIGNAL(singleCaptureDone(std::shared_ptr<QImage>)), mainWindow.get(),
                     SLOT(singleCaptureDone(std::shared_ptr<QImage>)), Qt::QueuedConnection);
    QObject::connect(workerThread.get(), SIGNAL(extraImageDone(std::shared_ptr<QImage>)), mainWindow.get(),
                     SLOT(extraImageDone(std::shared_ptr<QImage>)), Qt::QueuedConnection);

    QObject::connect(workerThread.get(), SIGNAL(capturingFinished()), mainWindow.get(), SLOT(capturingFinished()),
                     Qt::QueuedConnection);

    // printing
    QObject::connect(mainWindow.get(), SIGNAL(printImage(std::shared_ptr<QImage>)), workerThread.get(),
                     SLOT(printImage(std::shared_ptr<QImage>)), Qt::QueuedConnection);
    QObject::connect(workerThread.get(), SIGNAL(printingFinished()), mainWindow.get(), SLOT(printingFinished()),
                     Qt::QueuedConnection);
    /* pinting finished so set LED gice button free */
    QObject::connect(workerThread.get(), SIGNAL(printingFinished()), gpioThread.get(), SLOT(releaseButton()),
                     Qt::QueuedConnection);    
    // countdown
    QObject::connect(workerThread.get(), SIGNAL(startCountdown(int)), mainWindow.get(), SIGNAL(startCountdown(int)),
                     Qt::QueuedConnection);

    // start/stop live image
    QObject::connect(workerThread.get(), SIGNAL(enableVideoUpdate()), mainWindow.get(), SLOT(enableVideoUpdate()),
                     Qt::QueuedConnection);
    QObject::connect(workerThread.get(), SIGNAL(disableVideoUpdate()), mainWindow.get(), SLOT(disableVideoUpdate()),
                     Qt::QueuedConnection);

    // shutdown on error
    QObject::connect(workerThread.get(), SIGNAL(workerErrorOccured()), mainWindow.get(), SLOT(workerErrorOccured()),
                     Qt::QueuedConnection);

    QObject::connect(workerThread.get(), SIGNAL(startWork()), gpioThread.get(), SLOT(resetButton()),
                     Qt::QueuedConnection);    

    mainWindow->setWindowTitle("Fotomat");
    if (commandLineParser->isSet(fullscreenOption)) {
        mainWindow->showFullScreen();
        //hide cursor
        mainWindow->setCursor(Qt::BlankCursor);

    } else {
        mainWindow->show();
    }

    int fotomatRet = fotomat->exec();

    workerThread->stopLoop();
    gpioThread->stopLoop();
    workerThread->wait(20 * 1000);
    gpioThread->wait(1000 * 1000);

    return fotomatRet;
}
