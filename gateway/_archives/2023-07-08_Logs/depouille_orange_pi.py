import re
from datetime import datetime
from datetime import timedelta
import pandas as pd

import matplotlib
import matplotlib.pyplot as plt
from matplotlib import ticker
from matplotlib import dates
import numpy as np

import json
import csv


pData=re.compile(r"(\d{2})/(\d{2})/(\d{4}) (\d{2}):(\d{2}):(\d{2})> /wifiio/data/(.+) : b'(.+)'")
def read_orange_log(filename):
    events=[]
    mod={}
    with open(filename) as fp:
        for ln in fp.readlines():
            m=pData.match(ln.strip())
            if m!=None:
                day=int(m.group(1))
                month=int(m.group(2))
                year=int(m.group(3))
                hour=int(m.group(4))
                minute=int(m.group(5))
                sec=int(m.group(6))
                
                nme=m.group(7).strip()

                data=json.loads(m.group(8))
                data['name']=nme
                data['date']=datetime(year,month,day,hour,minute,sec)

                if nme not in mod:
                    mod[nme]=data
                    events.append(data)
                else:
                    ks=['cmd','n1','n2','n3']
                    for k in ks:
                        if mod[nme][k]!=data[k]:
                            events.append(data)
                            mod[nme]=data
                    
                
    return events

def fct(x,pos):
    return 'N3' if x==3 else 'N2' if x==2 else 'N1' if x==1 else '0'

def lvl(d):
    if d['n3']==True:
        return 3
    elif d['n2']==True:
        return 2
    elif d['n1']==True:
        return 1
    else:
        return 0

def rpt_levels(datas,np,ns,nd):
    addr='b'
    msk=0x01<<(ord(addr)-ord('a')+1)
    df1 = pd.DataFrame(data = {
                                ns: [d['%s_lvl' % (ns)] for d in datas]
                              },
                      index=[d['date'] for d in datas]
                      )

    df2 = pd.DataFrame(data = {
                                'Pompe': [1 if d['%s_cmd' % (np)]==True else 0 for d in datas]
                              },
                      index=[d['date'] for d in datas]
                      )

    df3 = pd.DataFrame(data = {
                                nd: [d['%s_lvl' % (nd)] for d in datas]
                              },
                      index=[d['date'] for d in datas]
                      )

    fig, axes = plt.subplots(nrows=3, ncols=1)
    df1.plot(ax=axes[0],color='blue')
    df2.plot(ax=axes[1],color='red')
    df3.plot(ax=axes[2],color='green')

    axes[0].set_title('Pompage %s vers %s' % (ns,nd))
    axes[0].set_xlabel(None)
    axes[0].set_ylabel("Niveau %s" % ns)
    axes[0].set_xticks([])
    axes[0].set_yticks([0,1,2,3])
    axes[0].yaxis.set_major_formatter(ticker.FuncFormatter(fct))
    axes[0].set_ylim(0,3.1)
    
    axes[1].set(xlabel=None)
    axes[1].set_ylabel("ON/OFF")
    axes[1].set_xticks([])
    axes[1].set_yticks([0,1])
    axes[1].set_ylim(0,1)

    axes[2].set_xlabel('Date')
    axes[2].set_ylabel("Niveau %s" % nd)
    axes[2].set_yticks([0,1,2,3])
    axes[2].yaxis.set_major_formatter(ticker.FuncFormatter(fct))
    axes[2].xaxis.set_major_locator(dates.DayLocator())
    axes[2].set_ylim(0,3.1)
        
    plt.show() 


def events_to_datas(evts):
    datas=[]
    last={}

    for n in ['main','paul','reduit','barbec']:
        last[n]={
            'date':None,
            'lvl':None,
            'cmd':None,
            'alive':None
    }

    for e in evts:
        nme=e['name']
        if nme not in last.keys():
            print('Unknown name %s' % nme)
            continue
        
        if lvl(e)!=last[nme]['lvl'] or e['cmd']!=last[nme]['cmd']:
            ser_obj={
                'date':e['date']
                }
            for n in last.keys():
                ser_obj['%s_lvl'%n]=last[n]['lvl']
                ser_obj['%s_cmd'%n]=last[n]['cmd']

                if e['cmd']==False and last[n]['cmd']==True:
                    if e['date']-last[n]['date'] > timedelta(minutes=15):
                        print('Attention cmd: %s %s/%s -> %s/%s' % (n,last[n]['cmd'],last[n]['date'],e['cmd'],e['date']))
                
            datas.append(ser_obj)
            
            ser_obj={
                'date':e['date']
                }
            for n in last.keys():
                if n!=nme:
                    ser_obj['%s_lvl'%n]=last[n]['lvl']
                    ser_obj['%s_cmd'%n]=last[n]['cmd']
                else:
                    ser_obj['%s_lvl'%n]=lvl(e)
                    ser_obj['%s_cmd'%n]=e['cmd']
            datas.append(ser_obj)
            
        last[nme]={
            'date':e['date'],
            'lvl':lvl(e),
            'cmd':e['cmd']
            }

    return datas


def datas_to_csv(datas,name):

    with open(name,'w',newline='') as fp:
        wr=csv.writer(fp,delimiter=';')
        hdr=[k for k in datas[0].keys()]
        wr.writerow(hdr)        
        
        for d in datas:
            row=[]
            for k in hdr:
                row.append(d[k])

            wr.writerow(row)

            

evts=read_orange_log('orange_pi.txt')
datas=events_to_datas(evts)

datas_to_csv(datas,'datas.csv')

#rpt_levels(datas,'main','main','reduit')
#rpt_levels(datas,'paul','main','paul')
#rpt_levels(datas,'reduit','reduit','barbec')






