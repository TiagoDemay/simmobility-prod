//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * NS3ClientRegistration.cpp
 *
 *  Created on: May 20, 2013
 *      Author: vahid
 */

#include "NS3ClientRegistration.hpp"
#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"
#include "entities/commsim/connection/ConnectionHandler.hpp"
#include "event/EventPublisher.hpp"
#include "entities/commsim/broker/Common.hpp"
#include "entities/commsim/event/AllLocationsEventArgs.hpp"
#include "AgentsInfo.hpp"
#include <boost/foreach.hpp>

namespace sim_mob {

NS3ClientRegistration::NS3ClientRegistration(/*ConfigParams::ClientType type_*/) : ClientRegistrationHandler(ConfigParams::NS3_SIMULATOR) {
	// TODO Auto-generated constructor stub

}

bool NS3ClientRegistration::handle(sim_mob::Broker& broker, sim_mob::ClientRegistrationRequest request){

	//This part is locked in fear of registered agents' iterator invalidation in the middle of the process
		AgentsList::Mutex registered_agents_mutex;
		AgentsList::type &registeredAgents = broker.getRegisteredAgents(&registered_agents_mutex);
		AgentsList::Lock lock(registered_agents_mutex);
	//some checks to avoid calling this method unnecessarily
	if(
			broker.getClientWaitingList().empty()
//			|| broker.getRegisteredAgents().empty()
			)
	{
		return false;
	}

		//use it to create a client entry
		boost::shared_ptr<ClientHandler> clientEntry(new ClientHandler(broker));
		boost::shared_ptr<sim_mob::ConnectionHandler > cnnHandler(new ConnectionHandler(
				request.session_
//				,broker
//				,&Broker::messageReceiveCallback
				,broker.getMessageReceiveCallBack()
				,request.clientID
				,ConfigParams::NS3_SIMULATOR
				,(unsigned long int)(0)//not needed
				)
				);
		clientEntry->cnnHandler = cnnHandler;
		clientEntry->AgentCommUtility_ = 0;//not needed
		//todo: some of there information are already available in the connectionHandler! omit redundancies  -vahid
		clientEntry->agent = 0;//not needed
		clientEntry->clientID = request.clientID;
		clientEntry->client_type = ConfigParams::NS3_SIMULATOR;
		clientEntry->requiredServices = request.requiredServices; //will come handy
		sim_mob::Services::SIM_MOB_SERVICE srv;
		int size_i = request.requiredServices.size();
		BOOST_FOREACH(srv, request.requiredServices)
		{
			switch(srv)
			{
			case sim_mob::Services::SIMMOB_SRV_TIME:{
				 PublisherList::dataType p = broker.getPublishers()[sim_mob::Services::SIMMOB_SRV_TIME];
				p->Subscribe(COMMEID_TIME, clientEntry.get(), CALLBACK_HANDLER(sim_mob::TimeEventArgs, ClientHandler::OnTime) );
				break;
			}
			case sim_mob::Services::SIMMOB_SRV_ALL_LOCATIONS:{
				PublisherList::dataType p = broker.getPublishers()[sim_mob::Services::SIMMOB_SRV_ALL_LOCATIONS];
				p->Subscribe(COMMEID_LOCATION,(void *) COMMCID_ALL_LOCATIONS, clientEntry.get()  ,CONTEXT_CALLBACK_HANDLER(AllLocationsEventArgs, ClientHandler::OnAllLocations) );
				break;
			}
			}
		}

		//also, add the client entry to broker(for message handler purposes)
		broker.insertClientList(clientEntry->clientID, ConfigParams::NS3_SIMULATOR,clientEntry);

		//send some initial configuration information to NS3
		std::set<sim_mob::Entity *> keys;
	{//multi-threaded section, need locking
		AgentsList::Mutex mutex;
		AgentsList::type & agents = broker.getRegisteredAgents(&mutex);
		AgentsList::Lock lock(mutex);
		//please mind the AgentInfo vs AgentsInfo
		AgentInfo agent;
		BOOST_FOREACH(agent, agents) {
			keys.insert(agent.agent);
		}
	}
	//no lock and const_cast at the cost of a lot of copying
		AgentsInfo info;
		info.insertInfo(AgentsInfo::ADD_AGENT, keys);
		clientEntry->cnnHandler->send(info.toJson());//send synchronously
		//start listening to the handler
		clientEntry->cnnHandler->start();
		return true;
}
NS3ClientRegistration::~NS3ClientRegistration() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
