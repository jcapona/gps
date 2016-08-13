#include <gps.h>
#include <serial.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>

#define MAX_LONGITUDE 180
#define MAX_LATITUDE 90
#define KNOTS_TO_KM 1.852

typedef struct {
    struct tm time;
    float longitude;
    float latitude;
    int n_satellites;
    float hdop;
    int altitude;
    char altitude_unit;
    float geoidHeight;
    char geoidHeight_unit;
} nmea_gpgga;

typedef struct {
    struct tm time;
    float longitude;
    float latitude;
    float speed;
    char magneticVar;
} nmea_gprmc;

typedef struct {
    int n_satellites;
    int prn;
    int elevation;
    int azimuth;
    int snr;
} nmea_gpgsv;

typedef struct {
    char mode;
    float fix3d;
    float pdop;
    float hdop;
    float vdop;    
} nmea_gpgsa;

class gps::impl {
    public:
        impl(std::string dev, int baud);
        ~impl();

       int read();
       int readFromFile(const char* file);
       float getLatitude();
       float getLongitude();
       float getSpeed();
       float getAltitude();
       std::string getTime();

    private:
       serial *serialPort;
       float m_latitude;
       float m_longitude;
       float m_speed;
       float m_altitude;
       struct tm m_time;
              
       nmea_gprmc m_gprmc;
       nmea_gpgga m_gpgga;
       nmea_gpgsa m_gpgsa;
       nmea_gpgsv m_gpgsv;
       
       float nmeaToDecimal(float coordinates, const char *val);
       float nmeaToDms(float coordinates, const char *val);
       
       void decodeGPRMC(char *line);
       void decodeGPGGA(char *line);
       void decodeGPGSA(char *line);
       void decodeGPGSV(char *line);
       void clearGPRMC();
       void clearGPGGA();
       void clearGPGSA();
       void clearGPGSV();
};

/**
    Constructor
*/
gps::impl::impl(std::string dev, int baud) {
    try {
        serialPort = new serial(dev, baud, '\n');
    }
    catch(std::exception &e) {
        std::cout << "\n" << e.what() << "\n";
        throw std::runtime_error("[GPS] Error communicating with device \n");
    }

    m_latitude = 0.0;
    m_longitude = 0.0;
    m_speed = 0.0;
    m_altitude = 0.0;

    time_t rawTime;
    ::time(&rawTime);
    m_time = *::localtime(&rawTime);

    clearGPGGA();
    clearGPRMC();
    clearGPGSA();
    clearGPGSV();
}

/**
    Destructor
*/
gps::impl::~impl() {
    serialPort->disconnect();
}

/**
    Decodes GPRMC command, iterating though line
*/
void gps::impl::decodeGPRMC(char *line) {
    clearGPRMC();    
    const char* delimiter = ",";
    char *token;
    int counter = 0;

    token = strtok(line, delimiter);
    float ltemp = 0.0;
 
    while(token != NULL)  {
        switch(counter) {
            case 1:
                m_gprmc.time.tm_hour = atoi(strndup(token,2));
                m_gprmc.time.tm_min = atoi(strndup(token+2,2));
                m_gprmc.time.tm_sec = atoi(strndup(token+4,2));
                break;
            case 2:        
                if(*token == 'V')
                    return ;
                break;
            case 3:        
                ltemp = atof(token); 
                ltemp = nmeaToDecimal(ltemp,"m");
                break;
            case 4:
                if (*token == 'S')
                    ltemp *= ( -1 );
                m_gprmc.latitude = ltemp;
                m_latitude = ltemp;
                break;
            case 5:
                ltemp = atof(token); 
                ltemp = nmeaToDecimal(ltemp, "p");
                break;
            case 6:
                if (*token == 'W')
                    ltemp *= ( -1 );
                m_gprmc.longitude = ltemp;
                m_longitude = ltemp;
                break;
            case 7:
               ltemp = atof(token);
                ltemp = ltemp*KNOTS_TO_KM;
                m_gprmc.speed = ltemp;
                m_speed = ltemp;
                break;
            case 9:
                m_gprmc.time.tm_mday = atoi(strndup(token,2));
                m_gprmc.time.tm_mon =atoi(strndup(token+2,2))-1;
                m_gprmc.time.tm_year = atoi(strndup(token+4,2)) + 100;
                m_time = m_gprmc.time;
                break;
            default:
                break;
        }

        token = strtok(NULL, delimiter);
        counter++;
    }
}

