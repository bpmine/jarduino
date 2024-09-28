#!/usr/bin/python3
from jard_remplissage_mgr import RempliApp
import json
import redis
import time

class RdRempliClient(RempliApp):
    def __init__(self,ip='192.168.3.200',port=6379):
        super(RdRempliClient,self).__init__(ip,port)

    def setOn(self,name,flgOn):
        self.set_rempli_bool(name,'on',flgOn,None)

    def hasRempli(self,name):
        res=self.r.get('remplis.%s.name' % name)
        if res!=None:
            return True

        return False

    def setConsSrc(self,name,lvl):
        if lvl not in [1,2,3]:
            lvl=3

        self.set_rempli_var(name,'cons_src',lvl,None)

    def setConsDst(self,name,lvl):
        if lvl not in [1,2,3]:
            lvl=1

        self.set_rempli_var(name,'cons_dst',lvl,None)

    def getRempliJson(self,name):
        mod={
            'name':name,
            'src':self.get_rempli_var(name,'src'),
            'dst':self.get_rempli_var(name,'dst'),
            'pmp':self.get_rempli_var(name,'pmp'),
            
            'on':self.get_rempli_bool(name,'on'),
            'cons_src':self.get_rempli_int(name,'cons_src'),
            'cons_dst':self.get_rempli_int(name,'cons_dst')
            }
        
        return mod

    def getJsonList(self):
        ret=[]

        ks=self.r.keys('remplis.*.name')
        for k in ks:
            name=self.r.get(k)
            if name!=None:
                ret.append(self.getRempliJson(name))

        return ret

     
if __name__=='__main__':
    cln=RdRempliClient()
    print(cln.getJsonList())
    print('_'*40)


