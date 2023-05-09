#ifndef MYSERVER_H
#define MYSERVER_H

class myWiFi;

class myServer {
    private:
        myWiFi *m_wifi;

    public:
        myServer(myWiFi *mWiFi);
        void setupAPServer();
        void setupRoutingAPServer(); 
        void setupMainServer();
        void setupRoutingHTTP();
        void mhandleClient();
};

#endif
