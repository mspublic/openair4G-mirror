import json, urllib2, datetime, threading, time

DB_URL = "https://fswsdb.com/wsd/index.php"
SDR_CONTROLLER = "lte-softmodem"
#KCL lat = 51.5119 lon = 0.1161
#Lat=60.45146165591333,lon=22.253150939941406):

lat = 51.5119
lon = 0.1161
h = 10.2

#time between geolocation database checks in minutes
database_recheck_period = 0.01

class GpsPoller(threading.Thread):
   
   def __init__(self):
       threading.Thread.__init__(self)
       self.session = gps(mode=WATCH_ENABLE)
       self.current_value = None

   def get_current_value(self):
       return self.current_value

   def run(self):
       try:
            while True:
                self.current_value = session.next()
                time.sleep(0.2) # tune this, you might not get values that quickly
       except StopIteration:
            pass

def periodic_ofcom():
    response = urllib2.urlopen('https://tvws-databases.ofcom.org.uk/weblist.xml')
    weblist = response.read()
    print (weblist)
    if not DB_URL in weblist:
        print("problem getting TVWSDB; kill lte-softmodem")
        #sys.exit()
    for item in weblist.split("\n"):
        if "refresh_rate" in item:
            refresh = item.strip()
    refresh = refresh[14:]
    refresh = refresh[:-15]
    refresh = float(refresh)
    print (refresh)
    print("")
    response.close()
    threading.Timer(float(refresh)*60, periodic_ofcom).start()

def query_db(lat="",lon="",return_all= False):
    with open("settings.json") as json_file:
        data = json.load(json_file)
        if lat != "":
            data['params']['location']['point']['center']['latitude'] = lat
        if lon != "":
            data['params']['location']['point']['center']['longitude'] = lon
        if h != "":
            data['params']['antenna']['height'] = h
        data_str = json.dumps(data)
        result = json.load(urllib2.urlopen(DB_URL,data_str))
        res0 = result['result']['spectrumSpecs']['spectrumSchedules'][0]
        spectrum_profiles = res0['spectra'][0]['profiles'][0]
        print(spectrum_profiles)
        print("")
        if return_all:
            return spectrum_profiles
        else:
            return max(spectrum_profiles,key=lambda profile: float(profile['dbm']))

def set_sdr_freq(lat,lon):
    try:
        modem_params = query_db(lat,lon)
        print "exec ./lte-softmodem -P %s -C %s" % (modem_params['dbm'],modem_params['hz'])
    except:
        print "problem querying database; kill lte-softmodem"
        #sys.exit()

def periodic_database():
    set_sdr_freq(float(lat), float(lon))
    threading.Timer(float(database_recheck_period)*60, periodic_database).start()

def periodic_latlon(gpsp):
    lat = gpsd.fix.latitude
    lon = gpsd.fix.latitude
    #h = gpsd.fix.altitude
    threading.Timer(1, periodic_latlon).start()

if __name__ == "__main__":
    import sys

#    gpsp = GpsPoller()
#    gpsp.start()
#    # gpsp now polls every .2 seconds for new data, storing it in self.current_value
#    while 1:
#        # In the main thread, every 5 seconds print the current value
#        time.sleep(5)
#        print gpsp.get_current_value() 

#    periodic_latlon(gpsp)
    periodic_ofcom()
    periodic_database()

