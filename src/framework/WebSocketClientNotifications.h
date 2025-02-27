//
// Created by stephane bourque on 2022-05-05.
//

#pragma once

#include "framework/MicroService.h"

namespace OpenWifi {

	struct WebNotificationSingleDevice {
		std::string		serialNumber;
		inline void to_json(Poco::JSON::Object &Obj) const {
			RESTAPI_utils::field_to_json(Obj,"serialNumber", serialNumber);
		}

		inline bool from_json(const Poco::JSON::Object::Ptr &Obj) {
			try {
				RESTAPI_utils::field_from_json(Obj,"serialNumber", serialNumber);
				return true;
			} catch (...) {

			}
			return false;
		}
	};

	struct WebNotificationSingleDeviceConfigurationChange {
		std::string		serialNumber;
		uint64_t 		oldUUID;
		uint64_t 		newUUID;

		inline void to_json(Poco::JSON::Object &Obj) const {
			RESTAPI_utils::field_to_json(Obj,"serialNumber", serialNumber);
			RESTAPI_utils::field_to_json(Obj,"oldUUID", oldUUID);
			RESTAPI_utils::field_to_json(Obj,"newUUID", newUUID);
		}

		inline bool from_json(const Poco::JSON::Object::Ptr &Obj) {
			try {
				RESTAPI_utils::field_from_json(Obj,"serialNumber", serialNumber);
				RESTAPI_utils::field_from_json(Obj,"oldUUID", oldUUID);
				RESTAPI_utils::field_from_json(Obj,"newUUID", newUUID);
				return true;
			} catch (...) {

			}
			return false;
		}
	};

	struct WebNotificationSingleDeviceFirmwareChange {
		std::string		serialNumber;
		std::string		newFirmware;
		inline void to_json(Poco::JSON::Object &Obj) const {
			RESTAPI_utils::field_to_json(Obj,"serialNumber", serialNumber);
			RESTAPI_utils::field_to_json(Obj,"newFirmware", newFirmware);
		}

		inline bool from_json(const Poco::JSON::Object::Ptr &Obj) {
			try {
				RESTAPI_utils::field_from_json(Obj,"serialNumber", serialNumber);
				RESTAPI_utils::field_from_json(Obj,"newFirmware", newFirmware);
				return true;
			} catch (...) {

			}
			return false;
		}
	};

	struct WebSocketClientNotificationNumberOfConnection {
		std::uint64_t 	numberOfDevices=0;
		std::uint64_t 	averageConnectedTime=0;
		std::uint64_t 	numberOfConnectingDevices=0;

		inline void to_json(Poco::JSON::Object &Obj) const {
			RESTAPI_utils::field_to_json(Obj,"numberOfDevices", numberOfDevices);
			RESTAPI_utils::field_to_json(Obj,"averageConnectedTime", averageConnectedTime);
			RESTAPI_utils::field_to_json(Obj,"numberOfConnectingDevices", numberOfConnectingDevices);
		}

		inline bool from_json(const Poco::JSON::Object::Ptr &Obj) {
			try {
				RESTAPI_utils::field_from_json(Obj,"numberOfDevices", numberOfDevices);
				RESTAPI_utils::field_from_json(Obj,"averageConnectedTime", averageConnectedTime);
				RESTAPI_utils::field_from_json(Obj,"numberOfConnectingDevices", numberOfConnectingDevices);
				return true;
			} catch (...) {

			}
			return false;
		}
	};

	inline void  WebSocketClientNotificationNumberOfConnections(std::uint64_t numberOfDevices,
															   std::uint64_t averageConnectedTime,
															   std::uint64_t numberOfConnectingDevices) {
		WebSocketNotification<WebSocketClientNotificationNumberOfConnection>	N;
		N.content.numberOfDevices = numberOfDevices;
		N.content.averageConnectedTime = averageConnectedTime;
		N.content.numberOfConnectingDevices = numberOfConnectingDevices;
		N.type = "device_connections_statistics";
		WebSocketClientServer()->SendNotification(N);
	}

	inline void WebSocketClientNotificationDeviceConfigurationChange(const std::string &SerialNumber, uint64_t oldUUID, uint64_t newUUID) {
		WebSocketNotification<WebNotificationSingleDeviceConfigurationChange>	N;
		N.content.serialNumber = SerialNumber;
		N.content.oldUUID = oldUUID;
		N.content.newUUID = newUUID;
		N.type = "device_configuration_upgrade";
		WebSocketClientServer()->SendNotification(N);
	}

	inline void WebSocketClientNotificationDeviceFirmwareUpdated(const std::string &SerialNumber, const std::string &Firmware) {
		WebSocketNotification<WebNotificationSingleDeviceFirmwareChange>	N;
		N.content.serialNumber = SerialNumber;
		N.content.newFirmware = Firmware;
		N.type = "device_firmware_upgrade";
		WebSocketClientServer()->SendNotification(N);
	}

	inline void WebSocketClientNotificationDeviceConnected(const std::string &SerialNumber) {
		WebSocketNotification<WebNotificationSingleDevice>	N;
		N.content.serialNumber = SerialNumber;
		N.type = "device_connection";
		WebSocketClientServer()->SendNotification(N);
	}

	inline void WebSocketClientNotificationDeviceDisconnected(const std::string & SerialNumber) {
		WebSocketNotification<WebNotificationSingleDevice>	N;
		N.content.serialNumber = SerialNumber;
		N.type = "device_disconnection";
		WebSocketClientServer()->SendNotification(N);
	}

