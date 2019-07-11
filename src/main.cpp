/*
 *  Created: Wei Meng, 2018.07.26
 */
#include <esp_log.h>
#include <esp_sleep.h>
//#include <esp_deepsleep.h>
#include <driver/rtc_io.h>
//#include <WiFi.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <Wire.h>

#define PWR_LED GPIO_NUM_0  //red of Wrover kit
//#define PWR_LED GPIO_NUM_2    //green of Wrover kit
//#define PWR_LED GPIO_NUM_4    //blue of Wrover kit
#define ACCELEROMETER GPIO_NUM_35
#define ACCELEROMETER_LED GPIO_NUM_13
#define MODEM_RI GPIO_NUM_34

/* RTC_DATA_ATTR - placing in RTC memory which retains during deep sleep */
/* DRAM_ATTR - placing in DRAM, could be called by ISR, as required by:
https://esp-idf.readthedocs.io/en/v2.0/general-notes.html#iram-instruction-ram */

RTC_DATA_ATTR const String ver = "Ver 1.0 Rev 20190311";

RTC_DATA_ATTR volatile bool DEVICE_light_sleeping = false;
RTC_DATA_ATTR volatile bool DEVICE_deep_sleeping = false;
DRAM_ATTR const String DEVICE_name = "Punttoo";
DRAM_ATTR String DEVICE_imei = "000000000000000";
DRAM_ATTR String DEVICE_imei_dectect = "000000000000000";
DRAM_ATTR int DEVICE_motion_interval = 60;
DRAM_ATTR int DEVICE_motion_persistence = 120;
DRAM_ATTR int DEVICE_stationary_interval = 3600;
DRAM_ATTR int DEVICE_led = 1;
DRAM_ATTR int DEVICE_battery = 1;
DRAM_ATTR int DEVICE_powerlevel;
DRAM_ATTR int DEVICE_in_hand = 1;
DRAM_ATTR String DEVICE_ip = "127.0.0.1";
DRAM_ATTR int DEVICE_udp_port = 16604;

DRAM_ATTR String NETWORK_apn = "soniya01.com.attz";
DRAM_ATTR int NETWORK_mcc = 310;
DRAM_ATTR int NETWORK_mnc = 410;
DRAM_ATTR int NETWORK_csq;
DRAM_ATTR int NETWORK_creg;
DRAM_ATTR int NETWORK_cereg;
DRAM_ATTR String NETWORK_lac = "FFFF";
DRAM_ATTR String NETWORK_tac = "FFFF";
DRAM_ATTR String NETWORK_cellid = "FFFFFF";
DRAM_ATTR String NETWORK_act = "LTE-M";
DRAM_ATTR volatile bool NETWORK_CS;
DRAM_ATTR volatile bool NETWORK_PS;

DRAM_ATTR volatile bool SERVICE_activation;

DRAM_ATTR String HOST_domain = "unitedtracking.com";
DRAM_ATTR String HOST_ip = "64.71.155.103";
DRAM_ATTR int HOST_udp_port = 16604;
DRAM_ATTR int HOST_tcp_port =5000;

DRAM_ATTR String GPS_reading;
DRAM_ATTR String GPS_report;
DRAM_ATTR volatile bool GPS_fixed;

//DRAM_ATTR volatile bool REPORT_do;
//DRAM_ATTR volatile bool REPORT_succeeded;

RTC_DATA_ATTR unsigned volatile long bootCount = 0;
DRAM_ATTR unsigned volatile long loop_count = 0;
DRAM_ATTR unsigned volatile long sms_count = 0;
DRAM_ATTR unsigned volatile long motion_count = 0;
DRAM_ATTR unsigned volatile long timestamp_last_motion = 0;

DRAM_ATTR const String ATW = "AT&W\r\n";
DRAM_ATTR const String CGSN = "AT+CGSN\r\n";
DRAM_ATTR const String CMGF = "AT+CMGF=1\r\n";
DRAM_ATTR const String CMGR = "AT+CMGR=0\r\n";
DRAM_ATTR const String CMDG4 = "AT+CMGD=0,4\r\n";
DRAM_ATTR const String CPIN = "AT+CPIN?\r\n";
DRAM_ATTR const String CSQ = "AT+CSQ\r\n";
DRAM_ATTR const String CREG2 = "AT+CREG=2\r\n";
DRAM_ATTR const String CREG = "AT+CREG?\r\n";
DRAM_ATTR const String CEREG2 = "AT+CEREG=2\r\n";
DRAM_ATTR const String CEREG = "AT+CEREG?\r\n";
//DRAM_ATTR String QICSGP = "QICSGP=1,1,\"soniya01.com.attz\",\"\",\"\",3";
DRAM_ATTR String QICSGP = "AT+QICSGP=";
DRAM_ATTR String QIACT = "AT+QIACT=";
DRAM_ATTR const int QIACT_timing = 150;
DRAM_ATTR String QIDEACT = "AT+QIDEACT=";
DRAM_ATTR const int QIDEACT_timing = 40;
DRAM_ATTR String QIOPEN = "AT+QIOPEN=";
DRAM_ATTR const int QIOPEN_timing = 150;
DRAM_ATTR String QICLOSE = "AT+QICLOSE=";
DRAM_ATTR const int QICLOSE_timing = 10;
DRAM_ATTR String QISTATE = "AT+QISTATE=";
DRAM_ATTR String QISEND = "AT+QISEND=";

DRAM_ATTR const String mtk_EASY = "$PMTK869,1,1*35\r\n";
DRAM_ATTR const String mtk_AlwaysLocate = "$PMTK223,1,25,180000,60000*38\r\n";
DRAM_ATTR const String mtk_AIC = "$PMTK286,1*23\r\n";
DRAM_ATTR const String mtk_DGPS_WAAS = "$PMTK301,2*2E\r\n";
DRAM_ATTR const String mtk_SBAS = "$PMTK313,1*2E\r\n";
DRAM_ATTR const String mtk_RMC_only = "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n";
//DRAM_ATTR const String mtk_NMEA_interval = "$PMTK220,100*2F\r\n";
DRAM_ATTR const String mtk_NMEA_interval = "$PMTK220,1000*1F\r\n";
//DRAM_ATTR const String mtk_NMEA_interval = "$PMTK220,2000*1C\r\n";
//DRAM_ATTR const String mtk_NMEA_interval = "$PMTK220,3000*1D\r\n";
//DRAM_ATTR const String mtk_NMEA_interval = "$PMTK220,5000*1B\r\n";
//DRAM_ATTR const String mtk_NMEA_interval = "$PMTK220,10000*2F\r\n";
DRAM_ATTR const String mtk_stop = "$PMTK161,0*28\r\n";
DRAM_ATTR const String mtk_sleep = "$PMTK161,1*29\r\n";
DRAM_ATTR const String mtk_AlwaysLocate_default_cfg = "$PMTK223,1,30,180000,60000*3C\r\n";
DRAM_ATTR const String mtk_normal = "$PMTK225,0*2B\r\n";
DRAM_ATTR const String mtk_periodic_backup = "$PMTK225,1,3000,12000,18000,72000*16\r\n";
DRAM_ATTR const String mtk_periodic_standby = "$PMTK225,2,3000,12000,18000,72000*15\r\n";
DRAM_ATTR const String mtk_AlwaysLocate_standby = "$PMTK225,8*23\r\n";
DRAM_ATTR const String mtk_AlwaysLocate_backup = "$PMTK225,9*22\r\n";


