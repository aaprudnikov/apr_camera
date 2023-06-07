#ifndef MYMQTT_H
#define MYMQTT_H

enum PublishData {
    MY_UPTIME,
    MY_CAPTURE_CNT
};

class myMQTT {
    private:
    public:
        void sendData(PublishData data_type, int value);
        void retringSendData();
};

#endif
