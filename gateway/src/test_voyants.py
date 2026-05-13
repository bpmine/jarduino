import OPi.GPIO as GPIO
import time

LED_BLUE=7
LED_RED=8

# Mode de numérotation physique
GPIO.setmode(GPIO.BOARD)

# Broche 7 en sortie
GPIO.setup(LED_BLUE, GPIO.OUT)
GPIO.setup(LED_RED, GPIO.OUT)
GPIO.output(LED_BLUE,GPIO.LOW)
GPIO.output(LED_RED,GPIO.LOW)
  
try:
    while True:
        GPIO.output(LED_BLUE,GPIO.HIGH)
        GPIO.output(LED_RED,GPIO.LOW)
        print("LED ON")
        time.sleep(1)

        GPIO.output(LED_BLUE,GPIO.LOW)
        GPIO.output(LED_RED,GPIO.HIGH)
        print("LED OFF")
        time.sleep(1)

except KeyboardInterrupt:
    print('Fin du programme...')
    GPIO.output(LED_RED,GPIO.LOW)
    GPIO.output(LED_BLUE,GPIO.LOW)
    GPIO.cleanup()