/* ---------- basics --------- */


bool config_load () {

    if (SPIFFS.begin(true)) {
        File configFile = SPIFFS.open("/punttoo.json");
        if (configFile) {
            /*Serial.println("File Content:");
            while(configFile.available()) {
                Serial.write(configFile.read());
            }
            */
            size_t size = configFile.size();
            // allocate a buffer to store contents of this file
            std::unique_ptr<char[]> buf(new char[size]);
            // read the file content to buffer
            configFile.readBytes(buf.get(), size);
            // prepare another buffer for ArduinoJson to serialize(parse)
            //DynamicJsonBuffer jsonBuffer;  /* for ArduinoJson 5 */
            DynamicJsonDocument jdoc(1024);
            // parse from the buffer to a jason object    
            //JsonObject& json = jsonBuffer.parseObject(buf.get()); /* for ArduinoJson 5 */
            // parse from the buffer to an ArduinoJson document "jdoc", and detect parsing error (if any)
            auto error = deserializeJson(jdoc, buf.get());
            Serial.print(millis()); Serial.println(" Configurations loaded:"); delay(200);
            //json.prettyPrintTo(Serial);  /* for ArduinoJson 5 */
            serializeJsonPretty(jdoc,Serial);
            Serial.println(); delay(200);
            // fetch the values
            // Most of the time, you can rely on the implicit casts.
            // In other case, you can do json["time"].as<long>();
            /* for ArduinoJson 5
            if (json.success()) {
                DEVICE_imei = json["DEVICE_imei"].as<String>();
                DEVICE_motion_interval = json["DEVICE_motion_interval"];
                DEVICE_motion_persistence = json["DEVICE_motion_persistence"];
                DEVICE_stationary_interval = json["DEVICE_stationary_interval"];
                DEVICE_led = json["DEVICE_led"];
                DEVICE_battery = json["DEVICE_battery"];
                NETWORK_apn = json["NETWORK_apn"].as<String>();
                NETWORK_mcc = json["NETWORK_mcc"];
                NETWORK_mnc = json["NETWORK_mnc"];
                HOST_domain = json["HOST_domain"].as<String>();
                HOST_ip = json["HOST_ip"].as<String>();
                HOST_udp_port = json["HOST_udp_port"];
                HOST_tcp_port = json["HOST_tcp_port"];
                configFile.close();
                return true;
            } */
            if (!error) {
                DEVICE_imei = jdoc["DEVICE_imei"].as<String>();
                DEVICE_motion_interval = jdoc["DEVICE_motion_interval"];
                DEVICE_motion_persistence = jdoc["DEVICE_motion_persistence"];
                DEVICE_stationary_interval = jdoc["DEVICE_stationary_interval"];
                DEVICE_led = jdoc["DEVICE_led"];
                DEVICE_battery = jdoc["DEVICE_battery"];
                NETWORK_apn = jdoc["NETWORK_apn"].as<String>();
                NETWORK_mcc = jdoc["NETWORK_mcc"];
                NETWORK_mnc = jdoc["NETWORK_mnc"];
                HOST_domain = jdoc["HOST_domain"].as<String>();
                HOST_ip = jdoc["HOST_ip"].as<String>();
                HOST_udp_port = jdoc["HOST_udp_port"];
                HOST_tcp_port = jdoc["HOST_tcp_port"];
                configFile.close();
                return true;
            }
            else {
                configFile.close();
                Serial.print(millis()); Serial.println(" -> parsing json error!!!"); delay(200);
                return false;
            }
        }
        else {
            Serial.print(millis()); Serial.println(" -> Failed opening punttoo.json!!!"); delay(200);
            return false;
        }
    }
    else {
        Serial.print(millis()); Serial.println(" -> Error mounting SPIFFS!!!"); delay(200);
        return false;
    }
}


bool config_save () {

    if (SPIFFS.begin(true)) {
        File configFile = SPIFFS.open("/punttoo.json", FILE_WRITE);
        if (configFile) {
            /* for ArduinoJson 5
            DynamicJsonBuffer jsonBuffer;
            JsonObject& json = jsonBuffer.createObject(); */
            // Most of the time, you can rely on the implicit casts.
            // In other case, you can do json.set<long>("time", 1351824120);
            /* for ArduinoJson 5
            json["DEVICE_imei"] = DEVICE_imei;
            json["DEVICE_motion_interval"] = DEVICE_motion_interval;
            json["DEVICE_motion_persistence"] = DEVICE_motion_persistence;
            json["DEVICE_stationary_interval"] = DEVICE_stationary_interval;
            json["DEVICE_led"] =DEVICE_led;
            json["DEVICE_battery"] = DEVICE_battery;
            json["NETWORK_apn"] = NETWORK_apn;
            json["NETWORK_mcc"] = NETWORK_mcc;
            json["NETWORK_mnc"] = NETWORK_mnc;
            json["HOST_domain"] = HOST_domain;
            json["HOST_ip"] = HOST_ip;
            json["HOST_udp_port"] = HOST_udp_port;
            json["HOST_tcp_port"] = HOST_tcp_port;
            if (json.printTo(configFile) == 0) {
                configFile.close();
                Serial.print(millis()); Serial.println(" -> Failed writing to punttoo.json!!!"); delay(200);
                return false;
            }
            else {
                Serial.print(millis()); Serial.println(" Configurations saved:"); delay(200);
                json.prettyPrintTo(Serial); 
                Serial.println(); delay(200);
                configFile.close();
                return true;
            } */
            DynamicJsonDocument jdoc(1024);
            jdoc["DEVICE_imei"] = DEVICE_imei;
            jdoc["DEVICE_motion_interval"] = DEVICE_motion_interval;
            jdoc["DEVICE_motion_persistence"] = DEVICE_motion_persistence;
            jdoc["DEVICE_stationary_interval"] = DEVICE_stationary_interval;
            jdoc["DEVICE_led"] =DEVICE_led;
            jdoc["DEVICE_battery"] = DEVICE_battery;
            jdoc["NETWORK_apn"] = NETWORK_apn;
            jdoc["NETWORK_mcc"] = NETWORK_mcc;
            jdoc["NETWORK_mnc"] = NETWORK_mnc;
            jdoc["HOST_domain"] = HOST_domain;
            jdoc["HOST_ip"] = HOST_ip;
            jdoc["HOST_udp_port"] = HOST_udp_port;
            jdoc["HOST_tcp_port"] = HOST_tcp_port;
            if (serializeJson(jdoc,configFile) == 0) {
                Serial.print(millis()); Serial.println(" -> saving configuration failed!!!"); delay(200);
                configFile.close();
                return false;
            }
            else {
                serializeJsonPretty(jdoc,Serial);
                Serial.println(); delay(200);
                configFile.close();
                return true;
            }
        }
        else {
            Serial.print(millis()); Serial.println(" -> Failed opening punttoo.json!!!"); delay(200);
            return false;
        }
    }
    else {
        Serial.print(millis()); Serial.println(" -> Error mounting SPIFFS!!!"); delay(200);
        return false;
    }
}


