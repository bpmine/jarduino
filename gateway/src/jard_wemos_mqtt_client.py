#!/usr/bin/python3
from jard_wemos_mqtt_mgr import RdApp
import json
import redis
import time

class RdWiioClient(RdApp):
    def __init__(self,ip,port=6379):
        super(RdWiioClient,self).__init__(ip,port)

    def setOn(self,flgOn,tm=None):
        if (flgOn==True):
            self.setSleep(False)
            
        self.set_app_var_bool('on',flgOn,tm)

    def setSleep(self,flgSleep):
        self.set_app_var_bool('sleep',flgSleep,None)        

    def setCmd(self,mname,flgOn,tm=None):
        self.set_mod_var(mname,'to_cmd',1 if flgOn==True else 0,tm)

    def getLvl(self,mname):
        n1=self.get_mod_var_bool(mname,'n1')
        n2=self.get_mod_var_bool(mname,'n2')
        n3=self.get_mod_var_bool(mname,'n3')

        lvl=None
        if n1==True:
            lvl=1
        if n2==True:
            lvl=2
        if n3==True:
            lvl=3

        return lvl

    def hasModule(self,name):
        res=self.r.smembers('%s.modules' % (self.kApp()))
        if res!=None:
            for s in res:
                if name==s:
                    return True

        return False        

    def getModuleJson(self,name):
        mod={
            'name':name,
            'cmd':self.get_mod_var_bool(name,'cmd'),
            'n1':self.get_mod_var_bool(name,'n1'),
            'n2':self.get_mod_var_bool(name,'n2'),
            'n3':self.get_mod_var_bool(name,'n3'),
            'rssi':self.get_mod_var_int(name,'rssi'),
            'pwr':self.get_mod_var_int(name,'pwr'),
            'valid':self.get_mod_var_bool(name,'valid'),
            'sleep':self.get_mod_var_bool(name,'sleep'),
            'date':self.get_mod_var(name,'date')
            }
        
        return mod

    def getJson(self):
        ret={
            'on': self.get_app_var_bool('on'),
            'alive': self.get_app_var_bool('alive'),
            'sleep': self.get_app_var_bool('sleep'),
            'modules':{},
            'remplis':{},
            }
        
        res=self.r.smembers('%s.modules' % (self.kApp()))
        for name in res:
            mod=self.getModuleJson(name)
            ret['modules'][name]=mod            


        return ret            

     
if __name__=='__main__':
    cln=RdWiioClient('192.168.3.200')
    print(cln.getJson())
    print('_'*40)


