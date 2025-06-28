from flask import Flask, render_template
import requests
from datetime import datetime

app = Flask(__name__)

WEBSERVICE_URL = 'http://192.168.3.200:5000/oyas'

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
        response = requests.get("http://192.168.3.200:5000/states", timeout=5)
        response.raise_for_status()
        data = response.json()
        cuves = []
        for cuve in data.get("modules", []):
            cuves.append({
                "name": cuve.get("name"),
                "date": datetime.fromisoformat(cuve["date"]).strftime("%d/%m/%Y %H:%M:%S"),
                "pwr": cuve.get("pwr"),
                "rssi": cuve.get("rssi"),
                "valid": cuve.get("valid", True),
                "sleep": cuve.get("sleep", False),
                "level": niveau_cuve(cuve)
            })
        return cuves
    except Exception as e:
        print(f"Erreur lors de l'appel des cuves : {e}")
        return []


@app.route('/')
def index():
    try:
        response = requests.get(WEBSERVICE_URL, timeout=5)
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
                "date": datetime.fromisoformat(module_info.get("date").replace("Z", "+00:00")).strftime("%d/%m/%Y %H:%M:%S"),
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

    return render_template('index.html', modules=modules,cuves=cuves)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080, debug=False)
