//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "NetworkLoader.hpp"

#include <stdexcept>
#include "logging/Log.hpp"
#include "SOCI_Converters.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/vehicle/VehicleBase.hpp"

using namespace sim_mob;

NetworkLoader* NetworkLoader::networkLoader = NULL;

namespace
{
const unsigned int TWIN_STOP_ID_START = 9900000;

/**Returns the required stored procedure from the map of stored procedures*/
string getStoredProcedure(const map<string, string>& storedProcs, const string& procedureName, bool mandatory = true)
{
	//Look for the stored procedure in the map
	map<string, string>::const_iterator itProcedure = storedProcs.find(procedureName);

	//Stored procedure found, return it
	if (itProcedure != storedProcs.end())
	{
		return itProcedure->second;
	}
	//Stored procedure not found, return empty string if not mandatory
	else if (!mandatory)
	{
		return "";
	}
	//Mandatory stored procedure not found, throw exception
	else
	{
		throw std::runtime_error("Stored-procedure '" + procedureName + "' not found in the configuration file");
	}
}
}

NetworkLoader::NetworkLoader() : roadNetwork(RoadNetwork::getWritableInstance()), isNetworkLoaded(false)
{
}

NetworkLoader::~NetworkLoader()
{
	safe_delete_item(roadNetwork);
}

void NetworkLoader::loadLanes(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<Lane> lanes = (sql.prepare << "select * from " + storedProc);

	for (soci::rowset<Lane>::const_iterator itLanes = lanes.begin(); itLanes != lanes.end(); ++itLanes)
	{
		//Create new lane and add it to the segment to which it belongs
		Lane *lane = new Lane(*itLanes);
		roadNetwork->addLane(lane);
	}
}

void NetworkLoader::loadLaneConnectors(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<LaneConnector> connectors = (sql.prepare << "select * from " + storedProc);

	for (soci::rowset<LaneConnector>::const_iterator itConnectors = connectors.begin(); itConnectors != connectors.end(); ++itConnectors)
	{
		//Create new lane connector and add it to the lane to which it belongs
		LaneConnector *connector = new LaneConnector(*itConnectors);
		roadNetwork->addLaneConnector(connector);
	}
}

void NetworkLoader::loadLanePolyLines(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<PolyPoint> points = (sql.prepare << "select * from " + storedProc);

	for (soci::rowset<PolyPoint>::const_iterator itPoints = points.begin(); itPoints != points.end(); ++itPoints)
	{
		//Create new point and add it to the poly-line, to which it belongs
		PolyPoint point(*itPoints);
		roadNetwork->addLanePolyLine(point);
	}
}

void NetworkLoader::loadLinks(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<Link> links = (sql.prepare << "select * from " + storedProc);

	for (soci::rowset<Link>::const_iterator itLinks = links.begin(); itLinks != links.end(); ++itLinks)
	{
		//Create new node and add it in the map of nodes
		Link* link = new Link(*itLinks);
		Node *fromNode = link->getFromNode();
		Node *toNode = link->getToNode();
		fromNode->addDownStreamlink(link);
		toNode->addUpStreamLink(link);
		roadNetwork->addLink(link);
	}
}

void NetworkLoader::loadNodes(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<Node> nodes = (sql.prepare << "select * from " + storedProc);

	for (soci::rowset<Node>::const_iterator itNodes = nodes.begin(); itNodes != nodes.end(); ++itNodes)
	{
		//Create new node and add it in the map of nodes
		Node* node = new Node(*itNodes);
		roadNetwork->addNode(node);
	}
}

void NetworkLoader::loadRoadSegments(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<RoadSegment> segments = (sql.prepare << "select * from " + storedProc);

	for (soci::rowset<RoadSegment>::const_iterator itSegments = segments.begin(); itSegments != segments.end(); ++itSegments)
	{
		//Create new road segment and add it to the link to which it belongs
		RoadSegment *segment = new RoadSegment(*itSegments);
		roadNetwork->addRoadSegment(segment);
	}
}

void NetworkLoader::loadSegmentPolyLines(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<PolyPoint> points = (sql.prepare << "select * from " + storedProc);

	for (soci::rowset<PolyPoint>::const_iterator itPoints = points.begin(); itPoints != points.end(); ++itPoints)
	{
		//Create new point and add it to the poly-line, to which it belongs
		PolyPoint point(*itPoints);
		roadNetwork->addSegmentPolyLine(point);
	}
}

