#ifndef GPIO_THREAD_HPP
#define GPIO_THREAD_HPP

extern "C"
{
#include "gpio.h"
}
#include <atomic>
#include <QThread>


class GpioThread : public QThread {
    Q_OBJECT
 public:
  explicit GpioThread(unsigned int button_gpio, 
                      unsigned int led_gpio,
                      QObject* parent = NULL);
  ~GpioThread();
  void stopLoop();

  static int cb_button_pressed(void* gpioThread)
  {
    /* Called multiple times (100+) */
//    if(!_lockButton.exchange(true)) 
    ((GpioThread*)gpioThread)->emitButtonPressed();
    return 0;
  };

  static int cb_is_ok(void* gpioThread)
  {
    /* Called multiple times (100+) */
    unsigned int exit = ((GpioThread*)gpioThread)->_shouldExit;
    if(((GpioThread*)gpioThread)->_shouldExit)
      return 0;
    return 1;
  };  
signals:
  void buttonPressed();
 public slots:
  void releaseButton();
  void setLed();
  void unsetLed();  
  void resetButton(); 
  void lockButton();
private:
  void emitButtonPressed();
  void run();
  void _setLed(unsigned int);
  std::atomic_bool _shouldExit;
  std::atomic_bool _unlockButton;
  unsigned int _button_gpio;
  unsigned int _led_gpio;
};

#endif // GPIO_THREAD_HPP    