/**
    Decodes GPGGA command
*/
void gps::impl::decodeGPGGA(char *line) {
    clearGPGGA();
    const char* delimiter = ",";
    char *token;
    int counter = 0;

    token = strtok(line, delimiter);
    float ltemp = 0.0;
 
    while(token != NULL) {
        switch(counter) {
            case 1:
                m_gpgga.time.tm_hour = atoi(strndup(token,2));
                m_gpgga.time.tm_min = atoi(strndup(token+2,2));
                m_gpgga.time.tm_sec = atoi(strndup(token+4,2));
                m_time.tm_hour = m_gpgga.time.tm_hour;
                m_time.tm_min = m_gpgga.time.tm_min;
                m_time.tm_sec = m_gpgga.time.tm_sec;
                break;
            case 2:
                ltemp = atof(token); 
                ltemp = nmeaToDecimal(ltemp,"m");
                break;
            case 3:
                if (*token == 'S')
                    ltemp *= ( -1 );
                m_gpgga.latitude = ltemp;
                m_latitude = ltemp;
                break;
            case 4:   
                ltemp = atof(token); 
                ltemp = nmeaToDecimal(ltemp, "p");
                break;
            case 5:
                if (*token == 'W')
                    ltemp *= ( -1 );
                m_gpgga.longitude = ltemp;
                m_longitude = ltemp;
                break;
            case 6:
                if(atoi(token) == 0)
                    return ;
                break;
            case 7:
                    m_gpgga.n_satellites = atof(token);
                break;
            case 8:
                m_gpgga.hdop = atof(token);
                break;
            case 9:
                m_gpgga.altitude = atof(token);
                m_altitude = atof(token); 
                break;
            case 10:
                m_gpgga.altitude_unit = *token;
                break;
            case 11:
                m_gpgga.geoidHeight = atof(token);
                break;
            case 12:
                m_gpgga.geoidHeight_unit = *token;
                break;
            default:
                break;
        }

        token = strtok(NULL, delimiter);
        counter++;
    }
}

/**
    Decodes GPGSA command
*/
void gps::impl::decodeGPGSA(char *line) {
    clearGPGSA();
    const char* delimiter = ",";
    char *token;
    int counter = 0;

    token = strtok(line, delimiter);

    while(token != NULL) {
        switch(counter) {
            case 1:
                m_gpgsa.mode = *token;
                break;
            case 2:
                m_gpgsa.fix3d = atof(token);
                break;
            case 15:
                m_gpgsa.pdop = atof(token);
                break;
            case 16:
                m_gpgsa.hdop = atof(token);
                break;
            case 17:
                m_gpgsa.vdop = atof(token);
                break;
            default:
                break;
        }

        token = strtok(NULL, delimiter);
        counter++;
    }
}

/**
    Decodes GPGSV command
*/
void gps::impl::decodeGPGSV(char *line) {
    clearGPGSA();
    const char* delimiter = ",";
    char *token;
    int counter = 0;

    token = strtok(line, delimiter);

    while(token != NULL) {
        // TODO
        token = strtok(NULL, delimiter);
        counter++;
    }
}

/**
    Converts NMEA latitude & longitude to decimal coordinate system
*/
float gps::impl::nmeaToDecimal(float coordinates, const char *val) {
    if((*val == 'm') && (coordinates < 0.0 && coordinates > MAX_LATITUDE))
        return 0;     

    if(*val == 'p' && (coordinates < 0.0 && coordinates > MAX_LONGITUDE))
        return 0;

    int deg;
    float dec;
    deg = coordinates/100; // 51 degrees
    dec= (coordinates/100 - deg)*100 ; //(51.536605 - 51)* 100 = 53.6605
    dec /= 60; // 53.6605 / 60 = 0.8943417
    dec += deg; // 0.8943417 + 51 = 51.8943417

    return (int)coordinates/100 + (coordinates/100 - (int)coordinates/100)*100/60;
}

