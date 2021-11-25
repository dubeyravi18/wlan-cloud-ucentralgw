//
// Created by stephane bourque on 2021-11-23.
//

#include "RTTYS_device.h"
#include "rttys/RTTYS_server.h"

namespace OpenWifi {

	RTTY_Device_ConnectionHandler::RTTY_Device_ConnectionHandler(Poco::Net::StreamSocket& socket,
															 Poco::Net::SocketReactor & reactor):
		socket_(socket),
		reactor_(reactor)
	{
		reactor_.addEventHandler(socket_, Poco::NObserver<RTTY_Device_ConnectionHandler, Poco::Net::ReadableNotification>(*this, &RTTY_Device_ConnectionHandler::onSocketReadable));
		reactor_.addEventHandler(socket_, Poco::NObserver<RTTY_Device_ConnectionHandler, Poco::Net::ShutdownNotification>(*this, &RTTY_Device_ConnectionHandler::onSocketShutdown));
	}

	RTTY_Device_ConnectionHandler::~RTTY_Device_ConnectionHandler()
	{
		reactor_.removeEventHandler(socket_, Poco::NObserver<RTTY_Device_ConnectionHandler, Poco::Net::ReadableNotification>(*this, &RTTY_Device_ConnectionHandler::onSocketReadable));
		reactor_.removeEventHandler(socket_, Poco::NObserver<RTTY_Device_ConnectionHandler, Poco::Net::WritableNotification>(*this, &RTTY_Device_ConnectionHandler::onSocketWritable));
		reactor_.removeEventHandler(socket_, Poco::NObserver<RTTY_Device_ConnectionHandler, Poco::Net::ShutdownNotification>(*this, &RTTY_Device_ConnectionHandler::onSocketShutdown));

		if(!id_.empty())
			RTTYS_server()->DeRegister(id_,this);
	}

	std::string RTTY_Device_ConnectionHandler::SafeCopy( const u_char * buf, int MaxSize, int & NewPos) {
		std::string     S;
		while(NewPos<MaxSize && buf[NewPos]!=0) {
			S += buf[NewPos++];
		}

		if(buf[NewPos]==0)
			NewPos++;

		return S;
	}

	void RTTY_Device_ConnectionHandler::PrintBuf(const u_char * buf, int size) {

		std::cout << "======================================" << std::endl;
		while(size) {
			std::cout << std::hex << (int) *buf++ << " ";
			size--;
		}
		std::cout << std::endl;
		std::cout << "======================================" << std::endl;
	}

	int RTTY_Device_ConnectionHandler::SendMessage(RTTY_MSG_TYPE Type, const u_char * Buf, int len) {
		u_char outBuf[ 8192 ];
		outBuf[0] = Type;
		outBuf[1] = (len >> 8);
		outBuf[2] = (len & 0x00ff);
		std::memcpy(&outBuf[3], Buf, len);
		return socket_.sendBytes(&outBuf[0],len+3) == len+3;
	}

	int RTTY_Device_ConnectionHandler::SendMessage(RTTY_MSG_TYPE Type, std::string &S ) {
		u_char outBuf[ 8192 ];
		auto len = S.size() + 1;
		outBuf[0] = Type;
		outBuf[1] = (len >> 8);
		outBuf[2] = (len & 0x00ff);
		std::memcpy(&outBuf[3], S.c_str(), len);
		outBuf[len+3] = 0 ;
		return socket_.sendBytes(&outBuf[0],len+3) == len+3;
	}

	int RTTY_Device_ConnectionHandler::SendMessage(RTTY_MSG_TYPE Type) {
		u_char outBuf[ 8192 ];
		outBuf[0] = Type;
		outBuf[1] = 0;
		outBuf[2] = 0;
		return socket_.sendBytes(&outBuf[0],3) == 3;
	}

	void RTTY_Device_ConnectionHandler::SendToClient(const u_char *Buf, int len) {
		auto Client = RTTYS_server()->GetClient(id_);
		if(Client!= nullptr)
			Client->SendData(Buf,len);
	}

