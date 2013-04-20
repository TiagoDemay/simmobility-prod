/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "BusStopAgent.hpp"
#include "entities/Person.hpp"
#include "entities/AuraManager.hpp"

using namespace std;
using namespace sim_mob;

typedef Entity::UpdateStatus UpdateStatus;

BusStopAgent::All_BusStopAgents BusStopAgent::all_BusstopAgents_;

void sim_mob::BusStopAgent::RegisterNewBusStopAgent(BusStop& busstop, const MutexStrategy& mtxStrat)
{
	//BusController* busctrller = new sim_mob::BusController(-1, mtxStrat);
	BusStopAgent* sig_ag = new BusStopAgent(busstop, mtxStrat);
	sig_ag->setBusStopAgentNo(busstop.getBusstopno_());
	busstop.generatedBusStopAgent = sig_ag;
	all_BusstopAgents_.push_back(sig_ag);
}

bool sim_mob::BusStopAgent::HasBusStopAgents()
{
	return !all_BusstopAgents_.empty();
}

void sim_mob::BusStopAgent::PlaceAllBusStopAgents(std::vector<sim_mob::Entity*>& agents_list)
{
	std::cout << "all_BusStopAgents size: " << all_BusstopAgents_.size() << std::endl;
	//Push every BusStopAgent on the list into the agents array as an active agent
	for (vector<BusStopAgent*>::iterator it=all_BusstopAgents_.begin(); it!=all_BusstopAgents_.end(); it++) {
		agents_list.push_back(*it);
	}
}

BusStopAgent* sim_mob::BusStopAgent::findBusStopAgentByBusStop(const BusStop* busstop)
{
	for (int i=0; i<all_BusstopAgents_.size(); i++) {
		if(all_BusstopAgents_[i]->getBusStop().getBusstopno_() == busstop->getBusstopno_()) {
			return all_BusstopAgents_[i];
		}
	}

	return nullptr;
}

BusStopAgent* sim_mob::BusStopAgent::findBusStopAgentByBusStopNo(const std::string& busstopno)
{
	for (int i=0; i<all_BusstopAgents_.size(); i++) {
		if(all_BusstopAgents_[i]->getBusStop().getBusstopno_() == busstopno) {
			return all_BusstopAgents_[i];
		}
	}

	return nullptr;
}

void sim_mob::BusStopAgent::buildSubscriptionList(vector<BufferedBase*>& subsList)
{
	Agent::buildSubscriptionList(subsList);
}

bool sim_mob::BusStopAgent::frame_init(timeslice now)
{
	return true;
}

void sim_mob::BusStopAgent::frame_output(timeslice now)
{
	if(now.frame() % 100 == 0) {// every 100 frames(10000ms-->10s) output
		LogOut("(\"BusStopAgent\""
			<<","<<now.frame()
			<<","<<getId()
			<<",{" <<"\"BusStopAgent no\":\""<<busstopAgentno_<<"\"})"<<std::endl);
	}
}

Entity::UpdateStatus sim_mob::BusStopAgent::frame_tick(timeslice now)
{
	const Agent* Persontemp = nullptr;
	if(now.frame() % 50 == 0) {// every 100 frames(10000ms-->10s) check AuraManager
		vector<const Agent*> nearby_agents = AuraManager::instance().agentsInRect(Point2D((busstop_.xPos - 3500),(busstop_.yPos - 3500)),Point2D((busstop_.xPos + 3500),(busstop_.yPos + 3500)));
		std::cout << "nearby_agents size: " << nearby_agents.size() << std::endl;
		for (vector<const Agent*>::iterator it = nearby_agents.begin();it != nearby_agents.end(); it++)
		{
			//Retrieve only Passenger agents.
		 	const Person* person = dynamic_cast<const Person *>(*it);
		 	Person* p = const_cast<Person *>(person);
		 	Persontemp = (*it);
		 	WaitBusActivityRole* waitbusactivityRole = p ? dynamic_cast<WaitBusActivityRole*>(p->getTempRole()) : nullptr;
		 	if(waitbusactivityRole) {
		 		if((!waitbusactivityRole->getRegisteredFlag()) && (waitbusactivityRole->getBusStopAgent() == this)) {// not registered and waiting in this BusStopAgent
		 			boarding_WaitBusActivities.push_back(waitbusactivityRole);
		 			waitbusactivityRole->setRegisteredFlag(true);// set this person's role to be registered
		 		}
		 			//std::cout << "WaitBusActivity: " << waitbusactivity->getParent()->getId() << std::endl;
		 	}
		}
		sort(boarding_WaitBusActivities.begin(),boarding_WaitBusActivities.end(),less_than_TimeOfReachingBusStop());
//		cout << "boarding_WaitBusActivities.size(): " << boarding_WaitBusActivities.size() << std::endl;
//		for(int i = 0 ; i < boarding_WaitBusActivities.size(); i ++) {
//			cout << "(" << boarding_WaitBusActivities[i]->getTimeOfReachingBusStop() << ")\n";
//		}
	}

	return Entity::UpdateStatus::Continue;
}
