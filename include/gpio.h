#ifndef GPIO_H
#define GPIO_H

int init_gpio(unsigned int gpio,unsigned int in_out);
int set_gpio(unsigned int gpio,unsigned int on_off);
int get_interrput_on_gpio(unsigned int gpio, 
                          int (*cb_fnct_is_ok)(void*),
                          void* cb_is_ok_obj, 
                          int (*cb_fnct)(void*),
                          void* cb_obj);
int exit_gpio(unsigned int gpio);

#endif
