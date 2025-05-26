import serial.tools.list_ports
import os

COMM_FILE = "comm.txt"

def get_serial_ports():
    return [port.device for port in serial.tools.list_ports.comports()]

def read_comm_file():
    if os.path.exists(COMM_FILE):
        with open(COMM_FILE, "r") as file:
            return file.read().strip()
    return None

def write_comm_file(port):
    with open(COMM_FILE, "w") as file:
        file.write(port)

def select_port(ports):
    if len(ports) == 1:
        return ports[0]
    
    saved_port = read_comm_file()
    if saved_port in ports:
        return saved_port
    
    print("Ports disponibles :")
    for i, port in enumerate(ports, start=1):
        print(f"{i}: {port}")
    
    while True:
        try:
            choice = int(input("Sélectionnez un port (numéro) : "))
            if 1 <= choice <= len(ports):
                selected_port = ports[choice - 1]
                write_comm_file(selected_port)
                return selected_port
        except ValueError:
            pass
        print("Entrée invalide, veuillez réessayer.")

def read_serial_data(port):
    try:
        with serial.Serial(port, baudrate=9600, bytesize=8, parity=serial.PARITY_NONE, stopbits=1, timeout=1) as ser:
            print(f"Lecture du port série {port}...")
            while True:
                data = ser.read_until(b'\x02')
                if data:
                    print(f"Donnée reçue: {data.hex()} ({data})")
                    ser.write(b'B')
    except serial.SerialException as e:
        print(f"Erreur d'accès au port série: {e}")

if __name__ == "__main__":
    ports = get_serial_ports()
    if not ports:
        print("Aucun port série détecté.")
    else:
        selected_port = select_port(ports)
        print(f"Port sélectionné : {selected_port}")
        read_serial_data(selected_port)

