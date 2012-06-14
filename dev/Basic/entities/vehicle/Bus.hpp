/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * \file Bus.hpp
 *
 * \author Seth N. Hetu
 */

#pragma once

#include "Vehicle.hpp"
#include "BusRoute.hpp"
#include "entities/BusController.hpp"


namespace sim_mob {

class PackageUtils;
class UnPackageUtils;

/**
 * A bus is similar to a Vehicle, except that it follows a BusRoute and has a passenger count.
 */
class Bus : public sim_mob::Vehicle {
public:
	Bus(const BusRoute& route, const Vehicle* clone)
	: Vehicle(*clone), passengerCount(0), route(route)
	{}

	BusRoute& getRoute() { return route; }
	int getPassengerCount() const { return passengerCount; }
	void setPassengerCount(int val) { passengerCount = val; }

private:
	int passengerCount;
	BusRoute route;

	//Serialization-related friends
	friend class BusController;
	friend class PackageUtils;
	friend class UnPackageUtils;
};

} // namespace sim_mob
