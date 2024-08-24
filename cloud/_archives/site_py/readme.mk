*Jarduino site How to

** Install it on a server

- Install git
- install python 3.6 or >
- install pymongo, requests, demjson, pika, flask (see requirements.txt )
- install npm, node and pm2
- Add the two scripts to the process list

** PM2 commands

pm2 startup
pm2 --interpreter python3 --name jard_rec start ./jarduino_recorder.py
pm2 --interpreter python3 --name jard_site start ./jarduino_site.py
pm2 save --force

Then, it will be possible to manage the processes by
- pm2 start/stop jard_rec 
- pm2 start/stop jard_site 