    struct WebSocketNotificationJobContent {
        std::string                 title,
                                    details,
                                    jobId;
        std::vector<std::string>    success,
                                    error,
                                    warning;
        uint64_t                    timeStamp=OpenWifi::Now();

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    inline void WebSocketNotificationJobContent::to_json(Poco::JSON::Object &Obj) const {
        RESTAPI_utils::field_to_json(Obj,"title",title);
        RESTAPI_utils::field_to_json(Obj,"jobId",jobId);
        RESTAPI_utils::field_to_json(Obj,"success",success);
        RESTAPI_utils::field_to_json(Obj,"error",error);
        RESTAPI_utils::field_to_json(Obj,"warning",warning);
        RESTAPI_utils::field_to_json(Obj,"timeStamp",timeStamp);
        RESTAPI_utils::field_to_json(Obj,"details",details);
    }

    inline bool WebSocketNotificationJobContent::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            RESTAPI_utils::field_from_json(Obj,"title",title);
            RESTAPI_utils::field_from_json(Obj,"jobId",jobId);
            RESTAPI_utils::field_from_json(Obj,"success",success);
            RESTAPI_utils::field_from_json(Obj,"error",error);
            RESTAPI_utils::field_from_json(Obj,"warning",warning);
            RESTAPI_utils::field_from_json(Obj,"timeStamp",timeStamp);
            RESTAPI_utils::field_from_json(Obj,"details",details);
            return true;
        } catch(...) {

        }
        return false;
    }

    typedef WebSocketNotification<WebSocketNotificationJobContent>  WebSocketClientNotificationVenueUpdateJob_t;

    inline void WebSocketClientNotificationVenueUpdateJobCompletionToUser( const std::string & User, WebSocketClientNotificationVenueUpdateJob_t &N) {
        N.type = "venue_configuration_update";
        WebSocketClientServer()->SendUserNotification(User,N);
    }

    /////
    /////
    /////

    struct WebSocketNotificationRebootList {
        std::string                 title,
                details,
                jobId;
        std::vector<std::string>    success,
                warning;
        uint64_t                    timeStamp=OpenWifi::Now();

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    typedef WebSocketNotification<WebSocketNotificationRebootList> WebSocketClientNotificationVenueRebootList_t;

    inline void WebSocketNotificationRebootList::to_json(Poco::JSON::Object &Obj) const {
        RESTAPI_utils::field_to_json(Obj,"title",title);
        RESTAPI_utils::field_to_json(Obj,"jobId",jobId);
        RESTAPI_utils::field_to_json(Obj,"success",success);
        RESTAPI_utils::field_to_json(Obj,"warning",warning);
        RESTAPI_utils::field_to_json(Obj,"timeStamp",timeStamp);
        RESTAPI_utils::field_to_json(Obj,"details",details);
    }

    inline bool WebSocketNotificationRebootList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            RESTAPI_utils::field_from_json(Obj,"title",title);
            RESTAPI_utils::field_from_json(Obj,"jobId",jobId);
            RESTAPI_utils::field_from_json(Obj,"success",success);
            RESTAPI_utils::field_from_json(Obj,"warning",warning);
            RESTAPI_utils::field_from_json(Obj,"timeStamp",timeStamp);
            RESTAPI_utils::field_from_json(Obj,"details",details);
            return true;
        } catch(...) {

        }
        return false;
    }

    inline void WebSocketClientNotificationVenueRebootCompletionToUser( const std::string & User, WebSocketClientNotificationVenueRebootList_t &N) {
        N.type = "venue_rebooter";
        WebSocketClientServer()->SendUserNotification(User,N);
    }

    /////
    /////
    /////

    struct WebSocketNotificationUpgradeList {
        std::string                 title,
                details,
                jobId;
        std::vector<std::string>    success,
                                    skipped,
                                    no_firmware,
                                    not_connected;
        uint64_t                    timeStamp=OpenWifi::Now();

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    typedef WebSocketNotification<WebSocketNotificationUpgradeList> WebSocketClientNotificationVenueUpgradeList_t;

    inline void WebSocketNotificationUpgradeList::to_json(Poco::JSON::Object &Obj) const {
        RESTAPI_utils::field_to_json(Obj,"title",title);
        RESTAPI_utils::field_to_json(Obj,"jobId",jobId);
        RESTAPI_utils::field_to_json(Obj,"success",success);
        RESTAPI_utils::field_to_json(Obj,"notConnected",not_connected);
        RESTAPI_utils::field_to_json(Obj,"noFirmware",no_firmware);
        RESTAPI_utils::field_to_json(Obj,"skipped",skipped);
        RESTAPI_utils::field_to_json(Obj,"timeStamp",timeStamp);
        RESTAPI_utils::field_to_json(Obj,"details",details);
    }

    inline bool WebSocketNotificationUpgradeList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            RESTAPI_utils::field_from_json(Obj,"title",title);
            RESTAPI_utils::field_from_json(Obj,"jobId",jobId);
            RESTAPI_utils::field_from_json(Obj,"success",success);
            RESTAPI_utils::field_from_json(Obj,"notConnected",not_connected);
            RESTAPI_utils::field_from_json(Obj,"noFirmware",no_firmware);
            RESTAPI_utils::field_from_json(Obj,"skipped",skipped);
            RESTAPI_utils::field_from_json(Obj,"timeStamp",timeStamp);
            RESTAPI_utils::field_from_json(Obj,"details",details);
            return true;
        } catch(...) {

        }
        return false;
    }

    inline void WebSocketClientNotificationVenueUpgradeCompletionToUser( const std::string & User, WebSocketClientNotificationVenueUpgradeList_t &N) {
        N.type = "venue_upgrader";
        WebSocketClientServer()->SendUserNotification(User,N);
    }

} // namespace OpenWifi

