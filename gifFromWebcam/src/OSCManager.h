#pragma once

#include "cinder/app/App.h"
#include "cinder/osc/Osc.h"

using namespace std;
using namespace ci;
using namespace ci::app;

const std::string destinationHost = "127.0.0.1";
const uint16_t destinationPort = 6969;
const uint16_t recievePort = 6161;
const uint16_t localPort = 6160;//for binding sender to

#define USE_UDP 1

#if USE_UDP
using Receiver = osc::ReceiverUdp;
using protocol = asio::ip::udp;
using Sender = osc::SenderUdp;
#else
using Receiver = osc::ReceiverTcp;
using protocol = asio::ip::tcp;
using Sender = osc::SenderTcp;
#endif

namespace mlx
{
	typedef std::shared_ptr<class OSCManager> OSCManagerRef;
	class OSCManager {
		public:
			OSCManager();
			~OSCManager(){}

			//recieve
		    Receiver mReceiver;
		    std::map<uint64_t, protocol::endpoint> mConnections;
		    
		    //send
		    void onSendError( asio::error_code error );
		    Sender    mSender;
		    bool    mIsConnected;
        
            //MY CALLBACK ON REIPT
            void callback(std::string message);
    private:
        void connectSender();
    };
}
