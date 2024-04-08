
import requests
import datetime
import json
FILE_PATH = ""

url  = "https://www.sahkohinta-api.fi/api/v1/halpa?"

query  = f"tunnit=24&tulos=sarja&aikaraja=2024-03-29" 

date = datetime.date.today() + datetime.timedelta(days = 1)
query  = f"tunnit=24&tulos=sarja&aikaraja={date}"

class DayPrices:
    tunnit= {key: None for key in range(24)}
    date_string = ""

    def __init__(self, j_data):
        date = (datetime.date.today() + datetime.timedelta(days = 1))
        self.date_string = date.strftime("%Y-%m-%d")

        for i in range(len(j_data)):
            print(float(j_data[i]["hinta"]))
            ## ne on järjestyksessä, ei ees chekata
            self.tunnit[i] = [float(j_data[i]["hinta"]) for k in range(4)] ## koska 15 min hinnoittelu incoming in 2025?
            print(self.tunnit[i])
    
    def __to_dict__(self):
        return {"date":self.date_string,  "tunnit":self.tunnit}


r = requests.get(f"{url}{query}")
if r.status_code != 200:
        quit()
data = r.json()
dayprices = DayPrices(data)
print(dayprices.tunnit)

x = json.dumps(dayprices.__to_dict__())
file_path = f"{FILE_PATH}dayprices.json"  
with open(file_path, 'w') as filetowrite:
    filetowrite.write(x)