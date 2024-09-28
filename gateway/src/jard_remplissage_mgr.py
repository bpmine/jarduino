#!/usr/bin/python3
import json
import redis
import time

class RempliApp():
    TMT=5
    
    def get_mod_var_bool(self,mname,vname):
        k='wiio.%s.%s' % (mname,vname)
        val=self.r.get(k)
        if val==None or val!='1':
            return False
        else:
            return True
    
    def set_mod_var_bool(self,mname,vname,val,tmt=TMT):
        k='wiio.%s.%s' % (mname,vname)
        val=self.r.set(k,1 if val==True else 0)
        if tmt!=None:
            self.r.expire(k,tmt)

    def get_rempli_var(self,rname,vname):        
        val=self.r.get('remplis.%s.%s' % (rname,vname))
        return val
    
    def get_rempli_bool(self,rname,vname):
        val=self.get_rempli_var(rname,vname)
        if val==None or val!='1':
            return False
        else:
            return True
        
    def get_rempli_int(self,rname,vname):        
        val=self.get_rempli_var(rname,vname)
        if val!=None and val.isnumeric():
            return int(val,10)
        else:
            return None

    def set_rempli_var(self,rname,vname,val,tmt=TMT):
        k='remplis.%s.%s' % (rname,vname)
        self.r.set(k,val)
        if tmt!=None:
            self.r.expire(k,tmt)

    def set_rempli_bool(self,rname,vname,val,tmt=TMT):
        self.set_rempli_var(rname,vname,1 if val==True else 0,tmt)


    def add_remplissage(self,name,pump,dst,src):
        self.set_rempli_var(name,'name',name,None)
        self.set_rempli_var(name,'src',src,None)
        self.set_rempli_var(name,'dst',dst,None)
        self.set_rempli_var(name,'pmp',pump,None)
        
        self.set_rempli_bool(name,'on',False,None)
        self.set_rempli_var(name,'cons_src',3,None)
        self.set_rempli_var(name,'cons_dst',0,None)
                  

    def __init__(self,ip='192.168.3.200',port=6379):
        self.r = redis.Redis(host=ip, port=port, db=0, decode_responses=True)
        
        # On efface tout et on recree selon initial
        ks=self.r.keys('remplis.*')
        p=self.r.pipeline()
        for k in ks:
            p.delete(k)
        p.execute()

        self.add_remplissage("main_reduit"  , "main"  , "reduit", "main")
        self.add_remplissage("main_paul"    , "paul"  , "paul"  , "main")
        self.add_remplissage("reduit_barbec", "reduit", "barbec", "reduit")
        

    def tolvl(self,n1,n2,n3):
        if n3==True:
            return 3
        elif n2==True:
            return 2
        elif n1==True:
            return 1
        return 0

    def loop(self):
        ks=self.r.keys('remplis.*.name')
        for k in ks:
            name=self.r.get(k)
            print(name)

            src=self.get_rempli_var(name,'src')
            dst=self.get_rempli_var(name,'dst')
            pmp=self.get_rempli_var(name,'pmp')
            
            on=self.get_rempli_bool(name,'on')
            
            cons_src=self.get_rempli_int(name,'cons_src')
            cons_dst=self.get_rempli_int(name,'cons_dst')

            lvl_src=None
            lvl_dst=None

            if on==True:
                cmd=False

                # Si le niveau de la source est donne, on arrete qd trop bas !
                src_too_low=False
                if src!=None:
                    if self.get_mod_var_bool(src,'valid')==False:
                        src_too_low=True
                    else:
                        n1=self.get_mod_var_bool(src,'n1')
                        n2=self.get_mod_var_bool(src,'n2')
                        n3=self.get_mod_var_bool(src,'n3')
                        lvl_src=self.tolvl(n1,n2,n3)
                        
                        if cons_src==1:
                            src_too_low=not n1
                        elif cons_src==2:
                            src_too_low=not n2
                        elif cons_src==3:
                            src_too_low=not n3
                    

                # Sinon, on cherche a atteindre la consigne
                if src_too_low==False:
                    if dst!=None:
                        if self.get_mod_var_bool(dst,'valid')==False:
                            cmd=False
                        else:
                            n1=self.get_mod_var_bool(dst,'n1')
                            n2=self.get_mod_var_bool(dst,'n2')
                            n3=self.get_mod_var_bool(dst,'n3')
                            lvl_dst=self.tolvl(n1,n2,n3)
                            print('%s %s %s' % (n1,n2,n3) )
                            
                            if cons_dst==1:
                                cmd=not n1
                            elif cons_dst==2:
                                cmd=not n2
                            elif cons_dst==3:
                                cmd=not n3
                            else:
                                cmd=False
                    else:
                        cmd=True
                        
                else:
                    cmd=False

                if pmp!=None:
                    #pass
                    self.set_mod_var_bool(pmp,'to_cmd',cmd)            

                txt='%-20s: on=%-6s, %s[%s] -> %s[%s], pmp=%s' % (name,on,lvl_src,cons_src,lvl_dst,cons_dst,cmd)
                print(txt)

      
if __name__=='__main__':
    print('Démarrage de Remplissage...')
    app=RempliApp()    

    while True:
        print('_'*40)
        app.loop()
        time.sleep(2)


