/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "m1.h"
#include "vector3D.h"
#include "map_data.h"
#include "mapObjects.h"
#include "nodeInfo.h"
#include "X11/keysymdef.h"
#include <X11/keysym.h>
#include <iostream>

#include <stddef.h>
#include <math.h>
#include <algorithm>

std::vector<std::string> travel_instructions(std::vector<unsigned> path_segments) {
    std::vector<std::string> travel_instructions;

    unsigned threshHold = path_segments.size();

    double streetDistance = 0;

    for (unsigned i = 0; i < path_segments.size() - 1; i++) {
        StreetSegmentInfo segment1 = map -> query_streetSegInfo_from_segID(path_segments[i]);
        StreetSegmentInfo segment2 = map -> query_streetSegInfo_from_segID(path_segments[i + 1]);

        if (i == 0) {
            travel_instructions.push_back("Drive on " + map -> streetName_from_streetID[segment1.streetID]);
        }

        streetDistance += find_street_segment_length(path_segments[i]);

        std::string streetName1 = map -> streetName_from_streetID[segment1.streetID];
        std::string streetName2 = map -> streetName_from_streetID[segment2.streetID];

        if (segment1.streetID != segment2.streetID && streetName1 != "<unknown>" && streetName2 != "<unknown>") {

            if (i < threshHold) {
                //Should only happen at the first street change because i becomes too big
                streetDistance = streetDistance / 1000;
                std::stringstream stream;
                stream << std::fixed << std::setprecision(1) << streetDistance;
                std::string streetDistanceToString = stream.str();
                travel_instructions[0] += " for " + streetDistanceToString + " km.";
                threshHold = i;
            } else {
                streetDistance = streetDistance / 1000;
                std::stringstream stream;
                stream << std::fixed << std::setprecision(1) << streetDistance;
                std::string streetDistanceToString = stream.str();
                travel_instructions[travel_instructions.size() - 1] += " and drive for " + streetDistanceToString + " km.";
            }

            vector3D vector1;
            vector3D vector2;


            if (segment1.from == segment2.from || segment1.from == segment2.to) {
                vector3D tempVec(segment1.to, segment1.from);
                vector1 = tempVec;

                if (segment2.from == segment1.from) {
                    //CASE  T<---1--F F---2-->T
                    vector3D tempVec2(segment2.from, segment2.to);
                    vector2 = tempVec2;
                } else if (segment2.to == segment1.from) {
                    //CASE T<---1--F T<--2---F
                    vector3D tempVec2(segment2.to, segment2.from);
                    vector2 = tempVec2;
                }

            } else if (segment1.to == segment2.from || segment1.to == segment2.to) {
                vector3D tempVec(segment1.from, segment1.to);
                vector1 = tempVec;

                if (segment2.from == segment1.to) {
                    //CASE F---1-->T F---2-->T
                    vector3D tempVec2(segment2.from, segment2.to);
                    vector2 = tempVec2;
                } else if (segment2.to == segment1.to) {
                    //CASE F---1-->T T<--2---F
                    vector3D tempVec2(segment2.to, segment2.from);
                    vector2 = tempVec2;
                }
            }

            vector3D cross_product_vector;
            cross_product_vector.cross_product(vector1, vector2);


            if (cross_product_vector.z > 0) {
                //RIGHT TURN
                travel_instructions.push_back("Turn left turn onto " + map -> streetName_from_streetID[segment2.streetID]);

            } else if (cross_product_vector.z < 0) {
                //LEFT TURN
                travel_instructions.push_back("Turn right turn onto " + map -> streetName_from_streetID[segment2.streetID]);
            } else {
                travel_instructions.push_back("WTF HAPPENED ");
            }

            streetDistance = 0;
        } else {

        }

    }
    streetDistance += find_street_segment_length(path_segments[path_segments.size() - 1]);
    streetDistance = streetDistance / 1000;
    std::stringstream stream;
    stream << std::fixed << std::setprecision(1) << streetDistance;
    std::string streetDistanceToString = stream.str();
    travel_instructions.push_back("You will reach the destination in " + streetDistanceToString + " km.");

    return travel_instructions;
}