#include "VideoWidget.hpp"

#include <QImage>
#include <QKeyEvent>
#include <QRectF>
#include <QVector>

VideoWidget::VideoWidget(QWidget* parent) : QWidget(parent), _imgLabel("", this) {
    _imgLabel.setAlignment(Qt::AlignHCenter);
}

bool VideoWidget::updateImage(std::shared_ptr<QImage> vidImg, VideoWidget::Mirror mirrow) {
    if (static_cast<bool>(vidImg) == false) {
        return false;
    }

    int height = size().height();
    int width = size().width();
    if (width < height) {
        if(mirrow == MIRRORED)
        {
            QPixmap vidPixmap = QPixmap::fromImage(vidImg->mirrored(true, false).scaledToWidth(width));
            _imgLabel.setPixmap(vidPixmap);
        }
        if(mirrow == NOT_MIRRORED)
        {
            QPixmap vidPixmap = QPixmap::fromImage(vidImg->mirrored(false, false).scaledToWidth(width));
            _imgLabel.setPixmap(vidPixmap);
        }    
    } else {
        if(mirrow == MIRRORED)
        {
            QPixmap vidPixmap = QPixmap::fromImage(vidImg->mirrored(true, false).scaledToHeight(height));
            _imgLabel.setPixmap(vidPixmap);
        }
        if(mirrow == NOT_MIRRORED)
        {
            QPixmap vidPixmap = QPixmap::fromImage(vidImg->mirrored(false, false).scaledToHeight(height));
            _imgLabel.setPixmap(vidPixmap);
        }    
    }

    return true;
}

void VideoWidget::resizeEvent(QResizeEvent* event) {
    QSize size = event->size();
    int width = size.width();
    int height = size.height();
    _imgLabel.resize(width, height);
}
