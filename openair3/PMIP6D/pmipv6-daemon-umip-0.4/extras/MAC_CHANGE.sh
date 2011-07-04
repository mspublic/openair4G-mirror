#!/bin/bash
ifconfig ethX down
macchanger -m XX:XX:XX:XX:XX:XX ethX
ifconfig ethX up
