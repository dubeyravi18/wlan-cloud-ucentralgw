//
// Created by stephane bourque on 2022-07-26.
//

#include "AP_WS_Connection.h"
#include "StorageService.h"
#include "framework/WebSocketClientNotifications.h"
#include "StateUtils.h"

namespace OpenWifi {
	void AP_WS_Connection::Process_state(Poco::JSON::Object::Ptr ParamsObj) {
		if (!State_.Connected) {
			poco_warning(Logger_, fmt::format(
									   "INVALID-PROTOCOL({}): Device '{}' is not following protocol", CId_, CN_));
			Errors_++;
			return;
		}

		if (ParamsObj->has(uCentralProtocol::UUID) && ParamsObj->has(uCentralProtocol::STATE)) {
			uint64_t UUID = ParamsObj->get(uCentralProtocol::UUID);
			auto StateStr = ParamsObj->get(uCentralProtocol::STATE).toString();
			auto StateObj = ParamsObj->getObject(uCentralProtocol::STATE);

			std::string request_uuid;
			if (ParamsObj->has(uCentralProtocol::REQUEST_UUID))
				request_uuid = ParamsObj->get(uCentralProtocol::REQUEST_UUID).toString();

			if (request_uuid.empty()) {
				poco_trace(Logger_, fmt::format("STATE({}): UUID={} Updating.", CId_, UUID));
			} else {
				poco_trace(Logger_, fmt::format("STATE({}): UUID={} Updating for CMD={}.",
												 CId_, UUID, request_uuid));
			}

			uint64_t UpgradedUUID;
			LookForUpgrade(UUID,UpgradedUUID);
			State_.UUID = UpgradedUUID;
			LastStats_ = StateStr;

			GWObjects::Statistics Stats{
				.SerialNumber = SerialNumber_, .UUID = UUID, .Data = StateStr};
			Stats.Recorded = OpenWifi::Now();
			StorageService()->AddStatisticsData(Stats);
			if (!request_uuid.empty()) {
				StorageService()->SetCommandResult(request_uuid, StateStr);
			}

			StateUtils::ComputeAssociations(StateObj, 	State_.Associations_2G,
														State_.Associations_5G);

			if (KafkaManager()->Enabled()) {
				Poco::JSON::Stringifier Stringify;
				std::ostringstream OS;
				Stringify.condense(ParamsObj, OS);
				KafkaManager()->PostMessage(KafkaTopics::STATE, SerialNumber_, OS.str());
			}

			WebSocketNotification<WebNotificationSingleDevice>	N;
			N.content.serialNumber = SerialNumber_;
			N.type = "device_statistics";
			WebSocketClientServer()->SendNotification(N);

		} else {
			poco_warning(Logger_, fmt::format("STATE({}): Invalid request. Missing serial, uuid, or state", CId_));
		}
	}
}