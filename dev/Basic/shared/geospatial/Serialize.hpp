//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/**
 * \file Serialize.hpp

 * This file contains a set of serialization functions (boost::serialize) for the Geospatial road
 *   network classes which are compatible with boost::xml.
 * Be careful when including this file, as it has a large number of dependencies coupled with template
 *   expansion.
 *
 * \note
 * In terms of justification for why we put all of these functions outside of their respective classes,
 * there are three. First, I don't want tiny classes like Point and Node having boost::xml as a
 * dependency. Second, no "friends" are needed if the proper accessors are provided. Third, a friend
 * class would have been needed anyway in boost::serialization::access, so we might as well remove
 * these functions. The downside is that changes to the RoadNetwork classes will force Serialize.hpp to
 * recompile, but that is mostly unavoidable (and the classes are mostly stable now anyway). If performance
 * really becomes a problem, we can force the template into a separate cpp file (although it's messy).
 *
 * \note
 * These serialization functions are used for *writing* only; they are untested and unintended for reading.
 *
 * \author Seth N. Hetu
 */


#pragma once

#include <set>
#include <vector>
#include <iostream>
#include <stdexcept>

#include <boost/foreach.hpp>

#include "util/XmlWriter.hpp"
#include "metrics/Length.hpp"

#include "geospatial/network/RoadNetwork.hpp"
#include "geospatial/network/RoadItem.hpp"
#include "geospatial/network/Link.hpp"
#include "geospatial/network/Lane.hpp"
#include "geospatial/network/LaneConnector.hpp"
#include "geospatial/network/Point.hpp"
#include "geospatial/network/PT_Stop.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "entities/signal/Signal.hpp"


namespace sim_mob {
namespace xml {

/////////////////////////////////////////////////////////////////////
// get_id()
/////////////////////////////////////////////////////////////////////

//Disallow get_id for Point
ERASE_GET_ID(sim_mob::Point);
ERASE_GET_ID(sim_mob::TrafficColor);

//Simple versions of get_id for most classes.
SPECIALIZE_GET_ID(sim_mob::RoadSegment, getRoadSegmentId);
SPECIALIZE_GET_ID(sim_mob::Lane,        getLaneId);
SPECIALIZE_GET_ID(sim_mob::Link,        getLinkId);
//SPECIALIZE_GET_ID(Node,   getID);
//SPECIALIZE_GET_ID(sim_mob::UniNode,     getID);
SPECIALIZE_GET_ID(sim_mob::Node,        getNodeId);
//SPECIALIZE_GET_ID(sim_mob::RoadItem,    getRoadItemID);
SPECIALIZE_GET_ID(sim_mob::Signal,    getSignalId);
//SPECIALIZE_GET_ID(sim_mob::LinkAndCrossing,    getId);
//SPECIALIZE_GET_ID(sim_mob::Crossing,    getRoadItemID);
SPECIALIZE_GET_ID(sim_mob::Phase,    getPhaseName);

//UNDEFINED_GET_ID(sim_mob::TrafficColor);

//get_id for Lane Connectors is more complicated than our macro can handle.
template <>
std::string get_id(const sim_mob::LaneConnector& lc)
{
    return boost::lexical_cast<std::string>(lc.getFromLaneId()) + ":" + boost::lexical_cast<std::string>(lc.getToLaneId());
}




/////////////////////////////////////////////////////////////////////
// write_xml() - Dispatch
//               Provide a sensible default for pairs of lane connectors (laneFrom/laneTo by ID)
/////////////////////////////////////////////////////////////////////
template <>
void write_xml(XmlWriter& write, const std::pair<const sim_mob::Lane*, const sim_mob::Lane* >& connectors)
{
    write_xml(write, connectors, namer("<laneFrom,laneTo>"), expander("<id,id>"));
}
template <>
void write_xml(XmlWriter& write, const std::pair<const sim_mob::Lane*, sim_mob::Lane* >& connectors)
{
    write_xml(write, std::pair<const sim_mob::Lane*, const sim_mob::Lane*>(connectors.first,connectors.second));
}
template <>
void write_xml(XmlWriter& write, const std::pair<sim_mob::Lane*, const sim_mob::Lane* >& connectors)
{
    write_xml(write, std::pair<const sim_mob::Lane*, const sim_mob::Lane*>(connectors.first,connectors.second));
}
template <>
void write_xml(XmlWriter& write, const std::pair<sim_mob::Lane*, sim_mob::Lane* >& connectors)
{
    write_xml(write, std::pair<const sim_mob::Lane*, const sim_mob::Lane*>(connectors.first,connectors.second));
}


/////////////////////////////////////////////////////////////////////
// write_xml() - Dispatch
//               Treat vectors of Point2Ds as poylines.
/////////////////////////////////////////////////////////////////////
template <>
void write_xml(XmlWriter& write, const std::vector<sim_mob::Point>& poly)
{
    int i=0;
    for (std::vector<sim_mob::Point>::const_iterator it=poly.begin(); it!=poly.end(); it++) {
        write.prop_begin("PolyPoint");
        write.prop("pointID", i++);
        write.prop("location", *it);
        write.prop_end();
    }
}


/////////////////////////////////////////////////////////////////////
// WORKAROUND: Currently, the new syntax can't handle certain STL combinations
//             with enough detail, so we have to provide this fallback here.
/////////////////////////////////////////////////////////////////////
template <>
void write_xml(XmlWriter& write, const std::vector< std::pair<const sim_mob::Lane*, sim_mob::Lane* > > & connectors)
{
    write_xml(write, connectors, namer("<Connector>"));
}


/////////////////////////////////////////////////////////////////////
// write_xml()
/////////////////////////////////////////////////////////////////////

template <>
void write_xml(XmlWriter& write, const sim_mob::Link& lnk)
{
    write.prop("linkID", lnk.getLinkId());
    write.prop("roadName", lnk.getRoadName());

    //TODO: This is a workaround at the moment. In reality, "<id>" should do all the work, but since
    //      we can't currently handle IDs of value types, the expander is actually ignored!
    write.prop("StartingNode", lnk.getFromNodeId(), namer(), expander("<id>"), false);
    write.prop("EndingNode", lnk.getToNodeId(), namer(), expander("<id>"), false);

    write.prop("Segments", lnk.getRoadSegments(), namer("<Segment>"));
}


//Another workaround needed for lane polylines.
namespace {
std::vector< std::pair<int, std::vector<Point> > > wrap_lanes(const std::vector< std::vector<Point> >& lanes)
{
    int i=0;
    std::vector< std::pair<int, std::vector<Point> > > res;
    for (std::vector< std::vector<Point> >::const_iterator it=lanes.begin(); it!=lanes.end(); it++) {
        res.push_back(std::make_pair(i++, *it));
    }
    return res;
}
} //End anon namespace


template <>
void write_xml(XmlWriter& write, const sim_mob::RoadSegment& rs)
{
    write.prop("segmentID", rs.getRoadSegmentId());

    //TODO: Similar workaround

    write.prop("maxSpeed", rs.getMaxSpeed());
    write.prop("Length", rs.getLength());

    //NOTE: We don't pass a namer in here, since vectors<> of Point2Ds are a special case.
    write.prop("polyline", rs.getPolyLine());
    std::vector< std::vector<Point> > laneLines;
    for (size_t i=0; i<=rs.getLanes().size(); i++) {
        laneLines.push_back(*(const_cast<sim_mob::RoadSegment&>(rs).getLane(i)->getPolyLine()));
    }
    write.prop("laneEdgePolylines_cached", wrap_lanes(laneLines), namer("<laneEdgePolyline_cached,<laneNumber,polyline>>"));
    write.prop("Lanes", rs.getLanes(), namer("<Lane>"));
    //write.prop("Obstacles", rs.getObstacles());

}

template <>
void write_xml(XmlWriter& write, const sim_mob::Lane& ln)
{
    write.prop("laneID", ln.getLaneId());
    write.prop("width", ln.getWidth());
    write.prop("is_road_shoulder", ln.doesLaneHaveRoadShoulder());
    write.prop("is_bicycle_lane", ln.isBicycleLane());
    write.prop("is_pedestrian_lane", ln.isPedestrianLane());
    /*write.prop("is_vehicle_lane", ln.is_vehicle_lane());
    write.prop("is_standard_bus_lane", ln.is_standard_bus_lane());
    write.prop("is_whole_day_bus_lane", ln.is_whole_day_bus_lane());
    write.prop("is_high_occupancy_vehicle_lane", ln.is_high_occupancy_vehicle_lane());
    write.prop("can_freely_park_here", ln.can_freely_park_here());
    write.prop("can_stop_here", ln.can_stop_here());
    write.prop("is_u_turn_allowed", ln.is_u_turn_allowed());*/
    write.prop("PolyLine", ln.getPolyLine());
}


template <>
void write_xml(XmlWriter& write, const sim_mob::Node& nd)
{
    write.prop("nodeID", nd.getNodeId());
    write.prop("location", nd.getLocation());
}

template <>
void write_xml(XmlWriter& write, const sim_mob::RoadItem& ri)
{
    throw std::runtime_error("RoadItems by themselves can't be serialized; try putting them in an Obstacle map.");
}

/*
//NOTE: This is another workaround for dealing with heterogeneous types.
//NOTE: It also deals with out-of-order properties.
template <>
void write_xml(XmlWriter& write, const std::map<sim_mob::centimeter_t, const sim_mob::RoadItem*>& obstacles, namer name, expander expand)
{
    for (std::map<centimeter_t, const RoadItem*>::const_iterator it=obstacles.begin(); it!=obstacles.end(); it++) {
        if (dynamic_cast<const Crossing*>(it->second)) {
            const Crossing* cr = dynamic_cast<const Crossing*>(it->second);
            write.prop_begin("Crossing");
            write.prop("id", cr->getRoadItemID());
            write.prop("Offset", it->first);
            write.prop("start", cr->getStart());
            write.prop("end", cr->getEnd());
            write.prop("nearLine", cr->nearLine);
            write.prop("farLine", cr->farLine);
            write.prop_end();
        } else if (dynamic_cast<const BusStop*>(it->second)) {
            const BusStop* bs = dynamic_cast<const BusStop*>(it->second);
            write.prop_begin("BusStop");
            write.prop("id", bs->getRoadItemID());
            write.prop("Offset", it->first);
            write.prop("start", bs->getStart());
            write.prop("end", bs->getEnd());
            write.prop("xPos", bs->xPos);
            write.prop("yPos", bs->yPos);
            write.prop("is_terminal", bs->is_terminal);
            write.prop("is_bay", bs->is_bay);
            write.prop("has_shelter", bs->has_shelter);
            write.prop("busCapacityAsLength", bs->busCapacityAsLength);
            write.prop("busstopno", bs->getRoadItemId());
            write.prop_end();
        } else {
            throw std::runtime_error("Unidentified RoadItem subclass.");
        }
    }
}
template <>
void write_xml(XmlWriter& write, const std::map<sim_mob::centimeter_t, const sim_mob::RoadItem*>& obstacles, namer name)
{
    write_xml(write, obstacles, name, expander());
}
template <>
void write_xml(XmlWriter& write, const std::map<sim_mob::centimeter_t, const sim_mob::RoadItem*>& obstacles, expander expand)
{
    write_xml(write, obstacles, namer("<item,<key,value>>"), expand);
}
template <>
void write_xml(XmlWriter& write, const std::map<sim_mob::centimeter_t, const sim_mob::RoadItem*>& obstacles)
{
    write_xml(write, obstacles, namer("<item,<key,value>>"), expander());
}
*/

template <>
void write_xml(XmlWriter& write, const sim_mob::Point& pt)
{
    write.prop("xPos", pt.getX());
    write.prop("yPos", pt.getY());
}


//Workaround:
//Out multi-node connectors are not actually represented in the format we'd expect.
/*
namespace {
std::map<const sim_mob::RoadSegment*, std::vector< std::pair<const sim_mob::Lane*, sim_mob::Lane*> > > warp_multi_connectors(const std::map<const sim_mob::RoadSegment*, std::set<sim_mob::LaneConnector*> > & connectors)
{
    std::map<const sim_mob::RoadSegment*, std::vector< std::pair<const sim_mob::Lane*, sim_mob::Lane*> > > res;
    for (std::map<const sim_mob::RoadSegment*, std::set<sim_mob::LaneConnector*> >::const_iterator it=connectors.begin(); it!=connectors.end(); it++) {
        for (std::set<sim_mob::LaneConnector*>::const_iterator lcIt=it->second.begin(); lcIt!=it->second.end(); lcIt++) {
            res[it->first].push_back(std::make_pair((*lcIt)->getLaneFrom(), const_cast<sim_mob::Lane*>((*lcIt)->getLaneTo())));
        }
    }
    return res;
}
} //End un-named namespace

template <>
void write_xml(XmlWriter& write, const sim_mob::Intersection& in)
{
    write_xml(write, dynamic_cast<const sim_mob::Node&>(in));
    write.prop("roadSegmentsAt", in.getRoadSegments(), namer("<segmentID>"), expander("<id>"));
    write.prop("Connectors", warp_multi_connectors(in.getConnectors()), namer("<MultiConnectors,<RoadSegment,Connectors>>"), expander("<*,<id,*>>"));

}

template <>
void write_xml(XmlWriter& write, const Node& mnd)
{
    //Try to dispatch to intersection.
    //TODO: This will fail when our list of Nodes also contains roundabouts.
    write_xml(write, dynamic_cast<const sim_mob::Intersection&>(mnd));
}

template <>
void write_xml(XmlWriter& write, const sim_mob::UniNode& und)
{
    write_xml(write, dynamic_cast<const sim_mob::Node&>(und));
    write.prop("firstPair", und.firstPair, expander("<id,id>"));
    if (und.secondPair.first && und.secondPair.second) {
        write.prop("secondPair", und.secondPair, expander("<id,id>"));
    }
    write.prop("Connectors", und.getConnectors(), namer("<Connector,<laneFrom,laneTo>>"), expander("<*,<id,id>>"));
}
*/

template <>
void write_xml(XmlWriter& write, const sim_mob::RoadNetwork& rn)
{
    /*
    //Start writing
    write.prop_begin("RoadNetwork");

    //Nodes are also wrapped
    write.prop_begin("Nodes");
    write.prop("UniNodes", rn.getUniNodes(), namer("<UniNode>"));

    //TODO: This will fail unless getNodes() returns ONLY intersections.
    write.prop("Intersections", rn.getNodes(), namer("<Intersection>"));
    write.prop_end(); //Nodes

    write.prop("Links", rn.getLinks(), namer("<Link>"));
    write.prop_end(); //RoadNetwork
    */
}

template <>
void write_xml(XmlWriter& write, const sim_mob::LinkAndCrossing& value)
{
    write.prop("ID", value.id);
    if(value.link){
        write.prop("linkID", value.link, namer(), expander("<id>"), false);
    }
    /*if(value.crossing){
        write.prop("crossingID", value.crossing->getRoadItemID());
    }*/
    write.prop("angle", value.angle);
}

template <>
void write_xml(XmlWriter& write, const sim_mob::TrafficColor &value) {
    write.prop("TrafficColor", value);
}

template <>
void write_xml(XmlWriter& write, const sim_mob::ColorSequence &value) {
    std::string type = (
            value.getTrafficLightType() == Driver_Light ? "Driver_Light" :
            (value.getTrafficLightType() == Pedestrian_Light ?
                    "Pedestrian_Light" : "InvalidTrafficLightType"));
    write.prop("TrafficLightType", type);
    const std::vector< std::pair<TrafficColor,int> > & cd = value.getColorDuration();
    std::vector< std::pair<TrafficColor,int> >::const_iterator it(cd.begin()), it_end(cd.end());
    for(; it != it_end; it++){
        write.prop_begin("ColorDuration");
        write.prop("TrafficColor", ColorSequence::getTrafficLightColorString(it->first));
        write.prop("Duration", it->second);
        write.prop_end();
    }
}

namespace {
/**
 * Constant strings for the following method
 */
const std::string  TAG_SPLIT_PLAN_ID = "splitplanID";
const std::string  TAG_CYCLE_LENGTH = "cycleLength";
const std::string  TAG_OFFSET = "offset";
const std::string  TAG_CHOICE_SET = "ChoiceSet";
const std::string  TAG_PLAN = "plan";
const std::string  TAG_PLAN_ID = "planID";
const std::string  TAG_PHASE_PERCENTAGE = "PhasePercentage";

const std::string  TAG_SINGNAL_ID = "signalID";
const std::string  TAG_NODE_ID = "nodeID";
const std::string  TAG_LINK_AND_CROSSING = "linkAndCrossings";
const std::string  TAG_PHASES = "phases";
const std::string  TAG_SCATS = "SCATS";
const std::string  TAG_SIGNAL_TIMING_MODE = "signalTimingMode";
const std::string  TAG_SPLIT_PLAN = "SplitPlan";
const std::string  _STM_ADAPTIVE = "STM_ADAPTIVE";

}
template<>
void write_xml(XmlWriter& write, const sim_mob::Phase& phase) {
    //name
    write.prop("name", phase.getPhaseName());
    
    //links_maps
    const links_map &data = phase.getLinkMaps();
    links_map::const_iterator it(data.begin()), it_end(data.end());
    write.prop_begin("links_maps");
    for (; it != it_end; it++) {

        write.prop_begin("links_map");
        if (it->first && it->second.LinkTo) {
            write.prop("LinkFrom", it->first, namer(), expander("<id>"), false);
            write.prop("LinkTo", it->second.LinkTo, namer(), expander("<id>"),
                    false);
        }
        if (it->second.RS_From && it->second.RS_To) {
            write.prop("SegmentFrom", it->second.RS_From, expander("<id>"),
                    false);
            write.prop("SegmentTo", it->second.RS_To, namer(), expander("<id>"),
                    false);
        }
        write.prop("ColorSequence", it->second.colorSequence);
        write.prop_end();
    }
    write.prop_end();

    /*
    {
        //crossings_maps
        const crossings_map &data = phase.getCrossingMaps();
        crossings_map::const_iterator it(data.begin()), it_end(data.end());
        write.prop_begin("crossings_maps");
        for (; it != it_end; it++) {

            write.prop_begin("crossings_map");
            if (it->first && it->second.link) {
                write.prop("linkID", it->second.link, namer(), expander("<id>"),
                        false);
                write.prop("crossingID", it->first->getRoadItemID());
            }
            write.prop("ColorSequence", it->second.colorSequence);
            write.prop_end();
        }
        write.prop_end();
    }
    */
}

template <>
void write_xml(XmlWriter& write, const sim_mob::SplitPlan & plan)
{
    write.prop(TAG_SPLIT_PLAN_ID, plan.TMP_PlanID);
    write.prop(TAG_CYCLE_LENGTH, plan.getCycleLength());
    write.prop(TAG_OFFSET, plan.getOffset());
    //split plan expanded here only
    write.prop_begin(TAG_CHOICE_SET);
    std::vector< double > outer;
    int i = 0;
    BOOST_FOREACH(outer, plan.getChoiceSet()){
        write.prop_begin(TAG_PLAN);
        write.prop(TAG_PLAN_ID, i++);
        double inner;
        BOOST_FOREACH(inner, outer){
            write.prop(TAG_PHASE_PERCENTAGE, inner);
        }
        write.prop_end();
    }
    write.prop_end();


}

template <>
void write_xml(XmlWriter& write, const sim_mob::Signal& signal)
{
    write.prop(TAG_SINGNAL_ID, signal.getSignalId());
    write.prop(TAG_NODE_ID, signal.getNode().getNodeId());
    write.prop(TAG_LINK_AND_CROSSING, signal.getLinkAndCrossing(), namer("<linkAndCrossing>"));
    write.prop(TAG_PHASES, signal.getPhases(), namer("<phase>"));
    //hard coding
    try {
        const sim_mob::Signal_SCATS & scats =
                dynamic_cast<const sim_mob::Signal_SCATS &>(signal);
        write.prop_begin(TAG_SCATS);
        write.prop(TAG_SIGNAL_TIMING_MODE, _STM_ADAPTIVE); //hardcode until using a decent enum in signal.hpp
        write.prop(TAG_SPLIT_PLAN, scats.getPlan());
    } catch (std::exception e) {
        throw std::runtime_error("Error serializing SCATS signal");
    }
    write.prop_end();
}



}}
