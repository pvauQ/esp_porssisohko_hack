# tekee jsonin jossa rakenne [ {{date:""}{tunnnit:[0,1,2,3]}}, toinen mokoma]
# lista  json objekteja joilla pvm ja 4  hinta arvoa joka tunnilta
#palauttaa nykyisen ja  huomisen hinnat
import requests
import datetime
import json
FILE_PATH = ""


def parseToDict(data, date):
        
    date_string = date.strftime("%Y-%m-%d") 
    tunnit = []
    if not data:
         return

    sourse_index = 0
    for i in range(24):
        tunti= []
        for j in range(4):
            tunti.append(data[sourse_index]["hinta"])
            sourse_index +=1
        tunnit.append(tunti)
    res = {"date":date_string,  "tunnit":tunnit}
    return res

def getDatFromInternets(date):
    query = f"vartit=96&tulos=sarja&aikaraja={date}"
    r = requests.get(f"{url}{query}")
    print(query)
    if r.status_code != 200:
            print(r.status_code)
            print(r.text)
            return
    data = r.json()
    return data


url  = "https://www.sahkohinta-api.fi/api/vartti/v1/halpa?" # vaihtaa sit tilale muun kun jos tarvii.

today = datetime.date.today()
tomorow = datetime.date.today()  + datetime.timedelta(days = 1)
prices_today  = getDatFromInternets(today)
prices_tomorow = getDatFromInternets(tomorow)
arr = []
p1 = parseToDict(prices_today,today)
p2= parseToDict(prices_tomorow,tomorow)
if prices_today: arr.append(p1)
if prices_tomorow: arr.append(p2)


prices = json.dumps(arr)
print(prices)

file_path = f"{FILE_PATH}dayprices.json"  

if prices: # jos ei palautunut mitään
    with open(file_path, 'w+') as filetowrite:
        filetowrite.write(prices)
        print("wrote some stuff")
else
    print("no prices for some reason! ;")