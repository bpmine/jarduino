import time
from jard_wemos_mqtt_mgr import RdApp

class RdWiioSimu(RdApp):
    def __init__(self,ip,port=6379):
        super(RdWiioSimu,self).__init__(ip,port)
        self.modules=['main','paul','reduit','barbec']
        self.ctx={}
       
        ks=self.r.keys('%s.*' % (self.kApp()))
        p=self.r.pipeline()
        for k in ks:
            p.delete(k)
        p.execute()

        self.set_app_var('alive',1)
        self.set_app_var('on',1)

        self.r.delete('%s.modules' %(self.kApp()))
        for m in self.modules:
            self.r.sadd('%s.modules' % (self.kApp()),m)
            self.ctx[m]={
                'name':m,
                'n1':False,
                'n2':False,
                'n3':False,
                'cmd':False,
                'rssi':-4,
                'pwr':1000,
                'level':0
                }
        
    def start(self):        
        print('Modules:')
        for m in self.modules:
            print('  - %s' % (m))
        print('_'*40)
        print('Start jarduino Wemos simulator...')
        
        while True:
            time.sleep(1)

            on = self.get_app_var('on')
            if on==None:
                on=False
            else:
                on=True if on==1 else False

##            if on==False:
##                for m in self.modules:
##                    self.ctx[m]['valid']=False
##                continue
            
            for n in self.modules:
                v=self.get_mod_var(n,'to_cmd')
                if v!=None and v==1:
                    self.ctx[n]['cmd']=True
                else:
                    self.ctx[n]['cmd']=False

                if self.ctx[n]['cmd']==True:
                    self.ctx[n]['level']-=1
                else:
                    self.ctx[n]['level']+=1

                if self.ctx[n]['level']<0:
                    self.ctx[n]['level']=0
                    
                if self.ctx[n]['level']>30:
                    self.ctx[n]['level']=30

                if self.ctx[n]['level']>8:
                    self.ctx[n]['n1']=True
                else:
                    self.ctx[n]['n1']=False

                if self.ctx[n]['level']>18:
                    self.ctx[n]['n2']=True
                else:
                    self.ctx[n]['n2']=False

                if self.ctx[n]['level']>25:
                    self.ctx[n]['n3']=True
                else:
                    self.ctx[n]['n3']=False


                self.set_mod_var(n,'name',n,None)
                self.set_mod_var(n,'cmd',1 if self.ctx[n]['cmd']==True else 0,None)
                self.set_mod_var(n,'n1',1 if self.ctx[n]['n1']==True else 0,None)
                self.set_mod_var(n,'n2',1 if self.ctx[n]['n2']==True else 0,None)
                self.set_mod_var(n,'n3',1 if self.ctx[n]['n3']==True else 0,None)
                self.set_mod_var(n,'rssi',self.ctx[n]['rssi'],None)
                self.set_mod_var(n,'pwr',self.ctx[n]['pwr'],None)
                self.set_mod_var(n,'valid',1)          
        

if __name__=='__main__':
    srv=RdWiioSimu('192.168.3.200')
    srv.start()





