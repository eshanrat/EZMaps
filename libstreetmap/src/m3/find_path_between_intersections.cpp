#include "m2.h"
#include "m1.h"
#include "m3.h"


#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "mapObjects.h"
#include "nodeInfo.h"
#include "X11/keysymdef.h"
#include <X11/keysym.h>

#include <stddef.h>
#include "graphics.h"
#include "OSMDatabaseAPI.h"
#include <math.h>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <unordered_map>


#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/register/point.hpp> 
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <limits>
#include <X11/Xutil.h>

#include <bits/stdc++.h>
#include "make_path.cpp"

#include "nodeInfo.h"
#include "map_data.h"

class CompareTimeGreater {
public:

    bool operator()(const std::pair<unsigned, double>& pair1, const std::pair<unsigned, double> &pair2) {
        return (pair1.second > pair2.second);
    }
};

double compute_path_travel_time(const std::vector <unsigned> & path, const double turn_penalty) {

    double time = 0;
    double turnCount = 0;

    if (path.size() == 0)
        return 0;
    else {

        //computing length of each segment
        for (unsigned i = 0; i < path.size(); i++) {

            time += find_street_segment_travel_time(path[i]);

            if (i > 0 && map -> query_streetSegInfo_from_segID(path[i]).streetID != map -> query_streetSegInfo_from_segID(path[i - 1]).streetID)
                turnCount++;
        }
        time = time + turn_penalty * turnCount;
        return time;
    }
}

std::vector<unsigned> find_path_between_intersections(unsigned intersect_id_start, unsigned intersect_id_end, const double turn_penalty) {

    //stores the final path
    std::vector<unsigned> finalPath;

    //vector with indexes corresponding to all the nodes in the map
    std::vector<node> set(getNumberOfIntersections());
    //std::vector<node> closedSet(getNumberOfIntersections());

    //priority queue stores the score and the node id
    std::priority_queue<std::pair<double, unsigned>, std::vector<std::pair<double, unsigned>>, std::greater<std::pair<double, unsigned>>> path;


    //initializing
    node start(intersect_id_start, 0, intersect_id_start, intersect_id_end);

    set[intersect_id_start] = start;
    start.travelTime = find_street_segment_travel_time(0);
    path.emplace(start.travelTime + start.timeFromDestination, intersect_id_start); //double is the total travel time

    //while priority queue is not empty
    while (!path.empty()) {

        //If already expanded, skip
        unsigned topNodeID = path.top().second;
        node topNodePriority = set[topNodeID];

        //if it is true, means node has already been expanded 
        if (topNodePriority.expanded == true) {
            path.pop();
            continue;
        }

        //getting a copy of the top before popping
        unsigned intersectID = path.top().second;
        node current = set[intersectID];

        //Update all vectors and queue
        path.pop();
        (set[intersectID]).expanded = true;
        //closedSet[intersectID] = set[intersectID];

        if (intersectID == intersect_id_end) {

            finalPath = make_path(set, intersect_id_start, intersect_id_end);
            return finalPath;
        }
        std::vector<unsigned> segs = map -> query_intersection_street_segments(intersectID);
        //Expansion
        for (unsigned i = 0; i < segs.size(); i++) {
            unsigned next = 0;
            StreetSegmentInfo info = map -> query_streetSegInfo_from_segID(segs[i]);
            unsigned tempSegment = segs[i];
            //Chick path legality
            if (intersectID == info.from) {
                next = info.to;
            } else if (!info.oneWay) {
                next = info.from;
            } else
                continue;
            node temp(intersectID, tempSegment, next, intersect_id_end);

            temp.travelTime += current.travelTime;

            StreetSegmentInfo currentInfo = map -> query_streetSegInfo_from_segID(current.fromSegment);
            StreetSegmentInfo tempInfo = map -> query_streetSegInfo_from_segID(temp.fromSegment);
            if (currentInfo.streetID != tempInfo.streetID)
                temp.travelTime += turn_penalty;

            //skip if already expanded
            if ((set[next]).expanded == true) {
                continue;
            } else if (((set[next]).expanded == false) && (set[next]).travelTime < temp.travelTime) {
                continue;
            }

            //store the required nodes in correct form
            (set[next]).expanded = true;
            set[next] = temp;
            path.emplace(temp.travelTime + temp.timeFromDestination, next);
        }
    }
    //If vector is empty i.e. not found end id
    return finalPath;
}

std::vector<unsigned> find_path_to_point_of_interest(const unsigned intersect_id_start, const std::string point_of_interest_name, const double turn_penalty) {

    //priority queue stores time and the node id
    std::priority_queue<std::pair<double, unsigned>, std::vector<std::pair<double, unsigned>>, std::greater<std::pair<double, unsigned>>> path;

    std::vector<unsigned> positions = map -> NameAndIntersectionPOI[point_of_interest_name];

    
    for (unsigned i = 0; i < positions.size(); i++) {
        //std::cout << positions[i] << std::endl;
        //if same
        if (intersect_id_start == positions[i]) {
            std::vector<unsigned> temp;
            return temp;
        }
    }

    std::vector<unsigned> finalPath;
    std::vector<node> set(getNumberOfIntersections());


    node start(intersect_id_start, 0);

    set[intersect_id_start] = start;
    start.travelTime = find_street_segment_travel_time(0);
    //start.travelTime = find_street_segment_travel_time(0);
    path.emplace(start.travelTime, intersect_id_start); //double is the total travel time

    std::vector<unsigned> ::iterator destination_found;
    //while priority queue is not empty
    while (!path.empty()) {

        //If already expanded, skip
        unsigned topNodeID = path.top().second;
        node topNodePriority = set[topNodeID];

        if (topNodePriority.expanded == true) {
            path.pop();
            continue;
        }
        //storing before popping off
        unsigned intersectID = path.top().second;
        
        node current = set[intersectID];
        path.pop();
        (set[intersectID]).expanded = true;
                                                                                        \
        //place it in the closed set
        //closedSet[intersectID] = set[intersectID];
        destination_found = std::find(positions.begin(), positions.end(), intersectID);
        if (destination_found != positions.end()) {

            finalPath = make_path(set, intersect_id_start, intersectID);
            
            return finalPath;

        }
        std::vector<unsigned> segs = map -> query_intersection_street_segments(intersectID);

        for (unsigned i = 0; i < segs.size(); i++) {
            unsigned next = 0;
            StreetSegmentInfo info = map -> query_streetSegInfo_from_segID(segs[i]);
            unsigned tempSegment = segs[i];
            //Chick path legality
            if (intersectID == info.from) {
                next = info.to;
            } else if (!info.oneWay) {
                next = info.from;
            } else
                continue;
            node temp(intersectID, tempSegment);

            temp.travelTime += current.travelTime;

            StreetSegmentInfo currentInfo = map -> query_streetSegInfo_from_segID(current.fromSegment);
            StreetSegmentInfo tempInfo = map -> query_streetSegInfo_from_segID(temp.fromSegment);
            if (currentInfo.streetID != tempInfo.streetID)
                temp.travelTime += turn_penalty;

            //skip if already expanded
            if ((set[next]).expanded == true) {

                continue;
            } else if (((set[next]).expanded == false) && (set[next]).travelTime < temp.travelTime) {

                continue;
            }

            //store the required nodes in correct form
            (set[next]).expanded = true;
            set[next] = temp;
            path.emplace(temp.travelTime, next);
        }
    }
    //If vector is empty i.e. not found end id
    return finalPath;
}