//
// Created by stephane bourque on 2021-09-07.
//
#include <thread>

#include "framework/MicroService.h"

#include "Poco/JSON/Array.h"
#include "Poco/Net/HTTPHeaderStream.h"
#include "Poco/URI.h"

#include "RESTAPI/RESTAPI_telemetryWebSocket.h"
#include "TelemetryStream.h"

namespace OpenWifi {

	int TelemetryStream::Start() {
		Running_ = true;
		Messages_->Readable_ += Poco::delegate(this,&TelemetryStream::onMessage);
		Thr_.start(Reactor_);
		Utils::SetThreadName(Thr_,"telemetry-svr");
		return 0;
	}

	void TelemetryStream::Stop() {
		poco_information(Logger(),"Stopping...");
		Reactor_.stop();
		Thr_.join();
		if(Running_) {
			Running_ = false;
			Messages_->Readable_ -= Poco::delegate( this, &TelemetryStream::onMessage);
		}
		poco_information(Logger(),"Stopped...");
	}

	bool TelemetryStream::IsValidEndPoint(uint64_t SerialNumber, const std::string & UUID) {
		std::lock_guard	G(Mutex_);

		auto U = Clients_.find(UUID);
		if(U == Clients_.end() )
			return false;

		auto N = SerialNumbers_.find(SerialNumber);
		if(N == SerialNumbers_.end())
			return false;

		return (N->second.find(UUID) != N->second.end());
	}

	bool TelemetryStream::CreateEndpoint(uint64_t SerialNumber, std::string &EndPoint, const std::string &UUID) {
		std::lock_guard	G(Mutex_);

		Poco::URI	Public(MicroService::instance().ConfigGetString("openwifi.system.uri.public"));
		Poco::URI	U;
		U.setScheme("wss");
		U.setHost(Public.getHost());
		U.setPort(Public.getPort());
		auto RESTAPI_Path = std::string(*(RESTAPI_telemetryWebSocket::PathName().begin()));
		U.setPath(RESTAPI_Path);
		U.addQueryParameter("uuid", UUID);
		U.addQueryParameter("serialNumber", Utils::IntToSerialNumber(SerialNumber));
		EndPoint = U.toString();
		auto H = SerialNumbers_.find(SerialNumber);
		if(H == SerialNumbers_.end()) {
			std::set<std::string>	UUIDs{UUID};
			SerialNumbers_[SerialNumber] = UUIDs;
		} else {
			H->second.insert(UUID);
		}
		Clients_[UUID] = nullptr;
		return true;
	}

	void TelemetryStream::UpdateEndPoint(uint64_t SerialNumber, const std::string &PayLoad) {
		{
			std::lock_guard M(Mutex_);
			if (SerialNumbers_.find(SerialNumber) == SerialNumbers_.end()) {
				return;
			}
		}
		Messages_->Write(QueueUpdate{.SerialNumber=SerialNumber, .Payload = PayLoad});
	}

	void TelemetryStream::onMessage(bool &b){
		if(b) {
			QueueUpdate Msg;

			auto S = Messages_->Read(Msg);

			if(S) {
				std::lock_guard	M(Mutex_);
				auto H1 = SerialNumbers_.find(Msg.SerialNumber);
				if (H1 != SerialNumbers_.end()) {
					for (auto &i : H1->second) {
						auto H2 = Clients_.find(i);
						if (H2 != Clients_.end() && H2->second != nullptr) {
							try {
								H2->second->Send(Msg.Payload);
							} catch (...) {
							}
						}
					}
				}
			}
		}
	}

	bool TelemetryStream::RegisterClient(const std::string &UUID, TelemetryClient *Client) {
		std::lock_guard	G(Mutex_);
		Clients_[UUID] = Client;
		return true;
	}

	void TelemetryStream::DeRegisterClient(const std::string &UUID) {
		std::lock_guard		G(Mutex_);

		auto Hint = Clients_.find(UUID);
		if(Hint!=Clients_.end()) {
			Clients_.erase(Hint);
			for(const auto &i:SerialNumbers_) {
				auto S = i.second;
				S.erase(UUID);
			}

			//	remove empty slots
			for( auto i = SerialNumbers_.begin(); i!= SerialNumbers_.end();) {
				if(i->second.empty()) {
					i = SerialNumbers_.erase(i);
				} else {
					++i;
				}
			}
		}
	}
}