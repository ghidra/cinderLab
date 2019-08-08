#include "OSCManager.h"

#include "cinder/Utilities.h"
#include "cinder/Log.h"

using namespace std;
using namespace ci;
using namespace ci::app;
using namespace mlx;

OSCManager::OSCManager()
: mReceiver( recievePort )
, mSender( localPort, destinationHost, destinationPort )
, mIsConnected( false )
{
	//reciever
    try {
        // Bind the receiver to the endpoint. This function may throw.
        mReceiver.bind();
    }
    catch( const osc::Exception &ex ) {
        CI_LOG_E( "Error binding: " << ex.what() << " val: " << ex.value() );
        //quit();
    }
    ///WAIT TO MAKE A SENDER.... AFTER WE KNOW SOMEONE IS SENDING US MESSAGES
    
#if USE_UDP
    // UDP opens the socket and "listens" accepting any message from any endpoint. The listen
    // function takes an error handler for the underlying socket. Any errors that would
    // call this function are because of problems with the socket or with the remote message.
    mReceiver.listen(
                     []( asio::error_code error, protocol::endpoint endpoint ) -> bool {
                         if( error ) {
                             CI_LOG_E( "Error Listening: " << error.message() << " val: " << error.value() << " endpoint: " << endpoint );
                             return false;
                         }
                         else
                             return true;
                     });
#else
    mReceiver.setConnectionErrorFn(
                                   // Error Function for Accepted Socket Errors. Will be called anytime there's an
                                   // error reading from a connected socket (a socket that has been accepted below).
                                   [&]( asio::error_code error, uint64_t identifier ) {
                                       if ( error ) {
                                           auto foundIt = mConnections.find( identifier );
                                           if( foundIt != mConnections.end() ) {
                                               // EOF or end of file error isn't specifically an error. It's just that the
                                               // other side closed the connection while you were expecting to still read.
                                               if( error == asio::error::eof ) {
                                                   CI_LOG_W( "Other side closed the connection: " << error.message() << " val: " << error.value() << " endpoint: " << foundIt->second.address().to_string()
                                                            << " port: " << foundIt->second.port() );
                                               }
                                               else {
                                                   CI_LOG_E( "Error Reading from Socket: " << error.message() << " val: "
                                                            << error.value() << " endpoint: " << foundIt->second.address().to_string()
                                                            << " port: " << foundIt->second.port() );
                                               }
                                               mConnections.erase( foundIt );
                                           }
                                       }
                                   });
    auto expectedOriginator = protocol::endpoint( asio::ip::address::from_string( "127.0.0.1" ), 10000 );
    mReceiver.accept(
                     // Error Handler for the acceptor. You'll return true if you want to continue accepting
                     // or fals otherwise.
                     []( asio::error_code error, protocol::endpoint endpoint ) -> bool {
                         if( error ) {
                             CI_LOG_E( "Error Accepting: " << error.message() << " val: " << error.value()
                                      << " endpoint: " << endpoint.address().to_string() );
                             return false;
                         }
                         else
                             return true;
                     },
                     // Accept Handler. Return whether or not the acceptor should cache this connection
                     // (true) or dismiss it (false).
                     [&, expectedOriginator]( osc::TcpSocketRef socket, uint64_t identifier ) -> bool {
                         // Here we return whether or not the remote endpoint is the expected endpoint
                         mConnections.emplace( identifier, socket->remote_endpoint() );
                         return socket->remote_endpoint() == expectedOriginator;
                     } );
#endif
    
}

void OSCManager::connectSender()
{
    if(!mIsConnected)
    {
        try {
            // Bind the sender to the endpoint. This function may throw. The exception will
            // contain asio::error_code information.
            mSender.bind();
        }
        catch ( const osc::Exception &ex ) {
            CI_LOG_E( "Error binding: " << ex.what() << " val: " << ex.value() );
            //quit();
        }
        
        #if USE_UDP
        // Udp doesn't "connect" the same way Tcp does. If bind doesn't throw, we can
        // consider ourselves connected.
        mIsConnected = true;
        #else
        mSender.connect(
                        // Set up the OnConnectFn. If there's no error, you can consider yourself connected to
                        // the endpoint supplied.
                        [&]( asio::error_code error ){
                            if( error ) {
                                CI_LOG_E( "Error connecting: " << error.message() << " val: " << error.value() );
                                quit();
                            }
                            else {
                                CI_LOG_V( "Connected" );
                                mIsConnected = true;
                            }
                        });
        #endif
    }
}
	///OSC
// Unified error handler. Easiest to have a bound function in this situation,
// since we're sending from many different places.

void OSCManager::onSendError( asio::error_code error )
{
    
    if( error ) {
        CI_LOG_E( "Error sending: " << error.message() << " val: " << error.value() );
        // If you determine that this error is fatal, make sure to flip mIsConnected. It's
        // possible that the error isn't fatal.
        mIsConnected = false;
        try {
#if ! USE_UDP
            // If this is Tcp, it's recommended that you shutdown before closing. This
            // function could throw. The exception will contain asio::error_code
            // information.
            mSender.shutdown();
#endif
            // Close the socket on exit. This function could throw. The exception will
            // contain asio::error_code information.
            mSender.close();
        }
        catch( const osc::Exception &ex ) {
            CI_LOG_EXCEPTION( "Cleaning up socket: val -" << ex.value(), ex );
        }
        //quit();
    }
}

void OSCManager::callback(std::string message)
{
    connectSender();
    if( ! mIsConnected )
        return;
    
    osc::Message msg( "/capture/received" );
    msg.append( message );
    //msg.append( mCurrentMousePositon.y );
    // Send the msg and also provide an error handler. If the message is important you
    // could store it in the error callback to dispatch it again if there was a problem.
    mSender.send( msg, std::bind( &OSCManager::onSendError,this, std::placeholders::_1 ) );
    //CI_LOG_W("---------------: " << message);
}

