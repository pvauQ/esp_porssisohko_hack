# tekee jsonin jossa rakenne [ {{date:""}{tunnnit:[0,1,2,3]}}, toinen mokoma]
# lista  json objekteja joilla pvm ja 4  hina arvoa joka tunnilta
#palauttaa nykyisen ja  huomisen hinnat
import requests
import datetime
import json
FILE_PATH = ""


def parseToDict(data, date):
        
    date_string = date.strftime("%Y-%m-%d") 
    tunnit = {}
    for i in range(len(data)):
        tunnit[i] = [float(data[i]["hinta"]) for k in range(4)] ## koska 15 min hinnoittelu incoming in 2025?
    res = {"date":date_string,  "tunnit":tunnit}
    return res

def getDatFromInternets(date):
    query = f"tunnit=24&tulos=sarja&aikaraja={date}"
    r = requests.get(f"{url}{query}")
    print(query)
    print("kissa")
    if r.status_code != 200:
            print(r.status_code)
            print(r.text)
            return
    data = r.json()
    return data


url  = "https://www.sahkohinta-api.fi/api/v1/halpa?" # vaihtaa sit tilale muun kun jos tarvii.

today = datetime.date.today()
tomorow = datetime.date.today()  + datetime.timedelta(days = 1)

prices_today  = getDatFromInternets(today)
prices_tomorow = getDatFromInternets(tomorow)
p1 = parseToDict(prices_today,today)
p2= parseToDict(prices_tomorow,tomorow)


prices = json.dumps([p1,p2])
print(prices)

file_path = f"{FILE_PATH}dayprices.json"  

if prices: # jos ei palautunut mitään
    with open(file_path, 'w+') as filetowrite:
        print("wrote some stuff")
        filetowrite.write(prices)

