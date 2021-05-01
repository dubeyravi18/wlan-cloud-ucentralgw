//
// Created by stephane bourque on 2021-03-19.
//

#include "RESTAPI_commands.h"
#include "uStorageService.h"

void RESTAPI_commands::handleRequest(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response)
{
    if(!ContinueProcessing(Request,Response))
        return;

    if(!IsAuthorized(Request,Response))
        return;

    try {
        ParseParameters(Request);

        if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
            auto SerialNumber = GetBinding("serialNumber", "");
            auto StartDate = RESTAPIHandler::from_RFC3339(GetParameter("startDate", ""));
            auto EndDate = RESTAPIHandler::from_RFC3339(GetParameter("endDate", ""));
            auto Offset = GetParameter("offset", 0);
            auto Limit = GetParameter("limit", 100);

            std::vector<uCentralCommandDetails> Commands;

            uCentral::Storage::GetCommands(SerialNumber, StartDate, EndDate, Offset, Limit,
                                           Commands);

			std::cout << "Commands: " << Commands.size() << std::endl;

            Poco::JSON::Array ArrayObj;

            for (const auto &i : Commands) {
                Poco::JSON::Object Obj = i.to_json();
                ArrayObj.add(Obj);
            }

            Poco::JSON::Object RetObj;
            RetObj.set("commands", ArrayObj);
            ReturnObject(RetObj, Response);

            return;

        } else if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE) {
            auto SerialNumber = GetBinding("serialNumber", "");
            auto StartDate = RESTAPIHandler::from_RFC3339(GetParameter("startDate", ""));
            auto EndDate = RESTAPIHandler::from_RFC3339(GetParameter("endDate", ""));

            if (uCentral::Storage::DeleteCommands(SerialNumber, StartDate, EndDate))
                OK(Response);
            else
                BadRequest(Response);

            return;
        }
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}