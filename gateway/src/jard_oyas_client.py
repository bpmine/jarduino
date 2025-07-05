#!/usr/bin/python3
from jard_oyas_mqtt_mgr import RdApp
import json
import redis
import time

### {
###   "on": false,
###   "alive": false,
###   "modules":
###   {
###      "barbec":
###		 {
###         "name":"barbec",
###         "slaves":14,	    ///< 1 bit = 1 esclave present: 14 veut dire bit 2..4 présents donc esclaves 1, 2, 3 et 4 presents
###         "cmds":6,			///< Même principe pour les commandes des EVs/Pompes (6 = 1 et 2 commandes)
###         "ons":6,			///< Même principe pour les etats ON/OFF des EVs/Pompe (6 = 1 et 2 ON)
###         "comms":14,			///< Même principe pour la comm (oyas/pompe communiquent avec le maitre ou pas)
###         "lows":4,			///< Même principe pour le capteur bas de chaque oya (4 = oya 2 low tous les autres pas low)
###         "highs":0,			///< Même principe pour le capteur haut de chaque oya (0 = aucun oya high)
###         "date_mqtt":"2025-05-14T00:45:12Z",
###         "date":"2025-05-14T00:46:12Z",
###         "slaves":
###         [
###           {
###              "addr":1,
###              "type":"pump",
###              "on":true,
###				 "flow":456,
###              "comm_ok":true
###           },
###           {
###              "addr":2,
###              "type":"oya",
###              "on":true,
###              "low":true,
###              "high":false,
###              "comm_ok":true
###           },
###           {
###              "addr":3,
###              "type":"oya",
###              "on":false,
###              "low":false,
###              "high":false,
###              "comm_ok":true
###           }
###         ]
###		 },
###      "reduit":
###		 {
###         "name":"reduit",
###         "slaves":2,
###         "cmds":0,
###         "ons":0,
###         "comms":2,
###         "lows":0,
###         "highs":0,
###         "date_mqtt":"2025-05-14T00:45:12Z",
###         "date":"2025-05-14T00:46:12Z",
###         "slaves":
###         [
###           {
###              "addr":1,
###              "type":"pump",
###              "on":false,
###				 "flow":0,
###              "comm_ok":true
###           }
###         ]
###		 },  
###   }
### }
###
###
### Il y'a plusieurs maitres des oyas (objet reduit:{} et barbec:{}). On en aura trois avec trois noms differents
###
### Ne pas decoder cmds, ons, lows et highs dont le detail est repris dans la liste slaves:[]
###
### La liste slaves[] est variable et contient tous les esclaves.
### NB: Il y'a soit une pompe, soit des oyas dans cette liste. L'IHM doit s'adapter...
###




class RdOyasClient(RdApp):
    def __init__(self,ip,port=6379):
        super(RdOyasClient,self).__init__(ip,port)

    def setOn(self,flgOn,tm=None):
        if (flgOn==True):
            self.setSleep(False)
            
        self.set_app_var_bool('on',flgOn,tm)

    #def setSleep(self,flgSleep):
    #    self.set_app_var_bool('sleep',flgSleep,None)        

    def setCmd(self,mname,flgOn,tm=None):
        self.set_mod_var(mname,'to_cmd',1 if flgOn==True else 0,tm)

    def hasModule(self,name):
        res=self.r.smembers('%s.modules' % (self.kApp()))
        if res!=None:
            for s in res:
                if name==s:
                    return True

        return False

    def getModuleJson(self,name):

        slaves=self.get_mod_var_int(name,'slaves')
        comms=self.get_mod_var_int(name,'comms')
        cmds=self.get_mod_var_int(name,'cmds')
        ons=self.get_mod_var_int(name,'ons')
        lows=self.get_mod_var_int(name,'lows')
        highs=self.get_mod_var_int(name,'highs')
        date=self.get_mod_var(name,'date')
        date_mqtt=self.get_mod_var(name,'date_mqtt')
            
        mod={
            'name':name,
            'slaves':self.get_mod_var_int(name,'slaves'),
            'cmds':self.get_mod_var_int(name,'cmds'),
            'ons':self.get_mod_var_int(name,'ons'),
            'comms':self.get_mod_var_int(name,'comms'),
            'lows':self.get_mod_var_int(name,'lows'),
            'highs':self.get_mod_var_int(name,'highs'),
            'date_mqtt':self.get_mod_var(name,'date_mqtt'),
            'date':self.get_mod_var(name,'date'),
            'slaves':[]
            }

        cfg=int(slaves)
        for addr in range(1,15):
            mask=1<<addr

            if (cfg&mask)==mask:
                obj={
                    'addr':addr,
                    'type':'pump' if addr==1 else 'oya',
                    'on':(ons & mask) == mask,
                    'comm_ok':(comms & mask) == mask,
                    }

                if addr!=1:
                    obj['high']=(highs & mask) == mask
                    obj['low']=(lows & mask) == mask
                
                mod['slaves'].append(obj)
            
        
        return mod

    def getJson(self):
        ret={
            'on': self.get_app_var_bool('on'),
            'alive': self.get_app_var_bool('alive'),
            #'sleep': self.get_app_var_bool('sleep'),
            'modules':{}
            }
        
        res=self.r.smembers('%s.modules' % (self.kApp()))
        for name in res:
            mod=self.getModuleJson(name)
            ret['modules'][name]=mod         


        return ret            

     
if __name__=='__main__':
    cln=RdOyasClient('192.168.3.200')
    print(json.dumps(cln.getJson(),indent=1))
    print('_'*40)


