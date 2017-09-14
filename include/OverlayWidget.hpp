#ifndef OVERLAY_WIDGET_HPP
#define OVERLAY_WIDGET_HPP

#include <memory>
#include <QTimer>
#include <QWidget>
#include <QJsonObject>
#include <memory>

class OverlayWidget : public QWidget {
    Q_OBJECT
   public:
    explicit OverlayWidget(std::shared_ptr<QJsonObject>& jsonConfig,
                           QWidget* parent = 0);
    void enable();
    void disable();

signals:

   public slots:
    void startCountdown(int seconds);
    void showQRCode(QString& s, int seconds);
    void showText(QString& s);

   protected slots:
    void update();
    void updateCountdown();
    void hideQRCode();

   protected:
    virtual void paintEvent(QPaintEvent* event);
    virtual void resizeEvent(QResizeEvent* event);

   private:
    std::shared_ptr<QTimer> _timer;
    bool _active;

    std::shared_ptr<QTimer> _countdownTimer;
    bool _countdownActive;
    int _countdownValue;

    unsigned int _position;

    std::shared_ptr<QTimer> _qrcodeTimer;
    bool _qrcodeActive;
    bool _disableOverlayAfterQRCodeWasShown;

    QString _qrCodeString;
    QString _displayText;
    std::shared_ptr<QJsonObject> _jsonConfig;
};

#endif  // OVERLAY_WIDGET_HPP
