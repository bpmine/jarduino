import pymodbus
import serial
from pymodbus.pdu import ModbusRequest
from pymodbus.client.sync import ModbusSerialClient as ModbusClient #initialize a serial RTU client instance
from pymodbus.transaction import ModbusRtuFramer
import time

import logging
logging.basicConfig()
log = logging.getLogger()
log.setLevel(logging.DEBUG)

def test_slave_rs485_set_address(cln,addr,newAddr):
    print('_'*40)
    print('MODIFICATION ADRESSE MODBUS')
    print('')    
    print('Lecture etat electrovanne: ', end='')
    try:
        result= client.read_coils(0,1,unit= addr)
        print(result.bits[0])
    except:
        print("[NOK] - Echec du changement d'adresse")
        return False

    print('Programmation adresse %s -> %s' % (addr,newAddr))
    try:
        client.write_registers(0,[newAddr],unit= addr)
        print('programmation réussie')
        print('Redemarrer pour prise en compte de la nouvelle adresse..')
        return True
    except:
        print("[NOK] - Echec du changement d'adresse")
        return False



def test_slave_rs485(cln,addr=0):
    print('Lecture etat electrovanne. Doit etre OFF.')
    result= client.read_coils(0,1,unit= addr)
    assert(result.bits[0]==False)

    print('OUVERTURE ELECTROVANNE')
    result= client.write_coils(0,[True],unit= addr)
    
    print('Lecture etat electrovanne. Doit etre ON.')
    result= client.read_coils(0,1,unit= addr)
    assert(result.bits[0]==True)

    print('Lecture etat des niveaux. Doivent etre 0.')
    result= client.read_discrete_inputs(0,2,unit= addr)
    assert(result.bits[0]==False)
    assert(result.bits[1]==False)

    print('Attente...')
    time.sleep(5)
    
    print('FERMETURE ELECTROVANNE')
    result= client.write_coils(0,[False],unit= addr)
    
    print('Lecture etat electrovanne. Doit etre OFF.')
    result= client.read_coils(0,1,unit= addr)
    assert(result.bits[0]==False)   
    
def test_slave_rs485_wdg(cln,addr=0):
    print('Lecture etat electrovanne. Doit etre OFF.')
    result= client.read_coils(0,1,unit= addr)
    assert(result.bits[0]==False)

    print('Entretien de la comm avec vanne ON...')
    for i in range(0,10):
        result= client.write_coils(0,[True],unit= addr)
        print('.',end='')
        time.sleep(0.8)
        result= client.read_coils(0,1,unit= addr)
        assert(result.bits[0]==True)
    print()

    print('Quelques lectures toujours on mais plus de write coil !')
    result= client.read_coils(0,1,unit= addr)
    assert(result.bits[0]==True)
    result= client.read_coils(0,1,unit= addr)
    assert(result.bits[0]==True)

    print('Attente...')
    time.sleep(3)
    print('La vanne doit maintenant être fermée')
    result= client.read_coils(0,1,unit= addr)
    assert(result.bits[0]==False)

    
def test_slave_rs485_asserv(cln,addr=0):
    print('Lecture etat electrovanne. Doit etre OFF.')
    result= client.read_coils(0,1,unit= addr)
    assert(result.bits[0]==False)

    while (True):
        result= client.read_discrete_inputs(0,2,unit= addr)
        level_low=result.bits[0]
        level_high=result.bits[1]
        print('Status %d %d' % (level_low,level_high) )

        if (level_low==False and level_high==False):
            client.write_coils(0,[True],unit= addr)
        elif (level_low==True and level_high==False):
            client.write_coils(0,[True],unit= addr)
        elif level_high==True:
            client.write_coils(0,[False],unit= addr)
            #break
        

def test_find_slaves(client,rng=None):    
    print('_'*40)
    print('RECHERCHE DES MODULES (env. 12s)...')
    print('')
    if rng==None:
        (a,b)=(0,127)
    else:
        (a,b)=rng

    if b<a:
        return []

    before_tmt=client.timeout
    client.timeout=0.05
    lst=[]
    for i in range(a,b+1):
        try:
            result= client.read_coils(0,1,unit= i)
            res=result.bits[0]
            print('  - Module @%d trouvé' % (i) )
            lst.append(i)
        except Exception as e:
            pass

    client.timeout=before_tmt

    s='' if len(lst)==1 else 's'
    
    print('_'*40)
    print('%d module%s trouvé%s. ' % (len(lst),s,s),end='')
    print('Liste des adresses: %s' % (lst))

    return lst
        
class ObjCmd:
    def __init__(self):
        self.value=False



client= ModbusClient(method = "rtu", port=r"\\.\COM8",stopbits = 1, bytesize = 8, parity='N',baudrate= 9600,timeout=2)

print('Connection:',end='')
res = client.connect()
if res==True:
    print('[OK]')
else:
    print('[NOK]')


time.sleep(2);

test_slave_rs485_set_address(client,0,1)

#test_slave_rs485_wdg(client,5)
#test_slave_rs485_asserv(client,10)
#lst=test_find_slaves(client)

client.close()
