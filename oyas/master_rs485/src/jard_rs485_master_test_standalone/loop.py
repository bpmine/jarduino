import serial
import threading

PORT1 = 'COM5'
PORT2 = 'COM20'
BAUDRATE = 9600

def forward_data(src, dst):
    while True:
        if src.in_waiting:
            data = src.read(src.in_waiting)
            dst.write(data)
            print(data,end='')

def main():
    try:
        ser1 = serial.Serial(PORT1, BAUDRATE, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE)
        ser2 = serial.Serial(PORT2, BAUDRATE, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE)
    except serial.SerialException as e:
        print(f"Erreur d'ouverture des ports : {e}")
        return

    print(f"Ports {PORT1} et {PORT2} ouverts. Démarrage du pont série...")

    t1 = threading.Thread(target=forward_data, args=(ser1, ser2), daemon=True)
    t2 = threading.Thread(target=forward_data, args=(ser2, ser1), daemon=True)

    t1.start()
    t2.start()

    try:
        while True:
            pass  # Boucle principale
    except KeyboardInterrupt:
        print("\nArrêt du programme.")
    finally:
        ser1.close()
        ser2.close()
        print("Ports fermés.")

if __name__ == '__main__':
    main()