bool config_delete () {
    if (SPIFFS.begin(true)) {
        if (SPIFFS.exists("/punttoo.json")) {
            if (SPIFFS.remove("/punttoo.json")) {
                Serial.print(millis()); Serial.println(" Config file deleted!"); delay(200);
                return true;
            }
            else {
                Serial.print(millis()); Serial.println(" -> Failed deleting config file!!!"); delay(200);
                return false;
            }
        }
        else {
            Serial.print(millis()); Serial.println(" -> Config file NOT EXISTS!!!"); delay(200);
            return false;
        }
    }
    else {
        Serial.print(millis()); Serial.println(" -> Error mounting SPIFFS!!!"); delay(200);
        return false;
    }
}


bool modem_IMEI_read () {
    Serial1.println(CGSN); delay(200);
    if (Serial1.available()) {
        String rsp = Serial1.readString();
        if (rsp.indexOf("CGSN") >= 0) {
            DEVICE_imei_dectect = rsp.substring((rsp.indexOf("CGSN")+7),(rsp.indexOf("OK")-4));
            Serial.print(millis()); Serial.println(" Detected modem IMEI: " + String (DEVICE_imei_dectect)); delay(200);
            return true;
        }
        else {
            Serial.print(millis()); Serial.println(" -> Reading IMEI ERROR!!!"); delay(200);
            return false;
        }
    }
    else {
        Serial.print(millis()); Serial.println(" -> Reading IMEI TIMEOUT!!!"); delay(200);
        return false;
    }
}


void modem_init () {
                
    Serial.print(millis()); Serial.println(" # Initializing LTE Modem..."); delay(200);
    
    // set modem echo on
    Serial1.println("ATE\r\n");
    delay(200);
    // clear the port by reading it
    if (Serial1.available()) {
        Serial1.readString();
    }
    
    // read modem IMEI
    modem_IMEI_read();
    
    // SMS in text format
    Serial1.println(CMGF);
    delay(300);
    if (Serial1.available()) {
        if (Serial1.find("OK")) {
            Serial.print(millis()); Serial.println(" AT+CMGF is OK"); delay(200);
        }
        else {
            Serial.print(millis()); Serial.println(" -> AT+CMGF NOT OK!!!"); delay(200);
        }
    }
    else {
        Serial.print(millis()); Serial.println(" -> AT+CMGF TIMEOUT!!!"); delay(200);
    }
    
    /* set creg to 2 ( +creg: <n>, <state>, [ <tower lac>, <cell id>, <Access technology>] )
        +CREG: 2,1,"FFFE","A20D311",8  */
    Serial1.println(CREG2);
    delay(300);
    if (Serial1.available()) {
        if (Serial1.find("OK")) {
            Serial.print(millis()); Serial.println(" AT+CREG=2 is OK"); delay(200);
        }
        else {
            Serial.print(millis()); Serial.println(" -> AT+CREG=2 NOT OK!!!"); delay(200);
        }
    }
    else {
        Serial.print(millis()); Serial.println(" -> AT+CREG=2 TIMEOUT!!!"); delay(200);
    }

    /* set cereg to 2 ( +cereg: <n>, <state>, [ <tac>, <cell id>, <Access technology>] )
        +CEREG: 2,1,"8B1B","A20D311",8  
        +CEREG: 4,1,"8B1B","1B","A20D311",8,,,"00000000","01100000"     */
    Serial1.println(CEREG2);
    delay(300);
    if (Serial1.available()) {
        if (Serial1.find("OK")) {
            Serial.print(millis()); Serial.println(" AT+CEREG=2 is OK"); delay(200);
        }
        else {
            Serial.print(millis()); Serial.println(" -> AT+CEREG=2 NOT OK!!!"); delay(200);
        }
    }
    else {
        Serial.print(millis()); Serial.println(" -> AT+CEREG=2 TIMEOUT!!!"); delay(200);
    }

    Serial1.println(ATW);
    delay(300);
    if (Serial1.available()) {
        if (Serial1.find("OK")) {
            Serial.print(millis()); Serial.println(" AT&W is OK"); delay(200);
        }
        else {
            Serial.print(millis()); Serial.println(" -> AT&W NOT OK!!!"); delay(200);
        }
    }
    else {
        Serial.print(millis()); Serial.println(" -> AT&W TIMEOUT!!!"); delay(200);
    }

}


bool modem_sim () {

    Serial1.println(CPIN);
    delay(500);
    if (Serial1.available()) {
        if (Serial1.find("READY")) {
            Serial.print(millis()); Serial.println(" SIM is OK."); delay(200);
            return true;
        }
        else {
            Serial.print(millis()); Serial.println(" -> SIM not detected!!!"); delay(200);
            return false;
        }
    }
    else {
        Serial.print(millis()); Serial.println(" -> Detecting SIM TIMEOUT!!!"); delay(200);
        return false;
    }
}


