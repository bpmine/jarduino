from flask import Flask, render_template
from pymongo import MongoClient
import pymongo;
import datetime;
import re;

app = Flask(__name__,
            static_url_path='', 
            static_folder='static',
            template_folder='templates')

p=re.compile("^([0-9]{4})-([0-9]{2})-([0-9]{2})T([0-9]{2}):([0-9]{2}):([0-9]{2})Z$")
def getDate(strDt):
    m=p.match(strDt)
    if (m!=None):
        year=int(m.group(1))
        month=int(m.group(2))
        day=int(m.group(3))
        hour=int(m.group(4))
        minute=int(m.group(5))
        second=int(m.group(6))

        dt=datetime.datetime(year,month,day,hour,minute,second)

        return dt

    return None

def getDatas(serial):
    jrs=[
        'lundi',
        'mardi',
        'mercredi',
        'jeudi',
        'vendredi',
        'samedi',
        'dimanche'
        ];
    
    datas={
    'state':'?',
    'vsun':'?',
    'vbatt':'?',
    'last':'?',
    'now':'?'
    }

    try:    
        mg=MongoClient("mongodb://127.0.0.1:27017");
        
        db=mg.jarduino;
        logs = db["logs"]
        res=logs.find({'serial':serial}).sort('date',pymongo.DESCENDING).limit(1)

        try:
            d=res[0]

            datas['state']=d['pompe']

            fTmp=float(int(d['vsun']))
            fTmp=fTmp/10;
            datas['vsun']='%.1f' % fTmp

            fTmp=float(int(d['vbatt']))
            fTmp=fTmp/10;
            datas['vbatt']='%.1f' % fTmp

            dte=d['date']
            jdls=jrs[dte.weekday()]
            datas['now']="%s %02d/%02d/%02d à %02d:%02d" % (jdls,dte.day,dte.month,dte.year,dte.hour,dte.minute)

        except:
            pass;

        res=logs.find({'pompe':1,'serial':serial}).sort('date',pymongo.DESCENDING).limit(1)

        try:
            dte=res[0]['date']

            print(dte)
            jdls=jrs[dte.weekday()]
            datas['last']="%s %02d/%02d/%02d à %02d:%02d" % (jdls,dte.day,dte.month,dte.year,dte.hour,dte.minute)
        except:
            datas['last']="?"
            
        mg.close()
    except Exception as ex:
        print(ex)
        datas['error']=True;

    datas['serial']=serial
    return datas;
    

@app.route("/") 
def home():
    datas=[]
    datas.append(getDatas('d477863a7d80'))
    datas.append(getDatas('5804c11f9c9c'))

    datas[0]['name']='Côté réduit'
    datas[1]['name']='Barbecue'
    
    return render_template(r'index.html',datas=datas)

@app.route("/robots.txt")
def robots():
	return "User-Agent: *\nDisallow: /"

if __name__=="__main__":    
    app.run(host="0.0.0.0",debug = False,port=80)

