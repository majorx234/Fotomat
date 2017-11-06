#include "GpioThread.hpp"

GpioThread::GpioThread(unsigned int button_gpio, 
                       unsigned int led_gpio,
                      QObject* parent)
    : QThread(parent),
      _shouldExit(false),
      _unlockButton(true),
      _button_gpio(button_gpio),
      _led_gpio(led_gpio)
{
}

GpioThread::~GpioThread()
{

}

void GpioThread::stopLoop()
{
  _shouldExit = true;
}

void GpioThread::run()
{
  printf("GpioThread::run start gpio thread \n");
  get_interrput_on_gpio(_button_gpio, cb_is_ok, (void*)this, cb_button_pressed, (void*)this);
  printf("GpioThread::end gpio thread \n");
}

void GpioThread::emitButtonPressed()
{
   if(_unlockButton.exchange(false)) {
     printf("GpioThread::emitButtonPressed: button pressed \n");
     emit buttonPressed();
   }
}

void GpioThread::_setLed(unsigned int led_state)
{
  set_gpio(_led_gpio, led_state);
}

void GpioThread::setLed()
{
  _setLed(1);
}

void GpioThread::unsetLed()
{
  _setLed(0);
}

void GpioThread::lockButton()
{
  unsetLed();
  _unlockButton = false;
}

void GpioThread::releaseButton()
{
  setLed();
  _unlockButton = true;
}

void GpioThread::resetButton()
{
//  printf("GpioThread::resetButton\n");
  setLed();
  _unlockButton = true;
}
