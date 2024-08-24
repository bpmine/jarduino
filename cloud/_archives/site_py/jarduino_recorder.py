import pika
import requests
import json
import demjson
from pymongo import MongoClient
from datetime import datetime
import re


DONT_CONSUME=True


class ReadJardin:
    ip=None
    login=None
    passe=None

    conn=None
    ch=None

    def connect(self):
        credentials = pika.PlainCredentials(self.login, self.passe)

        self.conn = pika.BlockingConnection(
            pika.ConnectionParameters(self.ip,
                                      5672,
                                      '/',
                                      credentials)
            )
        self.ch = self.conn.channel()
        
    
    def __init__(self,ip,login,passe):
        self.ip=ip
        self.login=login
        self.passe=passe

        self.connect()

    def send(self,msg,id):
        self.ch.basic_publish(exchange="amq.topic",
                              routing_key=".jarduino.cmd.%s" %id,
                              body=msg)

    def close(self):
        self.ch.close()
        self.conn.close()

    def intern_cb(self,ch, method, properties, body):
        key=method.routing_key
        msg=body.decode()
        ret=False
        if (self.callback!=None):
            ret=self.callback(key,msg);

        if (ret==True):
            self.ch.basic_ack(delivery_tag=method.delivery_tag)
        
    def start(self,callback):
        self.callback=callback;
        self.ch.basic_consume(queue="jarduino", on_message_callback=self.intern_cb)

        self.ch.start_consuming()

p=re.compile("([0-9]{4})-([0-9]{2})-([0-9]{2})T([0-9]{2}):([0-9]{2}):([0-9]{2})Z")
def getDateFromIso(strDt):
    m=p.match(strDt)
    if (m!=None):
        year=int(m.group(1))
        month=int(m.group(2))
        day=int(m.group(3))
        hour=int(m.group(4))
        minute=int(m.group(5))
        second=int(m.group(6))

        dt=datetime(year,month,day,hour,minute,second)

        return dt

    return None


with open('credentials.txt','r') as json_file:
    data = json.load(json_file)

IP=data['ip']
LOGIN=data['login']
PASS=data['pass']

r=ReadJardin(IP,LOGIN,PASS)


mg=MongoClient("mongodb://127.0.0.1:27017");
db=mg.jarduino;
logs = db["logs"]
msgs = db["msgs"]
cmds = db["cmds"]
stats = db["stats"]
errors=db["errors"]

def cb(key,msg):
    p=re.compile("^\\.jarduino\\.([a-z]+)\\.([A-Za-z0-9]+)$");
    m=p.match(key);
    if (m!=None):
        topic=m.group(1)
        mac=m.group(2)

        if (topic=="log"):
            try:
                obj=demjson.decode(msg)
                obj['date']=getDateFromIso(obj['date'])
                obj['serial']=mac
                dteComm=datetime.now()
                obj['dateComm']=dteComm
                print("Log received: %s" % (obj))            
                logs.insert_one(obj)

                if DONT_CONSUME==False:
                    return True;
                
            except Exception as ex:
                print(ex)
                errors.insert_one({'key':key,'msg':msg});
                
        elif (topic=="msg"):
            dteComm=datetime.now()
            obj={'dateComm':dteComm,'msg':msg,'serial':mac}        
            print("Message received: %s" % (obj))
            msgs.insert_one(obj)

            if DONT_CONSUME==False:
                return True;

        elif (topic=="stats"):
            try:
                obj=demjson.decode(msg)
                obj['date']=getDateFromIso(obj['date'])
                obj['serial']=mac
                dteComm=datetime.now()
                obj['dateComm']=dteComm
                print("Statistics received: %s" % (obj))
                stats.insert_one(obj)

                if DONT_CONSUME==False:
                    return True;
                
            except:
                p=re.compile("^{date:(.{20}),(.+)$")
                mm=p.match(msg)
                if (mm!=None):
                    
                    newStr="{date:'%s',%s" % (mm.group(1),mm.group(2))

                    try:
                        obj=demjson.decode(newStr)
                        obj['date']=getDateFromIso(obj['date'])
                        obj['serial']=mac
                        dteComm=datetime.now()
                        obj['dateComm']=dteComm
                        print("Statistics received: %s" % (obj))
                        stats.insert_one(obj)
                        
                        if DONT_CONSUME==False:
                            return True;

                    except Exception as ex:
                        print(ex)
                        errors.insert_one({'key':key,'msg':msg,'new_msg':newStr});
            
        elif (topic=="cmd"):
            dteComm=datetime.now()            
            obj={'dateComm':dteComm,'msg':msg,'serial':mac}
            print("Command sent to device: %s" % (obj))        
            cmds.insert_one(obj)

            if DONT_CONSUME==False:
                return True;
            
        else:
            print("UNKNOWN KEY: %s (%s)" % (key,msg))

    return False

r.start(cb)

        



