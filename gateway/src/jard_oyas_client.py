#!/usr/bin/python3
from jard_oyas_mqtt_mgr import RdApp
import json
import redis
import time

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


