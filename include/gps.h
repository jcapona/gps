#ifndef GPS_H_
#define GPS_H_

#include <memory>

class gps
    {
     public:
      gps(std::string dev, int baud);
      ~gps();
      int read();
      int readFromFile(const char* file);
      float getLatitude();
      float getLongitude();
      float getSpeed();
      float getAltitude();
      std::string getTime();
      
     private:
         class impl;
         std::unique_ptr<impl> m_impl;
};

#endif /* GPS_H_ */