bool modem_CSQ () {
    
    Serial1.println(CSQ);
    delay(300);
    if (Serial1.available()) {
        String rsp = Serial1.readString();
        if (rsp.indexOf("+CSQ:") >= 0) {
            String csq = rsp.substring((rsp.indexOf("+CSQ:")+6), rsp.indexOf(",99"));
            //Serial.print(millis()); Serial.println(" CSQ is " + String(csq));
            //delay(200);
            NETWORK_csq = csq.toInt();
            Serial.print(millis()); Serial.print(" NETWORK_csq (int) is: ");Serial.println(NETWORK_csq); delay(200);
            return true;
        }
        else {
            Serial.print(millis()); Serial.println(" -> Reading CSQ ERROR!!!"); delay(200);
            return false;
        }
    }
    else {
        Serial.print(millis()); Serial.println(" -> Reading CSQ TIMEOUT!!!"); delay(200);
        return false;
    }

}


bool modem_CREG () {
    
    Serial1.println(CREG);
    delay(300);
    if (Serial1.available()) {
        String rsp = Serial1.readString();
        if (rsp.indexOf("+CREG:") >= 0) {
            String creg = rsp.substring(28,29);
            //Serial.print(millis()); Serial.println(" creg (string) is " + String(creg));
            //delay(200);
            NETWORK_creg = creg.toInt();
            Serial.print(millis()); Serial.print(" NETWORK_creg (int) is: ");Serial.println(NETWORK_creg); delay(200);
            if ((NETWORK_creg == 1) || (NETWORK_creg == 5)) {
                NETWORK_CS = true;
                SERVICE_activation = true;
                /* format: +CREG: <stat>[,<lac>,<cell-id>[,<Act>]]   Act: 0(GSM), 8(LTE-Cat.M1), 9(LTE-Cat.NB1)
                e.g.:   +CREG: 2,1,"FFFE","A20D311",8   */
                String sub = rsp.substring(30); 
                NETWORK_lac = sub.substring(1,(sub.indexOf(",")-1));
                //Serial.print(millis()); Serial.println(" lac (string) is " + String(NETWORK_lac));
                //delay(200);
                sub = sub.substring(sub.indexOf(",")+2);
                NETWORK_cellid = sub.substring(0,(sub.indexOf(",")-1));
                //Serial.print(millis()); Serial.println(" cellid (string) is " + String(NETWORK_cellid));
                //delay(200);
                sub = sub.substring(sub.indexOf(","));
                int act = (sub.substring(1,2)).toInt();
                switch (act) {
                    case 0:   NETWORK_act = "GSM"; break;
                    case 8:   NETWORK_act = "LTE-M1"; break;
                    case 9:   NETWORK_act = "LTE-NB1"; break;
                    default:  NETWORK_act = "unknown"; break;
                } 
                //Serial.print(millis()); Serial.println(" act (string) is " + String(NETWORK_act));
                //delay(200);
            }
            else {
                NETWORK_CS = false;
                if (NETWORK_creg == 3) {SERVICE_activation = false;}
            }
            return true;
        }
        else {
            Serial.print(millis()); Serial.println(" -> Reading CREG ERROR!!!"); delay(200);
            return false;
        }
    }
    else {
        Serial.print(millis()); Serial.println(" -> Reading CEREG TIMEOUT!!!"); delay(200);
        return false;
    }
    
}


bool modem_CEREG () {
    
    Serial1.println(CEREG);
    delay(300);
    if (Serial1.available()) {
        String rsp = Serial1.readString();
        if (rsp.indexOf("+CEREG:") >= 0) {
            String cereg = rsp.substring(22,23);
            //Serial.print(millis()); Serial.println(" cereg (string) is " + String(cereg));
            //delay(200);
            NETWORK_cereg = cereg.toInt();
            Serial.print(millis()); Serial.print(" NETWORK_cereg (int) is: ");Serial.println(NETWORK_cereg); delay(200);
            if ((NETWORK_cereg == 1) || (NETWORK_cereg == 5)) {
                NETWORK_PS = true;
                SERVICE_activation = true;
                /* format: +CEREG: <stat>[,<tac>,<cell-id>[,<Act>]]   Act: 0(GSM), 8(LTE-Cat.M1), 9(LTE-Cat.NB1)
                e.g.:   +CEREG: 2,1,"8B1B","A20D311",8
                        +CEREG: 4,1,"8B1B","1B","A20D311",8,,,"00000000","01100000"     */
                String sub = rsp.substring(24); 
                NETWORK_tac = sub.substring(1,(sub.indexOf(",")-1));
                //Serial.print(millis()); Serial.println(" tac (string) is " + String(NETWORK_tac));
                //delay(200);
                sub = sub.substring(sub.indexOf(",")+2);
                NETWORK_cellid = sub.substring(0,(sub.indexOf(",")-1));
                //Serial.print(millis()); Serial.println(" cellid (string) is " + String(NETWORK_cellid));
                //delay(200);
                sub = sub.substring(sub.indexOf(","));
                int act = (sub.substring(1,2)).toInt();
                switch (act) {
                    case 0:   NETWORK_act = "GSM"; break;
                    case 8:   NETWORK_act = "LTE-M1"; break;
                    case 9:   NETWORK_act = "LTE-NB1"; break;
                    default:    NETWORK_act = "unknown"; break;
                } 
                //Serial.print(millis()); Serial.println(" act (string) is " + String(NETWORK_act));
                //delay(200);
            }
            else {
                NETWORK_PS = false;
                if (NETWORK_cereg == 3) {SERVICE_activation = false;}
            }
            return true;
        }
        else {
            Serial.print(millis()); Serial.println(" -> Reading CEREG ERROR!!!"); delay(200);
            return false;
        }
    }
    else {
        Serial.print(millis()); Serial.println(" -> Reading CEREG TIMEOUT!!!"); delay(200);
        return false;
    }
    
}


