import csv
import re
from datetime import datetime
import pandas as pd

import matplotlib
import matplotlib.pyplot as plt
import numpy as np


pDte=re.compile(r'^(\d{2})/(\d{2})/(\d{4})$')
pTime=re.compile(r'^(\d{2}):(\d{2}):(\d{2})$')
class MasterLog:
    @staticmethod
    def create(row):
        master=MasterLog()

        m=pDte.match(row[0])
        if m==None:
            return None
        year=int(m.group(3))
        month=int(m.group(2))
        day=int(m.group(1))

        m=pTime.match(row[1])
        if m==None:
            return None
        hour=int(m.group(1))
        minute=int(m.group(2))
        second=int(m.group(3))

        master.date=datetime(year,month,day,hour,minute,second)
        
        master.valid=int(row[2],16)
        master.on=int(row[3],16)
        master.low=int(row[4],16)
        master.high=int(row[5],16)

        master.temp_min=int(row[6])
        master.temp_moy=int(row[7])
        master.temp_max=int(row[8])

        master.hum_min=int(row[9])
        master.hum_moy=int(row[10])
        master.hum_max=int(row[11])

        return master

    def __str__(self):
        return '%s: %02x %02x %02x %02x %d°C %d%%' % (self.date,self.valid,self.on,self.low,self.high,self.temp_moy,self.hum_moy)
        

        

def read_maitre_oya_log(filename):
    datas=[]
    with open(filename) as fp:
        rdr=csv.reader(fp,delimiter=';')
        for row in rdr:
            if len(row)!=13:
                continue

            mst=MasterLog.create(row)
            if mst==None:
                print('Error')
                continue
            
            datas.append(mst)

    return datas


def rpt_temp_hum(datas):
    df = pd.DataFrame(data = {
                                'temp_min': [d.temp_min for d in datas],
                                'temp': [d.temp_moy for d in datas],
                                'temp_max': [d.temp_max for d in datas],
                                'hum_min': [d.hum_min for d in datas],
                                'hum': [d.hum_moy for d in datas],
                                'hum_max': [d.hum_max for d in datas],
                                
                              },
                      index=[d.date for d in datas]
                      )

    
    ax=df.plot()
    plt.title("Environnement des OYAS" )
    plt.xlabel('Date')
    plt.ylabel('Valeurs')
    ax.xaxis.set_major_locator(plt.MaxNLocator(22))
    plt.show()    

            
def rpt_levels(datas):
    addr='b'
    msk=0x01<<(ord(addr)-ord('a')+1)
    df = pd.DataFrame(data = {
                                'Niveau': [0 if d.high&msk==msk and d.low&msk==msk else 50 if d.high&msk==msk and d.low&msk==0 else 100 for d in datas],
                                'Humidité': [d.hum_moy for d in datas],
                                'Température': [d.temp_moy for d in datas],
                              },
                      index=[d.date for d in datas]
                      )

    
    ax=df.plot()
    plt.title("Activité de l'oya %s" % (addr))
    plt.xlabel('Date')
    plt.ylabel('Valeurs')
    #plt.xticks(df.index, df.index.strftime('%m-%d %H'), rotation=45)
    ax.xaxis.set_major_locator(plt.MaxNLocator(22))
    plt.show()    


def rpt_durees_remplissages(datas):
    cumuls={}
    for addr in ['b','c','d','e','f']:
        print('_'*40)
        print('OYA %s:' % (addr) )
        msk=0x01<<(ord(addr)-ord('a')+1)
        on=None
        start_on=None
        last_date=None
        cumuls[addr]=0
        
        for d in datas:
            if on==None:
                on=True if d.on&msk==msk else False

            non=True if d.on&msk==msk and d.valid&msk==msk else False        

            if (start_on!=None) and (d.date-last_date).total_seconds()>30*60:
                start_on=None
                on=False
                
            if non!=on:
                #print('%s> %s -> %s' % (d.date,on,non) )

                if non==True:
                    start_on=d.date
                else:
                    end_on=d.date
                    dur=end_on-start_on

                    lvl=0 if d.high&msk==msk and d.low&msk==msk else 50 if d.high&msk==msk and d.low&msk==0 else 100
                    
                    print('  %s> durée: %s (Niveau final=%s%%)' % (start_on,dur,lvl))
                    cumuls[addr]+=dur.total_seconds()
                    
                on=non

            last_date=d.date

        print('Total: %s s' % (cumuls[addr]))
            

def rpt_on(datas):
    for d in datas:
        d.ons={}
        for addr in ['b','c','d','e','f']:
            msk=0x01<<(ord(addr)-ord('a')+1)
            
            d.ons[addr]=100 if d.on&msk==msk else 0

    df = pd.DataFrame(data = {
                                'oya B': [d.ons['b'] for d in datas],
                                'oya C': [d.ons['c'] for d in datas],
                                'oya D': [d.ons['d'] for d in datas],
                                'oya E': [d.ons['e'] for d in datas],
                                'oya F': [d.ons['f'] for d in datas],
                              },
                      index=[d.date for d in datas]
                      )

    
    ax=df.plot()
    plt.title("Activité des oyas")
    plt.xlabel('Date')
    plt.ylabel('Valeurs')
    #plt.xticks(df.index, df.index.strftime('%m-%d %H'), rotation=45)
    ax.xaxis.set_major_locator(plt.MaxNLocator(22))
    plt.show()    
            

datas=read_maitre_oya_log('maitre_oya.txt')
#rpt_temp_hum(datas)
#rpt_levels(datas)
rpt_durees_remplissages(datas)
#rpt_on(datas)

