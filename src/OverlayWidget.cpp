#include "OverlayWidget.hpp"
#include "QRCodeGenerator.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <QDebug>
#include <QPainter>

OverlayWidget::OverlayWidget(std::shared_ptr<QJsonObject>& jsonConfig, QWidget* parent)
    : QWidget(parent),
      _timer(new QTimer),

      _active(false),
      _countdownTimer(new QTimer),
      _countdownActive(false),
      _countdownValue(-1),
      _position(0),
      _qrcodeTimer(new QTimer),
      _qrcodeActive(false),
      _displayText("Testtext"),
      _jsonConfig(jsonConfig) {
    connect(_timer.get(), SIGNAL(timeout()), this, SLOT(update()));
    connect(_countdownTimer.get(), SIGNAL(timeout()), this, SLOT(updateCountdown()));
    _countdownTimer->setInterval(1000);

    connect(_qrcodeTimer.get(), SIGNAL(timeout()), this, SLOT(hideQRCode()));
}

void OverlayWidget::startCountdown(int seconds) {
    qDebug() << "<OverlayWidget> Start countdown in widget";
    _countdownValue = seconds;
    _countdownActive = true;
    repaint();
    _countdownTimer->start();
}

void OverlayWidget::showQRCode(QString& s, int seconds) {
    qDebug() << "Activate QR Code";
    _qrcodeActive = true;
    _qrCodeString = s;
    _qrcodeTimer->start(seconds * 1000);
    repaint();
}

void OverlayWidget::showText(QString& s) {
    qDebug() << "<OverlayWidget> Set text: " << s;
    _displayText = s;
}

void OverlayWidget::hideQRCode() {
    qDebug() << "Disable QR Code";
    _qrcodeActive = false;
    _qrcodeTimer->stop();

    if (_disableOverlayAfterQRCodeWasShown) {
        _disableOverlayAfterQRCodeWasShown = false;
        disable();
    } else {
        repaint();
    }
}

void OverlayWidget::updateCountdown() {
    --_countdownValue;
    if (_countdownValue < 0) {
        _countdownActive = false;
        _countdownTimer->stop();
    }
    repaint();
}

void OverlayWidget::update() {
    repaint();
}

void OverlayWidget::enable() {
    if (!_active) {
        _active = true;
        _timer->start(1000.0 / 5.0);
        qDebug() << "<OverlayWidget> OverlayWidget now active";
        repaint();
    }
}

void OverlayWidget::disable() {
    if (_qrcodeActive) {
        _disableOverlayAfterQRCodeWasShown = true;
        return;
    }

    _active = false;
    if (_timer->isActive()) {
        _timer->stop();
    }
    repaint();
}

void OverlayWidget::paintEvent(QPaintEvent*) {
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int width = size().width();
    int height = size().height();

    QPointF center;
    QPointF centerText;

    double imageAspectRatio = 3.0/2.0;
    double imageWidth = height * imageAspectRatio;
    double borderWidth = (width - imageWidth)/2.0;
    double imageHeight = width / imageAspectRatio;
    // double borderHeight = (height - imageHeight);

    QPixmap fbLogo("/home/odroid/fotomat/fotoapp/FB.png");
    int fbLogoWidth = fbLogo.width();
    int fbLogoHeight = fbLogo.height();

double radius = 0.05 * std::min(width, height);
        double circleRadius = 0.0125 * std::min(width, height);
        double marginWidth = circleRadius / 3.0;

        QColor marginColor(221, 60, 60, 204);
        assert(marginColor.isValid());

        QColor fillColor(255, 234, 206, 204);
        assert(fillColor.isValid());
	
QBrush fillColorBrush(fillColor);
        QBrush marginColorBrush(marginColor);

    if ((*_jsonConfig)["monitor_aspect_ratio"].toString() == "16:9") {
        painter.drawPixmap(0, 0, 20, 20, fbLogo);
    } else {
        painter.drawPixmap(width - fbLogoWidth - 20, height - fbLogoHeight - 20, fbLogoWidth, fbLogoHeight, fbLogo);
    }

    if (_active) {
        if ((*_jsonConfig)["monitor_aspect_ratio"].toString() == "16:9") {
            center = QPointF(width / 2.0, 0.8 * height);
            centerText = QPointF (width / 2.0, height * 0.35);
        } else {
            center = QPointF(width / 2.0, 0.8 * imageHeight);
            centerText = QPointF (width / 2.0, imageHeight * 0.35);
        }

        QFont font;
        font.setPixelSize(0.3 * std::min(width, height));
        font.setBold(true);

        QPen pen(marginColor);
        pen.setWidth(marginWidth);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        painter.setPen(pen);

        

        if (_countdownActive) {
            if (_countdownValue > 0) {
                painter.setBrush(marginColorBrush);
                QPainterPath path;
                QString text(QString::number(_countdownValue));
                QFontMetrics fm(font);
                int textWidth = fm.width(text);
                int textHeight = fm.height();
                path.addText(centerText.x() - textWidth / 2.0, centerText.y() + textHeight / 4.0, font, text);
                painter.drawPath(path);
            }
            if (_countdownValue == 0) {
                painter.setBrush(marginColorBrush);
                painter.drawEllipse(centerText, 5.0 * circleRadius, 5.0 * circleRadius);
            }
        }

        if (_qrcodeActive) {
            auto image = QRCodeGenerator::encodeString(_qrCodeString);
            QPointF targetPos(width / 2.0 - width / 8.0, height / 2.0 - width / 8.0);
            painter.drawImage(targetPos, image->scaledToWidth(width / 4.0));
        }

        for (unsigned int j = _position; j <= _position + 2; ++j) {
           const QPointF centerCircle(center.x() + radius * cos((j * 45.0) / 180.0 * M_PI),
                                       center.y() + radius * sin((j * 45.0) / 180.0 * M_PI));
            painter.drawEllipse(centerCircle, circleRadius, circleRadius);
        }

        

        ++_position;
    }

    if (!_displayText.isEmpty()) {
            QRectF targetPos;

            if ((*_jsonConfig)["monitor_aspect_ratio"].toString() == "16:9") {
                targetPos = QRectF(borderWidth + imageWidth, 0, borderWidth, height);
            } else {
		int textHeight = 110;
		int widthOffset = 20;
                targetPos = QRectF(widthOffset, height-textHeight, width - widthOffset, textHeight);
            }
painter.setBrush(marginColorBrush);

	
        QPen pen(QColor("white"));
        pen.setWidth(marginWidth);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        painter.setPen(pen);

  	QFont font;
        font.setPixelSize(0.025 * std::min(width, height));
        font.setBold(true);
	painter.setFont(font);
        painter.drawText(targetPos, (*_jsonConfig)["INFOTEXT"].toString());
        }
    painter.end();
}

void OverlayWidget::resizeEvent(QResizeEvent*) {
}
