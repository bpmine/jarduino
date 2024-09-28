#!/usr/bin/python3
from flask import Flask,request
import json
import time
from jard_wemos_mqtt_client import RdWiioClient
from jard_remplissage_client import RdRempliClient
from jard_oyas_client import RdOyasClient

wiioCln = RdWiioClient('192.168.3.200')
rempliCln=RdRempliClient()
oyasCln=RdOyasClient('192.168.3.200')

app = Flask(__name__)

@app.route('/test', methods=['GET'])
def test():
    return 'ok'


@app.route('/wiio/modules/<name>', methods=['GET'])
def get_wiio_info_by_name(name):
    if name==None:
        abort(404)

    if wiioCln.hasModule(name)==False:
        abort(404)
        
    return json.dumps(wiioCln.getModuleJson(name));


@app.route('/wiio/modules', methods=['GET'])
def get_wiio_modules():
    modules=[]
    if 'modules' in wiioCln.getJson():
        for m in wiioCln.getJson()['modules']:
            modules.append(m)

    return json.dumps(modules)


@app.route('/wiio', methods=['GET'])
def get_wiio():
    ret=wiioCln.getJson()
    return json.dumps(ret)

@app.route('/wiio/do/on', methods=['GET'])
def wiio_on():
    wiioCln.setOn(True);
    return {'result':True,'msg':'MODULE WIIO ON.'}

@app.route('/wiio/do/off', methods=['GET'])
def wiio_off():
    wiioCln.setOn(False);
    return {'result':True,'msg':'MODULE WIIO OFF.'}

@app.route('/wiio/do/sleep', methods=['GET'])
def wiio_sleep():
    wiioCln.setSleep(True);
    return {'result':True,'msg':'MISE EN VEILLE DES MODULES...'}

@app.route('/wiio/modules/<name>/do/test', methods=['GET'])
def wiio_pump_test(name):
    if wiioCln.hasModule(name)==False:
        abort(404)

    wiioCln.setCmd(name,True,10)
    return {'result':True,'msg':'Test envoyé avec succès.'}

@app.route('/wiio/modules/<name>/do/on', methods=['GET'])
def wiio_pump_on(name):
    if wiioCln.hasModule(name)==False:
        abort(404)

    duree=request.args.get('duration')
    if duree==None or not duree.isnumeric() or int(duree)>3600:
        duree=15*60
    else:
        duree=int(duree,10)
        
    wiioCln.setCmd(name,True,duree)
    
    return {'result':True,'msg':'Pompe %s ON.' % name}

@app.route('/wiio/modules/<name>/do/off', methods=['GET'])
def wiio_pump_off(name):
    if wiioCln.hasModule(name)==False:
        abort(404)

    wiioCln.setCmd(name,False,None)
    return {'result':True,'msg':'Pompe %s OFF.' % name}

@app.route('/wiio/remplis', methods=['GET'])
def wiio_get_remplissages():
    lst=rempliCln.getJsonList()
    return json.dumps(lst)

@app.route('/wiio/remplis/<name>', methods=['GET'])
def wiio_get_rempli_byname(name):
    if (rempliCln.hasRempli(name))==False:
        abort(404)

    js=rempliCln.getRempliJson(name)
    return json.dumps(js)

@app.route('/wiio/remplis/<name>/do/on', methods=['GET'])
def wiio_set_remplissage_on(name):
    if (rempliCln.hasRempli(name))==False:
        abort(404)
        
    rempliCln.setOn(name,True)
    
    return {'result':True,'msg':'Remplissage %s ON.' % name}

@app.route('/wiio/remplis/<name>/do/off', methods=['GET'])
def wiio_set_remplissage_off(name):
    if (rempliCln.hasRempli(name))==False:
        abort(404)
        
    rempliCln.setOn(name,False)
    
    return {'result':True,'msg':'Remplissage %s OFF.' % name}

@app.route('/wiio/remplis/<name>/do/setcons', methods=['GET'])
def wiio_set_remplissage_consignes(name):
    if (rempliCln.hasRempli(name))==False:
        abort(404)

    cons_src=request.args.get('src')
    if (cons_src!=None):
        rempliCln.setConsSrc(name,int(cons_src))
    
    cons_dst=request.args.get('dst')
    if (cons_dst!=None):
        rempliCln.setConsDst(name,int(cons_dst))
    
    return {'result':True,'msg':'Reglage des consignes de %s (%s -> %s).' % (name,cons_src,cons_dst)}


@app.route('/states', methods=['GET'])
def get_states():
    ret={
        'modules':[],
        'remplissages':[]
        }
    
    js=wiioCln.getJson()
    if js!=None and 'modules' in js:
        for k,m in js['modules'].items():
            ret['modules'].append(m)

    js=rempliCln.getJsonList()
    if js!=None:
        for r in js:
            ret['remplissages'].append(r)
    
    return json.dumps(ret)


@app.route('/oyas', methods=['GET'])
def get_oyas():
    js=oyasCln.getJson()
    return js


if __name__=='__main__':
    app.debug = True
    app.run(host='0.0.0.0',port=5000,debug=False)

