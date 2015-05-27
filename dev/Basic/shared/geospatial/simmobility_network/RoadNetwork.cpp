//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <stdexcept>
#include <sstream>

#include "RoadNetwork.hpp"
#include "Link.hpp"

using namespace simmobility_network;

RoadNetwork::RoadNetwork()
{
}

RoadNetwork::~RoadNetwork()
{
	//Iterate through the map of nodes and delete the nodes
	std::map<unsigned int, Node *>::iterator itNodes = mapOfIdvsNodes.begin();
	while(itNodes != mapOfIdvsNodes.end())
	{
		delete itNodes->second;
		itNodes->second = NULL;
		++itNodes;
	}
	
	mapOfIdvsNodes.clear();
	
	//Iterate through the map of links and delete the links
	std::map<unsigned int, Link *>::iterator itLinks = mapOfIdVsLinks.begin();
	while(itLinks != mapOfIdVsLinks.end())
	{
		delete itLinks->second;
		itLinks->second = NULL;
		++itLinks;
	}
	
	mapOfIdVsLinks.clear();
	
	//All other maps can simply be cleared as the 'Node' and 'Link' classes contain the others.
	//So, when they get destroyed, the objects contained within them will be destroyed 
	mapOfIdvsTurningGroups.clear();
}

const std::map<unsigned int, Link*>& RoadNetwork::getMapOfIdVsLinks() const
{
	return mapOfIdVsLinks;
}

const std::map<unsigned int, Node*>& RoadNetwork::getMapOfIdvsNodes() const
{
	return mapOfIdvsNodes;
}

const std::map<unsigned int, TurningGroup*>& RoadNetwork::getMapOfIdvsTurningGroups() const
{
	return mapOfIdvsTurningGroups;
}

const std::map<unsigned int, TurningPath*>& RoadNetwork::getMapOfIdvsTurningPaths() const
{
	return mapOfIdvsTurningPaths;
}

const std::map<unsigned int, PolyLine*>& RoadNetwork::getMapOfIdVsTurningPolyLines() const
{
	return mapOfIdVsTurningPolyLines;
}

void RoadNetwork::addLane(Lane* lane)
{
	//Find the segment to which the lane belongs
	std::map<unsigned int, RoadSegment *>::iterator itSegments = mapOfIdVsRoadSegments.find(lane->getRoadSegmentId());
	
	//Check if the segment exists in the map
	if(itSegments != mapOfIdVsRoadSegments.end())
	{
		//Add the lane to the road segment
		itSegments->second->addLane(lane);
		
		//Add the lane to the map of lanes
		mapOfIdVsLanes.insert(std::make_pair(lane->getLaneId(), lane));
	}
	else
	{
		std::stringstream msg;
		msg << "Lane " << lane->getRoadSegmentId() << " refers to an invalid segment " << lane->getRoadSegmentId();
		throw std::runtime_error(msg.str());
	}
}

void RoadNetwork::addLanePolyLine(Point* point)
{
	//Find the lane to which the poly-line belongs
	std::map<unsigned int, Lane *>::iterator itLanes = mapOfIdVsLanes.find(point->getPolyLineId());
	
	//Check if the lane exists in the map
	if(itLanes != mapOfIdVsLanes.end())
	{
		//Check if the poly-line exists for this segment
		PolyLine *polyLine = itLanes->second->getPolyLine();
		
		if(polyLine == NULL)
		{
			//No poly-line exists, so create a new one
			polyLine = new PolyLine();
			polyLine->setPolyLineId(point->getPolyLineId());
			
			//Add poly-line to the map
			itLanes->second->setPolyLine(polyLine);
		}
		
		//Add the point to the poly-line
		polyLine->addPoint(point);
	}
	else
	{
		std::stringstream msg;
		msg << "Lane poly-line " << point->getPolyLineId() << " refers to an invalid lane " << point->getPolyLineId();
		throw std::runtime_error(msg.str());
	}
}

void RoadNetwork::addLink(Link *link)
{
	mapOfIdVsLinks.insert(std::make_pair(link->getLinkId(), link));
}

void RoadNetwork::addNode(Node *node)
{
	mapOfIdvsNodes.insert(std::make_pair(node->getNodeId(), node));
}

void RoadNetwork::addRoadSegment(RoadSegment* segment)
{
	//Find the link to which the segment belongs
	std::map<unsigned int, Link *>::iterator itLinks = mapOfIdVsLinks.find(segment->getLinkId());
	
	//Check if the link exists in the map
	if(itLinks != mapOfIdVsLinks.end())
	{
		//Add the road segment to the link
		itLinks->second->addRoadSegment(segment);
		
		//Add the road segment to the map of road segments
		mapOfIdVsRoadSegments.insert(std::make_pair(segment->getRoadSegmentId(), segment));
	}
	else
	{
		std::stringstream msg;
		msg << "RoadSegment " << segment->getRoadSegmentId() << " refers to an invalid link " << segment->getLinkId();
		throw std::runtime_error(msg.str());
	}
}

