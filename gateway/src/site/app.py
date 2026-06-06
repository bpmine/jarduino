from flask import Flask, render_template,jsonify,request
import requests
from datetime import datetime
import re

app = Flask(__name__)

IP_WEBSERVICE = 'http://127.0.0.1:5000'
#IP_WEBSERVICE = 'http://192.168.3.200:5000'

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
        response = requests.get(IP_WEBSERVICE+"/wiio", timeout=5)
        response.raise_for_status()
        data = response.json()
        cuves = []
        for nme,cuve in data.get("modules", {}).items():
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

        state={
            'on':data.get('on',False),
            'sleep':data.get('sleep',False),
            'alive':data.get('alive',False),
            'pumping':data.get('pumping',False),
            'waking':data.get('waking',False)
        }

        return cuves,state
    
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
            alert_count = sum(1 for oya in slaves if oya.get("type") == "oya" and not oya.get("high") and not oya.get("low") and oya.get("comm_ok"))
            comm_issues_count=sum(1 for slv in slaves if not slv.get("comm_ok"))
            module = {
                "name": module_info.get("name"),
                "date": iso_to_dte(module_info.get("date").replace("Z", "+00:00")),
                "level": level,
                "comm_issues": comm_issues,
                "high_count": high_count,
                "alert_count": alert_count,
                "comm_issues_count": comm_issues_count,
                "slaves": slaves
            }
            modules.append(module)
    except Exception as e:
        print(f"Erreur lors de l'appel : {e}")
        modules = []

    cuves, state=get_cuves()

    return render_template('index_bootstrap.html', modules=modules,cuves=cuves,state=state)

@app.route('/command/general/<state>', methods=['POST'])
def set_general_state(state):
    base_url = f"{IP_WEBSERVICE}/wiio/do"
    
    if state == "on":
        url = f"{base_url}/on"
    elif state == "off":
        url = f"{base_url}/off"
    elif state == "sleep":
        url = f"{base_url}/sleep"
    else:
        return jsonify({"status": "error", "message": "État invalide"}), 400

    print('URL: ',url)
    try:
        response = requests.get(url)
        return jsonify({"status": "ok", "response": response.text})
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/command/<module>/on', methods=['POST'])
def pump_on(module):
    duration = request.args.get('duration', default=1, type=int)
    url = f"{IP_WEBSERVICE}/wiio/modules/{module}/do/on?duration={duration}"
    try:
        r = requests.get(url)
        return jsonify({"status": "ok", "response": r.text})
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/command/<module>/off', methods=['POST'])
def pump_off(module):
    url = f"{IP_WEBSERVICE}/wiio/modules/{module}/do/off"
    try:
        r = requests.get(url)
        return jsonify({"status": "ok", "response": r.text})
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/maintenance/oyas/on', methods=['POST'])
def oyas_on():
    url = f"{IP_WEBSERVICE}/oyas/do/on"
    try:
        r = requests.get(url)
        return jsonify({"status": "ok", "message": "Oyas ON"})
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/maintenance/oyas/off', methods=['POST'])
def oyas_off():
    url = f"{IP_WEBSERVICE}/oyas/do/off"
    try:
        r = requests.get(url)
        return jsonify({"status": "ok", "message": "Oyas OFF"})
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/maintenance/oyas/modules/<name>/on', methods=['POST'])
def oyas_module_on(name):
    url = f"{IP_WEBSERVICE}/oyas/modules/{name}/do/on"
    try:
        r = requests.get(url)
        return jsonify({"status": "ok", "message": f"Oyas {name} ON"})
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/maintenance/oyas/modules/<name>/off', methods=['POST'])
def oyas_module_off(name):
    url = f"{IP_WEBSERVICE}/oyas/modules/{name}/do/off"
    try:
        r = requests.get(url)
        return jsonify({"status": "ok", "message": f"Oyas {name} OFF"})
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500


@app.route("/maintenance/oyas/modules/<name>")
def page_config(name):
    try:
        response = requests.get(IP_WEBSERVICE+"/oyas", timeout=5)
        response.raise_for_status()
        data = response.json()        
        modules=data.get('modules',{})
        module=None
        if name in modules.keys():
            module=modules[name]
            
    except Exception as ex:        
        data=None
        module=None

    return render_template("oyas_bootstrap.html", name=name, data=data,module=module)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080, debug=False)
