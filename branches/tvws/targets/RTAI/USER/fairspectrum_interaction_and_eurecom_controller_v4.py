import json, urllib2, datetime, threading, time, os, sys

#Unckech the following line if running on Linux machine with GPSD GPS
#from gps import *


#from time import *

OFCOM_REQUEST = "https://tvws-databases.ofcom.org.uk/weblist.xml?UniqueID=345345"
DB_URL = "https://fswsdb.com/wsd/index.php"
SDR_CONTROLLER = "lte-softmodem"
#KCL lat = 51.5119 lon = 0.1161
#Lat=60.45146165591333,lon=22.253150939941406):

#Denmark Hill IoP 51.470568,-0.088789
#h = 37.7

#Guys Tower 51.503434,-0.087201
#h = 65

#Queen Mary Eng roof 51.522946,-0.041613
#h = 15

#Strand: 51.511911,-0.11629
#h (max) = 31m

#Class 1 to 5 fake IDs
#fb4ab169-f8bf-49d0-9852-e69e044d1111
#fb4ab169-f8bf-49d0-9852-e69e044d2222
#fb4ab169-f8bf-49d0-9852-e69e044d3333
#fb4ab169-f8bf-49d0-9852-e69e044d4444
#fb4ab169-f8bf-49d0-9852-e69e044d5555

lat = 51.511911
lon = -0.11629
h = 6

#All the following are in dBs
#amp_gain = 60 not used currently
cable_loss = 1
antenna_gain = 13
#Following is power out at gain setting 21 in dBm
power_at_21 = 22
#Following is the actual power output in EIRP
actual_EIRP = 0

lat_gps = ""
lon_gps = ""

serial = "fb4ab169-f8bf-49d0-9852-e69e044d3333"

#time between geolocation database checks in minutes
database_recheck_period = 15

gpsd = None #setting the global variable
command = "kill lte-softmodem"
 
class GpsPoller(threading.Thread):
  def __init__(self):
    threading.Thread.__init__(self)
    global gpsd #bring it in scope
    gpsd = gps(mode=WATCH_ENABLE) #starting the stream of info
    self.current_value = None
    self.running = True #setting the thread running to true

  def run(self):
    global gpsd
    while gpsp.running:
      gpsd.next() #this will continue to loop and grab EACH set of gpsd info to clear the buffer

def periodic_ofcom():
    response = urllib2.urlopen(OFCOM_REQUEST)
    weblist = response.read()
    print (weblist)
    if not "https://www.fswsdb.com/wsd/index.php" in weblist:
	    os.system("kill lte-softmodem")
	    sys.exit()
    for item in weblist.split("\n"):
        if "refresh_rate" in item:
            refresh = item.strip()
    refresh = refresh[14:]
    refresh = refresh[:-15]
    refresh = float(refresh)
    #print (refresh)
    #print("")
    response.close()
    threading.Timer(float(refresh), periodic_ofcom).start()

def query_db(lat="",lon="",h="",serial="",return_all= False):
    f = open('settings.json', 'r')
    data = json.loads(f.read())
    if lat != "":
        data['params']['location']['point']['center']['latitude'] = lat
    if lon != "":
        data['params']['location']['point']['center']['longitude'] = lon
    if h != "":
        data['params']['antenna']['height'] = h
    data['params']['deviceDesc']['serialNumber'] = serial
    data_str = json.dumps(data)
    #print(data_str)
    result = json.load(urllib2.urlopen(DB_URL,data_str))
    print("")
    print(result)
    print("")
    #print(data)
    #print("")
    res0 = result['result']['spectrumSpecs']['spectrumSchedules'][0]
    spectrum_profiles = res0['spectra'][0]['profiles'][0]
    #print(spectrum_profiles)
    #print("")
    if return_all:
        return spectrum_profiles
    else:
        return max(spectrum_profiles,key=lambda profile: float(profile['dbm']))

