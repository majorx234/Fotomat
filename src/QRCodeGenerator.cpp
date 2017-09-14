#include "QRCodeGenerator.hpp"
#include <QColor>
#include <qrencode.h>

std::shared_ptr<QImage> QRCodeGenerator::encodeString(QString& s) {
    int caseSensitive = 1;
    QRcode* qrcode = QRcode_encodeString(s.toStdString().c_str(), 0, QR_ECLEVEL_H, QR_MODE_8, caseSensitive);

    auto image = std::make_shared<QImage>(QImage(qrcode->width, qrcode->width, QImage::Format_RGB888));
    image->fill(QColor(Qt::white).rgb());
    for (int x = 0; x < qrcode->width; ++x) {
        for (int y = 0; y < qrcode->width; ++y) {
            if (qrcode->data[y * qrcode->width + x] & 0x1)
                image->setPixel(x, y, QColor(Qt::black).rgb());
        }
    }

    QRcode_free(qrcode);

    return image;
}