	void RTTY_Device_ConnectionHandler::SendToDevice(const u_char *buf, int len) {
		u_char sendBuf[8192];
		if(buf[0]==0) {
			sendBuf[0] = msgTypeTermData;
			sendBuf[1] = len >> 8;
			sendBuf[2] = len & 0x00ff;
			sendBuf[3] = 0; // no SID
			memcpy(&sendBuf[4], &buf[1], len - 1);
			int bsize = 4 + len - 1;
			socket_.sendBytes(&sendBuf[0], bsize );
			std::cout << "Sending to device" << std::endl;
			PrintBuf(&sendBuf[0], bsize);
		}
	}

	bool RTTY_Device_ConnectionHandler::InitializeConnection( std::string & sid ) {
		sid = MicroService::instance().CreateHash(id_).substr(0,32);
		std::cout << "SID Size: " << sid.length() << std::endl;
		char buf[64];
		buf[0] = msgTypeLogin;
		buf[1] = 0;
		buf[2] = 32;
		strcpy(&buf[3],sid.c_str());
		socket_.sendBytes(&buf[0],sid.length()+3);
		return true;
	}

	void RTTY_Device_ConnectionHandler::onSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf)
	{
		try
		{
			int len = socket_.receiveBytes(&inBuf_[0],BUF_SIZE);
			if (len > 0)
			{
				RTTY_MSG_TYPE   msg;
				if(inBuf_[0]>=(u_char)msgTypeMax) {
					delete this;
				}
				msg = (RTTY_MSG_TYPE) inBuf_[0];
				int MsgLen = (int) inBuf_[1] * 256 + (int) inBuf_[2];

				switch(msg) {
					case msgTypeRegister: {
						PrintBuf(&inBuf_[0],len);
						proto_ = inBuf_[0];
						int pos=3;
						id_ = SafeCopy(&inBuf_[0],MsgLen,pos);
						desc_ = SafeCopy(&inBuf_[0],MsgLen,pos);
						token_ = SafeCopy(&inBuf_[0],MsgLen,pos);
						std::cout << "msgTypeRegister: id: " << id_ << "  desc: " << desc_ << "  token: " << token_ << std::endl;
						u_char  outBuf[7];
						outBuf[0] = 0;
						outBuf[1] = 'O' ;
						outBuf[2] = 'K' ;
						outBuf[3] = 0;
						SendMessage(msgTypeRegister, &outBuf[0], 4);
						RTTYS_server()->Register(id_,this);
					}
					break;

					case msgTypeLogin: {
						std::cout << "msgTypeLogin: len" << MsgLen << std::endl;
						sid_code_ = inBuf_[3];
					}
					break;

					case msgTypeLogout: {
						std::cout << "msgTypeLogout" << std::endl;

					}
					break;

					case msgTypeTermData: {
						std::cout << "msgTypeTermData: len" << MsgLen << std::endl;
						PrintBuf(&inBuf_[0],len);
						SendToClient(&inBuf_[3],MsgLen);
					}
					break;

					case msgTypeWinsize: {
						std::cout << "msgTypeWinsize" << std::endl;

					}
					break;

					case msgTypeCmd: {
						std::cout << "msgTypeCmd" << std::endl;

					}
					break;

					case msgTypeHeartbeat: {
						std::cout << "msgTypeHeartbeat" << std::endl;
						PrintBuf(&inBuf_[0], len);
						SendMessage(msgTypeHeartbeat);
					}
					break;

					case msgTypeFile: {
						std::cout << "msgTypeFile" << std::endl;

					}
					break;

					case msgTypeHttp: {
						std::cout << "msgTypeHttp" << std::endl;

					}
					break;

					case msgTypeAck: {
						std::cout << "msgTypeAck" << std::endl;

					}
					break;

					case msgTypeMax: {
						std::cout << "msgTypeMax" << std::endl;
					}
					break;
					}
				}
				else
				{
					std::cout << "Device " << id_ << " no data." << std::endl;
					delete this;
				}
		}
		catch (Poco::Exception& exc)
		{
			delete this;
		}
	}

	void RTTY_Device_ConnectionHandler::onSocketWritable(const Poco::AutoPtr<Poco::Net::WritableNotification>& pNf)
	{
		try
		{
		}
		catch (Poco::Exception& exc)
		{
			delete this;
		}
	}

	void RTTY_Device_ConnectionHandler::onSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf)
	{
		std::cout << "Device " << id_ << " closing socket." << std::endl;
		delete this;
	}
}