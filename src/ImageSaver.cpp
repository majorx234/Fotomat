#include "ImageSaver.hpp"
#include <cassert>
#include <QDebug>
#include <QDir>
#include <QPainter>

ImageSaver::ImageSaver( QString basePath, 
                        QString singlePicturePath, 
                        QString groupedPicturePath, 
                        int numberOfImages, 
                        int numberOfExtraImages, 
                        QString pathOfExtraImages)
    : _basePath(basePath), 
      _singlePicturePath(singlePicturePath), 
      _groupedPicturePath(groupedPicturePath),       
      _numberOfImages(numberOfImages),
      _numberOfExtraImages(numberOfExtraImages),
      _pathOfExtraImages(pathOfExtraImages) {
    // create paths, if they don't exist
    printf("ImageSaver:number of images%d\n", _numberOfImages);    
    if (!QDir(_basePath).exists()) {
        bool creationBaseDirSuccess = QDir().mkdir(basePath);
        assert(creationBaseDirSuccess == true);
    }

    if (!QDir(_basePath).exists()) {
        bool creationBaseDirSuccess = QDir().mkdir(basePath);
        assert(creationBaseDirSuccess == true);
    }

    QString fullSinglePicturePath = this->singlePicturePath();
    if (!QDir(fullSinglePicturePath).exists()) {
        bool creationSingleDirSuccess = QDir().mkdir(fullSinglePicturePath);
        assert(creationSingleDirSuccess == true);
    }

    QString fullGroupedPicturePath = this->groupedPicturePath();
    if (!QDir(fullGroupedPicturePath).exists()) {
        bool creationGroupedDirSuccess = QDir().mkdir(fullGroupedPicturePath);
        assert(creationGroupedDirSuccess == true);
    }
}

QString ImageSaver::singlePicturePath() {
    return _basePath + "/" + _singlePicturePath;
}

QString ImageSaver::groupedPicturePath() {
    return _basePath + "/" + _groupedPicturePath;
}

void ImageSaver::saveSingleImage(std::shared_ptr<QImage> image) {
    QDateTime currentTime = QDateTime::currentDateTime();
    QString filename = singlePicturePath() + "/" + currentTime.toString("yyyy-MM-dd hh-mm-ss.zzz") + ".jpeg";
    image->save(filename);
    qDebug() << "Saved " << filename;
}

void ImageSaver::saveGroupedImage(std::shared_ptr<QImage> image) {
    QDateTime currentTime = QDateTime::currentDateTime();
    QString filename = groupedPicturePath() + "/" + currentTime.toString("yyyy-MM-dd hh-mm-ss.zzz") + ".jpeg";
    image->save(filename);
    qDebug() << "Saved " << filename;
}

std::shared_ptr<QImage> ImageSaver::mergeImagesVertically(QVector<std::shared_ptr<QImage>> images, int numberOfImages, int numberOfExtraImages, QString pathOfExtraImages) {
    int height = images.at(0)->height();
    int width = images.at(0)->width();
    size_t numImages = images.size();
    assert(numImages == (unsigned int)numberOfImages);
    printf("mergeImagesVertically: %d extraImages \n", numberOfExtraImages);

    QString noneImage = QString("/none.png");
    int x = QString::compare(pathOfExtraImages, noneImage, Qt::CaseInsensitive);
    printf("mergeImagesVertically: compare %d\n", x);
    //assert(x != 0); //


    std::shared_ptr<QImage> merged{new QImage(width, height * numImages, QImage::Format_RGB32)};

    QPainter imagePainter(merged.get());
    for (size_t i = 0; i < numImages; ++i) {
        imagePainter.drawImage(QRectF(0.0, height * i, width, height), *images[i]);
    }

    return merged;
}

std::shared_ptr<QImage> ImageSaver::mergeImagesHorizontally(QVector<std::shared_ptr<QImage>> images) {
    int height = images.at(0)->height();
    int width = images.at(0)->width();
    size_t numImages = images.size();
    assert(numImages == 4);

    std::shared_ptr<QImage> merged{new QImage(width * numImages, height, QImage::Format_RGB32)};

    QPainter imagePainter(merged.get());
    for (size_t i = 0; i < numImages; ++i) {
        imagePainter.drawImage(QRectF(width * i, height, width, height), *images[i]);
    }

    return merged;
}

std::shared_ptr<QImage> ImageSaver::mergeImagesToQuad(QVector<std::shared_ptr<QImage>> images, int _numberOfExtraImages) {
    int height = images.at(0)->height();
    int width = images.at(0)->width();
    size_t numImages = images.size();
    assert(numImages == 4);

    std::shared_ptr<QImage> merged{new QImage(width * 2, height * 2, QImage::Format_RGB32)};

    QPainter imagePainter(merged.get());
    int idx = 0;
    for (size_t i = 0; i < 2; ++i) {
        for (size_t j = 0; j < 2; ++j) {
            if(idx<4-_numberOfExtraImages)
                imagePainter.drawImage(QRectF(width * i, height * j, width, height), *images[idx]);
            else
                imagePainter.drawImage(QRectF(width * i, height * j, width, height), images[idx]->mirrored(true, false));
            ++idx;
        }
    }

    return merged;
}
