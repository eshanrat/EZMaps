/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   map_data.cpp
 * Author: khanzaf2
 *
 * Created on February 1, 2018, 10:41 PM
 */

#include "map_data.h"
#include "mapObjects.h"
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include <math.h>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <set>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/register/point.hpp> 
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <limits>

//float area (std::vector<LatLon>);
double cartConvLong(double, double);

double cartConvLat(double);

double average_lat = 0;
double maximum_lat = 0;
double minimum_lat = 0;
double maximum_long = 0;
double minimum_long = 0;

float maxSpeedLimit;

map_data::map_data() {

    double maximum_lat = getIntersectionPosition(0).lat();
    double minimum_lat = maximum_lat;

    double maximum_long = getIntersectionPosition(0).lon();
    double minimum_long = maximum_long;

    intersections.resize(getNumberOfIntersections());

    //Load data structures


    for (unsigned i = 0; i < getNumberOfStreets(); ++i) {
        segmentIDs_from_streetID.push_back(std::vector<unsigned>());
        intersectionIDs_from_streetID.push_back(std::vector<unsigned>());

        std::string streetName = getStreetName(i);
        streetIDs_from_Name[streetName].push_back(i);
        streetName_from_streetID.push_back(streetName);
        combinedNames_from_ID.push_back(streetName);
    }

    for (unsigned i = 0; i < getNumberOfIntersections(); i++) {
        intersection_street_segments.push_back(std::vector<unsigned>());
        streetNames_from_Intersection.push_back(std::vector<std::string>());
        intersectionNames_from_IDs.push_back(getIntersectionName(i));
        
        for (unsigned j = 0; j < getIntersectionStreetSegmentCount(i); j++) {

            //Load in intersection_street_segments database
            auto const segmentID = getIntersectionStreetSegment(i, j);
            intersection_street_segments[i].push_back(segmentID);

            StreetSegmentInfo streetSegment = getStreetSegmentInfo(segmentID);
            unsigned street_id = streetSegment.streetID;
            std::string streetName = getStreetName(street_id);

            //Load in hash tables intersectionIDs from the streetID
            intersectionIDs_from_streetID[street_id].push_back(i);

            //Load in hash tables intersectionIDs from the street name

            streetNames_from_Intersection[i].push_back(streetName);

        }
        
        intersectionIDs_from_intersectionName[getIntersectionName(i)] = i;

        //Load in rTree
        LatLon insertIDLatLon = getIntersectionPosition(i);
        double lat = insertIDLatLon.lat();
        double lon = insertIDLatLon.lon();

        intersectionRTree.insert(std::make_pair(rTreeLatLon(lat, lon), i));

        pos_from_intersectionID.push_back(insertIDLatLon);

        intersections[i].position = insertIDLatLon;
        intersections[i].name = getIntersectionName(i);

        maximum_lat = std::max(maximum_lat, intersections[i].position.lat());
        minimum_lat = std::min(minimum_lat, intersections[i].position.lat());

        maximum_long = std::max(maximum_long, intersections[i].position.lon());
        minimum_long = std::min(minimum_long, intersections[i].position.lon());

        //nodeInfo(double _segmentTravelTime, unsigned _nodeID, unsigned _edge, unsigned _destination, unsigned _previousIndex, double _timeFromDestination);
        //nodeInfo_from_intersection_id.push_back(nodeInfo(INFINITY, i, , , , ));

    }

    average_lat = 0.5 * (maximum_lat + minimum_lat);

    //Load in data structure of segment length and speedlimit

    for (unsigned i = 0; i < getNumberOfWays(); i++) {
        const OSMWay* wayPointer = getWayByIndex(i);
        OSMID osm_ID = wayPointer->id();

        unsigned tagCount = getTagCount(wayPointer);

        std::unordered_map<std::string, std::vector < std::string>> OSMWayHash;

        for (unsigned i = 0; i < tagCount; ++i) {
            std::pair<std::string, std::string> keyValue;

            keyValue = getTagPair(wayPointer, i);

            OSMWayHash[keyValue.first].push_back(keyValue.second);
        }

        OSMWayHash_from_OSMID[osm_ID] = OSMWayHash;

    }

    
    maxSpeedLimit = getStreetSegmentInfo(0).speedLimit;
    
    for (unsigned i = 0; i < getNumberOfStreetSegments(); i++) {

        StreetSegmentInfo segment = getStreetSegmentInfo(i);
        
        if (maxSpeedLimit < segment.speedLimit)
            maxSpeedLimit = segment.speedLimit;

        streetSegInfo_from_segID.push_back(segment);

        curvePoints_from_segmentID.push_back(std::vector<LatLon>());

        for (unsigned j = 0; j < segment.curvePointCount; j++)
            curvePoints_from_segmentID[i].push_back(getStreetSegmentCurvePoint(i, j));

        segmentIDs_from_streetID[segment.streetID].push_back(i);

        OSMID osm_ID = streetSegInfo_from_segID[i].wayOSMID;

        std::unordered_map<std::string, std::vector < std::string>> OSMWayHash = OSMWayHash_from_OSMID[osm_ID];

        std::vector<std::string> highWayTags = OSMWayHash["highway"];
    }

    numberOfStreetSegments = streetSegInfo_from_segID.size();

    //Load in point of intersection rTree
    for (unsigned i = 0; i < getNumberOfPointsOfInterest(); i++) {
        LatLon insertIDLatLon = getPointOfInterestPosition(i);
        double lat = insertIDLatLon.lat();
        double lon = insertIDLatLon.lon();

        POI_RTree.insert(std::make_pair(rTreeLatLon(lat, lon), i));

        POItypes.push_back(getPointOfInterestType(i));
        combinedNames_from_ID.push_back(getPointOfInterestName(i));
        
        poi_name_from_id.push_back(getPointOfInterestName(i));
        poi_pos_from_id.push_back(getPointOfInterestPosition(i));
    }
    std::cout << "NUMBER OF STREETS IS:    " << getNumberOfStreets() << std::endl;
    std::cout << "NUMBER OF POIs IS:    " << getNumberOfPointsOfInterest() << std::endl;
    std::cout << "STREET DATABASE IS:    " << streetName_from_streetID.size() << std::endl;
    std::cout << "COMBINED DATABASE IS:  " << combinedNames_from_ID.size() << std::endl;

    for (unsigned i = 0; i < getNumberOfFeatures(); ++i) {

        double area = 0;

        featurePoints_from_featureID.push_back(std::vector<LatLon>());


        featureName_from_featureID.push_back(getFeatureName(i));


        for (unsigned j = 0; j < getFeaturePointCount(i); ++j) {
            LatLon featurePoint = getFeaturePoint(i, j);
            featurePoints_from_featureID[i].push_back(featurePoint);
        }
        featureType_from_featureID.push_back(getFeatureType(i));

        if (getFeatureName(i) != "<unknown>" || getFeatureType(i) != 10 || getFeatureType(i) != 4) {
            float numberOfPoints = getFeaturePointCount(i);
            unsigned b = numberOfPoints - 1;
            for (unsigned a = 0; a < numberOfPoints; a++) {
                area += (cartConvLong(getFeaturePoint(i, b).lon(), average_lat) + cartConvLong(getFeaturePoint(i, a).lon(), average_lat)) *
                        (cartConvLat(getFeaturePoint(i, b).lat()) - cartConvLat(getFeaturePoint(i, a).lat()));
                b = a;
            }
            area = abs(area / 2);
        }

        areaVector.push_back(std::make_pair(area, i));

    }

    std::sort(areaVector.begin(), areaVector.end());
    std::reverse(areaVector.begin(), areaVector.end());

    for (unsigned i = 0; i < getNumberOfPointsOfInterest(); i++) {

        if (POItypes[i] == "dojo" ||
                POItypes[i] == "fitness_center" ||
                POItypes[i] == "swimming_pool"
                || POItypes[i] == "sauna"
                || POItypes[i] == "gym"
                || POItypes[i] == "martial_arts"
                || POItypes[i] == "fitness_centre" ||
                POItypes[i] == "community_centre" ||
                POItypes[i] == "yoga" ||
                POItypes[i] == "gym" ||
                POItypes[i] == "gymnasium"
                || POItypes[i] == "community_center") {

            fitnessPOILatLon.push_back(getPointOfInterestPosition(i));
            POItypes_of_POIcategory["fitness"].push_back(POItypes[i]);
            POInames_of_POIcategory["fitness"].push_back(getPointOfInterestName(i));

        } else if (POItypes[i] == "stripclub" ||
                POItypes[i] == "bar" ||
                POItypes[i] == "convention_centre" ||
                POItypes[i] == "nightclub" ||
                POItypes[i] == "arts_centre" ||
                POItypes[i] == "banquet_hall" ||
                POItypes[i] == "cinema" ||
                POItypes[i] == "casino" ||
                POItypes[i] == "bingo" ||
                POItypes[i] == "psychic" ||
                POItypes[i] == "theatre" ||
                POItypes[i] == "hackerspace" ||
                POItypes[i] == "billboard" ||
                POItypes[i] == "internet_cafe") {
            entertainmentPOILatLon.push_back(getPointOfInterestPosition(i));
            POItypes_of_POIcategory["entertainment"].push_back(POItypes[i]);
            POInames_of_POIcategory["entertainment"].push_back(getPointOfInterestName(i));
        } else if (POItypes[i] == "doctors " ||
                POItypes[i] == "veterinary " ||
                POItypes[i] == "dentist" ||
                POItypes[i] == "pharmacy" ||
                POItypes[i] == "hospital" ||
                POItypes[i] == "optometrist" ||
                POItypes[i] == "clinic" ||
                POItypes[i] == "orthodontist" ||
                POItypes[i] == "emergency_room" ||
                POItypes[i] == "physiotherapist" ||
                POItypes[i] == "laser_eye_surgery" ||
                POItypes[i] == "chiropractor" ||
                POItypes[i] == "chiropodist" ||
                POItypes[i] == "cosmetic_surgery" ||
                POItypes[i] == "health_specialty" ||
                POItypes[i] == "natural_healing" ||
                POItypes[i] == "health_center" ||
                POItypes[i] == "chiropractic" ||
                POItypes[i] == "optician" ||
                POItypes[i] == "psychologist" ||
                POItypes[i] == "reflexology" ||
                POItypes[i] == "ambulance_station" ||
                POItypes[i] == "ems_station") {

            healthPOILatLon.push_back(getPointOfInterestPosition(i));
            POItypes_of_POIcategory["health"].push_back(POItypes[i]);
            POInames_of_POIcategory["health"].push_back(getPointOfInterestName(i));
        } else if (POItypes[i] == "fast_food" ||
                POItypes[i] == "restaurant" ||
                POItypes[i] == "cafe" ||
                POItypes[i] == "bakery" ||
                POItypes[i] == "ice_cream" ||
                POItypes[i] == "future_fast_food" ||
                POItypes[i] == "banquet_hall" ||
                POItypes[i] == "old_restaurant" ||
                POItypes[i] == "deli" ||
                POItypes[i] == "drinking_water" ||
                POItypes[i] == "food_court"
                ) {

            foodPOILatLon.push_back(getPointOfInterestPosition(i));
            POItypes_of_POIcategory["food"].push_back(POItypes[i]);
            POInames_of_POIcategory["food"].push_back(getPointOfInterestName(i));
        } else if (POItypes[i] == "bank" ||
                POItypes[i] == "atm" ||
                POItypes[i] == "bureau_de_change" ||
                POItypes[i] == "money_lender" ||
                POItypes[i] == "credit_union" ||
                POItypes[i] == "mortgage_broker" ||
                POItypes[i] == "tax" ||
                POItypes[i] == "money_lender" ||
                POItypes[i] == "financial") {

            financePOILatLon.push_back(getPointOfInterestPosition(i));
            POItypes_of_POIcategory["finance"].push_back(POItypes[i]);
            POInames_of_POIcategory["finance"].push_back(getPointOfInterestName(i));
        } else if (POItypes[i] == "school" ||
                POItypes[i] == "childcare" ||
                POItypes[i] == "child_care" ||
                POItypes[i] == "college" ||
                POItypes[i] == "kindergarten" ||
                POItypes[i] == "montessori_school" ||
                POItypes[i] == "language_school" ||
                POItypes[i] == "music_school" ||
                POItypes[i] == "english_school" ||
                POItypes[i] == "tutor" ||
                POItypes[i] == "preschool" ||
                POItypes[i] == "tutoring" ||
                POItypes[i] == "education" ||
                POItypes[i] == "music_lessons" ||
                POItypes[i] == "lab" ||
                POItypes[i] == "university" ||
                POItypes[i] == "kindergarten" ||
                POItypes[i] == "montessori_school") {

            educationPOILatLon.push_back(getPointOfInterestPosition(i));
            POItypes_of_POIcategory["education"].push_back(POItypes[i]);
            POInames_of_POIcategory["education"].push_back(getPointOfInterestName(i));
        } else if (POItypes[i] == "taxi" ||
                POItypes[i] == "old_car_rental" ||
                POItypes[i] == "bicycle_parking" ||
                POItypes[i] == "car_sharing" ||
                POItypes[i] == "mechanic" ||
                POItypes[i] == "bus_station ") {
            transportationPOILatLon.push_back(getPointOfInterestPosition(i));
            POItypes_of_POIcategory["transportation"].push_back(POItypes[i]);
            POInames_of_POIcategory["transportation"].push_back(getPointOfInterestName(i));

        } else if (POItypes[i] == "childcare" ||
                POItypes[i] == "charity " ||
                POItypes[i] == "clothing_donation" ||
                POItypes[i] == "shelter" ||
                POItypes[i] == "post_box" ||
                POItypes[i] == "recycling" ||
                POItypes[i] == "food_bank " ||
                POItypes[i] == "social_facility" ||
                POItypes[i] == "passport_office" ||
                POItypes[i] == "retirement_home" ||
                POItypes[i] == "photography_club" ||
                POItypes[i] == "driving_school " ||
                POItypes[i] == "waste_disposal" ||
                POItypes[i] == "traffic_ticket" ||
                POItypes[i] == "answering_service" ||
                POItypes[i] == "consulting") {

            publicPOILatLon.push_back(getPointOfInterestPosition(i));
            POItypes_of_POIcategory["public"].push_back(POItypes[i]);
            POInames_of_POIcategory["public"].push_back(getPointOfInterestName(i));
        } else if (POItypes[i] == "police" || POItypes[i] == "fire_station") {
            emergencyPOILatLon.push_back(getPointOfInterestPosition(i));
            POItypes_of_POIcategory["emergency"].push_back(POItypes[i]);
            POInames_of_POIcategory["emergency"].push_back(getPointOfInterestName(i));

        } else if (POItypes[i] == "vacant" ||
                POItypes[i] == "parking_entrance" ||
                POItypes[i] == "parking_closed") {
            parkingPOILatLon.push_back(getPointOfInterestPosition(i));
            POItypes_of_POIcategory["parking"].push_back(POItypes[i]);
            POInames_of_POIcategory["parking"].push_back(getPointOfInterestName(i));

        } else if (POItypes[i] == "courthouse" ||
                POItypes[i] == "campaign_office " ||
                POItypes[i] == "employment_agency " ||
                POItypes[i] == "mpp_office" ||
                POItypes[i] == "employment_office" ||
                POItypes[i] == "pardons" ||
                POItypes[i] == "government_office " ||
                POItypes[i] == "elections" ||
                POItypes[i] == "mpp") {
            govtPOILatLon.push_back(getPointOfInterestPosition(i));
            POItypes_of_POIcategory["govt"].push_back(POItypes[i]);
            POInames_of_POIcategory["govt"].push_back(getPointOfInterestName(i));
        } else if (POItypes[i] == "marketplace" ||
                POItypes[i] == "parking_entrance" ||
                POItypes[i] == "parking_closed" ||
                POItypes[i] == "pharmacy parking") {
            shoppingPOILatLon.push_back(getPointOfInterestPosition(i));
            POItypes_of_POIcategory["shopping"].push_back(POItypes[i]);
            POInames_of_POIcategory["shopping"].push_back(getPointOfInterestName(i));

        } else if (POItypes[i] == "fuel") {
            gasPOILatLon.push_back(getPointOfInterestPosition(i));
            POItypes_of_POIcategory["gas"].push_back(POItypes[i]);
            POInames_of_POIcategory["gas"].push_back(getPointOfInterestName(i));
        } else if (POItypes[i] == "hotel" ||
                POItypes[i] == "motel") {
            hotelPOILatLon.push_back(getPointOfInterestPosition(i));
            POItypes_of_POIcategory["hotel"].push_back(POItypes[i]);
            POInames_of_POIcategory["hotel"].push_back(getPointOfInterestName(i));
        }
    }


    POILatLonCategory_from_POInumber.push_back(std::vector<LatLon>());
    POILatLonCategory_from_POInumber[0] = emergencyPOILatLon;

    POILatLonCategory_from_POInumber.push_back(std::vector<LatLon>());
    POILatLonCategory_from_POInumber[1] = healthPOILatLon;

    POILatLonCategory_from_POInumber.push_back(std::vector<LatLon>());
    POILatLonCategory_from_POInumber[2] = hotelPOILatLon;

    POILatLonCategory_from_POInumber.push_back(std::vector<LatLon>());
    POILatLonCategory_from_POInumber[3] = entertainmentPOILatLon;

    POILatLonCategory_from_POInumber.push_back(std::vector<LatLon>());
    POILatLonCategory_from_POInumber[4] = fitnessPOILatLon;

    POILatLonCategory_from_POInumber.push_back(std::vector<LatLon>());
    POILatLonCategory_from_POInumber[5] = publicPOILatLon;

    POILatLonCategory_from_POInumber.push_back(std::vector<LatLon>());
    POILatLonCategory_from_POInumber[6] = parkingPOILatLon;

    POILatLonCategory_from_POInumber.push_back(std::vector<LatLon>());
    POILatLonCategory_from_POInumber[7] = shoppingPOILatLon;

    POILatLonCategory_from_POInumber.push_back(std::vector<LatLon>());
    POILatLonCategory_from_POInumber[8] = financePOILatLon;

    POILatLonCategory_from_POInumber.push_back(std::vector<LatLon>());
    POILatLonCategory_from_POInumber[9] = transportationPOILatLon;

    POILatLonCategory_from_POInumber.push_back(std::vector<LatLon>());
    POILatLonCategory_from_POInumber[10] = gasPOILatLon;

    POILatLonCategory_from_POInumber.push_back(std::vector<LatLon>());
    POILatLonCategory_from_POInumber[11] = foodPOILatLon;

    POILatLonCategory_from_POInumber.push_back(std::vector<LatLon>());
    POILatLonCategory_from_POInumber[12] = educationPOILatLon;

    POILatLonCategory_from_POInumber.push_back(std::vector<LatLon>());
    POILatLonCategory_from_POInumber[13] = govtPOILatLon;

}

double cartConvLong(double lon, double average_latitude) {
    return lon * cos(average_latitude);
}

double cartConvLat(double lat) {
    return lat;
}

//Query functions to access data bases

std::vector<unsigned> map_data::query_streetIDs_from_Name(std::string street_name) {
    return streetIDs_from_Name[street_name];
}

std::vector<unsigned> map_data::query_segmentIDs_from_streetID(unsigned street_id) {
    return segmentIDs_from_streetID[street_id];
}

std::vector<unsigned> map_data::query_intersectionIDs_from_streetID(unsigned street_id) {
    return intersectionIDs_from_streetID[street_id];
}

std::vector<unsigned> map_data::query_intersectionIDs_from_Name(std::string street_name) {
    return intersectionIDs_from_Name[street_name];
}

std::vector<std::string> map_data::query_streetNames_from_Intersection(unsigned intersection_id) {
    return streetNames_from_Intersection[intersection_id];
}

std::vector<unsigned> map_data::query_intersection_street_segments(unsigned intersection_id) {
    return intersection_street_segments[intersection_id];
}

StreetSegmentInfo map_data::query_streetSegInfo_from_segID(unsigned segment_id) {
    return streetSegInfo_from_segID[segment_id];
}
