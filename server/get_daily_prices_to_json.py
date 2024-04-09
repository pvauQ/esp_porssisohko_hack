# tekee jsonin jossa rakenne [ {{date:""}{tunnnit:[0,1,2,3]}}, toinen mokoma]
# lista  json objekteja joilla pvm ja 4  hina arvoa joka tunnilta
#palauttaa nykyisen ja  huomisen hinnat
import requests
import datetime
import json
FILE_PATH = ""


class DayPrices:
    tunnit= {key: None for key in range(24)}
    date_string = ""

    def __init__(self, j_data,date):
        self.date_string = date.strftime("%Y-%m-%d")

        for i in range(len(j_data)):
            ## ne on järjestyksessä, ei ees chekata
            self.tunnit[i] = [float(j_data[i]["hinta"]) for k in range(4)] ## koska 15 min hinnoittelu incoming in 2025?
    
    def __to_dict__(self):
        return {"date":self.date_string,  "tunnit":self.tunnit}


def getDatFromInternets(date):
    query = f"tunnit=24&tulos=sarja&aikaraja={date}"
    r = requests.get(f"{url}{query}")
    if r.status_code != 200:
            print(r.status_code)
            print(r.text)
            return
    data = r.json()
    dayprices = DayPrices(data,date)

    return dayprices
    return json.dumps(dayprices.__to_dict__())



url  = "https://www.sahkohinta-api.fi/api/v1/halpa?" # vaihtaa sit tilale muun kun jos tarvii.

today = datetime.date.today()
tomorow = datetime.date.today()  + datetime.timedelta(days = 1)


prices_today  = getDatFromInternets(today)
prices_tomorow = getDatFromInternets(tomorow)
p_list =[]
for p in [prices_today,prices_tomorow]:
     if p:
          p_list.append(p.__to_dict__())

prices = json.dumps(p_list, default=lambda x: x.__dict__)
print(prices)

file_path = f"{FILE_PATH}dayprices.json"  

if prices: # jos ei palautunut mitään
    with open(file_path, 'w+') as filetowrite:
        print("wrote some stuff")
        filetowrite.write(prices)

