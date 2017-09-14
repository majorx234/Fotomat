#ifndef QRCODEGENERATOR_HPP
#define QRCODEGENERATOR_HPP

#include <QImage>
#include <memory>

class QRCodeGenerator {
   public:
    static std::shared_ptr<QImage> encodeString(QString& s);

   protected:
};

#endif  // QRCODEGENERATOR_HPP
