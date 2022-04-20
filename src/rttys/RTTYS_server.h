//
// Created by stephane bourque on 2021-11-23.
//

#ifndef UCENTRALGW_RTTYS_SERVER_H
#define UCENTRALGW_RTTYS_SERVER_H

#include "framework/MicroService.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SocketAcceptor.h"

#include "RTTYS_device.h"
#include "rttys/RTTYS_ClientConnection.h"

namespace OpenWifi {

	class RTTYS_server : public SubSystemServer
	{
	  public:
		static auto instance() {
			static auto instance = new RTTYS_server;
			return instance;
		}

		int Start() final;
		void Stop() final;

		inline auto UIAssets() { return RTTY_UIAssets_; }

		inline void Register(const std::string &Id, RTTYS_ClientConnection *Client) {
			std::lock_guard	G(M_);
			auto It = EndPoints_.find(Id);
			if(It!=EndPoints_.end()) {
				It->second.Client = Client;
				It->second.ClientConnected = OpenWifi::Now();
			}
		}

		inline void DeRegister(const std::string &Id, RTTYS_ClientConnection *Client) {
			std::lock_guard	G(M_);
			auto It = EndPoints_.find(Id);
			if(It==EndPoints_.end())
				return;
			if(It->second.Client!=Client)
				return;
			It->second.Client = nullptr;
			It->second.Done = true;
			It->second.ClientConnected = 0 ;
		}

		inline RTTYS_ClientConnection * GetClient(const std::string &Id) {
			std::lock_guard	G(M_);
			auto It = EndPoints_.find(Id);
			if(It==EndPoints_.end()) {
				return nullptr;
			}
			return It->second.Client;
		}

		inline bool Register(const std::string &Id, const std::string &Token, RTTY_Device_ConnectionHandler *Device) {
			std::lock_guard	G(M_);
			auto It = EndPoints_.find(Id);
			if(It!=EndPoints_.end()) {
				std::cout << "Updating connection" << std::endl;
				if(It->second.Device!= nullptr) {
					std::cout << "Removing old device connection" << std::endl;
					delete It->second.Device;
				}
				It->second.Device = Device;
				It->second.Token = Token;
				It->second.DeviceConnected = OpenWifi::Now();
				Logger().information(fmt::format("Creating session: {}, device:'{}'",Id,It->second.SerialNumber));
				return true;
			}
			return false;
		}

		inline void DeRegister(const std::string &Id, RTTY_Device_ConnectionHandler *Conn) {
			std::lock_guard	G(M_);
			auto It = EndPoints_.find(Id);
			if(It==EndPoints_.end())
				return;
			if(It->second.Device!=Conn)
				return;
			Logger().information(fmt::format("DeRegistering session: {}, device:'{}'",Id,It->second.SerialNumber));
			It->second.Device = nullptr;
			It->second.Done = true;
			It->second.DeviceConnected = 0 ;
		}

		inline RTTY_Device_ConnectionHandler * GetDevice(const std::string &id) {
			std::lock_guard	G(M_);
			auto It = EndPoints_.find(id);
			if(It==EndPoints_.end()) {
 				return nullptr;
			}
			return It->second.Device;
		}

		inline bool CreateEndPoint(const std::string &Id, const std::string & Token, const std::string & UserName, const std::string & SerialNumber ) {
			std::lock_guard	G(M_);

			EndPoint E;
			E.Done = false;
			E.Token = Token;
			E.TimeStamp = std::time(nullptr);
			E.SerialNumber = SerialNumber;
			E.UserName = UserName;
			EndPoints_[Id] = E;
			return true;
		}

		inline std::string SerialNumber(const std::string & Id) {
			std::lock_guard	G(M_);

			auto It = EndPoints_.find(Id);
			if(It==EndPoints_.end())
				return "";
			return It->second.SerialNumber;
		}

		inline void LoginDone(const std::string & Id) {
			std::lock_guard	G(M_);

			auto It = EndPoints_.find(Id);
			if(It==EndPoints_.end())
				return;
			Logger().information(fmt::format("User: {}, Serial: {} logged in.",It->second.UserName, It->second.SerialNumber ));
		}

		inline bool ValidEndPoint(const std::string &Id, const std::string &Token) {
			std::lock_guard	G(M_);
			auto It = EndPoints_.find(Id);
			if(It==EndPoints_.end()) {
				return false;
            }
			uint64_t Now = std::time(nullptr);
			return ((It->second.Token == Token) && ((Now-It->second.TimeStamp)<30));
		}

		inline bool CanConnect( const std::string &Id, RTTYS_ClientConnection *Conn) {
			std::lock_guard	G(M_);

			auto It = EndPoints_.find(Id);
			if(It!=EndPoints_.end() && It->second.Client==Conn && It->second.ClientConnected==0) {
				It->second.ClientConnected = std::time(nullptr);
				return true;
			}
			return false;
		}

		inline bool IsDeviceRegistered( const std::string &Id, const std::string &Token, [[maybe_unused]] RTTY_Device_ConnectionHandler *Conn) {
			std::lock_guard	G(M_);

			auto It = EndPoints_.find(Id);
			if(It == EndPoints_.end() || It->second.Token != Token )
				return false;
			return true;
		}

		bool Login(const std::string & Id_);
		bool Logout(const std::string & Id_);
		bool Close(const std::string & Id_);

		inline uint64_t DeviceSessionID(const std::string & Id) {
			auto it = EndPoints_.find(Id);
			if(it==EndPoints_.end()) {
				std::cout << "No ID found" << std::endl;
				return 0;
			} else {
				if(it->second.Device== nullptr) {
					std::cout << "No device for ID found" << std::endl;
					return 0;
				} else {
					return it->second.Device->SessionID();
				}
			}
		}

		struct EndPoint {
			std::string 					Token;
			RTTYS_ClientConnection *		Client = nullptr;
			RTTY_Device_ConnectionHandler *	Device = nullptr;
			uint64_t 						TimeStamp = std::time(nullptr);
			uint64_t 						DeviceConnected = 0;
			uint64_t 						ClientConnected = 0;
			std::string 					UserName;
			std::string 					SerialNumber;
			bool 							Done = false;
		};

		inline bool UseInternal() const {
			return Internal_;
		}

	  private:
		std::recursive_mutex		M_;
		Poco::Net::SocketReactor	DeviceReactor_;
		Poco::Net::SocketReactor	ClientReactor_;
		Poco::Thread				DeviceReactorThread_;
		Poco::Thread				ClientReactorThread_;
		std::string 				RTTY_UIAssets_;
		std::atomic_bool 			Internal_ = false;

		std::map<std::string, EndPoint> 			EndPoints_;			//	id, endpoint

		std::unique_ptr<Poco::Net::SocketAcceptor<RTTY_Device_ConnectionHandler>>	DeviceAcceptor_;
		std::unique_ptr<Poco::Net::HTTPServer>				WebServer_;

		explicit RTTYS_server() noexcept:
		SubSystemServer("RTTY_Server", "RTTY-SVR", "rtty.server")
			{
			}
	};

	inline RTTYS_server * RTTYS_server() { return RTTYS_server::instance(); }

} // namespace OpenWifi

#endif // UCENTRALGW_RTTYS_SERVER_H