def notify(lat="",lon="",serial="",freq="",power=""):
    g = open('notify.json', 'r')
    notifydata = json.loads(g.read())
    if lat != "":
        notifydata['params']['location']['point']['center']['latitude'] = lat
    if lon != "":
        notifydata['params']['location']['point']['center']['longitude'] = lon
    #if h != "":
        #notifydata['params']['antenna']['height'] = h
    print(freq)
    print(power)
    if freq != "":
        notifydata['params']['spectra'][0]['profiles'][0][0]['hz'] = freq
    if power != "":
        notifydata['params']['spectra'][0]['profiles'][0][0]['dbm'] = power
    if serial != "":
        notifydata['params']['deviceDesc']['serialNumber'] = serial
    data_str_notify = json.dumps(notifydata)
    print(data_str_notify)
    #print(data_str_notify)
    #print(data_str_notify)
    result_notify = json.load(urllib2.urlopen(DB_URL,data_str_notify))
    print("")
    #print(result_notify)
    print("")
    return result_notify

def set_sdr_freq(lat,lon,h,serial):
    try:
        modem_params = query_db(lat,lon,h,serial)
        gain = modem_params['dbm'] + 19.03 - power_at_21 - antenna_gain + cable_loss + 21
        if gain > 21:
            gain = 21
        actual_EIRP = power_at_21 + antenna_gain - cable_loss + gain - 21
        print(gain)
        print(actual_EIRP)
        response = notify(lat,lon,serial,freq=modem_params['hz'],power=actual_EIRP)
        print(response)
        #This is version 1 which executes on the ExpressMIMO2 at the freqeuncy directly. Comment if not used
        #command = './lte-softmodem -g ' + str(gain) + ' -C ' + str(modem_params['hz']) + ' -d -M'
        #This is version 2 which uses the R&S as LO. Comment if not used
        command = './lte-softmodem -g ' + str(gain) + ' -C 1551800000'
        LOFreq = 1551800000 + modem_params['hz']
        s = 'EthernetRawCommand 192.168.12.202 "SOURce:FREQuency:CW ' + str(LOFreq) + '"';
        print(s)
        print(command)
        #Uncomment following if the R&S LO (version 2) is used
        os.system(s)
        os.system(command)
    except:
        os.system("kill lte-softmodem")
        sys.exit()

def periodic_database():
    set_sdr_freq(float(lat), float(lon), float(h), serial)
    threading.Timer(float(database_recheck_period)*60, periodic_database).start()

#Unckech the following line if running on Linux machine with GPSD GPS
#gpsp = GpsPoller()
#Unckech the following line if running on Linux machine with GPSD GPS
#gpsp.start()

def periodic_gps(lat_gps="",lon_gps=""): 
  print
  print ' GPS reading'
  print '----------------------------------------'
  print 'latitude    ' , gpsd.fix.latitude
  lat_gps = gpsd.fix.latitude
  print 'longitude   ' , gpsd.fix.longitude
  lon_gps = gpsd.fix.longitude
  print 'time utc    ' , gpsd.utc,' + ', gpsd.fix.time
  print 'altitude (m)' , gpsd.fix.altitude
  #h_gps = gpsd.fix.altitude
  print 'eps         ' , gpsd.fix.eps
  print 'epx         ' , gpsd.fix.epx
  print 'epv         ' , gpsd.fix.epv
  print 'ept         ' , gpsd.fix.ept
  print 'speed (m/s) ' , gpsd.fix.speed
  print 'climb       ' , gpsd.fix.climb
  print 'track       ' , gpsd.fix.track
  print 'mode        ' , gpsd.fix.mode
  print
  print 'sats        ' , gpsd.satellites 
  #time.sleep(5) #set to whatever
  threading.Timer(1, periodic_gps).start()
  return lat_gps, lon_gps

#Unckech the following line if running on Linux machine with GPSD GPS
#periodic_gps()
periodic_ofcom()
periodic_database()

