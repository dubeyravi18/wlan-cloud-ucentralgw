//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include "framework/MicroService.h"
#include "framework/StorageClass.h"
#include "RESTObjects//RESTAPI_GWobjects.h"
#include "Poco/Net/IPAddress.h"

namespace OpenWifi {

    class Storage : public StorageClass {

    public:

		enum class CommandExecutionType {
			COMMAND_PENDING,
			COMMAND_EXECUTED,
			COMMAND_COMPLETED,
			COMMAND_TIMEDOUT,
			COMMAND_FAILED,
		  	COMMAND_EXPIRED,
			COMMAND_EXECUTING
		};

		inline std::string to_string(const CommandExecutionType &C) {
			switch(C) {
				case CommandExecutionType::COMMAND_PENDING: return "pending";
				case CommandExecutionType::COMMAND_EXECUTED: return "executed";
				case CommandExecutionType::COMMAND_COMPLETED: return "completed";
				case CommandExecutionType::COMMAND_TIMEDOUT: return "timedout";
				case CommandExecutionType::COMMAND_FAILED: return "failed";
				case CommandExecutionType::COMMAND_EXPIRED: return "expired";
				case CommandExecutionType::COMMAND_EXECUTING:
				default:
					return "executing";
			}
		}

		[[nodiscard]] inline std::string ComputeRange(uint64_t From, uint64_t HowMany) {
			if(dbType_==sqlite) {
				return " LIMIT " + std::to_string(From) + ", " + std::to_string(HowMany) + " ";
			} else if(dbType_==pgsql) {
				return " LIMIT " + std::to_string(HowMany) + " OFFSET " + std::to_string(From) + " ";
			} else if(dbType_==mysql) {
				return " LIMIT " + std::to_string(HowMany) + " OFFSET " + std::to_string(From) + " ";
			}
			return " LIMIT " + std::to_string(HowMany) + " OFFSET " + std::to_string(From) + " ";
		}

		inline std::string ConvertParams(const std::string & S) const {
			std::string R;
			R.reserve(S.size()*2+1);
			if(dbType_==pgsql) {
				auto Idx=1;
				for(auto const & i:S)
				{
					if(i=='?') {
						R += '$';
						R.append(std::to_string(Idx++));
					} else {
						R += i;
					}
				}
			} else {
				R = S;
			}
			return R;
		}

        static auto instance() {
			static auto instance_ = new Storage;
			return instance_;
        }

		// typedef std::map<std::string,std::string>	DeviceCapabilitiesCache;

        bool AddLog(const GWObjects::DeviceLog & Log);
		bool AddStatisticsData(const GWObjects::Statistics & Stats);
		bool GetStatisticsData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
							   std::vector<GWObjects::Statistics> &Stats);
		bool DeleteStatisticsData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate );
		bool GetNewestStatisticsData(std::string &SerialNumber, uint64_t HowMany, std::vector<GWObjects::Statistics> &Stats);

		bool AddHealthCheckData(const GWObjects::HealthCheck &Check);
		bool GetHealthCheckData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
								std::vector<GWObjects::HealthCheck> &Checks);
		bool DeleteHealthCheckData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate );
		bool GetNewestHealthCheckData(std::string &SerialNumber, uint64_t HowMany,
									  std::vector<GWObjects::HealthCheck> &Checks);

		bool UpdateDeviceConfiguration(std::string &SerialNumber, std::string &Configuration, uint64_t & NewUUID );

		bool CreateDevice(GWObjects::Device &);
		bool CreateDefaultDevice(std::string & SerialNumber, std::string & Capabilities, std::string & Firmware, std::string &Compatible,const Poco::Net::IPAddress & IPAddress);

		bool GetDevice(std::string &SerialNumber, GWObjects::Device &);
		bool GetDevices(uint64_t From, uint64_t HowMany, std::vector<GWObjects::Device> &Devices, const std::string & orderBy="");
