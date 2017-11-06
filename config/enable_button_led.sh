#!/bin/sh
echo "204" > /sys/class/gpio/export
echo "in" > /sys/class/gpio/gpio204/direction
echo "rising" > /sys/class/gpio/gpio204/edge
chmod 666 /sys/class/gpio/gpio204/direction
chmod 666 /sys/class/gpio/gpio204/edge
echo "199" > /sys/class/gpio/export
echo "out" > /sys/class/gpio/gpio199/direction
chmod 666 /sys/class/gpio/gpio199/direction
chmod 666 /sys/class/gpio/gpio199/value