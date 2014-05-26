//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Sensor.hpp"
#include "geospatial/Lane.hpp"

namespace sim_mob {

Sensor::~Sensor() {
    std::map<Lane const *, Shared<CountAndTimePair> *>::iterator iter;
    for (iter = data.begin(); iter != data.end(); ++iter)
    {
        Shared<CountAndTimePair> * pair = iter->second;
        delete pair;
    }
}

void
Sensor::buildSubscriptionList(std::vector<BufferedBase*>& subsList)
{
    std::map<Lane const *, Shared<CountAndTimePair> *>::iterator iter;
    for (iter = data.begin(); iter != data.end(); ++iter)
    {
        Shared<CountAndTimePair> * pair = iter->second;
        subsList.push_back(pair);
    }
}

Sensor::CountAndTimePair const &
Sensor::getCountAndTimePair(Lane const & lane) const
{
    std::map<Lane const *, Shared<CountAndTimePair> *>::const_iterator iter = data.find(&lane);
    if (iter != data.end())
    {
        Shared<CountAndTimePair> const * pair = iter->second;
        return pair->get();
    }
    std::ostringstream stream;
    stream << "Sensor::getCountAndTimePair() was called on invalid lane" << &lane;
    throw std::runtime_error(stream.str());
}

}