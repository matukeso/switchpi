#from cmath import exp
from machine import UART, Pin
from machine import mem32
import time
from machine import idle

from ws2812 import WS2812

uart = UART(0, baudrate=9600, tx=Pin(0), rx=Pin(1), timeout=1)
led = WS2812(12,1)
led.brightness=0.1


pin1 = Pin(26, Pin.OUT)
pin2 = Pin(27, Pin.OUT)
pin3 = Pin(28, Pin.OUT)
pin4 = Pin(29, Pin.OUT)


# Drive Strength : 12mA
PADS_BANK0 = const(0x4001c000)
mem32[PADS_BANK0 + 4 + 4*26] |= 0x30
mem32[PADS_BANK0 + 4 + 4*27] |= 0x30
mem32[PADS_BANK0 + 4 + 4*28] |= 0x30
mem32[PADS_BANK0 + 4 + 4*29] |= 0x30

#bin(mem32[PADS_BANK0 + 0x6c])
#bin(mem32[PADS_BANK0 + 0x70])
#bin(mem32[PADS_BANK0 + 0x74])
#bin(mem32[PADS_BANK0 + 0x78])


UART0_BASEb = const(0x40034000)
QueryFader = b"\2QPL:7;"
QueryAll = b"\2QPL:8;"

expect_ack = 0
timeout_ack = 1*1000000

pgl = 1
pst = 2
fader = 255

def output(pgl, pst, fader):
    global led
    outpins = [0]*4
    if pgl >0: outpins[pgl-1] = 1
    if pst and fader != 255:    outpins[pst-1] = 1
    pin1.value( outpins[0])
    pin2.value( outpins[1])
    pin3.value( outpins[2])
    pin4.value( outpins[3])            
    led.pixels_fill(  (outpins[0]*64 + outpins[3]*128, outpins[1]*64 + outpins[3]*128, outpins[2]*64+ + outpins[3]*128) )
    led.pixels_show()
#    print(outpins)

#'\x02SPL:2:2:0:0:0:0:0:0'
def parsecmd(cmd):
    global pgl, pst, fader
    spls = cmd.split(b':')
    if spls[0] == b'QPL' or spls[0] == b'\2QPL':
        if len(spls) == 2:
            fader = int(spls[1])
        else:
            pgl = int(spls[1])+1
            pst = int(spls[2])+1
            fader = int(spls[8])
        if(fader==0) : fader = 255 
        output(pgl, pst, fader)

def run():
    global expect_ack
    uart.write( QueryAll )
    expect_ack = time.ticks_us()

    while(True):
        idle()
        r = uart.read(1)
        now = time.ticks_us()
        if r == None: 
            if  expect_ack == 0:
                expect_ack = time.ticks_us()
                uart.write(QueryFader)
            elif now - expect_ack > timeout_ack:
                print("TIMEOUT", now - expect_ack)
                led.pixels_fill(  (0,0,0) )
                led.pixels_show()
                expect_ack = 0
                time.sleep_ms(1000)
            continue
        if r[0] == 6: expect_ack = 0
        if r[0] == 2:
            for cmd in uart.read().split( b';' ):
                if len(cmd) == 0 : continue
                if cmd[0] == 6: 
                    cmd = cmd.lstrip(b'\6')
                    expect_ack = 0
                if len(cmd) == 0 : continue
                parsecmd( cmd )