bool modem_APN (int cid) {
    // e.g. AT+QICSGP=1,1,"soniya01.com.attz","","",3
    String get_apn = QICSGP + cid + "\r\n";
    Serial1.println(get_apn);
    delay(200);
    if (Serial1.available()) {
        String rsp = Serial1.readString();
        String keyword = cid + ",\"" + NETWORK_apn + "\"";
        if (rsp.indexOf(keyword) >= 0) {
            Serial.print(millis()); Serial.print(" APN set OK (already): "); Serial.println(NETWORK_apn); delay(200);
            return true;
        }
        else {
            String set_apn = QICSGP + cid + ",1," + "\"" + NETWORK_apn + "\"" + "," + "\"\",\"\",3\r\n";
            //Serial.print(millis()); Serial.print(" set_apn: "); Serial.println(set_apn);
            Serial1.println(set_apn);
            delay(300);
            if (Serial1.available()) {
                if (Serial1.find("OK")) {
                    Serial.print(millis()); Serial.print(" APN set OK: "); Serial.println(NETWORK_apn); delay(200);
                    return true;
                }
                else {
                    Serial.print(millis()); Serial.println(" -> Set APN ERROR!!!"); delay(200);
                    return false;
                }
            }
            else {
                Serial.print(millis()); Serial.println(" -> Set APN TIMEOUT!!!"); delay(200);
                return false;
            }
        }
    }
    else {
        Serial.print(millis()); Serial.println(" -> Query APN TIMEOUT!!!"); delay(200);
        return false;
    }
}


bool modem_PDP (int cid, int operation) {

    String query, cmd, the_cmd, _operation;
    int timing;

    if (operation == 0) { // deactivate PDP Context
        timing = 1000000LL*QIACT_timing;
        _operation = "deactivating PDP context";
        query = QIACT.substring(0,QIACT.indexOf("=")) + "?\r\n";
        Serial1.println(query);
        delay(300);
        if (Serial1.available()) {
            if (Serial1.find(cid+",1,1,")) {
                Serial.print(millis()); Serial.print(" found PDP activated on context: "); Serial.println(cid); delay(200);
                cmd = QIDEACT + cid + "\r\n";
                the_cmd = QIDEACT.substring(3,QIDEACT.indexOf("="));
            } 
            else {
                Serial.print(millis()); Serial.print(" OK, PDP already deactivated on context: "); Serial.println(cid); delay(200);
                return true;
            }
        }
        else {
            Serial.print(millis()); Serial.println(" -> PDP query TIMEOUT!!!"); delay(200);
            return false;
        }
    }
    if (operation == 1) { // activate PDP Context
        timing = 1000000LL*QIDEACT_timing;
        _operation = "activating PDP context";
        query = QIACT.substring(0,QIACT.indexOf("=")) + "?\r\n";
        Serial1.println(query);
        delay(300);
        if (Serial1.available()) {
            if (Serial1.find(cid+",1,1,")) {
                Serial.print(millis()); Serial.print(" OK, PDP already activated on context: "); Serial.println(cid); delay(200);
                return true;
            } 
            else {
                cmd = QIACT + cid + "\r\n";
                the_cmd = QIACT.substring(3,QIACT.indexOf('='));
            }
        }
        else {
            Serial.print(millis()); Serial.println(" -> PDP query TIMEOUT!!!"); delay(200);
            return false;
        }
    }

    Serial1.println(cmd);
    int begin_time = millis();
    delay(300);
    while ((millis()-begin_time)<=timing) {
        if (Serial1.available()) {
            if (Serial1.find("OK")) {
                Serial.print(millis()); Serial.print(" "); Serial.print(_operation); Serial.println(" is OK."); delay(200);
                return true;
            }
            else {
                Serial.print(millis()); Serial.print(" -> "); Serial.print(_operation); Serial.println(" ERROR!!!"); delay(200);
                return false;
            }
        }
        else {
            Serial.print(millis()); Serial1.print(" waiting for "); Serial.println(_operation); Serial.print(" PDP on context " + int(cid)); delay(200);
        }
    }
    Serial.print(millis()); Serial.print(" -> "); Serial.print(the_cmd); Serial.println(" TIMEOUT!!!"); delay(200);
    return false;
 
}


bool modem_UDP_socket (int context, int connection, int operation) {

    String cmd, the_cmd;
    
    if (operation == 0) {   // close socket

        cmd = QICLOSE + connection + "\r\n";
        the_cmd = QICLOSE.substring(3,QICLOSE.indexOf("="));
        Serial1.println(cmd);
        int begin_time = millis();
        delay(300);
        while ((millis()-begin_time)<=(1000000LL*QICLOSE_timing)) {
            if (Serial1.available()) {
                if (Serial1.find("OK")) {
                    Serial.print(millis()); Serial.print(" socket close OK,"); Serial.print(" context "); Serial.print(context); Serial.print(" connection "); Serial.println(connection); delay(200);
                    return true;
                }
                else {
                    Serial.print(millis()); Serial.print(" -> socket close ERROR!!!"); Serial.print(" context "); Serial.print(context); Serial.print(" connection "); Serial.println(connection); delay(200);
                    return false;
                }
            }
            else {
                Serial.print(millis()); Serial.println(" waiting for socket to close..."); delay(200);
            }
        }
    }

    if (operation == 1) {   // open socket
        cmd = QIOPEN+context+","+connection+","+"\""+"UDP SERVICE"+"\""+","+"\""+HOST_ip+"\""+","+HOST_udp_port+","+DEVICE_udp_port+","+0+"\r\n";
        the_cmd = QIOPEN.substring(3,QIOPEN.indexOf("="));
        //Serial.print(millis()); Serial.print(" open cmd: "); Serial.println(cmd);
        //delay(200);
        Serial1.println(cmd);
        delay(300);
        int begin_time = millis();
        while ((millis()-begin_time)<=(1000000LL*QIOPEN_timing)) {
            if (Serial1.available()) {
                if (Serial1.find(",0")) {
                    Serial.print(millis()); Serial.print(" socket open OK:"); Serial.print(" context "); Serial.print(context); Serial.print(" connection "); Serial.println(connection); delay(200);
                    return true;
                }
                else {
                    if (Serial1.find(",563")) {
                        Serial.print(millis()); Serial.print(" socket open already,"); Serial.print(" context "); Serial.print(context); Serial.print(" connection "); Serial.println(connection); delay(200);
                        return true;
                    }
                    else {
                        Serial.print(millis()); Serial.print(" -> socket open ERROR!!!"); Serial.print(" context "); Serial.print(context); Serial.print(" connection "); Serial.println(connection); delay(200);
                        return false;
                    }
                }
            }
            else {
                Serial.print(millis()); Serial.println(" waiting for socket to open..."); delay(200);
            }
        }
    }
    
    Serial.print(millis()); Serial.print(" -> "); Serial.print(the_cmd); Serial.println(" TIMEOUT!!!"); delay(200);
    return false;
}


