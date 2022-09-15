//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include <shared_mutex>

#include "Poco/JSON/Object.h"
#include "RESTObjects//RESTAPI_GWobjects.h"
#include "framework/MicroService.h"

namespace OpenWifi {

	class AP_WS_Connection;
    class DeviceRegistry : public SubSystemServer {
    public:
		struct RegistryConnectionEntry {
			GWObjects::ConnectionState 	State_;
			std::string        			LastStats;
			GWObjects::HealthCheck		LastHealthcheck;
			uint64_t 					ConnectionId=0;
			uint64_t 					SerialNumber_=0;
		};

        static auto instance() {
            static auto instance_ = new DeviceRegistry;
            return instance_;
        }

		int Start() override;
		void Stop() override;

		inline bool GetStatistics(const std::string &SerialNumber, std::string & Statistics) {
			return GetStatistics(Utils::SerialNumberToInt(SerialNumber),Statistics);
		}
		bool GetStatistics(std::uint64_t SerialNumber, std::string & Statistics);

		inline void SetStatistics(const std::string &SerialNumber, const std::string &Statistics) {
			return SetStatistics(Utils::SerialNumberToInt(SerialNumber),Statistics);
		}
		void SetStatistics(std::uint64_t SerialNumber, const std::string &stats);

		inline bool GetState(const std::string & SerialNumber, GWObjects::ConnectionState & State) {
			return GetState(Utils::SerialNumberToInt(SerialNumber), State);
		}
		bool GetState(std::uint64_t SerialNumber, GWObjects::ConnectionState & State);

		inline void SetState(const std::string & SerialNumber, const GWObjects::ConnectionState & State) {
			return SetState(Utils::SerialNumberToInt(SerialNumber), State);
		}
		void SetState(uint64_t SerialNumber, const GWObjects::ConnectionState & State);

		inline bool GetHealthcheck(const std::string &SerialNumber, GWObjects::HealthCheck & CheckData) {
			return GetHealthcheck(Utils::SerialNumberToInt(SerialNumber), CheckData);
		}
		bool GetHealthcheck(std::uint64_t SerialNumber, GWObjects::HealthCheck & CheckData);

		inline void SetHealthcheck(const std::string &SerialNumber, const GWObjects::HealthCheck &H) {
			return SetHealthcheck(Utils::SerialNumberToInt(SerialNumber),H);
		}
		void SetHealthcheck(std::uint64_t SerialNumber, const GWObjects::HealthCheck &H);

/*		inline bool Connected(const std::string & SerialNumber) {
			return Connected(Utils::SerialNumberToInt(SerialNumber));
		}
*/
		bool Connected(uint64_t SerialNumber);

		inline bool SendFrame(const std::string & SerialNumber, const std::string & Payload) {
			return SendFrame(Utils::SerialNumberToInt(SerialNumber), Payload);
		}

		bool SendFrame(std::uint64_t SerialNumber, const std::string & Payload);

		inline void SetPendingUUID(const std::string & SerialNumber, uint64_t PendingUUID) {
			return SetPendingUUID(Utils::SerialNumberToInt(SerialNumber), PendingUUID);
		}

		void SetPendingUUID(std::uint64_t SerialNumber, std::uint64_t PendingUUID);
		bool SendRadiusAuthenticationData(const std::string & SerialNumber, const unsigned char * buffer, std::size_t size);
		bool SendRadiusAccountingData(const std::string & SerialNumber, const unsigned char * buffer, std::size_t size);
		bool SendRadiusCoAData(const std::string & SerialNumber, const unsigned char * buffer, std::size_t size);

		[[nodiscard]] inline std::shared_ptr<RegistryConnectionEntry> StartSession(uint64_t ConnectionId) {
			std::unique_lock	G(M_);
			auto NewSession = std::make_shared<RegistryConnectionEntry>();
			Sessions_[ConnectionId] = NewSession;
			NewSession->ConnectionId = ConnectionId;
			return NewSession;
		}

		inline void SetSessionDetails(std::uint64_t connection_id, AP_WS_Connection * connection, uint64_t SerialNumber) {
			std::unique_lock	G(M_);
			auto Hint = Sessions_.find(connection_id);
			if(Hint!=Sessions_.end()) {
				auto Ptr = Hint->second;
				Ptr->SerialNumber_ = SerialNumber;
				Ptr->State_.Connected = true;
				Ptr->State_.LastContact = OpenWifi::Now();
				Ptr->State_.VerifiedCertificate = GWObjects::CertificateValidation::NO_CERTIFICATE;
				SerialNumbers_[SerialNumber] = std::make_pair(Ptr,connection);
			}
		}

		void EndSession(std::uint64_t connection_id, AP_WS_Connection * connection, std::uint64_t serial_number);

		void SetWebSocketTelemetryReporting(uint64_t SerialNumber, uint64_t Interval, uint64_t Lifetime);
		void StopWebSocketTelemetry(uint64_t SerialNumber);
		void SetKafkaTelemetryReporting(uint64_t SerialNumber, uint64_t Interval, uint64_t Lifetime);
		void StopKafkaTelemetry(uint64_t SerialNumber);
		void GetTelemetryParameters(uint64_t SerialNumber , bool & TelemetryRunning,
									uint64_t & TelemetryInterval,
									uint64_t & TelemetryWebSocketTimer,
									uint64_t & TelemetryKafkaTimer,
									uint64_t & TelemetryWebSocketCount,
									uint64_t & TelemetryKafkaCount,
									uint64_t & TelemetryWebSocketPackets,
									uint64_t & TelemetryKafkaPackets);

	  private:
		std::shared_mutex														M_;
		std::map<std::uint64_t ,std::shared_ptr<RegistryConnectionEntry>>  		Sessions_;
		std::map<uint64_t, std::pair<std::shared_ptr<RegistryConnectionEntry>,AP_WS_Connection *>>			SerialNumbers_;

		DeviceRegistry() noexcept:
    		SubSystemServer("DeviceRegistry", "DevStatus", "devicestatus") {
		}
	};

	inline auto DeviceRegistry() { return DeviceRegistry::instance(); }

}  // namespace

