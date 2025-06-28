from flask import Flask, render_template
import requests
from datetime import datetime
import re

app = Flask(__name__)

IP_WEBSERVICE = 'http://127.0.0.1:5000'
IP_WEBSERVICE = 'http://192.168.3.200:5000'

pDte=re.compile(r'^(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})')
def iso_to_dte(iso_str):
    m=pDte.match(iso_str)
    if m!=None:
        year=int(m.group(1))
        month=int(m.group(2))
        day=int(m.group(3))
        hour=int(m.group(4))
        minute=int(m.group(5))
        sec=int(m.group(6))
        
        dte=datetime(year,month,day,hour,minute,sec)

        return dte.strftime("%d/%m/%Y %H:%M:%S")

    return '???'    
    

def resume_levels(oyas):
    tot=len(oyas)
    if tot==0:
        return 0
    
    nb_low=0
    for o in oyas:
        try:
            low=o['low']
            high=o['high']
            if low==True:
                nb_low+=1
        except:
            pass
        
    return nb_low*100/tot
    

def niveau_cuve(cuve):
    n1=cuve.get('n1',False)
    n2=cuve.get('n2',False)
    n3=cuve.get('n3',False)

    level=0
    if n1==True and n2==False and n3==False:
        level=30
    elif n1==True and n2==True and n3==False:
        level=70
    elif n1==True and n2==True and n3==True:
        level=100
    
    return level

def get_cuves():
    try:
        response = requests.get(IP_WEBSERVICE+"/states", timeout=5)
        response.raise_for_status()
        data = response.json()
        cuves = []
        for cuve in data.get("modules", []):
            cuves.append({
                "name": cuve.get("name"),
                "date": iso_to_dte(cuve["date"]),
                "pwr": int(cuve.get("pwr"))/10.0,
                "rssi": cuve.get("rssi"),
                "valid": cuve.get("valid", True),
                "sleep": cuve.get("sleep", False),
                'cmd':cuve.get("cmd", False),
                "level": niveau_cuve(cuve)                
            })            

        return cuves
    except Exception as e:
        print(f"Erreur lors de l'appel des cuves : {e}")
        return []


@app.route('/')
def index():
    try:
        response = requests.get(IP_WEBSERVICE+"/oyas", timeout=5)
        response.raise_for_status()
        data = response.json()
        modules = []

        for module_name, module_info in data.get("modules", {}).items():
            slaves = module_info.get("slaves", [])
            level = resume_levels([oya for oya in slaves if oya.get("type") == "oya"])
            comm_issues = any(not oya.get("comm_ok", True) for oya in slaves)
            high_count = sum(1 for oya in slaves if oya.get("type") == "oya" and oya.get("high"))
            alert_count = sum(1 for oya in slaves if oya.get("type") == "oya" and not oya.get("high") and not oya.get("low"))
            module = {
                "name": module_info.get("name"),
                "date": iso_to_dte(module_info.get("date").replace("Z", "+00:00")),
                "level": level,
                "comm_issues": comm_issues,
                "high_count": high_count,
                "alert_count": alert_count,
                "slaves": slaves
            }
            modules.append(module)
    except Exception as e:
        print(f"Erreur lors de l'appel : {e}")
        modules = []

    cuves=get_cuves()

    return render_template('index_bootstrap.html', modules=modules,cuves=cuves)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080, debug=False)
