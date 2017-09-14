#ifndef IMAGESAVER_HPP
#define IMAGESAVER_HPP

#include <memory>
#include <QDateTime>
#include <QImage>
#include <QString>
#include <QVector>

class ImageSaver {
   public:
    ImageSaver(QString basePath, QString singlePicturePath, QString groupedPicturePath, int numberOfImages = 4, int numberOfExtraImages = 0, QString pathOfExtraImages = "/none.png");

    static std::shared_ptr<QImage> mergeImagesVertically(QVector<std::shared_ptr<QImage>> images, int numberOfImages, int numberOfExtraImages, QString pathOfExtraImages);
    static std::shared_ptr<QImage> mergeImagesHorizontally(QVector<std::shared_ptr<QImage>> images);
    static std::shared_ptr<QImage> mergeImagesToQuad(QVector<std::shared_ptr<QImage>> images, int _numberOfExtraImages = 0);

    void saveSingleImage(std::shared_ptr<QImage> image);
    void saveGroupedImage(std::shared_ptr<QImage> image);

   private:
    QString singlePicturePath();
    QString groupedPicturePath();

    QString _basePath;
    QString _singlePicturePath;
    QString _groupedPicturePath;
    int _numberOfImages;
    int _numberOfExtraImages;
    QString _pathOfExtraImages;
};

#endif  // IMAGESAVER_HPP