void RoadNetwork::addSegmentPolyLine(Point* point)
{
	//Find the road segment to which the poly-line belongs
	std::map<unsigned int, RoadSegment *>::iterator itSegments = mapOfIdVsRoadSegments.find(point->getPolyLineId());
	
	//Check if the segment exists in the map
	if(itSegments != mapOfIdVsRoadSegments.end())
	{
		//Check if the poly-line exists for this segment
		PolyLine *polyLine = itSegments->second->getPolyLine();
		
		if(polyLine == NULL)
		{
			//No poly-line exists, so create a new one
			polyLine = new PolyLine();
			polyLine->setPolyLineId(point->getPolyLineId());
			
			//Add poly-line to the map
			itSegments->second->setPolyLine(polyLine);
		}
		
		//Add the point to the poly-line
		polyLine->addPoint(point);
	}
	else
	{
		std::stringstream msg;
		msg << "Segment poly-line " << point->getPolyLineId() << " refers to an invalid road segment " << point->getPolyLineId();
		throw std::runtime_error(msg.str());
	}
}

void RoadNetwork::addTurningConflict(TurningConflict* turningConflict)
{
	TurningPath *first = NULL, *second = NULL;
	
	//First turning
	//Find the turning path to which the conflict belongs
	std::map<unsigned int, TurningPath *>::iterator itPaths = mapOfIdvsTurningPaths.find(turningConflict->getFirstTurningId());
	
	//Check if the turning path exists in the map
	if(itPaths != mapOfIdvsTurningPaths.end())
	{
		first = itPaths->second;
		
		//Set the turning path to the conflict
		turningConflict->setFirstTurning(itPaths->second);
	}
	else
	{
		std::stringstream msg;
		msg << "Turning conflict " << turningConflict->getConflictId() << " refers to an invalid turning path " << turningConflict->getFirstTurningId();
		throw std::runtime_error(msg.str());
	}
	
	//Second turning
	//Find the turning path to which the conflict belongs
	itPaths = mapOfIdvsTurningPaths.find(turningConflict->getSecondTurningId());
	
	//Check if the turning path exists in the map
	if(itPaths != mapOfIdvsTurningPaths.end())
	{
		second = itPaths->second;
		
		//Set the turning path to the conflict
		turningConflict->setSecondTurning(itPaths->second);
	}
	else
	{
		std::stringstream msg;
		msg << "Turning conflict " << turningConflict->getConflictId() << " refers to an invalid turning path " << turningConflict->getSecondTurningId();
		throw std::runtime_error(msg.str());
	}
	
	//Add the conflict to the turning - to both the turnings
	first->addTurningConflict(second, turningConflict);
	second->addTurningConflict(first, turningConflict);
}

void RoadNetwork::addTurningGroup(TurningGroup *turningGroup)
{
	//Find the node to which the turning group belongs
	std::map<unsigned int, Node *>::iterator itNodes = mapOfIdvsNodes.find(turningGroup->getNodeId());
	
	//Check if the node exists in the map
	if(itNodes != mapOfIdvsNodes.end())
	{
		//Add the turning to the node
		itNodes->second->addTurningGroup(turningGroup);
		
		//Add the turning group to the map of turning groups
		mapOfIdvsTurningGroups.insert(std::make_pair(turningGroup->getTurningGroupId(), turningGroup));
	}
	else
	{
		std::stringstream msg;
		msg << "Turning group " << turningGroup->getTurningGroupId() << " refers to an invalid Node " << turningGroup->getNodeId();
		throw std::runtime_error(msg.str());
	}
}

void RoadNetwork::addTurningPath(TurningPath* turningPath)
{
	//Find the turning group to which the turning path belongs
	std::map<unsigned int, TurningGroup *>::iterator itGroups = mapOfIdvsTurningGroups.find(turningPath->getTurningGroupId());
	
	//Check if the turning group exists in the map
	if(itGroups != mapOfIdvsTurningGroups.end())
	{
		//Add the turning path to the turning group
		itGroups->second->addTurningPath(turningPath);
		
		//Add the turning path to the map of turning paths
		mapOfIdvsTurningPaths.insert(std::make_pair(turningPath->getTurningPathId(), turningPath));
	}
	else
	{
		std::stringstream msg;
		msg << "Turning path " << turningPath->getTurningPathId() << " refers to an invalid turning group " << turningPath->getTurningGroupId();
		throw std::runtime_error(msg.str());
	}
}

void RoadNetwork::addTurningPolyLine(Point* point)
{
	//Find the turning path to which the poly-line belongs
	std::map<unsigned int, TurningPath *>::iterator itTurnings = mapOfIdvsTurningPaths.find(point->getPolyLineId());
	
	//Check if the turning path exists in the map
	if(itTurnings != mapOfIdvsTurningPaths.end())
	{
		//Check if the poly-line exists for this turning
		PolyLine *polyLine = itTurnings->second->getPolyLine();
		
		if(polyLine == NULL)
		{
			//No poly-line exists, so create a new one
			polyLine = new PolyLine();
			polyLine->setPolyLineId(point->getPolyLineId());
			
			//Add poly-line to the map
			itTurnings->second->setPolyLine(polyLine);
		}
		
		//Add the point to the poly-line
		polyLine->addPoint(point);
	}
	else
	{
		std::stringstream msg;
		msg << "Turning poly-line " << point->getPolyLineId() << " refers to an invalid turning path " << point->getPolyLineId();
		throw std::runtime_error(msg.str());
	}
}