#!/usr/bin/python3
import socket
import time
import datetime
import MySQLdb

# datalogger for hot water tank temperatures
#
# writes tank sensor temperature and external temperature
# to local MySQL database every 15 minutes

# read tank sensors
def read_tank_data():
    sfd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sfd.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listen_address = ('0.0.0.0',52004)
    sfd.bind(listen_address)
    packet, address = sfd.recvfrom(256)
    tank_data = packet.split()
    utc_timestamp = int(tank_data[0])
    tank_lower = float(tank_data[1])
    tank_upper = float(tank_data[2])
    return utc_timestamp, tank_lower, tank_upper
    
# read latest weather data from rpi4 broadcast packet
def read_weather_data():
    data_available = False

    sfd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sfd.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listen_address = ('0.0.0.0',52003)
    sfd.bind(listen_address)
    packet, address = sfd.recvfrom(256)
    weather_data = packet.split()
    temperature = float(weather_data[1])
    return temperature

# update readings to the database
def update_db(timestamp,tank_lower,tank_upper,outside_temp):
    try:
        db = MySQLdb.connect(host="localhost",
                             user="hotwater",
                             passwd="mysql-password",
                             db="hotwater")
        cur = db.cursor()
        mysql_datetime = timestamp.strftime("%Y-%m-%d %H:%M:%S")
        cur.execute("INSERT INTO hotwater(Timestamp,TankUpper,TankLower,Outside) VALUES('{0}',{1},{2},{3})".format(
            mysql_datetime,tank_upper,tank_lower,outside_temp))
        cur.close()
        db.commit()
        db.close()
    except MySQLdb.Error as e:
        print("MySQL Error: %s" % str(e))

while True:
    tank_utc, tank_lower, tank_upper = read_tank_data()
    outside_temp = read_weather_data()

    local_timestamp = datetime.datetime.fromtimestamp(tank_utc)
    update_db(local_timestamp,tank_lower,tank_upper,outside_temp)
    time.sleep(15*60)
