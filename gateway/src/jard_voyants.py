import OPi.GPIO as GPIO
import time
from jard_wemos_mqtt_mgr import RdApp
import threading

LED_BLUE=7
LED_RED=8


alive=None
on=None
sleep=None
pumping=None
waking=None

class RdVoyants(RdApp):

    def __init__(self,ip,port=6379):
        super(RdVoyants,self).__init__(ip,port)


        # Mode de numérotation physique
        GPIO.setmode(GPIO.BOARD)

        # Broche 7 en sortie

        GPIO.setup(LED_BLUE, GPIO.OUT)
        GPIO.setup(LED_RED, GPIO.OUT)

        GPIO.output(LED_BLUE,GPIO.LOW)
        GPIO.output(LED_RED,GPIO.LOW)

    def run(self):
        global alive
        global on
        global sleep
        global pumping
        global waking

        while True:
            time.sleep(2)
            alive=self.get_app_var_bool('alive')
            on=self.get_app_var_bool('on')
            sleep=self.get_app_var_bool('sleep')
            pumping=self.get_app_var_bool('pumping')
            waking=self.get_app_var_bool('waking')


    def start(self):
        thread=threading.Thread(target=self.run)
        thread.start()


def set_blue(flg):
    if flg==False:
        GPIO.output(LED_BLUE,GPIO.LOW)
    else:
        GPIO.output(LED_BLUE,GPIO.HIGH)


def set_red(flg):
    if flg==False:
        GPIO.output(LED_RED,GPIO.LOW)
    else:
        GPIO.output(LED_RED,GPIO.HIGH)



def main():
    global alive
    global on
    global sleep
    global pumping

    flg_250MS=False
    flg_500MS=False
    flg_1S=False
    flg_2S=False

    pulse_500MS=False
    pulse_1S=False
    pulse_2S=False

    while True:
        time.sleep(0.25)
        flg_250MS=not flg_250MS       
        pulse_500MS=False
        pulse_1S=False
        pulse_2S=False
        if flg_250MS==False:
            flg_500MS=not flg_500MS
            if flg_500MS==False:
                flg_1S=not flg_1S
                #print(alive,'   ',on)
                if flg_1S==False:
                    flg_2S=not flg_2S
                    if flg_2S==True:
                        pulse_2S=True
                else:
                    pulse_1S=True
            else:
                pulse_500MS=True

        if on==None or alive!=True:
            set_red(flg_250MS)
            set_blue(False)
        elif sleep==True:
            set_red(pulse_2S)
            set_blue(False)
        elif on==False:
            set_red(pulse_500MS)
            set_blue(False)
        elif on==True:
            set_red(True)
            if pumping==True:
                set_blue(flg_250MS)
            elif waking==True:
                set_blue(pulse_2S)
            else:
                set_blue(pulse_1S)



if __name__=='__main__':
    srv=RdVoyants('192.168.3.200')
    srv.start()

    main()