bool modem_UDP_send (int connection, int length) {

    String cmd = QISEND + connection + "," + length + "," + "\"" + HOST_ip + "\"" + "," + HOST_udp_port + "\r\n";
    //Serial.print(millis()); Serial.print(" send cmd: "); Serial.println(cmd);
    //delay(200);
    Serial1.println(cmd);
    delay(300);
    if (Serial1.available()) {
        /*
        String rsp = Serial1.readString();
        Serial.println(rsp);
        delay(200);
        return true;
        */
        if (Serial1.find(">")) {
            Serial1.println(GPS_report);
            delay(1000);
            if (Serial1.available()) {
                if (Serial1.find("OK")) {
                    Serial.print(millis()); Serial.println(" UDP send OK!"); delay(200);
                    return true;
                }
                else {
                    Serial.print(millis()); Serial.println(" UDP send ERROR!!!"); delay(200);
                    return false;
                }
            }
            else {
                Serial.print(millis()); Serial.println(" UDP send TIMEOUT!!!"); delay(200);
                return false;
            }
        }
        else {
            Serial.print(millis()); Serial.println(" UDP send BUFFER ERROR!!!"); delay(200);
            return false;
        }
    }
    else {
        Serial.print(millis()); Serial.println(" UDP send QISEND TIMEOUT!!!"); delay(200);
        return false;
    }
}


bool device_power_read () {

    double total = 0, reading = 0, volt = 0;
    for (int i=0;i<=99;i++) {
        reading = analogRead(36); //VP: GPIO36, VN: GPIO39
        total = total + reading;
        reading = 0;
    }
    reading = total/100;
    //Serial.print("Average ADC raw reading: "); Serial.print(reading,0);
    volt = volt + (-0.000000000000016 * pow(reading,4) + 0.000000000118171 * pow(reading,3)- 0.000000301211691 * pow(reading,2)+ 0.001109019271794 * reading + 0.034143524634089);
    /*
    Calculating battery voltage:
    1. Because of resistor 1:1 divider, the read voltage is 1/2 of the battery. 
    2. Read voltage maybe +/- 0.01-0.1v than actual (actually measured by multimeter).
    3. Bipolar junction transistor (MUN5211DW1T1G) voltage drop is 0.26v must be in the calculation.
    */
    //volt = 2*volt + (0.26);
    volt = 2*volt;
    //Serial.print(millis()); Serial.print(" Battery voltage: "); Serial.println(volt,3); delay(200);

    DEVICE_powerlevel = 50;
    return true;
}


void GPS_init () {
                
    Serial.print(millis()); Serial.println(" # Initializing MTK GPS...");
    // RMC output only
    Serial2.println(mtk_RMC_only);
    if (Serial2.available()) {
        if (Serial2.find("$PMTK001,314,3*36")) {
            Serial.print(millis()); Serial.println(" mtk_RMC_only is OK."); delay(200);
        }
        else {
            Serial.print(millis()); Serial.println(" -> mtk_RMC_only NOT OK!!!"); delay(200);
        }
    }
    else {
        Serial.print(millis()); Serial.println(" -> mtk_RMC_only TIMEOUT!!!"); delay(200);
    }
    // set NMEA output interval
    Serial2.println(mtk_NMEA_interval);
    delay(200);
    if (Serial2.available()) {
        if (Serial2.find("$PMTK001,220,3*30")) {
            Serial.print(millis()); Serial.println(" mtk_NMEA_interval is OK."); delay(200);
        }
        else {
            Serial.print(millis()); Serial.println(" -> mtk_NMEA_interval NOT OK!!!"); delay(200);
        }
    }
    else {
        Serial.print(millis()); Serial.println(" -> mtk_NMEA_interval TIMEOUT!!!"); delay(200);
    }

}


bool GPS_change_mode (int mode) {

    Serial.print(millis()); Serial.print(" Changing MTK GPS mode to: "); delay(200);
    
    switch (mode) {
        
        case 0:
            Serial.print("Normal"); delay(200);
            Serial2.println(mtk_normal);
            break;
        case 1:
            Serial.print("Periodic Backup"); delay(200);
            Serial2.println(mtk_periodic_backup);
            break;
        case 2:
            Serial.print("Periodic Standby"); delay(200);
            Serial2.println(mtk_periodic_standby);
            break;
        case 8:
            Serial.print("AlwaysLocate Standby"); delay(200);
            Serial2.println(mtk_AlwaysLocate_standby);
            break;
        case 9:
            Serial.print("AlwaysLocate Backup"); delay(200);
            Serial2.println(mtk_AlwaysLocate_backup);
            break;
        case 10:
            Serial.print("Stop"); delay(200);
            Serial2.println(mtk_stop);
            break;
        case 11:
            Serial.print("Sleep"); delay(200);
            Serial2.println(mtk_sleep);
            break;
        default: 
            Serial.println("mode not specified!"); delay(200);
            break;
    }

    delay(200);

    if (Serial2.available()) {
        String rsp = Serial2.readString();
        if (rsp.indexOf("PMTK001") && rsp.indexOf("3*")) {
            Serial.println(" ...successful"); delay(200);
            return true;
        }
        else {
            Serial.println(" -> NOT OK!!!"); delay(200);
            return false;
        }
    }
    else {
        Serial.println(" -> TIMEOUT!!!"); delay(200);
        return false;
    }

}


bool GPS_read (int wait_time) {

    delay(1000LL*wait_time);

    if (Serial2.available()) {
        String reading = Serial2.readString();
        if (reading.indexOf("GPRMC") >= 0) {
            GPS_reading = reading.substring(reading.lastIndexOf("$GPRMC"),reading.lastIndexOf("\r"));
            //Serial.print(millis()); Serial.print(" read_GPS: "); Serial.println(GPS_reading);
            return true;
        }
        else {
            GPS_reading = "$GPRMC,000000.000,V,,,,,0.00,0.00,000000,,,0*00";
            Serial.print(millis()); Serial.println(" -> Reading Serial2 - THERE IS GPS DATA, BUT RMC SENTENCE NOT FOUND!"); delay(200);
            return true;
        }
    }
    else {
        GPS_reading = "$GPRMC,000000.000,V,,,,,0.00,0.00,000000,,,0*00";
        Serial.print(millis()); Serial.println(" -> Reading Serial2 - GPS DATA UNAVAILABLE!!!"); delay(200);
        return true;
    }
}


