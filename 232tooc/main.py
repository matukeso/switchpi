import t
from machine import Pin

led = Pin(25, Pin.OUT)
power = Pin(11, Pin.OUT)
power.value(1)

try:
    led.value(1)
    t.run()
except:
    pass
led.value(0)
power.value(0)    
