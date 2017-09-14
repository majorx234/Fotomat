#ifndef VIDEOWIDGET
#define VIDEOWIDGET

#include "CameraInterface.hpp"
#include "ImageSaver.hpp"
#include <memory>
#include <QApplication>
#include <QLabel>
#include <QRect>
#include <QtGui>



class VideoWidget : public QWidget {
    Q_OBJECT
   public:
    enum Mirror { MIRRORED, NOT_MIRRORED };
    VideoWidget(QWidget* parent = 0);

    bool updateImage(std::shared_ptr<QImage> vidImg, Mirror mirrow = MIRRORED );
   
   protected:
    virtual void resizeEvent(QResizeEvent* event);

    QLabel _imgLabel;

   private:
    VideoWidget(const VideoWidget&);
    VideoWidget& operator=(const VideoWidget&);
};

#endif