bool Punttoo_realtime_report (int power_state) {

    if (modem_sim()) {
        if (modem_CREG() && modem_CEREG()) {
            if (SERVICE_activation) {
                if (modem_APN(1)) { // cid 1
                    if (modem_PDP(1,1)) { // cid 1, activate
                        if (modem_UDP_socket(1,2,1)) {    // context 1, connection 2, open
                            if (modem_CSQ() && device_power_read() && GPS_read(1)) {
                                // assemble the report sentence
                                GPS_report = DEVICE_name+","+DEVICE_imei
                                +",$C,"+NETWORK_csq+","+NETWORK_act+","+NETWORK_mcc+","+NETWORK_mnc+","+NETWORK_tac+","+NETWORK_cellid
                                +",$P,"+power_state+","+DEVICE_powerlevel
                                +","+GPS_reading;
                                int len = GPS_report.length();
                                //Serial.print(millis()); Serial.println(GPS_report);
                                //delay(300);
                                if (modem_UDP_send(2,len)) {    // connection 2
                                    modem_UDP_socket(1,2,0);  // context 1, connection 2, close
                                    modem_PDP(1,0);   // context 1, deactivate
                                    return true;
                                }
                                else {
                                    return false;
                                }
                            }
                            else {
                                modem_UDP_socket(1,2,0);  // context 1, connection 2, close
                                return false;
                            }
                        }
                        else {
                            modem_PDP(1,0);
                            return false;
                        }
                    }
                    else {
                        return false;
                    }
                }
                else {
                    return false;
                }
            }
            else {
                Serial.print(millis()); Serial.println(" -> NO SERVICE!!!"); delay(200);
                return false;
            }
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
}


void goto_light_sleep () {
    //GPS_change_mode(9);    // put GPS to AlwaysLocate Backup
    esp_sleep_enable_ext0_wakeup(ACCELEROMETER,1);    //RTC GPIOs only: 0,2,4,12-15,25-27,32-39.
    //esp_sleep_enable_ext0_wakeup(MODEM_RI,0);    //RTC GPIOs only: 0,2,4,12-15,25-27,32-39.
    esp_sleep_enable_timer_wakeup(1000000LL*DEVICE_motion_interval);
    DEVICE_light_sleeping = true;
    DEVICE_deep_sleeping = false;
    digitalWrite(PWR_LED,LOW);
    esp_light_sleep_start();
}

void goto_deep_sleep () {
    GPS_change_mode(11);    // put GPS to sleep mode
    rtc_gpio_pulldown_en(ACCELEROMETER);
    //rtc_gpio_pullup_en(MODEM_RI);
    esp_sleep_enable_ext0_wakeup(ACCELEROMETER,1);    //RTC GPIOs only: 0,2,4,12-15,25-27,32-39.
    //esp_sleep_enable_ext0_wakeup(MODEM_RI,0);    //RTC GPIOs only: 0,2,4,12-15,25-27,32-39.
    esp_sleep_enable_timer_wakeup(1000000LL*DEVICE_stationary_interval);
    DEVICE_light_sleeping = false;
    DEVICE_deep_sleeping = true;
    digitalWrite(PWR_LED,LOW);
    esp_deep_sleep_start();
}


/* ----------- wifi ----------*/
/*
int wifi_scan() {

    Serial.println("WiFi scan start");
    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
            delay(10);
        }
    }
    Serial.println("");
    // Wait a bit before scanning again, add a loop here.
    delay(5000);

    return n;
}
*/

/* ---------------------------------- I.S.R. ---------------------------------- */

portMUX_TYPE demoMux = portMUX_INITIALIZER_UNLOCKED;
void IRAM_ATTR isr_demo() {
    portENTER_CRITICAL_ISR(&demoMux);
    portEXIT_CRITICAL_ISR(&demoMux);
}

portMUX_TYPE smsMux = portMUX_INITIALIZER_UNLOCKED;
void IRAM_ATTR isr_SMS() {
    portENTER_CRITICAL_ISR(&smsMux);

    ++sms_count;

    detachInterrupt(MODEM_RI);

    Serial.print(millis()); Serial.print(" ===> SMS: "); Serial.println(sms_count); delay(200);
    /*if (sms_count <= 1) {
        Serial.println(" fake, ignored."); delay(200);
    }*/
    Serial1.println("AT+CMGR=0\r\n"); delay(200);
    if (Serial1.available()) {
        String sms = Serial1.readString();
        Serial.println(millis()); Serial.println(sms); delay(200);
    }

    //attachInterrupt(digitalPinToInterrupt(MODEM_RI),isr_SMS,FALLING);

    portEXIT_CRITICAL_ISR(&smsMux);
}


/* --------------------------------- SETUP ------------------------------------------- */

void setup()
{
    ++bootCount;

    if (!DEVICE_light_sleeping) {   //either a first-time power-on, or a wake-up from deep-sleep
        /* 
        restore RTC-io pads to digital-GPIO 
        only needed for RTC GPIOs: 0,2,4,12-15,25-27,32-39.
        */ 
        rtc_gpio_deinit(GPIO_NUM_14);
        rtc_gpio_deinit(GPIO_NUM_27);
        rtc_gpio_deinit(MODEM_RI);
        rtc_gpio_deinit(ACCELEROMETER);
        rtc_gpio_deinit(ACCELEROMETER_LED);
        //pinMode(ACCELEROMETER,INPUT_PULLUP);  // Comus AU2401-1
        pinMode(ACCELEROMETER,INPUT_PULLDOWN);    // ST LSM6DSL interrupt pin
        pinMode(ACCELEROMETER_LED,OUTPUT);
        pinMode(MODEM_RI,INPUT_PULLUP);
        /* determine power LED according to LED preference setting */ 
        pinMode(PWR_LED,OUTPUT);
        if (DEVICE_led == 1) {
            digitalWrite(PWR_LED,HIGH);
        }
        else {
            digitalWrite(PWR_LED,LOW);
        }

        /* init uart ports */
        Serial.begin(115200,SERIAL_8N1,3,1);        //rx:io3, tx:io1, PC
        while (!Serial) continue; //wait for Serial to initialize
        Serial1.begin(115200,SERIAL_8N1,25,26);     //rx:io14, tx:io27, to LTE modem
        while (!Serial1) continue; //wait for Serial1 to initialize
        Serial2.begin(9600,SERIAL_8N1,19,18);       //rx:io19, tx:io18, to MTK GPS
        while (!Serial2) continue; //wait for Serial2 to initialize

        // Set WiFi to station mode and disconnect from an AP if it was previously connected
        /*
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        delay(100);
        */

        Wire.begin(); //join the i2C as master
        /* beging configuring embedded-significant-motion for LSM6DSL */
        Wire.beginTransmission(0x6A); // select slave device @ 0x6A
        Wire.write(0x01); // FUNC_CFG_ACCESS register @ 0x01
        Wire.write(0x80); // Enable access to embedded functions registers (bank A)
        Wire.endTransmission(); // transmit

        Wire.beginTransmission(0x6A); // select slave device @ 0x6A
        Wire.write(0x13); // SM_THS register @ 0x13
        Wire.write(0x01); // Set significant motion threshold
        Wire.endTransmission(); // transmit

        Wire.beginTransmission(0x6A); // select slave device @ 0x6A
        Wire.write(0x01); // FUNC_CFG_ACCESS register @ 0x1
        Wire.write(0x00); // Disable access to embedded functions registers (bank A)
        Wire.endTransmission(); // transmit

        Wire.beginTransmission(0x6A); // select slave device @ 0x6A
        Wire.write(0x10); // point to the CTRL1_XL register @ 0x10
        Wire.write(0x20); // Turn on the accelerometer, ODR_XL = 26 Hz, FS_XL = ±2 g
        Wire.endTransmission(); // transmit

        Wire.beginTransmission(0x6A); // select slave device @ 0x6A
        Wire.write(0x19); // CTRL10_C register @ 0x19
        Wire.write(0x05); // Enable embedded functions, Enable significant motion detection
        Wire.endTransmission(); // transmit

        Wire.beginTransmission(0x6A); // select slave device @ 0x6A
        Wire.write(0x0D); // INT1_CTRL register @ 0x0D
        Wire.write(0x40); // Significant motion interrupt enable on INT1 pad
        Wire.endTransmission(); // transmit
        /* end configuring embedded-significant-motion for LSMDSL */

        /* to save new defaults
        config_save();
        */
        
        /*
        load in punttoo.json from flash:
        * if loading failed, create all using defaults.
        */
        if (!config_load()) {config_save();}

        if (!DEVICE_deep_sleeping) {    // if this is a first-time power-on
            modem_init();
            if (!(DEVICE_imei.equals(DEVICE_imei_dectect))) {
                DEVICE_imei = DEVICE_imei_dectect;
                config_save();
            }
            GPS_init();
            Serial.println("\r\n=========== JinGuBang as Punttoo LTE  " + String (ver) + " IMEI: " + String(DEVICE_imei) + " ============\r\n" );
            /*
            send P1 report here
            */
            Punttoo_realtime_report(1);
            /*
            go to light sleep
            */
            goto_light_sleep();
        }

    }

    
}


/* ----------------------------------- LOOP ------------------------------------*/


void loop()
{
    ++loop_count; 
    
    Serial.print(millis()); Serial.print(" [ Boot " + String(bootCount) + ", Loop " + String(loop_count) + " ] ");
    if (DEVICE_light_sleeping) {Serial.print("light-sleep woke up by: ");} delay(200);
    if (DEVICE_deep_sleeping) {Serial.print("deep-sleep woke up by: ");} delay(200);
    esp_sleep_wakeup_cause_t wakeup_cause = esp_sleep_get_wakeup_cause();
    switch (wakeup_cause) {

        case 1 :    Serial.println("RTC_IO"); delay(200);

                    if (DEVICE_light_sleeping) {
                        DEVICE_light_sleeping = false;
                        timestamp_last_motion = millis();
                    }

                    if (DEVICE_deep_sleeping) {
                        DEVICE_deep_sleeping = false;
                        timestamp_last_motion = millis();
                        /* 
                        send P7 report here
                        */
                        GPS_change_mode(0);
                        Punttoo_realtime_report(7);
                    }

                    goto_light_sleep();

                    break;

        case 2 :    Serial.println("RTC_CNTL (accelerometer? sms?)"); delay(200);

                    if (DEVICE_light_sleeping) {
                        DEVICE_light_sleeping = false;
                        timestamp_last_motion = millis();
                    }
                    
                    if (DEVICE_deep_sleeping) {
                        DEVICE_deep_sleeping = false;
                        timestamp_last_motion = millis();
                        /* 
                        send P7 report here
                        */
                        GPS_change_mode(0);
                        Punttoo_realtime_report(7);
                    }
                    
                    goto_light_sleep();

                    break;

        case 3 :    Serial.println("timer"); delay(200);
                    
                    if (DEVICE_light_sleeping) {
                        unsigned long timestamp_now = millis();
                        if ((timestamp_now - timestamp_last_motion) < 1000LL*DEVICE_motion_persistence) {
                            /* 
                            send P2 report here
                            */
                            Punttoo_realtime_report(2);
                            goto_light_sleep();
                        }
                        else {
                            /* 
                            send P6 report here
                            */
                            Punttoo_realtime_report(6);
                            goto_deep_sleep();
                        }
                    }
                    
                    if (DEVICE_deep_sleeping) {
                        DEVICE_deep_sleeping = true;
                        // turn LED on, if not muted.
                        if (DEVICE_led == 1) {
                            digitalWrite(PWR_LED,HIGH);
                        }
                        /*
                        send P8 report here
                        */
                        Punttoo_realtime_report(8);
                        goto_deep_sleep();
                    }

                    break;

        case 4 :    Serial.println("touchpad (timer?)"); delay(200);

                    if (DEVICE_light_sleeping) {
                        unsigned long timestamp_now = millis();
                        if ((timestamp_now - timestamp_last_motion) < 1000LL*DEVICE_motion_persistence) {
                            /* 
                            send P2 report here
                            */
                            Punttoo_realtime_report(2);
                            goto_light_sleep();
                        }
                        else {
                            /* 
                            send P6 report here
                            */
                            Punttoo_realtime_report(6);
                            goto_deep_sleep();
                        }
                    }
                    
                    if (DEVICE_deep_sleeping) {
                        DEVICE_deep_sleeping = true;
                        // turn LED on, if not muted.
                        if (DEVICE_led == 1) {
                            digitalWrite(PWR_LED,HIGH);
                        }
                        /*
                        send P8 report here
                        */
                        Punttoo_realtime_report(8);
                        goto_deep_sleep();
                    }

                    break;

        case 5 :    Serial.println("ULP"); delay(200);

                    if (DEVICE_light_sleeping) {goto_light_sleep();}
                    if (DEVICE_deep_sleeping) {goto_deep_sleep();}
                    goto_light_sleep();
                    
                    break;

        default:    Serial.println("undefined"); delay(200);

                    if (DEVICE_light_sleeping) {goto_light_sleep();}
                    if (DEVICE_deep_sleeping) {goto_deep_sleep();}
                    goto_light_sleep();

                    break;
    }
    
}