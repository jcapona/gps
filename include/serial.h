#ifndef SERIAL_H_
#define SERIAL_H_
#include <memory>

class serial {
    public:
        serial(const std::string &dev, int baud, char eol);
        ~serial();
        void connect();
        void disconnect();
        void write(const std::string &msg);
        void read(std::string &msg);
        bool isConnected();

    private:
        class impl;
        std::unique_ptr<impl> m_impl;
};

#endif /* SERIAL_H_ */