//		bool GetDevices(uint64_t From, uint64_t HowMany, const std::string & Select, std::vector<GWObjects::Device> &Devices, const std::string & orderBy="");
		bool DeleteDevice(std::string &SerialNumber);
		bool UpdateDevice(GWObjects::Device &);
		bool DeviceExists(std::string & SerialNumber);
		bool SetConnectInfo(std::string &SerialNumber, std::string &Firmware);
		bool GetDeviceCount(uint64_t & Count);
		bool GetDeviceSerialNumbers(uint64_t From, uint64_t HowMany, std::vector<std::string> & SerialNumbers, const std::string & orderBy="");
		bool GetDeviceFWUpdatePolicy(std::string & SerialNumber, std::string & Policy);
		bool SetDevicePassword(std::string & SerialNumber, std::string & Password);
		bool UpdateSerialNumberCache();
		void GetDeviceDbFieldList( Types::StringVec & Fields);

		bool ExistingConfiguration(std::string &SerialNumber, uint64_t CurrentConfig, std::string &NewConfig, uint64_t &);

		bool UpdateDeviceCapabilities(std::string &SerialNumber, std::string &State, std::string & Compatible);
		bool GetDeviceCapabilities(std::string &SerialNumber, GWObjects::Capabilities &);
		bool DeleteDeviceCapabilities(std::string & SerialNumber);
		bool CreateDeviceCapabilities(std::string & SerialNumber, std::string & Capabilities);
		bool InitCapabilitiesCache();

		bool GetLogData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
						std::vector<GWObjects::DeviceLog> &Stats, uint64_t Type);
		bool DeleteLogData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Type);
		bool GetNewestLogData(std::string &SerialNumber, uint64_t HowMany, std::vector<GWObjects::DeviceLog> &Stats, uint64_t Type);

		bool CreateDefaultConfiguration(std::string & name, GWObjects::DefaultConfiguration & DefConfig);
		bool DeleteDefaultConfiguration(std::string & name);
		bool UpdateDefaultConfiguration(std::string & name, GWObjects::DefaultConfiguration & DefConfig);
		bool GetDefaultConfiguration(std::string &name, GWObjects::DefaultConfiguration & DefConfig);
		bool GetDefaultConfigurations(uint64_t From, uint64_t HowMany, std::vector<GWObjects::DefaultConfiguration> &Devices);
		bool FindDefaultConfigurationForModel(const std::string & Model, GWObjects::DefaultConfiguration & DefConfig );
		uint64_t GetDefaultConfigurationsCount();
		bool DefaultConfigurationAlreadyExists(std::string &Name);

		bool AddCommand(std::string & SerialNumber, GWObjects::CommandDetails & Command,CommandExecutionType Type);
		bool GetCommands(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany, std::vector<GWObjects::CommandDetails> & Commands);
		bool DeleteCommands(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate);
		bool GetNonExecutedCommands( uint64_t Offset, uint64_t HowMany, std::vector<GWObjects::CommandDetails> & Commands );
		bool UpdateCommand( std::string &UUID, GWObjects::CommandDetails & Command );
		bool GetCommand( const std::string &UUID, GWObjects::CommandDetails & Command );
		bool DeleteCommand( std::string &UUID );
		bool GetReadyToExecuteCommands( uint64_t Offset, uint64_t HowMany, std::vector<GWObjects::CommandDetails> & Commands );
		bool CommandExecuted(std::string & UUID);
		bool CommandCompleted(std::string & UUID, const Poco::JSON::Object & ReturnVars, const std::chrono::duration<double, std::milli> & execution_time, bool FullCommand);
//		bool AttachFileToCommand(std::string & UUID);
		bool AttachFileDataToCommand(std::string & UUID, const std::stringstream &s);
		bool CancelWaitFile( std::string & UUID, std::string & ErrorText );
//		bool GetAttachedFile(std::string & UUID, const std::string & SerialNumber, const std::string & FileName, std::string &Type);
		bool GetAttachedFileContent(std::string & UUID, const std::string & SerialNumber, std::string & FileContent, std::string &Type);
		bool RemoveAttachedFile(std::string & UUID);
		bool SetCommandResult(std::string & UUID, std::string & Result);
		bool GetNewestCommands(std::string &SerialNumber, uint64_t HowMany, std::vector<GWObjects::CommandDetails> & Commands);
		bool SetCommandExecuted(std::string & CommandUUID);
		bool SetCommandTimedOut(std::string &CommandUUID);

		void RemovedExpiredCommands();
		void RemoveTimedOutCommands();

		bool RemoveOldCommands(std::string & SerilNumber, std::string & Command);

		bool AddBlackListDevices(std::vector<GWObjects::BlackListedDevice> &  Devices);
		bool AddBlackListDevice(GWObjects::BlackListedDevice &  Device);
		bool GetBlackListDevice(std::string & SerialNumber, GWObjects::BlackListedDevice & Device);
		bool DeleteBlackListDevice(std::string & SerialNumber);
		bool IsBlackListed(std::string & SerialNumber);
		bool InitializeBlackListCache();
		bool GetBlackListDevices(uint64_t Offset, uint64_t HowMany, std::vector<GWObjects::BlackListedDevice> & Devices );
		bool UpdateBlackListDevice(std::string & SerialNumber, GWObjects::BlackListedDevice & Device);
		uint64_t GetBlackListDeviceCount();

		bool RemoveHealthChecksRecordsOlderThan(uint64_t Date);
		bool RemoveDeviceLogsRecordsOlderThan(uint64_t Date);
		bool RemoveStatisticsRecordsOlderThan(uint64_t Date);
		bool RemoveCommandListRecordsOlderThan(uint64_t Date);

		int Create_Tables();
		int Create_Statistics();
		int Create_Devices();
		int Create_Capabilities();
		int Create_HealthChecks();
		int Create_DeviceLogs();
		int Create_DefaultConfigs();
		int Create_CommandList();
		int Create_BlackList();
		int Create_FileUploads();

		bool AnalyzeCommands(Types::CountedMap &R);
		bool AnalyzeDevices(GWObjects::Dashboard &D);

		int 	Start() override;
		void 	Stop() override;

	  private:

   };

   inline auto StorageService() { return Storage::instance(); }

}  // namespace