/**
    Converts NMEA latitude & longitude to degree-minute-second (DMS) coordinate system
*/
float gps::impl::nmeaToDms(float coordinates, const char *val) {
    if((*val == 'm') && (coordinates < 0.0 && coordinates > MAX_LATITUDE))
        return 0;     

    if(*val == 'p' && (coordinates < 0.0 && coordinates > MAX_LONGITUDE))
        return 0;

    float decimalCoord =(int)coordinates/100 + (coordinates/100 - (int)coordinates/100)*100/60;
    int deg = decimalCoord;
    int min = (decimalCoord - deg)*60; std::cout << (decimalCoord - deg)*60 << "\n";
    int sec = ((decimalCoord - deg)*60 - min)*60;
    std::cout << decimalCoord << " : " << deg << "º" << min <<"'"<< sec<<"\"\n";
    return 0;
}

/**
    Reads line and saves latitude and longitude
*/
int gps::impl::read() {
    std::string cmd;
    serialPort->read(cmd);

    //std::cout << cmd << "\n";

    if(cmd.find("GPRMC") != std::string::npos) {
        decodeGPRMC((char*)cmd.c_str());
        return 0;
    } else if(cmd.find("GPGGA") != std::string::npos) {
        decodeGPGGA((char*)cmd.c_str());
        return 0;
    } else if(cmd.find("GPGSA") != std::string::npos) {
        decodeGPGSA((char*)cmd.c_str());
        return 0;
    } else if(cmd.find("GPGSV") != std::string::npos) {
        decodeGPGSV((char*)cmd.c_str());
        return 0;
    } else
        return -1;
}

/**
    Reads data from file (DEBUG)
*/
int gps::impl::readFromFile(const char* file) {
    char line[80];
    FILE *in = fopen(file, "r");
    if ( in != NULL ) {
        while(fscanf (in, "%79[^\r]\n", line) == 1) {
            if(strstr(line,"$GPRMC"))
                decodeGPRMC(line);
            else if(strstr(line,"$GPGGA"))
                decodeGPGGA(line);
            else if(strstr(line,"$GPGSA"))
                decodeGPGSA(line);
            else if(strstr(line,"$GPGSV"))
                decodeGPGSV(line);
        }
    }
    else 
        return -1;

    fclose(in);
    return 0;
}

float gps::impl::getLatitude() {
    return m_latitude;
}

float gps::impl::getLongitude() {
    return m_longitude;
}

float gps::impl::getSpeed() {
    return m_speed;
}

float gps::impl::getAltitude() {
    return m_altitude;
}

std::string gps::impl::getTime() {
    char buff[100];
    strftime(buff, 100, "%Y-%m-%d %H:%M:%S", &m_time);
    return (std::string(buff)+" UTC");
}

void gps::impl::clearGPRMC() {
    m_gprmc.time = {0};
    m_gprmc.longitude = 0.0;
    m_gprmc.latitude = 0.0;
    m_gprmc.speed = 0.0;
    m_gprmc.magneticVar = '-';
}

void gps::impl::clearGPGGA() {
    m_gpgga.time = {0};
    m_gpgga.longitude = 0.0;
    m_gpgga.latitude = 0.0;
    m_gpgga.n_satellites = 0;
    m_gpgga.altitude = 0;
    m_gpgga.altitude_unit = '-';
}

void gps::impl::clearGPGSA() {
    m_gpgsa.mode = '-';
    m_gpgsa.fix3d = 0.0;
    m_gpgsa.pdop = 0.0;
    m_gpgsa.hdop = 0.0;
    m_gpgsa.vdop = 0.0;
}

void gps::impl::clearGPGSV() {
    m_gpgsv.n_satellites = 0;
    m_gpgsv.prn = 0;
    m_gpgsv.elevation = 0;
    m_gpgsv.azimuth = 0;
    m_gpgsv.snr = 0;
}


// Public API

gps::gps(std::string dev, int baud)
  : m_impl(new gps::impl(dev,baud))
{
}

gps::~gps() {
}

int gps::read() {
    return m_impl->read();
}

int gps::readFromFile(const char* file) {
    return m_impl->readFromFile(file);
}

float gps::getLatitude() {
    return m_impl->getLatitude();
}

float gps::getLongitude() {
    return m_impl->getLongitude();
}

float gps::getSpeed() {
    return m_impl->getSpeed();
}

float gps::getAltitude() {
    return m_impl->getAltitude();
}

std::string gps::getTime() {
    return m_impl->getTime();
}