void NetworkLoader::loadTurningConflicts(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<TurningConflict> turningConflicts = (sql.prepare << "select * from " + storedProc);

	for (soci::rowset<TurningConflict>::const_iterator itTurningConflicts = turningConflicts.begin(); itTurningConflicts != turningConflicts.end(); ++itTurningConflicts)
	{
		//Create new turning conflict and add it to the turning paths to which it belongs
		TurningConflict* turningConflict = new TurningConflict(*itTurningConflicts);
		roadNetwork->addTurningConflict(turningConflict);
	}
}

void NetworkLoader::loadTurningGroups(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<TurningGroup> turningGroups = (sql.prepare << "select * from " + storedProc);

	for (soci::rowset<TurningGroup>::const_iterator itTurningGroups = turningGroups.begin(); itTurningGroups != turningGroups.end(); ++itTurningGroups)
	{
		//Create new turning group and add it in the map of turning groups
		TurningGroup* turningGroup = new TurningGroup(*itTurningGroups);
		roadNetwork->addTurningGroup(turningGroup);
	}
}

void NetworkLoader::loadTurningPaths(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<TurningPath> turningPaths = (sql.prepare << "select * from " + storedProc);

	for (soci::rowset<TurningPath>::const_iterator itTurningPaths = turningPaths.begin(); itTurningPaths != turningPaths.end(); ++itTurningPaths)
	{
		//Create new turning path and add it in the map of turning paths
		TurningPath* turningPath = new TurningPath(*itTurningPaths);
		roadNetwork->addTurningPath(turningPath);
	}
}

void NetworkLoader::loadTurningPolyLines(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<PolyPoint> points = (sql.prepare << "select * from " + storedProc);

	for (soci::rowset<PolyPoint>::const_iterator itPoints = points.begin(); itPoints != points.end(); ++itPoints)
	{
		//Create new point and add it to the poly-line, to which it belongs
		PolyPoint point(*itPoints);
		roadNetwork->addTurningPolyLine(point);
	}
}

void NetworkLoader::loadBusStops(const std::string& storedProc)
{
	sim_mob::ConfigParams& config = sim_mob::ConfigManager::GetInstanceRW().FullConfig();
	if(!config.isPublicTransitEnabled())
	{
		sim_mob::Print() << "\nWARNING: public-transit is not enabled in the config file " << std::endl;
		return;
	}

	if (storedProc.empty())
	{
		Print() << "Stored procedure to load bus stops not specified in the configuration file." 
				<< "\nBus Stops not loaded..." << std::endl;
		return;
	}

	//SQL statement
	soci::rowset<sim_mob::BusStop> stops = (sql.prepare << "select * from " + storedProc);

	for (soci::rowset<BusStop>::const_iterator itStop = stops.begin(); itStop != stops.end(); ++itStop)
	{
		if (!sim_mob::ConfigManager::GetInstance().FullConfig().isGenerateBusRoutes() && itStop->getStopName().find("Virtual Bus Stop") != std::string::npos)
		{
			continue;
		}
		
		if (!itStop->getStopStatus().compare("NOP"))
		{
			continue;
		}

		//Create new bus stop and add it to road network
		BusStop* stop = new BusStop(*itStop);

		//hackish data validation to evade errors
		if(stop->getLength() < sim_mob::BUS_LENGTH)
		{
			stop->setLength(sim_mob::BUS_LENGTH);
		}

		RoadSegment* parentSegment = const_cast<RoadSegment*>(roadNetwork->getById(roadNetwork->getMapOfIdVsRoadSegments(), stop->getRoadSegmentId()));
		roadNetwork->addBusStop(stop);

		if(stop->getReverseSectionId() != 0) // this condition is true only for bus interchange stops
		{
			//Create twin bus stop for this interchange stop and add it to road network
			BusStop* twinStop = new BusStop();
			unsigned int twinStopId = TWIN_STOP_ID_START + stop->getStopId(); // expected result is 990<orig. stop id>
			std::string twinStopCode = "twin_" + stop->getStopCode(); //"twin_<orig. stop code>
			twinStop->setVirtualStop();
			twinStop->setStopId(twinStopId);
			twinStop->setStopCode(twinStopCode);
			twinStop->setRoadSegmentId(stop->getReverseSectionId());
			twinStop->setStopName(stop->getStopName());
			twinStop->setLength(stop->getLength());
			twinStop->setOffset(stop->getOffset());
			twinStop->setReverseSectionId(stop->getRoadSegmentId());
			twinStop->setTerminalNodeId(stop->getTerminalNodeId());
			twinStop->setStopLocation(stop->getStopLocation());
			roadNetwork->addBusStop(twinStop);

			//source and sink must be determined after adding the stop since the parentSegment will be available only now
			if(stop->getParentSegment()->getParentLink()->getFromNode()->getNodeId() == stop->getTerminalNodeId())
			{
				stop->setTerminusType(sim_mob::SOURCE_TERMINUS);
				twinStop->setTerminusType(sim_mob::SINK_TERMINUS);
			}
			else if(stop->getParentSegment()->getParentLink()->getToNode()->getNodeId() == stop->getTerminalNodeId())
			{
				twinStop->setTerminusType(sim_mob::SOURCE_TERMINUS);
				stop->setTerminusType(sim_mob::SINK_TERMINUS);
			}
			else
			{
				throw std::runtime_error("Invalid assignment of terminal node for interchange bus stop");
			}
			stop->setTwinStop(twinStop);
			twinStop->setTwinStop(stop);

			//yet another hackish fix to make up for data errors
			//All bus interchanges are made long enough to accept at least 10 buses at a time
			stop->setLength(std::max(stop->getLength(), 10 * sim_mob::BUS_LENGTH));
			twinStop->setLength(std::max(twinStop->getLength(), 10 * sim_mob::BUS_LENGTH));
		}
	}
}

void NetworkLoader::loadNetwork(const string& connectionStr, const map<string, string>& storedProcs)
{
	try
	{
		sim_mob::Print() << "Database connection: ";

		//Open the connection to the database
		sql.open(soci::postgresql, connectionStr);
		
		sim_mob::Print() << "successful\nSimMobility network loading: ";

		//Load the components of the network
		
		loadNodes(getStoredProcedure(storedProcs, "nodes"));

		loadLinks(getStoredProcedure(storedProcs, "links"));

		loadRoadSegments(getStoredProcedure(storedProcs, "road_segments"));

		loadSegmentPolyLines(getStoredProcedure(storedProcs, "segment_polylines"));

		loadLanes(getStoredProcedure(storedProcs, "lanes"));

		loadLanePolyLines(getStoredProcedure(storedProcs, "lane_polylines"));

		loadLaneConnectors(getStoredProcedure(storedProcs, "lane_connectors"));
		
		loadTurningGroups(getStoredProcedure(storedProcs, "turning_groups"));

		loadTurningPaths(getStoredProcedure(storedProcs, "turning_paths"));

		loadTurningPolyLines(getStoredProcedure(storedProcs, "turning_polylines"));

		loadTurningConflicts(getStoredProcedure(storedProcs, "turning_conflicts"));
		
		loadBusStops(getStoredProcedure(storedProcs, "bus_stops", false));
		
		//Close the connection
		sql.close();

		isNetworkLoaded = true;
		sim_mob::Print() << "done\n";
	}
	catch (soci::soci_error const &err)
	{
		sim_mob::Print() << "failed\nException occurred while loading the network!\n" << err.what() << std::endl;
		exit(-1);
	}
	catch (runtime_error const &err)
	{
		sim_mob::Print() << "failed\nException occurred while loading the network!\n" << err.what() << std::endl;
		exit(-1);
	}
}

void NetworkLoader::processNetwork()
{
	//Calculate the lengths of all the links
	const std::map<unsigned int, Link *> &mapOfLinks = roadNetwork->getMapOfIdVsLinks();
	std::map<unsigned int, Link *>::const_iterator itLinks = mapOfLinks.begin();
	
	while(itLinks != mapOfLinks.end())
	{
		itLinks->second->calculateLength();
		++itLinks;
	}
}

NetworkLoader* NetworkLoader::getInstance()
{
	if(networkLoader == NULL)
	{
		networkLoader = new NetworkLoader();
	}
	
	return networkLoader;
}

void NetworkLoader::deleteInstance()
{
	safe_delete_item(networkLoader);
}
