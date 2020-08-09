/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "travel_instructions.cpp"
#include "m3.h"
#include "m2.h"
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "mapObjects.h"
#include "X11/keysymdef.h"
#include <X11/keysym.h>
#include <readline/readline.h>
#include <readline/history.h>
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

std::string userInputDefaultSB = "Search for an intersection or street";
std::string userInputSecondSB = "Search for an intersection or street";

std::string streetName1;
std::string streetName2;
std::string streetName3;
std::string streetName4;


bool parsedValidity = true; //for drawing intersection
bool intersectingStreets = false; //for drawing intersection
bool searchFlagDraw = false;
bool searchFlagDraw2 = false;
bool highlightStreet = false; //for highlighting the whole street
std::string streetToBeHighlighted;

bool createPathEnable = false;

bool drawFindPathError = false;

std::vector<unsigned> clickCreatedPath;
bool foundClickCreatedPath = false;

bool menuBoxClicked = false;
bool dontDisplayMenu = false;
bool xOnMenuClicked = false;

double maximum_latitudeHOME = 0;
double minimum_latitudeHOME = 0;

double maximum_longitudeHOME = 0;
double minimum_longitudeHOME = 0;

bool firstSBEmpty = false;
bool secondSBEmpty = false;

std::string firstStreet;

std::vector<unsigned> path;
std::vector<std::string> travelInstructions;

void draw_path_found();
void draw_travel_instructions();

void find_path_button(void (*drawscreen) (void));

void createPath(void (*drawscreen) (void));

void highlightStreetFunction(); //highlights the user requested street

void draw_findPath_errorBox();

//draws the graphics that need to be for the finding intersections
//will be dependent on a button
void callDrawFindPathGraphics(void (*drawscreen) (void));

void draw_travel_instructions();

void travelInstruction_text_size(std::string);

void drawFindPathGraphics();

bool helpButtonActivated = false;

bool findPath = false;

bool cursorClicked = false; //goes true as soon as the cursor clicks 
//somewhere on the screen

t_bound_box POIboxes[14];

int doubleScreenClick = 0;

bool showPOIButton = false; //Global variable to check is button is pressed

bool showPOIOptions = false;

bool clickOnScreen = false;

bool dropDownMenu = false;


bool arrowKeysPressed = false;

std::vector <LatLon> clickedPoints;

int arrayIndexCount = -1;

void setColourBlind();

void showSpecificPOI(void (*drawscreen) (void));

bool colourBlindMode = false;

int poi = 0; //stores the user input

void drawAllPOI();

void drawSymbolPOI(int poi);

double average_latitude;

double fullMapArea;

bool findDistanceButtonPressed = false;

bool showIntersections = false;

bool queryIsAnIntersection = false;

bool drawMarkerOnClick = false; //when this goes true, marker is drawn and info 

bool bothPOIsBad = false;
bool bothIntersecInvalid = false;
bool firstInterBad = false;
bool secondInterBad = false;
bool destinPOIbad = false;
bool startPOIbad = false;

bool drawPath = false;



void act_on_mousebutton(float x, float y, t_event_buttonPressed button_info);

void act_on_mousemove(float x, float y);

int getZoomValue();

bool validToDrawFeatureText(FeatureType feature_type);

void draw_feature_name();

int checkQueries();

void draw_features();

void draw_closed_feature_name(std::vector<LatLon> feature_points_vector, unsigned tail, unsigned feature_id);

void draw_open_feature_name(std::vector<LatLon> feature_points_vector, unsigned tail, unsigned feature_id);

//displays intersection info
void displayIntersectionMarkerInfo(float x, float y, unsigned closestIntersecion);

void draw_screen();

void draw_POI_information_box();

void draw_ID_information_box();

std::string getCategoryPOINames();

void autoComplete(std::string& streetName);

void autoComplete2();

void autoCompleteIntersection();

isHighlighted isMarked;

void draw_helpMenu();

std::string closestStrings[4];

void draw_path();

poiInformationBox poiInformationBox;

//Function Prototypes
bool checkStreetNameValidity(std::string &streetName);

int computeLenvenshteinDistance(std::string string1, std::string string2);

//acts after its been verified streets do exist
void drawIntersectionIfExist(std::string &street1,
        std::string &street2);

void draw_drop_downMenu();

struct intersection_data {
    LatLon position;
    std::string name;
};

//creating a structures to store the last click
t_point markerCoordinates;
std::vector<intersection_data> intersections;
bool displayAllPOI = false; //when this is true all POI's 

//for search box
void draw_searchBar();

void act_on_keypress(char c, int keysym);

unsigned roadType(unsigned streetSegmentID) {

    OSMID osm_ID = map -> query_streetSegInfo_from_segID(streetSegmentID).wayOSMID;

    std::unordered_map<std::string, std::vector < std::string>> OSMWayHash = map -> OSMWayHash_from_OSMID[osm_ID];

    std::vector<std::string> highWayTags = OSMWayHash["highway"];

    if (highWayTags.size() > 0) {
        for (unsigned i = 0; i < highWayTags.size(); ++i) {
            std::string roadType = highWayTags[i];
            //Highways
            if (roadType == "motorway" || roadType == "motorway_link" || roadType == "escape" || roadType == "raceway") {
                return 1;
            }//Minor Roads
            else if (roadType == "tertiary" || roadType == "tertiary_link" || roadType == "unclassified"
                    || roadType == "residential" ||
                    roadType == "living_street" || roadType == "service"
                    || roadType == "pedestrian" || roadType == "track" || roadType == "bus_guideway") {
                return 3;
            }//Major Roads
            else if (roadType == "primary" || roadType == "trunk" || roadType == "secondary_link" ||
                    roadType == "secondary" || roadType == "trunk_link" || roadType == "primary_link") { //MAJOR ROADS
                return 2;
            } else {
                return 4;
            }
        }
    }
    return 0;

    //OWMWay* way = new OSMWay(osmID);
}

bool isOneWay(unsigned streetSegmentID) {
    return (map -> query_streetSegInfo_from_segID(streetSegmentID).oneWay);
}

double longitude_to_cartesian(double lon, double average_latitude) {
    return lon * cos(average_latitude);
}

double latitude_to_cartesian(double lat) {
    return lat;
}


//find function implementation of highlighting streets

void draw_POI_Options() {
    float x1 = get_visible_world().left();
    float y1 = get_visible_world().bottom();
    float x2 = get_visible_world().right();
    float y2 = y1 + get_visible_world().get_height() / 4;

    setcolor(ORANGE);
    fillrect(x1, y1, x2, y2 - get_visible_world().get_height() / 12);

    float xOffSet = get_visible_world().get_width() / 40;
    float yOffSet = get_visible_world().get_height() / 40;

    Surface pngs[14];

    Surface emergency = load_png_from_file("libstreetmap/resources/emergency.png");
    Surface health = load_png_from_file("libstreetmap/resources/health.png");

    pngs[0] = load_png_from_file("libstreetmap/resources/emergency.png");
    pngs[1] = load_png_from_file("libstreetmap/resources/health.png");
    pngs[2] = load_png_from_file("libstreetmap/resources/hotel.png");
    pngs[3] = load_png_from_file("libstreetmap/resources/entertainment.png");
    pngs[4] = load_png_from_file("libstreetmap/resources/gym.png");
    pngs[5] = load_png_from_file("libstreetmap/resources/public.png");
    pngs[6] = load_png_from_file("libstreetmap/resources/parking.png");
    pngs[7] = load_png_from_file("libstreetmap/resources/shopping.png");
    pngs[8] = load_png_from_file("libstreetmap/resources/bank.png");
    pngs[9] = load_png_from_file("libstreetmap/resources/transportation.png");
    pngs[10] = load_png_from_file("libstreetmap/resources/gas.png");
    pngs[11] = load_png_from_file("libstreetmap/resources/food.png");
    pngs[12] = load_png_from_file("libstreetmap/resources/education.png");
    pngs[13] = load_png_from_file("libstreetmap/resources/govt.png");

    set_coordinate_system(GL_WORLD);
    setcolor(BLUE);
    setlinewidth(4);
    drawrect(x1, y1, x2, y2 - get_visible_world().get_height() / 12);

    std::string poiCategories1[7] = {
        "Emergency",
        "Health",
        "Hotel",
        "Entertainment",
        "Gym",
        "Public",
        "Parking",
    };

    std::string poiCategories2[7] = {
        "Shopping",
        "Bank",
        "Transportation",
        "Gas",
        "Food",
        "Education",
        "Government"
    };

    for (unsigned i = 0; i < 7; i++) {

        setcolor(BLUE);
        fillrect((i * get_visible_world().get_width() / 7) + x1 + xOffSet, (y1 + y2 - get_visible_world().get_height() / 12) / 2 + yOffSet, (i * get_visible_world().get_width() / 7) + x1 + get_visible_world().get_width() / 7 - xOffSet, y2 - get_visible_world().get_height() / 12 - yOffSet);
        fillrect((i * get_visible_world().get_width() / 7) + x1 + xOffSet, y1 + yOffSet, (i * get_visible_world().get_width() / 7) + x1 + get_visible_world().get_width() / 7 - xOffSet, (y1 + y2 - get_visible_world().get_height() / 12) / 2 - yOffSet);

        setcolor(GREEN);
        POIboxes[i] = t_bound_box((i * get_visible_world().get_width() / 7) + x1 + xOffSet, (y1 + y2 - get_visible_world().get_height() / 12) / 2 + yOffSet, (i * get_visible_world().get_width() / 7) + x1 + get_visible_world().get_width() / 7 - xOffSet, y2 - get_visible_world().get_height() / 12 - yOffSet);
        drawtext_in(POIboxes[i], poiCategories1[i]);

        POIboxes[i + 7] = t_bound_box((i * get_visible_world().get_width() / 7) + x1 + xOffSet, y1 + yOffSet, (i * get_visible_world().get_width() / 7) + x1 + get_visible_world().get_width() / 7 - xOffSet, (y1 + y2 - get_visible_world().get_height() / 12) / 2 - yOffSet);
        drawtext_in(POIboxes[i + 7], poiCategories2[i]);

        //draw_surface(pngs[i], x2 / 28, y2 / 2);
    }
}

void draw_ID_information_box() {

    float x1 = get_visible_world().left();
    float y1 = get_visible_world().bottom();
    float x2 = get_visible_world().right();
    float y2 = y1 + get_visible_world().get_height() / 4;
    //float x2 = x1 + get_visible_world().get_width() / 4;
    //float y2 = y1 + get_visible_world().get_height() / 4;

    unsigned numberOfIntersectionIDs = isMarked.intersection_x_y.size();

    t_bound_box box = t_bound_box(x1, y1, x2, y2);
    setcolor(BLACK);
    fillrect(x1, y1, x2, y2 - get_visible_world().get_height() / 12);


    float xOffSet = get_visible_world().get_height() / 50;

    float yOffSet = get_visible_world().get_height() / 40;

    std::string line_text[numberOfIntersectionIDs][3];

    for (unsigned i = 0; i < numberOfIntersectionIDs; i++) {
        line_text[i][0] = "Intersection: " + isMarked.namesVector[i];

        if (isMarked.latVector[i] < 0) {
            line_text[i][1] = "Latitude: " + std::to_string(isMarked.latVector[i]) + "° W";
        } else
            line_text[i][1] = "Latitude: " + std::to_string(isMarked.latVector[i]) + "° E";

        if (isMarked.lonVector[i] < 0)
            line_text[i][2] = "Longitude: " + std::to_string(isMarked.lonVector[i]) + "° S";
        else
            line_text[i][2] = "Longitude: " + std::to_string(isMarked.lonVector[i]) + "° N";
    }

    if (highlightStreet && parsedValidity) {
        setfontsize(12);
        setcolor(GREEN);
        settextrotation(0);


        //t_bound_box box3 = t_bound_box(x1 + xOffSet + 4 * get_visible_world().get_width() / 6, y1 - (yOffSet), x1 + 5 * (get_visible_world().get_width() / 6) - xOffSet, y2 - (yOffSet));
        drawtext_in(box, streetToBeHighlighted + " is now highlighted in red. Press clear search to continue.");
        //drawrect(box3);
        return;
    }


    //if no intersections
    if (!parsedValidity) {
        setfontsize(12);
        setcolor(GREEN);
        settextrotation(0);


        //t_bound_box box3 = t_bound_box(x1 + xOffSet + 4 * get_visible_world().get_width() / 6, y1 - (yOffSet), x1 + 5 * (get_visible_world().get_width() / 6) - xOffSet, y2 - (yOffSet));
        drawtext_in(box, "Invalid street name. Please try again.  Press clear search to continue.");
        //drawrect(box3);
        return;

    }

    if (!intersectingStreets) {
        setfontsize(12);
        setcolor(GREEN);
        settextrotation(0);

        //t_bound_box box3 = t_bound_box(x1 + xOffSet + 4 * get_visible_world().get_width() / 6, y1 - (yOffSet), x1 + 5 * (get_visible_world().get_width() / 6) - xOffSet, y2 - (yOffSet));
        drawtext_in(box, "Sorry these streets do not intersect. Press clear search to continue.");
        //drawrect(box3);
        return;
    }
    for (unsigned i = 0; i < numberOfIntersectionIDs; i++) {
        setfontsize(12);
        setcolor(GREEN);
        settextrotation(0);

        t_bound_box box2 = t_bound_box(x1 + xOffSet, y1 - (i * yOffSet), x1 + 4 * (get_visible_world().get_width() / 6) - xOffSet, y2 - (i * yOffSet));
        drawtext_in(box2, line_text[i][0]);
        //drawrect(box2);

        t_bound_box box3 = t_bound_box(x1 + xOffSet + 4 * get_visible_world().get_width() / 6, y1 - (i * yOffSet), x1 + 5 * (get_visible_world().get_width() / 6) - xOffSet, y2 - (i * yOffSet));
        drawtext_in(box3, line_text[i][1]);
        //drawrect(box3);

        t_bound_box box4 = t_bound_box(x1 + xOffSet + 5 * (get_visible_world().get_width() / 6), y1 - (i * yOffSet), x2 - xOffSet, y2 - (i * yOffSet));
        drawtext_in(box4, line_text[i][2]);
        //drawrect(box4);
    }
}


//draw the additional buttons

void resetSearch(void (*drawscreen) (void)) {

    set_visible_world(minimum_longitudeHOME * std::cos(average_latitude), minimum_latitudeHOME, maximum_longitudeHOME * std::cos(average_latitude), maximum_latitudeHOME);
    drawFindPathError = false;
    searchFlagDraw = false;
    searchFlagDraw2 = false;
    showPOIButton = false;
    isMarked.drawBox = false;
    isMarked.drawPins = false;
    intersectingStreets = false;
    highlightStreet = false;
    findDistanceButtonPressed = false;
    findPath = false;
    doubleScreenClick = 0;
    drawscreen();
    xOnMenuClicked = false;
    dropDownMenu = false;
    drawPath = false;
    foundClickCreatedPath = false;
    //createPathEnable = false;
    userInputDefaultSB = "Search for an intersection or street";
    userInputSecondSB = "Search for an intersection or street";
}

void find_path_button(void (*drawscreen) (void)) {
    userInputDefaultSB = "Search for an intersection or street";
    userInputSecondSB = "Search for an intersection or street";
    findPath = true;
    update_message("     ");
    draw_screen();
}

void createPath(void (*drawscreen) (void)) {

    update_message("Please click on Point 1");

    findDistanceButtonPressed = true;

}

void colourBlindModeTog(void (*drawscreen) (void)) {

    if (colourBlindMode == true) {
        colourBlindMode = false;
    } else {
        colourBlindMode = true;
    }

    setColourBlind();

}

void helpButton(void (*drawscreen) (void)) {

    if (helpButtonActivated == true) {
        helpButtonActivated = false;
        std::cout << "Help Button is FALSE" << std::endl;
    } else {
        helpButtonActivated = true;
        std::cout << "Help Button is TRUE" << std::endl;
    }

    draw_screen();
}

void allButtons() {

    create_button("Window", "Find POIs", showSpecificPOI);
    create_button("Find POIs", "Clear Search", resetSearch);
    create_button("Clear Search", "Create Path", createPath);

    if (colourBlindMode) {
        create_button("Create Path", "Revert Colours", colourBlindModeTog);
        create_button("Revert Colours", "Find Path", find_path_button);

    } else {
        create_button("Create Path", "Colour Blind", colourBlindModeTog);
        create_button("Colour Blind", "Find Path", find_path_button);
    }

    create_button("Find Path", "Help", helpButton);

    //create_button("Help", "New Map", newMapLoader);

    //create_button("Dark Mode", "Find Path", );


    //create_button("Dark Mode", "Find Path", );


}

int getZoomValue() {

    double mapAreaRatio = 2.79;
    int maxZoom = 15;

    if (get_visible_world().area() > fullMapArea) {
        return -1;
    } else if (get_visible_world().area() == fullMapArea) {
        return 1;
    } else {
        for (unsigned i = 0; i < maxZoom; ++i) {
            if ((get_visible_world().area() * pow(mapAreaRatio, i) < fullMapArea) && (get_visible_world().area() * pow(mapAreaRatio, i + 1) > fullMapArea)) {
                return i + 2;
            }
        }
    }
}

bool segmentInMapView(float x1, float x2, float y1, float y2) {

    bool x1InBounds = true;
    bool y1InBounds = true;
    bool x2InBounds = true;
    bool y2InBounds = true;

    bool point1 = true;
    bool point2 = true;

    if (!(x1 > get_visible_world().bottom_left().x && x1 < get_visible_world().top_right().x))
        x1InBounds = false;

    if (!(x2 > get_visible_world().bottom_left().x && x2 < get_visible_world().top_right().x))
        x2InBounds = false;

    if (!(y1 > get_visible_world().bottom_left().y && y1 < get_visible_world().top_right().y))
        y1InBounds = false;

    if (!(y2 > get_visible_world().bottom_left().y && y2 < get_visible_world().top_right().y))
        y2InBounds = false;

    if (!x1InBounds || !y1InBounds)
        point1 = false;
    if (!x2InBounds || !y2InBounds)
        point2 = false;

    if (!point1 && !point2)
        return false;

    return true;
}

bool x_y_in_mapView(float x, float y) {


    if (!(x > get_visible_world().bottom_left().x && x < get_visible_world().top_right().x))
        return false;

    if (!(y > get_visible_world().bottom_left().y && y < get_visible_world().top_right().y))
        return false;

    return true;

}

void printSegName(float x1, float x2, float y1, float y2, std::string segName) {
    float xCentre = (x1 + x2) / 2;
    float yCentre = (y1 + y2) / 2;

    float deltaY = y2 - yCentre;
    float deltaX = x2 - xCentre;

    float ratio = deltaY / deltaX;

    float thetha = atan(ratio) * (57.295);
    int angle = (int) thetha;


    setcolor(BLACK);
    settextrotation(angle);

    //float bound = 0.7 * hypot(abs(x1 - x2), abs(y1-y2));
    float boundx = abs(x1 - x2);
    float boundy = abs(y1 - y2);

    drawtext(xCentre, yCentre, segName, boundx, boundy);

}

void setColourForFeature(FeatureType feature_type) {
    if (colourBlindMode == false) {
        switch (feature_type) {
            case Unknown:
                setcolor(237, 237, 237);
                break;
            case Park:
                setcolor(47, 218, 35);
                break;
            case Beach:
                setcolor(250, 242, 199);
                break;
            case Lake:
                setcolor(111, 211, 255);
                break;
            case River:
                setcolor(111, 211, 255);
                setlinewidth(5);
                break;
            case Island:
                setcolor(76, 196, 112);
                break;
            case Shoreline:
                setcolor(170, 212, 177);
                setlinewidth(1);
                break;
            case Building:
                setcolor(105, 105, 105);
                break;
            case Greenspace:
                setcolor(47, 218, 35);
                break;
            case Golfcourse:
                setcolor(34, 206, 115);
                break;
            case Stream:
                setcolor(111, 211, 255);
                setlinewidth(2);
                break;
        }
    } else {
        switch (feature_type) {
            case Unknown:
                setcolor(237, 237, 237);
                break;
            case Park:
                setcolor(42, 162, 42);
                break;
            case Beach:
                setcolor(250, 242, 199);
                break;
            case Lake:
                setcolor(34, 0, 255);
                break;
            case River:
                setcolor(34, 0, 255);
                setlinewidth(5);
                break;
            case Island:
                setcolor(42, 162, 42);
                break;
            case Shoreline:
                setcolor(170, 212, 177);
                setlinewidth(1);
                break;
            case Building:
                setcolor(105, 105, 105);
                //setcolor(253, 247, 240);
                break;
            case Greenspace:
                setcolor(0, 255, 60);
                break;
            case Golfcourse:
                setcolor(0, 255, 60);
                break;
            case Stream:
                setcolor(34, 0, 255);
                setlinewidth(2);
                break;
        }
    }

}

bool validToDrawFeature(FeatureType feature_type) {
    int currentZoom = getZoomValue();
    bool valid = false;

    switch (feature_type) {
        case Unknown:
            valid = true;
            break;
        case Park:
            valid = true;
            break;
        case Beach:
            if (currentZoom >= 2)
                valid = true;
            break;
        case Lake:
            valid = true;
            break;
        case River:
            if (currentZoom >= 2)
                valid = true;
            break;
        case Island:
            valid = true;
            break;
        case Shoreline:
            valid = true;
            break;
        case Building:
            if (currentZoom >= 11)
                valid = true;
            break;
        case Greenspace:
            if (currentZoom >= 6)
                valid = true;
            break;
        case Golfcourse:
            if (currentZoom >= 8)
                valid = true;
            break;
        case Stream:
            if (currentZoom >= 5)
                valid = true;
            break;

    }
    return valid;

}

bool validToDrawFeatureText(FeatureType feature_type) {
    int currentZoom = getZoomValue();
    bool valid = false;

    switch (feature_type) {
        case Unknown:
            valid = false;
            break;
        case Park:
            if (currentZoom >= 7)
                valid = true;
            break;
        case Beach:
            if (currentZoom >= 4)
                valid = true;
            break;
        case Lake:
            valid = true;
            break;
        case River:
            if (currentZoom >= 5)
                valid = true;
            break;
        case Island:
            if (currentZoom >= 5)
                valid = true;
            break;
        case Shoreline:
            if (currentZoom >= 5)
                valid = true;
            break;
        case Building:
            if (currentZoom >= 11)
                valid = true;
            break;
        case Greenspace:
            if (currentZoom >= 6)
                valid = true;
            break;
        case Golfcourse:
            if (currentZoom >= 8)
                valid = true;
            break;
        case Stream:
            if (currentZoom >= 5)
                valid = true;
            break;

    }
    return valid;

}

bool validToDrawRoad(unsigned roadTypeNumber) {
    int currentZoom = getZoomValue();
    bool valid = false;

    switch (roadTypeNumber) {
        case 1:
            //Highways
            valid = true;
            break;
        case 2:
            //Major Roads
            valid = true;
            break;
        case 3:
            //Minor Roads
            if (currentZoom >= 6)
                valid = true;
            break;
    }
    return valid;

}

bool validToDrawRoadText(unsigned roadTypeNumber) {
    int currentZoom = getZoomValue();
    bool valid = false;

    switch (roadTypeNumber) {
        case 1:
            //Highways
            if (currentZoom >= 2) {
                setfontsize(15);
                valid = true;
            }
            break;
        case 2:
            //Major Roads
            if (currentZoom >= 3) {
                setfontsize(10);
                valid = true;
            }
            break;
        case 3:
            //Minor Roads
            if (currentZoom >= 9) {
                setfontsize(8);
                valid = true;
            }
            break;
    }
    return valid;
}


//off screen copy updcreen

void setRoadGraphics(unsigned roadType) {
    if (colourBlindMode == false) {
        switch (roadType) {
            case 0:
                setcolor(RED);
                break;
            case 1:
                //Highways
                setlinewidth(8);
                setcolor(255, 204, 0);
                break;
            case 2:
                //Major Roads
                setlinewidth(6);
                setcolor(252, 247, 210);
                break;
            case 3:
                //Minor Roads
                setlinewidth(5);
                setcolor(255, 255, 255);
                break;
            case 4:
                //Unknown
                setlinewidth(0);
                break;
        }
    } else {
        switch (roadType) {
            case 0:
                setcolor(RED);
                break;
            case 1:
                //Highways
                setlinewidth(5);
                setcolor(255, 102, 0);
                break;
            case 2:
                //Major Roads
                setlinewidth(3);
                setcolor(134, 134, 134);
                break;
            case 3:
                //Minor Roads
                setlinewidth(1);
                setcolor(255, 255, 255);
                break;
            case 4:
                //Unknown
                setlinewidth(0);
                setcolor(RED);
                break;
        }
    }
}

void draw_features() {
    for (unsigned i = 0; i < map -> featureType_from_featureID.size(); ++i) {

        unsigned featureID = map-> areaVector[i].second;
        double featureArea = map-> areaVector[i].first;

        if (getZoomValue() < 2 && featureArea < 7.09044e-06) {
            return;
        } else if (getZoomValue() <= 6 && featureArea < 5.59044e-06) {
            return;
        } else if (getZoomValue() < 8 && featureArea < 5.59044e-07) {
            return;
        }
        FeatureType featureType = map -> featureType_from_featureID[featureID];

        std::vector<LatLon> feature_points = map -> featurePoints_from_featureID[featureID];

        unsigned featurePointCount = feature_points.size();

        setColourForFeature(featureType);

        if (validToDrawFeature(featureType)) {
            if ((feature_points[0].lat() == feature_points[featurePointCount - 1].lat()) &&
                    (feature_points[0].lon() == feature_points[featurePointCount - 1].lon())) {

                t_point pointsArray[featurePointCount];

                std::string featureName = map -> featureName_from_featureID[featureID];

                for (unsigned j = 0; j < featurePointCount; ++j) {
                    //LatLon featurePt = getFeaturePoint(i, j);

                    float x = longitude_to_cartesian(feature_points[j].lon(), average_latitude);
                    pointsArray[j].x = x;

                    float y = latitude_to_cartesian(feature_points[j].lat());
                    pointsArray[j].y = y;
                }

                fillpoly(pointsArray, featurePointCount);


            } else {
                for (unsigned j = 0; j < featurePointCount - 1; ++j) {
                    //LatLon featurePt1 = getFeaturePoint(i, j);
                    //LatLon featurePt2 = getFeaturePoint(i, j + 1);

                    float x1 = longitude_to_cartesian(feature_points[j].lon(), average_latitude);
                    float y1 = latitude_to_cartesian(feature_points[j].lat());
                    float x2 = longitude_to_cartesian(feature_points[j].lon(), average_latitude);
                    float y2 = latitude_to_cartesian(feature_points[j].lat());
                    if (segmentInMapView(x1, x2, y1, y2)) {
                        drawline(x1, y1, x2, y2);
                    }
                }
            }
        }

    }
}

void draw_feature_name() {
    int zoom_count = getZoomValue();

    unsigned featureIdCount = map-> featureType_from_featureID.size();
    if (zoom_count > 2) {
        int i = 0;
        unsigned featureID = map-> areaVector[i].second;
        while (i < featureIdCount) {

            FeatureType featureType = map -> featureType_from_featureID[i];

            if (validToDrawFeatureText(featureType)) {

                std::vector<LatLon> feature_points_vector = map -> featurePoints_from_featureID[i];

                unsigned tail = feature_points_vector.size() - 1;


                if (feature_points_vector[0].lon() == feature_points_vector[tail].lon() && feature_points_vector[0].lat() == feature_points_vector[tail].lat())
                    draw_closed_feature_name(feature_points_vector, tail, i);
                else {
                    draw_open_feature_name(feature_points_vector, tail, i);
                }

            }
            i++;
        }
    }
    return;
}

void draw_closed_feature_name(std::vector<LatLon> feature_points_vector, unsigned tail, unsigned feature_id) {

    std::string featureName = map-> featureName_from_featureID[feature_id];

    for (unsigned id = 0; id < feature_id; id++) {
        if (map -> featureName_from_featureID[id] == featureName)
            return;
    }

    float sumx = 0;
    float sumy = 0;

    float avex = 0;
    float avey = 0;

    float minX = 0;
    float maxX = 0;
    float minY = 0;
    float maxY = 0;

    for (unsigned i = 0; i <= tail; i++) {

        float x = longitude_to_cartesian(feature_points_vector[i].lon(), average_latitude);
        float y = latitude_to_cartesian(feature_points_vector[i].lat());

        sumx += x;
        sumy += y;

        if (x < minX)
            minX = x;

        if (x > maxX)
            maxX = x;

        if (y < minY)
            minY = y;

        if (y > maxY)
            maxY = y;
    }

    avex = sumx / (tail + 1);
    avey = sumy / (tail + 1);

    if (featureName == "<noname>" || featureName == "<Unnamed>")
        return;

    if (map -> featureType_from_featureID[feature_id] == 3) {
        settextrotation(0);
        setcolor(BLACK);
        setfontsize(10);
        drawtext(avex, avey, featureName);
        return;
    }


    t_bound_box featureNameBox(avex - fabs(minX - maxX) / 2, avey - fabs(minY - maxY) / 2, avex + fabs(minX - maxX) / 2, avey + fabs(minY - maxY) / 2);
    settextrotation(0);
    setcolor(BLACK);
    setfontsize(10);

    //drawrect(minX, minY, maxX, maxY);

    //drawtext(featureNameBox.get_center(), featureName, fabs(minX - maxX), fabs(minY - maxY));
    drawtext_in(featureNameBox, featureName);

    //drawtext(avex, avey, featureName, fabs(minX - maxX), fabs(minY - maxY));

}

void draw_open_feature_name(std::vector<LatLon> feature_points_vector, unsigned tail, unsigned feature_id) {

    int zoomLevel = getZoomValue();

    std::string featureName = map -> featureName_from_featureID[feature_id];

    if (featureName == "<noname>" || featureName == "<Unnamed>")
        return;

    float x1 = longitude_to_cartesian(feature_points_vector[0].lon(), average_latitude);
    float y1 = latitude_to_cartesian(feature_points_vector[0].lat());

    float x2 = longitude_to_cartesian(feature_points_vector[feature_points_vector.size() - 1].lon(), average_latitude);
    float y2 = latitude_to_cartesian(feature_points_vector[feature_points_vector.size() - 1].lat());

    if (x1 > x2) {
        float temp = x1;
        x1 = x2;
        x2 = temp;
    }
    if (y1 > y2) {
        float temp = y1;
        y1 = y2;
        y2 = temp;
    }

    t_bound_box featureNameBox(x1, y1, x2, y2);

    settextrotation(0);
    setcolor(BLACK);
    setfontsize(10);

    //t_bound_box textBox(xCoordinate, yCoordinate, xCoordinate+10, xCoordinate+10);
    drawtext_in(featureNameBox, featureName);
    //drawtext(x1, y1, featureName, 0.1, 0.1);
}

void draw_streets() {
    for (unsigned i = 0; i < map->numberOfStreetSegments; ++i) {

        StreetSegmentInfo segment = map -> query_streetSegInfo_from_segID(i);

        unsigned numberOfCurves = segment.curvePointCount;

        unsigned roadTypeNumber = roadType(i);

        setRoadGraphics(roadTypeNumber);

        if (validToDrawRoad(roadTypeNumber)) {

            //if no curve points, compute distance between lat/lon both both ends of segment
            if (numberOfCurves == 0) {

                LatLon startPoint = map -> pos_from_intersectionID[segment.from];
                LatLon endPoint = map -> pos_from_intersectionID[segment.to];

                float x1 = longitude_to_cartesian(startPoint.lon(), average_latitude);
                float y1 = latitude_to_cartesian(startPoint.lat());
                float x2 = longitude_to_cartesian(endPoint.lon(), average_latitude);
                float y2 = latitude_to_cartesian(endPoint.lat());

                drawline(x1, y1, x2, y2);
                //                if (segmentInMapView(x1, x2, y1, y2)) {
                //                    //setcolor(BLACK);
                //                    
                //                }

            } else {

                //Drawing a line between each curve point in vector
                std::vector<LatLon> curvePoints = map -> curvePoints_from_segmentID[i];
                for (unsigned j = 0; j < numberOfCurves - 1; ++j) {

                    LatLon curvePoint = curvePoints[j];
                    LatLon curvePoint2 = curvePoints[j + 1];

                    float x1 = longitude_to_cartesian(curvePoint.lon(), average_latitude);
                    float y1 = latitude_to_cartesian(curvePoint.lat());
                    float x2 = longitude_to_cartesian(curvePoint2.lon(), average_latitude);
                    float y2 = latitude_to_cartesian(curvePoint2.lat());

                    drawline(x1, y1, x2, y2);
                    /*
                    if (segmentInMapView(x1, x2, y1, y2)) {
                        //setcolor(BLACK);
                        
                    }
                     * */
                }

                LatLon startPoint = map -> pos_from_intersectionID[segment.from];
                LatLon endPoint = map -> pos_from_intersectionID[segment.to];

                float x1 = longitude_to_cartesian(startPoint.lon(), average_latitude); //drawing first point to first curve point
                float y1 = latitude_to_cartesian(startPoint.lat());
                float x2 = longitude_to_cartesian(curvePoints[0].lon(), average_latitude);
                float y2 = latitude_to_cartesian(curvePoints[0].lat());
                //setcolor(BLACK);
                drawline(x1, y1, x2, y2);
                //                if (segmentInMapView(x1, x2, y1, y2)) {
                //                    
                //                }

                float x3 = longitude_to_cartesian(curvePoints[numberOfCurves - 1].lon(), average_latitude); //drawing last curve point to last point
                float y3 = latitude_to_cartesian(curvePoints[numberOfCurves - 1].lat());
                float x4 = longitude_to_cartesian(endPoint.lon(), average_latitude);
                float y4 = latitude_to_cartesian(endPoint.lat());
                drawline(x3, y3, x4, y4);
                //                if (segmentInMapView(x3, x4, y3, y4)) {
                //                    //setcolor(BLACK);
                //                    
                //                }

            }
        }
    }
}

void setColourBlind() {
    close_graphics();
    if (colourBlindMode) {
        t_color colour;
        colour.red = 255;
        colour.green = 255;
        colour.blue = 255;
        init_graphics("City Map", colour);
        allButtons();
        draw_screen();
    }
    if (!colourBlindMode) {
        t_color colour;
        colour.red = 234;
        colour.green = 230;
        colour.blue = 223;
        init_graphics("City Map", colour);
        allButtons();
        draw_screen();
    }
}

void draw_street_names() {
    t_bound_box world = get_visible_world();
    //street directions
    std::string pointRight = "--->";
    std::string pointLeft = "<---";
    unsigned i = 0;
    while (i < getNumberOfStreetSegments()) {
        StreetSegmentInfo segInfo = map -> query_streetSegInfo_from_segID(i);
        //<unknown> streets not printed
        if (map->streetName_from_streetID[segInfo.streetID] != "<unknown>") {
            unsigned startPos = segInfo.from;
            unsigned endPos = segInfo.to;
            //getting the position in terms of latitude and longitude
            LatLon posStart = map -> pos_from_intersectionID[startPos];
            float xPosStart = (posStart.lon() * cos(average_latitude));
            float yPosStart = (posStart.lat());
            LatLon posEnd;
            float xPosEnd;
            float yPosEnd;
            bool curvePoint = false;
            //dealing with curve points first
            if (segInfo.curvePointCount > 0) {
                std::vector<LatLon> curvePoints = map -> curvePoints_from_segmentID[i];
                posEnd = curvePoints[0];
                xPosEnd = (posEnd.lon() * cos(average_latitude));
                yPosEnd = (posEnd.lat());
                curvePoint = true;
            }//otherwise if not curved
            else {
                posEnd = map -> pos_from_intersectionID[endPos];
                xPosEnd = (posEnd.lon() * cos(average_latitude));
                yPosEnd = (posEnd.lat());
            }
            unsigned roadTypeNumber = roadType(i);
            setRoadGraphics(roadTypeNumber);
            if (validToDrawRoadText(roadTypeNumber)) {

                setcolor(BLACK);

                //setting up the text
                t_bound_box streetSeg(xPosStart, yPosStart, xPosEnd, yPosEnd);
                int degrees = 0;
                if (posStart.lat() < 0) {
                    yPosStart = abs(yPosStart);
                    yPosEnd = abs(yPosEnd);
                }
                if (posStart.lon() > 0) {
                    xPosStart = abs(xPosStart);
                    xPosEnd = abs(xPosEnd);
                }
                degrees = ((atan((yPosEnd - yPosStart) / (xPosEnd - xPosStart))) / DEG_TO_RAD);
                settextrotation(degrees);
                std::string nameOfStreet = map->streetName_from_streetID[segInfo.streetID];
                //if street is one way
                if (segInfo.oneWay) {
                    if ((xPosStart < xPosEnd && yPosStart < yPosEnd)
                            || (xPosStart == xPosEnd && yPosStart > yPosEnd)
                            || (xPosStart < xPosEnd && yPosStart > yPosEnd)
                            || (xPosStart < xPosEnd && yPosStart == yPosEnd)) {
                        std::string name = pointRight + nameOfStreet + pointRight;
                        drawtext(streetSeg.get_center(), name, fabs(xPosEnd - xPosStart), fabs(yPosEnd - yPosStart));
                    } else if ((xPosStart == xPosEnd && yPosStart < yPosEnd)
                            || (xPosStart > xPosEnd && yPosStart < yPosEnd)
                            || (xPosStart > xPosEnd && yPosStart == yPosEnd)
                            || (xPosStart > xPosEnd && yPosStart > yPosEnd)) {
                        std::string name = pointLeft + nameOfStreet + pointLeft;
                        drawtext(streetSeg.get_center(), name, fabs(xPosEnd - xPosStart), fabs(yPosEnd - yPosStart));
                    }
                } else {
                    drawtext(streetSeg.get_center(), nameOfStreet, fabs(xPosEnd - xPosStart), fabs(yPosEnd - yPosStart));
                }
            }
        }
        ++i;
    }
}

void draw_screen() {

    // if (goToredrawFunction) {
    //set the buffer to off screen
    set_drawing_buffer(OFF_SCREEN);
    // }
    //just the first draw screen will not be drawn off screen

    //draw only the search bar repeatedly if user is typing

    if ((searchFlagDraw == true || searchFlagDraw2 == true) && dropDownMenu) {
        draw_searchBar();

        draw_drop_downMenu();

        copy_off_screen_buffer_to_screen();

        return;
    }

    if (helpButtonActivated && !searchFlagDraw && !searchFlagDraw2) {
        draw_helpMenu();
        copy_off_screen_buffer_to_screen();

        return;
    }

    clearscreen();

    draw_features();

    draw_streets();

    draw_street_names();

    draw_feature_name();

    draw_searchBar();

    if (showPOIButton == false)
        drawAllPOI();
    else {
        drawSymbolPOI(poi);
    }


    if (isMarked.drawPins == true && intersectingStreets) {

        for (unsigned a = 0; a < isMarked.intersection_x_y.size(); ++a) {
            Surface intersectionMarker = load_png_from_file
                    ("libstreetmap/resources/intersectionMarker.png");
            //to draw medical
            set_coordinate_system(GL_WORLD);
            draw_surface(intersectionMarker, isMarked.intersection_x_y[a].first - get_visible_world().get_width() / 86,
                    isMarked.intersection_x_y[a].second + get_visible_world().get_height() / 31);
        }


    }

    if (highlightStreet) {
        highlightStreetFunction();
    }

    if (clickOnScreen && !findDistanceButtonPressed) {

        //draw the marker png only after a certain value of zoom in otherwise super laggy
        if (getZoomValue() >= 5) {
            Surface drawMarker = load_png_from_file("libstreetmap/resources/markerPin.png");

            //to draw marker
            set_coordinate_system(GL_WORLD);
            draw_surface(drawMarker, markerCoordinates.x - get_visible_world().get_width() / 86, markerCoordinates.y + get_visible_world().get_height() / 31);
        }
    }



    if (createPathEnable && doubleScreenClick >= 1) {
        Surface drawMarker = load_png_from_file("libstreetmap/resources/startNew.png");

        //to start flag
        set_coordinate_system(GL_WORLD);
        draw_surface(drawMarker, longitude_to_cartesian(clickedPoints[0].lon(), average_latitude) - get_visible_world().get_width() / 86,
                latitude_to_cartesian(clickedPoints[0].lat()) + get_visible_world().get_height() / 31);
    }

    if (createPathEnable && doubleScreenClick == 2) {
        Surface drawMarker = load_png_from_file("libstreetmap/resources/destinationNew.png");

        //to destination flag
        set_coordinate_system(GL_WORLD);
        draw_surface(drawMarker, longitude_to_cartesian(clickedPoints[1].lon(), average_latitude) - get_visible_world().get_width() / 86,
                latitude_to_cartesian(clickedPoints[1].lat()) + get_visible_world().get_height() / 31);
    }

    if (poiInformationBox.draw == true) {
        draw_POI_information_box();
    }

    if (isMarked.drawBox == true && !findPath) {
        draw_ID_information_box();
    }

    if (showPOIOptions) {
        draw_POI_Options();
    }

    if (drawFindPathError)
        draw_findPath_errorBox();

    if (drawPath == true) {
        draw_path_found();
        draw_travel_instructions();
    }


    copy_off_screen_buffer_to_screen();
}

void draw_map() {


    maximum_latitudeHOME = getIntersectionPosition(0).lat();
    minimum_latitudeHOME = maximum_latitudeHOME;

    maximum_longitudeHOME = getIntersectionPosition(0).lon();
    minimum_longitudeHOME = maximum_longitudeHOME;

    intersections.resize(map->intersectionNames_from_IDs.size());
    for (unsigned id = 0; id < getNumberOfIntersections(); id++) {
        intersections[id].position = getIntersectionPosition(id);
        intersections[id].name = getIntersectionName(id);

        maximum_latitudeHOME = std::max(maximum_latitudeHOME, intersections[id].position.lat());
        minimum_latitudeHOME = std::min(minimum_latitudeHOME, intersections[id].position.lat());

        maximum_longitudeHOME = std::max(maximum_longitudeHOME, intersections[id].position.lon());
        minimum_longitudeHOME = std::min(minimum_longitudeHOME, intersections[id].position.lon());
    }

    average_latitude = 0.5 * (maximum_latitudeHOME + minimum_latitudeHOME) * DEG_TO_RAD;

    //t_color(240, 237, 229);
    t_color colour;
    colour.red = 234;
    colour.green = 230;
    colour.blue = 223;
    init_graphics("City Map", colour);

    //creating the buttons 
    allButtons();

    set_visible_world(minimum_longitudeHOME * std::cos(average_latitude), minimum_latitudeHOME, maximum_longitudeHOME * std::cos(average_latitude), maximum_latitudeHOME);
    fullMapArea = get_visible_world().area();


    //changing event loop function from nullptrs to work on mouse click
    //and key board button presses etc

    set_mouse_move_input(true);


    event_loop(act_on_mousebutton, act_on_mousemove, act_on_keypress, draw_screen);

    close_graphics();

}

//checks if the passed street name is valid

bool checkStreetNameValidity(std::string & streetName) {

    //getting everything in the required format
    //First letter is uppercase all others lower case
    std::string realStreet = " ";
    streetName[0] = toupper(streetName[0]);

    int i = 0;

    //setting all characters after fist letter to lowercase 
    for (i = 1; streetName[i] != ' ' && i < streetName.length(); ++i)
        streetName[i] = tolower(streetName[i]);


    //now i is either at the end of the string or stopped at the first space
    if (streetName[i] == ' ') {
        if ((i + 1) < streetName.length()) {
            //now next word's first character is upper case
            ++i;
            streetName[i] = toupper(streetName[i]);
        }
        ++i;
        //make all the following ones lower case before next space
        while (i < streetName.length() && streetName[i] != ' ') {
            streetName[i] = tolower(streetName[i]);
            ++i;
        }
    }

    if (streetName[i] == ' ') {
        if ((i + 1) < streetName.length()) {
            //now next word's first character is upper case
            ++i;
            streetName[i] = toupper(streetName[i]);
        }
        ++i;
        //make all the following ones lower case before next space
        while (i < streetName.length() && streetName[i] != ' ') {
            streetName[i] = tolower(streetName[i]);
            ++i;
        }
    }

    if (streetName[i] == ' ') {
        if ((i + 1) < streetName.length()) {
            //now next word's first character is upper case
            ++i;
            streetName[i] = toupper(streetName[i]);
        }
        ++i;
        //make all the following ones lower case before next space
        while (i < streetName.length() && streetName[i] != ' ') {
            streetName[i] = tolower(streetName[i]);
            ++i;
        }
    }

    if (streetName[i] == ' ') {
        if ((i + 1) < streetName.length()) {
            //now next word's first character is upper case
            ++i;
            streetName[i] = toupper(streetName[i]);
        }
        ++i;
        //make all the following ones lower case before next space
        while (i < streetName.length() && streetName[i] != ' ') {
            streetName[i] = tolower(streetName[i]);
            ++i;
        }
    }

    if (streetName[i] == ' ') {
        if ((i + 1) < streetName.length()) {
            //now next word's first character is upper case
            ++i;
            streetName[i] = toupper(streetName[i]);
        }
        ++i;
        //make all the following ones lower case before next space
        while (i < streetName.length() && streetName[i] != ' ') {
            streetName[i] = tolower(streetName[i]);
            ++i;
        }
    }
    //exiting this while loop either when a space was seen or length is done

    std::vector<unsigned> streetExists = find_street_ids_from_name(streetName);

    //empty vector means that no street exists with that name
    std::cout << "JUST BEFORE AUTOCOMPLETE FUNC IN CHECK VALID: " << streetName << std::endl;
    if (streetExists.size() == 0) {
        autoComplete(streetName);
        return true;

    }//otherwise street exists
    else {
        std::cout << "Valid name" << std::endl;

        //int checkNum = checkQueries();

        return true;
    }
}


//acts every time a mouse button is pressed

void act_on_mousebutton(float x, float y, t_event_buttonPressed button_info) {

    t_point newPoint(x, y);
    t_point pointOnScreen = world_to_scrn(newPoint);

    float x1 = get_visible_world().left() + (get_visible_world().get_width() / 23);
    float y2 = get_visible_world().top() - (get_visible_world().get_height() / 23);

    float x2 = get_visible_world().left() + (get_visible_world().get_width() / 23) + (get_visible_world().get_width() / 4);
    float y1 = get_visible_world().top() - (get_visible_world().get_height() / 23) - (get_visible_world().get_height() / 14.88);

    //If mouse pointer is already on screen, get rid of the old image
    //if user clicks within the search bar 
    //so doesn't mess with marker on the screen




    if (button_info.button == 1) {
        if (getZoomValue() < 5)

            std::cout << "X Point is: " << pointOnScreen.x << "    " << "Y Point is: " << pointOnScreen.y << std::endl;
        if ((x < x2 && x > x1) && (y < y2 && y > y1)) {
            searchFlagDraw2 = false;
            set_keypress_input(true);
            arrayIndexCount = -1;
            searchFlagDraw = true;
            dropDownMenu = true;
            return;
        } else {
            if (!findPath)
                update_message("Please zoom in further and then click to see intersection");

            searchFlagDraw = false;
            set_keypress_input(false);
        }

        y1 = y1 - (get_visible_world().get_height() / 14.88);
        y2 = y2 - (get_visible_world().get_height() / 14.88);

        if ((x < x2 && x > x1) && (y < y2 && y > y1) && findPath) {
            searchFlagDraw = false;
            set_keypress_input(true);
            arrayIndexCount = -1;
            searchFlagDraw2 = true;
            dropDownMenu = true;
            return;
        } else {
            if (!findPath)
                update_message("Please zoom in further and then click to see intersection");

            searchFlagDraw2 = false;
            set_keypress_input(false);
        }

    }


    if (button_info.button == 1) {

        float x2 = get_visible_world().left() + (get_visible_world().get_width() / 23) + (get_visible_world().get_width() / 4);

        float y4 = get_visible_world().top() - (get_visible_world().get_height() / 23) - (get_visible_world().get_height() / 17);
        float menuY = y4 - (get_visible_world().get_height() / 17);

        float blankBoxX1 = x2 - (get_visible_world().get_width() / 16);
        float blankBoxY1 = menuY - (get_visible_world().get_height() / 5.25) - (get_visible_world().get_width() / 100);
        float blankBoxX2 = x2 - (get_visible_world().get_width() / 16) + (get_visible_world().get_width() / 100);
        float blankBoxY2 = menuY - (get_visible_world().get_height() / 5.25) + (get_visible_world().get_height() / 62.5) - (get_visible_world().get_width() / 100);

        if (x >= blankBoxX1 && x <= blankBoxX2 && y >= blankBoxY1 && y <= blankBoxY2 && !menuBoxClicked)
            menuBoxClicked = true;
        else if (x >= blankBoxX1 && x <= blankBoxX2 && y >= blankBoxY1 && y <= blankBoxY2 && menuBoxClicked) {
            menuBoxClicked = false;
            dontDisplayMenu = false;
        }

        float xX1 = x2 - (get_visible_world().get_width() / 43);
        float xY1 = menuY - (get_visible_world().get_height() / 25) - (get_visible_world().get_width() / 100);
        float xX2 = xX1 + (get_visible_world().get_width() / 100);
        float xY2 = xY1 + (get_visible_world().get_height() / 62.5);

        if (x >= xX1 && x <= xX2 && y >= xY1 && y <= xY2 && !xOnMenuClicked) {
            xOnMenuClicked = true;

            if (menuBoxClicked)
                dontDisplayMenu = true;

            searchFlagDraw = false;
        }
    }


    if (clickOnScreen) {

        clickOnScreen = false;
        draw_screen();
    }





    //Need a method to check if the user has clicked on a blank spot that is 
    //not an intersection
    //can also display nearest POI to that intersection like a food joint

    //now if intersection of 2 streets was clicked
    //need to show an icon here as well

    int i = 0;

    //total number of intersections
    //int totalNumberOfIntersections = getNumberOfIntersections();

    //left click
    if (button_info.button == 1) {

        markerCoordinates.x = x;
        markerCoordinates.y = y;



        //markerCordinates will be used in draw_screen to redraw


        //converting to lat and lon to find the closest Intersection
        float newXLon = x / cos(average_latitude);
        float newYLat = y;

        std::cout << "XLon = " << newXLon << std::endl;
        std::cout << "YLat = " << newYLat << std::endl;

        LatLon newPositionLonLat(newYLat, newXLon);
        //getting the closest intersection to the point that has been clicked
        unsigned closestIntersecion = find_closest_intersection(newPositionLonLat);

        if (findDistanceButtonPressed == false) {
            displayIntersectionMarkerInfo(newXLon, newYLat, closestIntersecion);
        } else {
            if (doubleScreenClick == 1) {
                clickedPoints.push_back(newPositionLonLat);
                double distance = find_distance_between_two_points(clickedPoints[0], clickedPoints[1]);
                std::cout << "Distance between these points is: " << distance / 1000 << " km" << std::endl;
                std::string s = boost::lexical_cast<std::string>(distance / 1000);


                unsigned intersectionID1 = find_closest_intersection(clickedPoints[0]);
                unsigned intersectionID2 = find_closest_intersection(clickedPoints[1]);

                clickCreatedPath = find_path_between_intersections(intersectionID1, intersectionID2, 0);
                travelInstructions = travel_instructions(clickCreatedPath);
                foundClickCreatedPath = true;
                drawPath = true;


                clickedPoints.clear();
                update_message("Distance between these points is: " + s + " km");
                doubleScreenClick++;
            }

            if (doubleScreenClick == 0) {
                clickedPoints.push_back(newPositionLonLat);
                update_message("Please click on Point 2");
                doubleScreenClick++;
            }
        }
    }
    if (button_info.button == 1 && showPOIOptions == true) {
        for (unsigned i = 0; i < 14; i++) {
            if (x > POIboxes[i].left() && x < POIboxes[i].right() && y > POIboxes[i].bottom() && y < POIboxes[i].top()) {
                poi = i + 1;

                break;
            }
        }

        showPOIOptions = false;
        showPOIButton = true;
    }

    //searchFlagDraw = false;
    clickOnScreen = true;
    draw_screen();


}

void draw_POI_information_box() {

    float POIx = poiInformationBox.POIx;
    float POIy = poiInformationBox.POIy;
    unsigned poiID = poiInformationBox.poiID;

    //std::cout << POIx << "      " << POIy << std::endl;

    std::string poiCategory = getCategoryPOINames();

    std::vector<std::string> poiTypesVector = map -> POItypes_of_POIcategory[poiCategory];
    std::string poiType = poiTypesVector[poiID];

    std::vector<std::string> poiNamesVector = map -> POInames_of_POIcategory[poiCategory];
    std::string poiName = poiNamesVector[poiID];

    t_bound_box text_bbox = t_bound_box(POIx - 0.002, POIy + 0.0001, POIx + 0.002, POIy + 0.0001 + 0.001);

    setcolor(255, 128, 0);
    fillrect(POIx - 0.002 - 0.00005, POIy + 0.0001 - 0.00005, POIx + 0.002 + 0.00005, POIy + 0.0007 + 0.00005);
    setcolor(255, 218, 200);
    fillrect(POIx - 0.002, POIy + 0.0001, POIx + 0.002, POIy + 0.0007);
    setfontsize(10);
    setcolor(BLACK);
    settextrotation(0);
    drawtext_in(text_bbox, poiType);

    float yRatio = 0.0001 + 0.00008;

    t_bound_box text_bbox2 = t_bound_box(POIx - 0.001, POIy + 0.0001 - 1.5 * yRatio, POIx + 0.001, POIy + 0.0001 - 1.5 * yRatio + 0.001);
    drawtext_in(text_bbox2, poiName);

}

bool cursorOnHealth = false;
bool curserOnFinance = false;

void act_on_mousemove(float x, float y) {

    // draw_screen();


    //std::cout << "MouseX: " << std::fixed << std::setprecision(15) << x << "\t\tMouseY: " << std::fixed << std::setprecision(15) << y << std::endl;

    int zoomValue = getZoomValue();

    if (showPOIButton == false) {
        //Health and Finance POIs are on the screen

        if (zoomValue > 7) {
            //std::cout << "showPOIButton = false" << std::endl;

            float POIx = 0;
            float POIy = 0;

            for (unsigned i = 0; i < map -> healthPOILatLon.size() && !curserOnFinance && !poiInformationBox.draw; ++i) {

                POIx = longitude_to_cartesian(map -> healthPOILatLon[i].lon(), average_latitude);
                POIy = latitude_to_cartesian(map -> healthPOILatLon[i].lat());

                if ((x_y_in_mapView(POIx, POIy)) && (x >= POIx) && (x <= POIx + 0.000286) && (y >= POIy - 0.000286) && (y <= POIy)) {


                    poiInformationBox.POIx = POIx;
                    poiInformationBox.POIy = POIy;
                    poiInformationBox.poiID = i;
                    poiInformationBox.draw = true;

                    poi = 2;

                    cursorOnHealth = true;

                }
            }

            for (unsigned i = 0; i < map -> financePOILatLon.size() && !poiInformationBox.draw && !cursorOnHealth; ++i) {
                POIx = longitude_to_cartesian(map -> financePOILatLon[i].lon(), average_latitude);
                POIy = latitude_to_cartesian(map -> financePOILatLon[i].lat());

                if ((x_y_in_mapView(POIx, POIy)) && (x >= POIx) && (x <= POIx + 0.000286) && (y >= POIy - 0.000286) && (y <= POIy)) {


                    poiInformationBox.POIx = POIx;
                    poiInformationBox.POIy = POIy;
                    poiInformationBox.poiID = i;
                    poiInformationBox.draw = true;

                    curserOnFinance = true;

                    poi = 9;

                    break;
                }
            }

            if (poiInformationBox.draw && !((x >= poiInformationBox.POIx) && (x <= poiInformationBox.POIx + 0.000286) && (y >= poiInformationBox.POIy - 0.000286) && (y <= poiInformationBox.POIy))) {
                poiInformationBox.draw = false;
                cursorOnHealth = false;
                curserOnFinance = false;
            }

        }
    } else {
        //Only specified POIs are on the screen
        std::vector<LatLon> specificPOIs = map -> POILatLonCategory_from_POInumber[poi - 1];

        for (unsigned i = 0; i < specificPOIs.size() && !poiInformationBox.draw; ++i) {
            float POIx = longitude_to_cartesian(specificPOIs[i].lon(), average_latitude);
            float POIy = latitude_to_cartesian(specificPOIs[i].lat());

            if (x_y_in_mapView(POIx, POIy) && (x >= POIx) && (x <= POIx + 0.000286) && (y >= POIy - 0.000286) && (y <= POIy)) {
                poiInformationBox.POIx = POIx;
                poiInformationBox.POIy = POIy;
                poiInformationBox.poiID = i;
                poiInformationBox.draw = true;
            }
        }
    }

    if (poiInformationBox.draw && !((x >= poiInformationBox.POIx) && (x <= poiInformationBox.POIx + 0.000286) && (y >= poiInformationBox.POIy - 0.000286) && (y <= poiInformationBox.POIy))) {

        poiInformationBox.draw = false;
    }
}

//passed  x and y in terms of Lat and Lot

void displayIntersectionMarkerInfo(float x, float y, unsigned closestIntersecion) {

    std::vector<std::string> names = find_intersection_street_names(closestIntersecion);


    //that is at least one street is there at the nearest intersection
    std::string output;
    if (names.size() != 0) {
        for (unsigned i = 0; i < names.size(); i++) {
            if (i != (names.size() - 1)) {
                std::cout << names[i] + " & ";
                output += names[i] + " & ";
            } else {
                std::cout << names[i] << std::endl;
                output += names[i];
            }
        }

        if (getZoomValue() > 5)
            update_message("closest intersection: " + output);
    }
}
//display the points of interest

void drawAllPOI() {
    int currentZoom = getZoomValue();
    if (currentZoom > 7) {
        //if this is true, POI's have already been drawn, need to erase them
        if (!displayAllPOI) {

            std::vector<std::string> healthPOInames = map -> POItypes_of_POIcategory["health"];
            std::vector<std::string> financePOInames = map -> POItypes_of_POIcategory["finance"];

            for (unsigned i = 0; i < map -> healthPOILatLon.size(); i += 3) {
                LatLon pos = map -> healthPOILatLon[i];

                std::string POIname = healthPOInames[i];

                float x = longitude_to_cartesian(pos.lon(), average_latitude);
                float y = latitude_to_cartesian(pos.lat());

                Surface medical = load_png_from_file("libstreetmap/resources/health.png");
                //to draw medical
                set_coordinate_system(GL_WORLD);
                draw_surface(medical, x, y);


            }

            for (unsigned i = 0; i < map -> financePOILatLon.size(); i += 3) {
                LatLon pos = map -> financePOILatLon[i];

                std::string POIname = financePOInames[i];

                float x = longitude_to_cartesian(pos.lon(), average_latitude);
                float y = latitude_to_cartesian(pos.lat());

                Surface finance = load_png_from_file("libstreetmap/resources/bank.png");
                //to draw medical
                set_coordinate_system(GL_WORLD);
                draw_surface(finance, x, y);
            }
        } else {

            //redraw screen without POI's
            //basically make them disappear
        }

    }
}

void showSpecificPOI(void (*drawscreen) (void)) {

    //    if (poiInformationBox.draw) {
    //        poiInformationBox.draw = false;
    //        draw_screen();
    //        poiInformationBox.draw = true;
    //    }

    showPOIOptions = true;

    //display this info on the screen
    std::cout << "Which specific POI's would you like see ? Enter the number of the POI below." << std::endl;

    //giving options to user
    std::cout << "1: Emergency Services" << std::endl;
    std::cout << "2: Health Care" << std::endl;
    std::cout << "3: Hotels" << std::endl;
    std::cout << "4: Entertainment" << std::endl;
    std::cout << "5: Fitness" << std::endl;
    std::cout << "6: Public Services" << std::endl;
    std::cout << "7: Parking" << std::endl;
    std::cout << "8: Shopping" << std::endl;
    std::cout << "9: ATM and Banks" << std::endl;
    std::cout << "10: Transportation" << std::endl;
    std::cout << "11: Gas" << std::endl;
    std::cout << "12: Food" << std::endl;
    std::cout << "13: Education" << std::endl;
    std::cout << "14: Government Buildings" << std::endl;

    draw_screen();


}

std::string getCategoryPOINames() {

    std::string POItype;

    switch (poi) {
        case 1:
            POItype = "emergency";
            return POItype;
            break;
        case 2:
            POItype = "health";
            return POItype;
            break;
        case 3:
            POItype = "hotel";
            return POItype;
            break;
        case 4:
            POItype = "entertainment";
            return POItype;
            break;
        case 5:
            POItype = "fitness";
            return POItype;
            break;
        case 6:
            POItype = "public";
            return POItype;
            break;
        case 7:
            POItype = "parking";
            return POItype;
            break;
        case 8:
            POItype = "shopping";
            return POItype;
            break;
        case 9:
            //std::vector<std::string> test = "finance";
            POItype = "finance";
            return POItype;
            break;
        case 10:
            POItype = "transportation";
            return POItype;
            break;
        case 11:
            POItype = "gas";
            return POItype;
            break;
        case 12:
            POItype = "food";
            return POItype;
            break;
        case 13:
            POItype = "education";
            return POItype;
            break;
        case 14:
            POItype = "govt";
            return POItype;

            break;
        default:
            break;
    }


}

void drawSymbolPOI(int poi) {
    std::string drawThis;

    int currentZoom = getZoomValue();

    if (currentZoom > 2) {
        //if fitness type POI
        if (poi == 5) {
            for (unsigned i = 0; i < map -> fitnessPOILatLon.size(); i++) {
                //now have to draw things only in the visible area
                //pos is an object that gets the position of the POI's lat an lon

                //gets the position of the POI
                LatLon pos = map -> fitnessPOILatLon[i];

                float x = longitude_to_cartesian(pos.lon(), average_latitude);
                float y = latitude_to_cartesian(pos.lat());
                //only check if its in the visible area
                if (segmentInMapView(x, x, y, y)) {
                    //fitness
                    //drawing here
                    Surface gym = load_png_from_file("libstreetmap/resources/gym.png");
                    //to draw medical
                    set_coordinate_system(GL_WORLD);
                    draw_surface(gym, x, y);
                }
            }
        }//if emergency type POI
        else if (poi == 1) {
            for (unsigned i = 0; i < map -> emergencyPOILatLon.size(); i++) {
                //now have to draw things only in the visible area
                //pos is an object that gets the position of the POI's lat an lon

                //gets the position of the POI
                LatLon pos = map -> emergencyPOILatLon[i];

                float x = longitude_to_cartesian(pos.lon(), average_latitude);
                float y = latitude_to_cartesian(pos.lat());
                //only check if its in the visible area
                if (segmentInMapView(x, x, y, y)) {
                    //fitness
                    //drawing here
                    Surface emergency = load_png_from_file("libstreetmap/resources/emergency.png");
                    //to draw medical
                    set_coordinate_system(GL_WORLD);
                    draw_surface(emergency, x, y);
                }
            }
        }//if emergency type POI
        else if (poi == 2) {
            for (unsigned i = 0; i < map -> healthPOILatLon.size(); i++) {
                //now have to draw things only in the visible area
                //pos is an object that gets the position of the POI's lat an lon

                //gets the position of the POI
                LatLon pos = map -> healthPOILatLon[i];

                float x = longitude_to_cartesian(pos.lon(), average_latitude);
                float y = latitude_to_cartesian(pos.lat());
                //only check if its in the visible area
                if (segmentInMapView(x, x, y, y)) {
                    //fitness
                    //drawing here
                    Surface health = load_png_from_file("libstreetmap/resources/health.png");
                    //to draw medical
                    set_coordinate_system(GL_WORLD);
                    draw_surface(health, x, y);
                }
            }
        } else if (poi == 4) {
            for (unsigned i = 0; i < map -> entertainmentPOILatLon.size(); i++) {
                //now have to draw things only in the visible area
                //pos is an object that gets the position of the POI's lat an lon

                //gets the position of the POI
                LatLon pos = map -> entertainmentPOILatLon[i];

                float x = longitude_to_cartesian(pos.lon(), average_latitude);
                float y = latitude_to_cartesian(pos.lat());
                //only check if its in the visible area
                if (segmentInMapView(x, x, y, y)) {
                    //fitness
                    //drawing here
                    Surface entertainment = load_png_from_file("libstreetmap/resources/entertainment.png");
                    //to draw medical
                    set_coordinate_system(GL_WORLD);
                    draw_surface(entertainment, x, y);
                }
            }
        } else if (poi == 9) {
            for (unsigned i = 0; i < map -> financePOILatLon.size(); i++) {
                //now have to draw things only in the visible area
                //pos is an object that gets the position of the POI's lat an lon

                //gets the position of the POI
                LatLon pos = map -> financePOILatLon[i];

                float x = longitude_to_cartesian(pos.lon(), average_latitude);
                float y = latitude_to_cartesian(pos.lat());
                //only check if its in the visible area
                if (segmentInMapView(x, x, y, y)) {
                    //fitness
                    //drawing here
                    Surface bank = load_png_from_file("libstreetmap/resources/bank.png");
                    //to draw medical
                    set_coordinate_system(GL_WORLD);
                    draw_surface(bank, x, y);
                }
            }
        } else if (poi == 10) {
            for (unsigned i = 0; i < map -> transportationPOILatLon.size(); i++) {
                //now have to draw things only in the visible area
                //pos is an object that gets the position of the POI's lat an lon

                //gets the position of the POI
                LatLon pos = map -> transportationPOILatLon[i];

                float x = longitude_to_cartesian(pos.lon(), average_latitude);
                float y = latitude_to_cartesian(pos.lat());
                //only check if its in the visible area
                if (segmentInMapView(x, x, y, y)) {
                    //fitness
                    //drawing here
                    Surface transportation = load_png_from_file("libstreetmap/resources/transportation.png");
                    //to draw medical
                    set_coordinate_system(GL_WORLD);
                    draw_surface(transportation, x, y);
                }
            }
        } else if (poi == 6) {
            for (unsigned i = 0; i < map -> publicPOILatLon.size(); i++) {
                //now have to draw things only in the visible area
                //pos is an object that gets the position of the POI's lat an lon

                //gets the position of the POI
                LatLon pos = map -> publicPOILatLon[i];

                float x = longitude_to_cartesian(pos.lon(), average_latitude);
                float y = latitude_to_cartesian(pos.lat());
                //only check if its in the visible area
                if (segmentInMapView(x, x, y, y)) {
                    //fitness
                    //drawing here
                    Surface publicPNG = load_png_from_file("libstreetmap/resources/public.png");
                    //to draw medical
                    set_coordinate_system(GL_WORLD);
                    draw_surface(publicPNG, x, y);
                }
            }
        } else if (poi == 7) {
            for (unsigned i = 0; i < map -> parkingPOILatLon.size(); i++) {
                //now have to draw things only in the visible area
                //pos is an object that gets the position of the POI's lat an lon

                //gets the position of the POI
                LatLon pos = map -> parkingPOILatLon[i];

                float x = longitude_to_cartesian(pos.lon(), average_latitude);
                float y = latitude_to_cartesian(pos.lat());
                //only check if its in the visible area
                if (segmentInMapView(x, x, y, y)) {
                    //fitness
                    //drawing here
                    Surface parking = load_png_from_file("libstreetmap/resources/parking.png");
                    //to draw medical
                    set_coordinate_system(GL_WORLD);
                    draw_surface(parking, x, y);
                }
            }
        } else if (poi == 8) {
            for (unsigned i = 0; i < map -> shoppingPOILatLon.size(); i++) {
                //now have to draw things only in the visible area
                //pos is an object that gets the position of the POI's lat an lon

                //gets the position of the POI
                LatLon pos = map -> shoppingPOILatLon[i];

                float x = longitude_to_cartesian(pos.lon(), average_latitude);
                float y = latitude_to_cartesian(pos.lat());
                //only check if its in the visible area
                if (segmentInMapView(x, x, y, y)) {
                    //fitness
                    //drawing here
                    Surface shopping = load_png_from_file("libstreetmap/resources/shopping.png");
                    //to draw medical
                    set_coordinate_system(GL_WORLD);
                    draw_surface(shopping, x, y);
                }
            }
        } else if (poi == 11) {
            for (unsigned i = 0; i < map -> gasPOILatLon.size(); i++) {
                //now have to draw things only in the visible area
                //pos is an object that gets the position of the POI's lat an lon

                //gets the position of the POI
                LatLon pos = map -> gasPOILatLon[i];

                float x = longitude_to_cartesian(pos.lon(), average_latitude);
                float y = latitude_to_cartesian(pos.lat());
                //only check if its in the visible area
                if (segmentInMapView(x, x, y, y)) {
                    //fitness
                    //drawing here
                    Surface gas = load_png_from_file("libstreetmap/resources/gas.png");
                    //to draw medical
                    set_coordinate_system(GL_WORLD);
                    draw_surface(gas, x, y);
                }
            }
        } else if (poi == 3) {
            for (unsigned i = 0; i < map -> hotelPOILatLon.size(); i++) {
                //now have to draw things only in the visible area
                //pos is an object that gets the position of the POI's lat an lon

                //gets the position of the POI
                LatLon pos = map -> hotelPOILatLon[i];

                float x = longitude_to_cartesian(pos.lon(), average_latitude);
                float y = latitude_to_cartesian(pos.lat());
                //only check if its in the visible area
                if (segmentInMapView(x, x, y, y)) {
                    //fitness
                    //drawing here
                    Surface hotel = load_png_from_file("libstreetmap/resources/hotel.png");
                    //to draw medical
                    set_coordinate_system(GL_WORLD);
                    draw_surface(hotel, x, y);
                }
            }
        } else if (poi == 12) {
            for (unsigned i = 0; i < map -> foodPOILatLon.size(); i++) {
                //now have to draw things only in the visible area
                //pos is an object that gets the position of the POI's lat an lon

                //gets the position of the POI
                LatLon pos = map -> foodPOILatLon[i];

                float x = longitude_to_cartesian(pos.lon(), average_latitude);
                float y = latitude_to_cartesian(pos.lat());
                //only check if its in the visible area
                if (segmentInMapView(x, x, y, y)) {
                    //fitness
                    //drawing here
                    Surface food = load_png_from_file("libstreetmap/resources/food.png");
                    //to draw medical
                    set_coordinate_system(GL_WORLD);
                    draw_surface(food, x, y);
                }
            }
        } else if (poi == 13) {
            for (unsigned i = 0; i < map -> educationPOILatLon.size(); i++) {
                //now have to draw things only in the visible area
                //pos is an object that gets the position of the POI's lat an lon

                //gets the position of the POI
                LatLon pos = map -> educationPOILatLon[i];

                float x = longitude_to_cartesian(pos.lon(), average_latitude);
                float y = latitude_to_cartesian(pos.lat());
                //only check if its in the visible area
                if (segmentInMapView(x, x, y, y)) {
                    //fitness
                    //drawing here
                    Surface education = load_png_from_file("libstreetmap/resources/education.png");
                    //to draw medical
                    set_coordinate_system(GL_WORLD);
                    draw_surface(education, x, y);
                }
            }
        } else if (poi == 14) {
            for (unsigned i = 0; i < map -> govtPOILatLon.size(); i++) {
                //now have to draw things only in the visible area
                //pos is an object that gets the position of the POI's lat an lon

                //gets the position of the POI
                LatLon pos = map -> govtPOILatLon[i];

                float x = longitude_to_cartesian(pos.lon(), average_latitude);
                float y = latitude_to_cartesian(pos.lat());
                //only check if its in the visible area
                if (segmentInMapView(x, x, y, y)) {
                    //fitness
                    //drawing here

                    Surface govt = load_png_from_file("libstreetmap/resources/govt.png");
                    //to draw medical
                    set_coordinate_system(GL_WORLD);
                    draw_surface(govt, x, y);
                }
            }
        }

    }

}

void find_path_onEnter() {

    drawPath = true;

    float minX = INFINITY;
    float maxX = -INFINITY;
    float minY = INFINITY;
    float maxY = -INFINITY;



    //path = find_path_between_intersections(11666, 54122, 0);
    if (!createPathEnable)
        travelInstructions = travel_instructions(path);

    for (unsigned i = 0; i < path.size(); ++i) {//loop through all segID

        StreetSegmentInfo segment = map -> query_streetSegInfo_from_segID(path[i]); //get one segment
        std::string streetName = map -> streetName_from_streetID[segment.streetID];
        unsigned numberOfCurves = segment.curvePointCount;

        float x1 = (map -> pos_from_intersectionID[segment.from]).lon() * cos(average_latitude);
        float x2 = (map -> pos_from_intersectionID[segment.to]).lon() * cos(average_latitude);
        float y1 = (map -> pos_from_intersectionID[segment.from]).lat();
        float y2 = (map -> pos_from_intersectionID[segment.to]).lat();

        if (x1 < minX)
            minX = x1;
        if (x2 < minX)
            minX = x2;

        if (y1 < minY)
            minY = y1;
        if (y2 < minY)
            minY = y2;

        if (x1 > maxX)
            maxX = x1;
        if (x2 > maxX)
            maxX = x2;

        if (y1 > maxY)
            maxY = y1;
        if (y2 > maxY)
            maxY = y2;
    }


    //set_visible_world(minX - 0.001, minY - 0.001, maxX + 0.001, maxY + 0.001);
    set_visible_world(minX, minY, maxX, maxY);
    float xLeftOffSet = get_visible_world().get_width() / 3;
    //float xRightOffSet = get_visible_world().get_width() / 30;
    set_visible_world(minX - xLeftOffSet, minY - xLeftOffSet, maxX + xLeftOffSet, maxY + xLeftOffSet);

    for (unsigned i = 0; i < travelInstructions.size(); i++) {
        std::cout << travelInstructions[i] << std::endl;
    }

}



//draw search bar

void act_on_keypress(char key_pressed, int keysym) {


    if (searchFlagDraw2 && userInputSecondSB == "Search for an intersection or street") {
        userInputSecondSB.clear();
    } else if (searchFlagDraw && userInputDefaultSB == "Search for an intersection or street") {
        userInputDefaultSB.clear();
    }

    std::string street1;
    std::string street2;

    parsedValidity = true;

    std::string delim1 = " and ";
    std::string delim2 = " & ";

    if (searchFlagDraw) {
        //input a character other than backspace and enter
        if (key_pressed != NULL && keysym != XK_BackSpace && keysym != XK_Return && keysym != XK_Down && keysym != XK_Up) {
            arrowKeysPressed = false;
            userInputDefaultSB.push_back(key_pressed);\

            if (userInputDefaultSB.find(delim1) != std::string::npos || userInputDefaultSB.find(delim2) != std::string::npos) {
                queryIsAnIntersection = true;
                autoCompleteIntersection();
            } else {
                autoComplete2();
                queryIsAnIntersection = false;
            }
        }
        if (keysym == XK_Down && arrayIndexCount != 3 && userInputDefaultSB.length() > 0) {
            arrayIndexCount++;
            arrowKeysPressed = true;
        }
        if (keysym == XK_Up && arrayIndexCount != 0 && userInputDefaultSB.length() > 0) {
            arrayIndexCount--;
            arrowKeysPressed = true;
        }
        //searchFlagDraw = true;

        if (keysym == XK_BackSpace && userInputDefaultSB.length() > 0) //if backspace is pressed
        {
            if (userInputDefaultSB.length() == 0 && userInputSecondSB.length() == 0) {
                dropDownMenu = false;
            }

            arrowKeysPressed = false;
            userInputDefaultSB.pop_back();

            if (userInputDefaultSB.find(delim1) != std::string::npos || userInputDefaultSB.find(delim2) != std::string::npos) {
                autoCompleteIntersection();
            } else {
                autoComplete2();
            }

            float x1 = get_visible_world().left() + (get_visible_world().get_width() / 23);
            float y2 = get_visible_world().top() - (get_visible_world().get_height() / 23) - 121;

            float x2 = get_visible_world().left() + (get_visible_world().get_width() / 23) + (get_visible_world().get_width() / 4);
            float y1 = get_visible_world().top() - (get_visible_world().get_height() / 23) - (get_visible_world().get_height() / 14.88 - 121);

            t_bound_box box(x1, y1, x2, y2);

            //user input text style
            settextrotation(0);
            setcolor(BLACK);
            setfontsize(11);

        } else if (keysym == XK_Return && userInputDefaultSB.size() > 0) //if enter is pressed 
        {

            dropDownMenu = false;

            if (!findPath)
                isMarked.drawBox = true;

            if (arrowKeysPressed == true) {
                userInputDefaultSB = closestStrings[arrayIndexCount];
                streetToBeHighlighted = userInputDefaultSB;
            }

            std::stringstream linestream(userInputDefaultSB);

            if (highlightStreet)
                highlightStreet = false;

            isMarked.drawBox = true;

            //searchFlagDraw = false;

            //street 1 and 2 are cleared when the user presses enter
            std::string newString;

            street1.clear();
            street2.clear();

            //new input holds the users complete input
            if (arrowKeysPressed) {
                newString = closestStrings[arrayIndexCount];
            }
            linestream >> newString;

            street1 = newString;
            //searchFlagDraw = false;

            bool flag = true;
            //parse for street1 name 
            while (!linestream.eof() && flag) {
                linestream >> newString;
                //if its an and that means next street name will be inputted

                if (newString == "&" || newString == "and") {
                    flag = false; //if flag goes to false, expect next name
                    queryIsAnIntersection = true;
                } else if (flag == true) {
                    //solves the problem if someone writes college street 
                    // now college street can be stored into one string
                    std::string c = " ";
                    street1.append(c);
                    street1.append(newString);
                }
            }

            std::cout << "PREEEE checkStreetNameValidity with :" << street1 << std::endl;

            //std::cout << "s1:" << street1 << std::endl;
            bool checkValidity1 = checkStreetNameValidity(street1);
            if (!checkValidity1) {
                update_message("Street does not exist");
            }

            std::cout << "CAME BACK FROM checkStreetNameValidity1 with :" << street1 << std::endl;

            //read the second street name now

            if (!linestream.eof()) {
                linestream >> newString;
                street2.append(newString);
            }

            //keep read for street2 name 
            while (!linestream.eof()) {
                linestream>>newString;
                std::string a = " ";
                street2.append(a);
                street2.append(newString);
            }

            //"and" was not entered and street 2 not inputted

            if (flag && street2.length() == 0) {
                //highlight the whole street
                if (findPath == false)
                    highlightStreet = true;

                if (arrowKeysPressed == true) {
                    streetToBeHighlighted = userInputDefaultSB;
                } else {
                    streetToBeHighlighted = street1; //highlights street
                }
                draw_screen();

                if (!checkValidity1) {
                    parsedValidity = false;
                }

                //userInputDefaultSB = "Search for an intersection or street";
                set_keypress_input(false); //goes to true on mouse click

                if (keysym == XK_Return && userInputDefaultSB.length() > 0 && userInputDefaultSB != "Search for an intersection or street" && userInputSecondSB.length() > 0 && userInputSecondSB != "Search for an intersection or street") {
                    //                    int error = checkQueries();
                    //                    if (error >= 6) {
                    //                        drawFindPathError = true;
                    //                    } else {
                    //                        drawFindPathError = false;
                    //                    }

                    int check = checkQueries();

                    std::cout << "checkQueries " << check << std::endl;

                    if (check == 2) {
                        unsigned Ids = map -> intersectionIDs_from_intersectionName[userInputDefaultSB];

                        path = find_path_to_point_of_interest(Ids, userInputSecondSB, 0);

                    } else if (check == 3) {
                        unsigned Ids = map -> intersectionIDs_from_intersectionName[userInputSecondSB];

                        path = find_path_to_point_of_interest(Ids, userInputDefaultSB, 0);
                    } else if (check == 4) {

                        //unsigned Ids = map -> intersectionIDs_from_intersectionName[userInputDefaultSB];
                        //unsigned Ids2 = map -> intersectionIDs_from_intersectionName[userInputSecondSB];



                        std::vector<unsigned> id = find_intersection_ids_from_street_names(streetName1, streetName2);
                        std::vector<unsigned> id2 = find_intersection_ids_from_street_names(streetName3, streetName4);


                        path = find_path_between_intersections(id[0], id2[0], 0);

                        //std::cout << "path.size() " << path.size() << std::endl;

                    }

                    if (findPath) {
                        find_path_onEnter();
                    }
                }

                return;
            }

            //std::cout << "s2:" << street2 << std::endl;
            bool checkValidity2 = checkStreetNameValidity(street2);
            if (!checkValidity2) {
                update_message("Street does not exist");
            }

            std::cout << "CAME BACK FROM checkStreetNameValidity with :" << street1 << std::endl;

            //invalid street names

            if (!checkValidity1 || !checkValidity2) {
                parsedValidity = false;
                isMarked.drawPins = false;
                isMarked.drawBox = true;
            }

            if (parsedValidity && !findPath)//means street names are valid
                drawIntersectionIfExist(street1, street2);
            //userInputDefaultSB = "Search for an intersection or street";

            set_keypress_input(false); //goes to true on mouse click
        } else if (userInputDefaultSB.length() <= 0 && keysym == XK_Return && !findPath) {

            update_message("Invalid street name");
            parsedValidity = false;
        }
        //        if (findPath) {
        //            std::cout << checkQueries() << std::endl;
        //        }
    } else if (searchFlagDraw2) {
        //input a character other than backspace and enter
        if (key_pressed != NULL && keysym != XK_BackSpace && keysym != XK_Return && keysym != XK_Down && keysym != XK_Up) {
            arrowKeysPressed = false;

            userInputSecondSB.push_back(key_pressed);

            if (userInputSecondSB.find(delim1) != std::string::npos || userInputSecondSB.find(delim2) != std::string::npos) {
                queryIsAnIntersection = true;
                autoCompleteIntersection();
            } else {
                autoComplete2();
                queryIsAnIntersection = false;
            }
        }
        if (keysym == XK_Down && arrayIndexCount != 3 && userInputSecondSB.length() > 0) {
            arrayIndexCount++;
            arrowKeysPressed = true;
        }
        if (keysym == XK_Up && arrayIndexCount != 0 && userInputSecondSB.length() > 0) {
            arrayIndexCount--;
            arrowKeysPressed = true;
        }
        //searchFlagDraw = true;

        if (keysym == XK_BackSpace && userInputSecondSB.length() > 0) //if backspace is pressed
        {
            if (userInputDefaultSB.length() == 0 && userInputSecondSB.length() == 0) {
                dropDownMenu = false;
            }

            arrowKeysPressed = false;
            userInputSecondSB.pop_back();

            if (userInputSecondSB.find(delim1) != std::string::npos || userInputSecondSB.find(delim2) != std::string::npos) {
                autoCompleteIntersection();
            } else {
                autoComplete2();
            }

            float x1 = get_visible_world().left() + (get_visible_world().get_width() / 23);
            float y2 = get_visible_world().top() - (get_visible_world().get_height() / 23) - 121;

            float x2 = get_visible_world().left() + (get_visible_world().get_width() / 23) + (get_visible_world().get_width() / 4);
            float y1 = get_visible_world().top() - (get_visible_world().get_height() / 23) - (get_visible_world().get_height() / 14.88 - 121);

            t_bound_box box(x1, y1, x2, y2);

            //user input text style
            settextrotation(0);
            setcolor(BLACK);
            setfontsize(11);

        } else if (keysym == XK_Return && userInputSecondSB.size() > 0) //if enter is pressed 
        {

            dropDownMenu = false;

            if (!findPath)
                isMarked.drawBox = true;

            //            if (userInputDefaultSB == "Search for an intersection or street" || userInputDefaultSB.length() == 0) {
            //                firstSBEmpty = true;
            //                update_message("First Search Bar is blank. Please enter a starting point");
            //            }
            //
            //            if (userInputSecondSB == "Search for an intersection or street" || userInputSecondSB.length() == 0) {
            //                secondSBEmpty = true;
            //                update_message("Second Search Bar is blank. Please enter a destination");
            //            }

            if (arrowKeysPressed == true) {
                userInputSecondSB = closestStrings[arrayIndexCount];
                streetToBeHighlighted = userInputSecondSB;
            }



            std::stringstream linestream(userInputSecondSB);

            if (highlightStreet)
                highlightStreet = false;

            isMarked.drawBox = true;

            searchFlagDraw = false;

            //street 1 and 2 are cleared when the user presses enter
            std::string newString;

            street1.clear();
            street2.clear();

            //new input holds the users complete input
            if (arrowKeysPressed) {
                newString = closestStrings[arrayIndexCount];
            }
            linestream >> newString;

            street1 = newString;
            //searchFlagDraw = false;

            bool flag = true;
            //parse for street1 name 
            while (!linestream.eof() && flag) {
                linestream >> newString;
                //if its an and that means next street name will be inputted

                if (newString == "&" || newString == "and") {
                    flag = false; //if flag goes to false, expect next name
                    queryIsAnIntersection = true;
                } else if (flag == true) {
                    //solves the problem if someone writes college street 
                    // now college street can be stored into one string
                    std::string c = " ";
                    street1.append(c);
                    street1.append(newString);
                }
            }

            std::cout << "PREEEE checkStreetNameValidity with :" << street1 << std::endl;

            //std::cout << "s1:" << street1 << std::endl;
            bool checkValidity1 = checkStreetNameValidity(street1);
            if (!checkValidity1) {
                update_message("Street does not exist");
            }

            std::cout << "CAME BACK FROM checkStreetNameValidity with :" << street1 << std::endl;

            //read the second street name now

            if (!linestream.eof()) {
                linestream >> newString;
                street2.append(newString);
            }

            //keep read for street2 name 
            while (!linestream.eof()) {
                linestream>>newString;
                std::string a = " ";
                street2.append(a);
                street2.append(newString);
            }
            std::cout << "LEFT WHILTE LOOP WITH STREET :" << street2 << std::endl;
            //"and" was not entered and street 2 not inputted

            if (flag && street2.length() == 0) {
                std::cout << "ENTERED IF STATEMENT WITH :" << street2 << std::endl;
                //highlight the whole street
                if (findPath == false)
                    highlightStreet = true;

                if (arrowKeysPressed == true) {
                    streetToBeHighlighted = userInputSecondSB;
                } else {
                    streetToBeHighlighted = street1; //highlights street
                }
                draw_screen();

                if (!checkValidity1) {
                    parsedValidity = false;
                }

                //userInputSecondSB = "Search for an intersection or street";
                set_keypress_input(false); //goes to true on mouse click

                if (keysym == XK_Return && userInputDefaultSB.length() > 0 && userInputDefaultSB != "Search for an intersection or street" && userInputSecondSB.length() > 0 && userInputSecondSB != "Search for an intersection or street") {
                    //                    int error = checkQueries();
                    //                    if (error >= 6) {
                    //                        drawFindPathError = true;
                    //                    } else {
                    //                        drawFindPathError = false;
                    //                    }

                    int check = checkQueries();

                    if (check == 2) {
                        unsigned Ids = map -> intersectionIDs_from_intersectionName[userInputDefaultSB];

                        path = find_path_to_point_of_interest(Ids, userInputSecondSB, 0);

                    } else if (check == 3) {
                        unsigned Ids = map -> intersectionIDs_from_intersectionName[userInputSecondSB];

                        path = find_path_to_point_of_interest(Ids, userInputDefaultSB, 0);
                    } else if (check == 4) {
                        std::vector<unsigned> id = find_intersection_ids_from_street_names(streetName1, streetName2);
                        std::vector<unsigned> id2 = find_intersection_ids_from_street_names(streetName3, streetName4);


                        path = find_path_between_intersections(id[0], id2[0], 0);

                    }

                    if (findPath) {
                        find_path_onEnter();
                    }
                }

                return;
            }

            //std::cout << "s2:" << street2 << std::endl;
            bool checkValidity2 = checkStreetNameValidity(street2);
            if (!checkValidity2) {
                update_message("Street does not exist");
            }

            std::cout << "CAME BACK FROM checkStreetNameValidity2222 with :" << street2 << std::endl;

            //invalid street names

            if (!checkValidity1 || !checkValidity2) {
                parsedValidity = false;
                isMarked.drawPins = false;
                isMarked.drawBox = true;
            }

            if (parsedValidity && !findPath)//means street names are valid
                drawIntersectionIfExist(street1, street2);
            //userInputSecondSB = "Search for an intersection or street";

            set_keypress_input(false); //goes to true on mouse click
        } else if (userInputSecondSB.length() <= 0 && keysym == XK_Return && !findPath) {

            update_message("Invalid street name");
            parsedValidity = false;
        }

    }
    //    if (findPath) {
    //        std::cout << checkQueries() << std::endl;
    //    }



    // std::cout << "last" << userInput << std::endl;


    if (keysym == XK_Return && userInputDefaultSB.length() > 0 && userInputDefaultSB != "Search for an intersection or street" && userInputSecondSB.length() > 0 && userInputSecondSB != "Search for an intersection or street") {
        int check = checkQueries();

        if (check == 2) {
            unsigned Ids = map -> intersectionIDs_from_intersectionName[userInputDefaultSB];

            path = find_path_to_point_of_interest(Ids, userInputSecondSB, 0);

        } else if (check == 3) {
            unsigned Ids = map -> intersectionIDs_from_intersectionName[userInputSecondSB];

            path = find_path_to_point_of_interest(Ids, userInputDefaultSB, 0);
        } else if (check == 4) {
            std::vector<unsigned> id = find_intersection_ids_from_street_names(streetName1, streetName2);
            std::vector<unsigned> id2 = find_intersection_ids_from_street_names(streetName3, streetName4);


            path = find_path_between_intersections(id[0], id2[0], 0);

        }

    }


    draw_screen();

}


//now its verified streets exist

void drawIntersectionIfExist(std::string &street1, std::string & street2) {

    //every time I come into this function I need to display an output
    //either no intersection or yes intersection

    isMarked.drawBox = true;

    //    if (isMarked.drawPins) {
    //        std::cout << "mRK XEHC" << std::endl;
    //        isMarked.drawPins = false;
    //        draw_screen();
    //    }

    //    if (!showIntersections) {
    //        showIntersections = true;

    //returns a vector of intersections
    if (queryIsAnIntersection) {
        closestStrings[arrayIndexCount];
    }

    std::vector<unsigned> finder = find_intersection_ids_from_street_names(street1, street2);


    //checking if there are no intersections
    if (finder.size() == 0) {
        std::cout << "finder size: " << finder.size() << std::endl;

        //isMarked.drawPins = false;
        std::cout << "Sorry these streets do not intersect" << std::endl;
        intersectingStreets = false;
        //now print no intersection message on screen
    } else {//now they definitely intersect
        std::string intersectionName;
        isMarked.namesVector.clear();
        isMarked.lonVector.clear();
        isMarked.latVector.clear();
        isMarked.intersection_x_y.clear();
        isMarked.drawPins = true;
        intersectingStreets = true;
        unsigned i = 0;
        while (i < finder.size()) {



            //std::cout<< finder[i] << std::endl;

            //get intersection position
            LatLon intersectionPosition = getIntersectionPosition(finder[i]);

            //get intersection name     
            if (arrowKeysPressed) {
                intersectionName = closestStrings[arrayIndexCount];
            } else {
                intersectionName = map->intersectionNames_from_IDs[finder[i]];
            }


            std::cout << "Found the intersection!" << std::endl;
            std::cout << "Intersection name: " << intersectionName << std::endl;

            //Now to print the intersection position
            if (intersectionPosition.lat() < 0) {
                std::cout << intersectionPosition.lat() << "° " << "W" << std::endl;
            } else
                std::cout << intersectionPosition.lat() << "° " << "E" << std::endl;

            //printing latitude now
            if (intersectionPosition.lon() < 0) {
                std::cout << intersectionPosition.lon() << "° " << "S" << std::endl;
            } else std::cout << intersectionPosition.lon() << "° " << "N" << std::endl;

            double intersectionXPosition = longitude_to_cartesian(intersectionPosition.lon(), average_latitude);
            double intersectionYPosition = latitude_to_cartesian(intersectionPosition.lat());

            isMarked.namesVector.push_back(intersectionName);
            isMarked.lonVector.push_back(intersectionPosition.lon());
            isMarked.latVector.push_back(intersectionPosition.lat());

            isMarked.intersection_x_y.push_back(std::pair<double, double>());
            isMarked.intersection_x_y[i].first = intersectionXPosition;
            isMarked.intersection_x_y[i].second = intersectionYPosition;



            //isMarked.xCoordinate = intersectionXPosition;
            //isMarked.yCoordinate = intersectionYPosition;

            std::cout << intersectionXPosition << "   " << intersectionYPosition << std::endl;

            std::cout << intersectionXPosition << "   " << intersectionYPosition << std::endl;

            ++i;
            //}

            if (isMarked.drawPins == true) {
                float minX = isMarked.intersection_x_y[0].first;
                float maxX = isMarked.intersection_x_y[0].first;
                float minY = isMarked.intersection_x_y[0].second;
                float maxY = isMarked.intersection_x_y[0].second;

                for (unsigned a = 0; a < isMarked.intersection_x_y.size(); ++a) {

                    if (isMarked.intersection_x_y[a].first < minX)
                        minX = isMarked.intersection_x_y[a].first;

                    if (isMarked.intersection_x_y[a].first > maxX)
                        maxX = isMarked.intersection_x_y[a].first;

                    if (isMarked.intersection_x_y[a].second < minY)
                        minY = isMarked.intersection_x_y[a].second;

                    if (isMarked.intersection_x_y[a].second > maxY)
                        maxY = isMarked.intersection_x_y[a].second;

                }

                std::cout << "How does this effin work???" << std::endl;
                set_visible_world(minX - 0.001, minY - 0.001, maxX + 0.001, maxY + 0.001);
            }

            isMarked.drawBox = true;


        }
    }//else if its on turn it off
    //    else {
    //        showIntersections = false;
    //
    //        isMarked.drawPins = false;
    //    }


}

void draw_searchBar() {


    //t_bound_box box(100, 30, 400, 87);

    //set_coordinate_system(GL_SCREEN);

    if (findPath) {
        set_coordinate_system(GL_WORLD);
        //load in the search bar image
        Surface search_bar = load_png_from_file("libstreetmap/resources/searchBar.png");
        Surface path_help_menu = load_png_from_file("libstreetmap/resources/helpMenu.png");
        Surface xIcon = load_png_from_file("libstreetmap/resources/x.png");
        Surface blankCheckBox = load_png_from_file("libstreetmap/resources/blankCheckBox.png");
        Surface checkedBox = load_png_from_file("libstreetmap/resources/checkedBox.png");

        float x1 = get_visible_world().left() + (get_visible_world().get_width() / 23);
        float y2 = get_visible_world().top() - (get_visible_world().get_height() / 23);

        float x2 = get_visible_world().left() + (get_visible_world().get_width() / 23) + (get_visible_world().get_width() / 4);
        float y1 = get_visible_world().top() - (get_visible_world().get_height() / 23) - (get_visible_world().get_height() / 14.88);

        draw_surface(search_bar, x1, y2);

        //user input text style
        settextrotation(0);
        setcolor(BLACK);
        setfontsize(11);

        t_bound_box box1(x1, y1, x2, y2);
        drawtext_in(box1, userInputDefaultSB + "|");


        y1 = y1 - (get_visible_world().get_height() / 14.88);
        y2 = y2 - (get_visible_world().get_height() / 14.88);

        draw_surface(search_bar, x1, y2);

        //user input text style
        settextrotation(0);
        setcolor(BLACK);
        setfontsize(11);

        t_bound_box box2(x1, y1, x2, y2);
        drawtext_in(box2, userInputSecondSB + "|");

        if (!dontDisplayMenu && !xOnMenuClicked) {



            float testX2 = get_visible_world().left() + (get_visible_world().get_width() / 23) + (get_visible_world().get_width() / 4);

            float testY = get_visible_world().top() - (get_visible_world().get_height() / 23) - (get_visible_world().get_height() / 17);
            float menuY = testY - (get_visible_world().get_height() / 17);

            float blankBoxX1 = testX2 - (get_visible_world().get_width() / 16);
            float blankBoxY1 = menuY - (get_visible_world().get_height() / 5.25) - (get_visible_world().get_height() / 100);
            float blankBoxX2 = testX2 - (get_visible_world().get_width() / 16) + (get_visible_world().get_width() / 100);
            float blankBoxY2 = menuY - (get_visible_world().get_height() / 5.25) + (get_visible_world().get_height() / 62.5) - (get_visible_world().get_height() / 100);

            draw_surface(path_help_menu, x1, menuY);

            drawrect(blankBoxX1, blankBoxY1, blankBoxX2, blankBoxY2);
            if (menuBoxClicked) {
                draw_surface(checkedBox, blankBoxX1, blankBoxY2);
            } else {
                draw_surface(blankCheckBox, blankBoxX1, blankBoxY2);
            }

            float xX1 = x2 - (get_visible_world().get_width() / 43);
            float xY1 = menuY - (get_visible_world().get_height() / 25) - (get_visible_world().get_width() / 100);
            float xX2 = xX1 + (get_visible_world().get_width() / 100);
            float xY2 = xY1 + (get_visible_world().get_height() / 62.5);

            draw_surface(xIcon, xX1, xY2);

        }

    } else {
        Surface search_bar = load_png_from_file("libstreetmap/resources/searchBar.png");

        float x1 = get_visible_world().left() + (get_visible_world().get_width() / 23);
        float y2 = get_visible_world().top() - (get_visible_world().get_height() / 23);

        float x2 = get_visible_world().left() + (get_visible_world().get_width() / 23) + (get_visible_world().get_width() / 4);
        float y1 = get_visible_world().top() - (get_visible_world().get_height() / 23) - (get_visible_world().get_height() / 14.88);

        draw_surface(search_bar, x1, y2);

        //user input text style
        settextrotation(0);
        setcolor(BLACK);
        setfontsize(11);

        t_bound_box box1(x1, y1, x2, y2);
        drawtext_in(box1, userInputDefaultSB + "|");
    }

}

void draw_drop_downMenu() {

    if (userInputSecondSB.length() > 0 || userInputDefaultSB.length() > 0) {
        set_coordinate_system(GL_WORLD);

        for (unsigned i = 0; i < 4; i++) {

            float x1 = get_visible_world().left() + (get_visible_world().get_width() / 23);
            float y2 = get_visible_world().top() - (get_visible_world().get_height() / 23);
            y2 = y2 - get_visible_world().get_height() / 17 * (i + 1) - (get_visible_world().get_height() / 100) * i;

            float x2 = get_visible_world().left() + (get_visible_world().get_width() / 23) + (get_visible_world().get_width() / 4);
            float y1 = get_visible_world().top() - (get_visible_world().get_height() / 23) - (get_visible_world().get_height() / 14.88);
            y1 = y1 - get_visible_world().get_height() / 17 * (i + 1) - (get_visible_world().get_height() / 100) * i;


            if (findPath) {
                y1 = y1 - (get_visible_world().get_height() / 14.88);
                y2 = y2 - (get_visible_world().get_height() / 14.88);
            }

            t_bound_box box(x1, y1, x2, y2);

            if (closestStrings[i].length() > 15 && closestStrings[i].length() < 30) {
                setfontsize(10);
            } else if (closestStrings[i].length() > 30 && closestStrings[i].length() < 60) {
                setfontsize(7);
            } else if (closestStrings[i].length() > 60) {
                setfontsize(6);
            } else {
                setfontsize(12);
            }

            if (arrowKeysPressed == true && i == arrayIndexCount) {
                //setcolor(t_color(0, 255, 0, 255 / 4)); // 50% transparent
                setcolor(BLACK);
                fillrect(x1, y1, x2, y2);
                setcolor(BLACK);
                drawline(x1, y1, x2, y1);
                //user input text style
                settextrotation(0);
                setcolor(WHITE);
            } else {
                //setcolor(t_color(0, 255, 0, 255 / 4)); // 50% transparent
                setcolor(WHITE);
                fillrect(x1, y1, x2, y2);
                setcolor(BLACK);
                drawline(x1, y1, x2, y1);
                //user input text style
                settextrotation(0);
                setcolor(BLACK);

            }

            drawtext_in(box, closestStrings[i]);
        }
    }
}

void drawFindPathGraphics() {

    std::cout << "Draw Find Path" << std::endl;
    //set_coordinate_system(GL_SCREEN);
    set_coordinate_system(GL_WORLD);
    //load in the search bar image
    Surface search_bar = load_png_from_file("libstreetmap/resources/searchBar.png");

    float x1 = get_visible_world().left() + (get_visible_world().get_width() / 23);
    float y2 = get_visible_world().top() - (get_visible_world().get_height() / 23);

    float x2 = get_visible_world().left() + (get_visible_world().get_width() / 23) + (get_visible_world().get_width() / 4);
    float y1 = get_visible_world().top() - (get_visible_world().get_height() / 23) - (get_visible_world().get_height() / 14.88);

    draw_surface(search_bar, x1, y2);

    t_bound_box box(x1, y1, x2, y2);

    //user input text style
    settextrotation(0);
    setcolor(BLACK);
    setfontsize(11);

    drawtext_in(box, userInputDefaultSB + "|");

    //drawing final point box
    //only need to change the y coordinates, push it down

    float x3 = get_visible_world().left() + (get_visible_world().get_width() / 23);
    float y3 = get_visible_world().top() - (get_visible_world().get_height() / 8);

    draw_surface(search_bar, x3, y3);
    draw_screen();
}

void highlightStreetFunction() {
    //already have the name of the street to be highlighted.
    //get visible world coordinates and highlight in that


    float minX = 0;
    float minY = 0;
    float maxX = 0;
    float maxY = 0;


    std::vector<unsigned> streetIDs = find_street_ids_from_name(streetToBeHighlighted); //all the streetIDs with the name

    for (unsigned a = 0; a < streetIDs.size(); ++a) {

        std::vector<unsigned> streetSegments = find_street_street_segments(streetIDs[a]); //get all of the segmentsID


        for (unsigned i = 0; i < streetSegments.size(); ++i) {//loop through all segID


            StreetSegmentInfo segment = map -> query_streetSegInfo_from_segID(streetSegments[i]); //get one segment

            std::string streetName = map -> streetName_from_streetID[segment.streetID];

            if (streetName != streetToBeHighlighted)
                continue;

            unsigned numberOfCurves = segment.curvePointCount;




            //if no curve points, compute distance between lat/lon both both ends of segment
            if (numberOfCurves == 0) {

                LatLon startPoint = map -> pos_from_intersectionID[segment.from];
                LatLon endPoint = map -> pos_from_intersectionID[segment.to];

                float x1 = longitude_to_cartesian(startPoint.lon(), average_latitude);
                float y1 = latitude_to_cartesian(startPoint.lat());
                float x2 = longitude_to_cartesian(endPoint.lon(), average_latitude);
                float y2 = latitude_to_cartesian(endPoint.lat());

                if (segmentInMapView(x1, x2, y1, y2)) {
                    setcolor(RED);
                    drawline(x1, y1, x2, y2);


                }
            } else {

                //Drawing a line between each curve point in vector
                std::vector<LatLon> curvePoints = map -> curvePoints_from_segmentID[streetSegments[i]];
                for (unsigned j = 0; j < numberOfCurves - 1; ++j) {

                    LatLon curvePoint = curvePoints[j];
                    LatLon curvePoint2 = curvePoints[j + 1];

                    float x1 = longitude_to_cartesian(curvePoint.lon(), average_latitude);
                    float y1 = latitude_to_cartesian(curvePoint.lat());
                    float x2 = longitude_to_cartesian(curvePoint2.lon(), average_latitude);
                    float y2 = latitude_to_cartesian(curvePoint2.lat());
                    //    if (segmentInMapView(x1, x2, y1, y2)) {
                    setcolor(RED);
                    drawline(x1, y1, x2, y2);
                    //    }


                }

                LatLon startPoint = map -> pos_from_intersectionID[segment.from];
                LatLon endPoint = map -> pos_from_intersectionID[segment.to];

                float x1 = longitude_to_cartesian(startPoint.lon(), average_latitude); //drawing first point to first curve point
                float y1 = latitude_to_cartesian(startPoint.lat());
                float x2 = longitude_to_cartesian(curvePoints[0].lon(), average_latitude);
                float y2 = latitude_to_cartesian(curvePoints[0].lat());
                setcolor(RED);
                drawline(x1, y1, x2, y2);

                float x3 = longitude_to_cartesian(curvePoints[numberOfCurves - 1].lon(), average_latitude); //drawing last curve point to last point
                float y3 = latitude_to_cartesian(curvePoints[numberOfCurves - 1].lat());
                float x4 = longitude_to_cartesian(endPoint.lon(), average_latitude);
                float y4 = latitude_to_cartesian(endPoint.lat());
                //   if (segmentInMapView(x3, x4, y3, y4)) {
                setcolor(RED);
                drawline(x3, y3, x4, y4);
                //  }


            }

        }
    }

    //set_visible_world(minX - get_visible_world().get_width() / 12, minY - get_visible_world().get_height() / 12, maxX + get_visible_world().get_width() / 12, maxY + get_visible_world().get_height() / 12);

}

void draw_helpMenu() {

    Surface Help_searchBar = load_png_from_file("libstreetmap/resources/Help_searchBar.png");
    Surface Help_ColourBlind = load_png_from_file("libstreetmap/resources/Help_ColourBlind.png");
    Surface Help_findPOIs = load_png_from_file("libstreetmap/resources/Help_findPOIs.png");
    Surface Help_findDistance = load_png_from_file("libstreetmap/resources/Help_findDistance.png");
    Surface Help_clearSearch = load_png_from_file("libstreetmap/resources/Help_clearSearch.png");
    Surface Help_findPath = load_png_from_file("libstreetmap/resources/Help_findPath.png");

    float x = get_visible_world().left() + (get_visible_world().get_width() / 23);
    float y = get_visible_world().top() - (get_visible_world().get_height() / 23) - (get_visible_world().get_height() / 14.88);

    draw_surface(Help_searchBar, x, y);

    x = get_visible_world().right() - (get_visible_world().get_width() / 6.2);
    y = get_visible_world().top() - (get_visible_world().get_height() / 3.9);
    draw_surface(Help_findPOIs, x, y);

    x -= (get_visible_world().get_width() / 6.3);
    y -= (get_visible_world().get_height() / 27);
    draw_surface(Help_clearSearch, x, y);

    x -= (get_visible_world().get_width() / 6.3);
    y -= (get_visible_world().get_height() / 27);
    draw_surface(Help_findDistance, x, y);

    x -= (get_visible_world().get_width() / 6.3);
    y -= (get_visible_world().get_height() / 27);
    draw_surface(Help_ColourBlind, x, y);

    x -= (get_visible_world().get_width() / 6.3);
    y -= (get_visible_world().get_height() / 27);
    draw_surface(Help_findPath, x, y);

}

int computeLenvenshteinDistance(std::string string1, std::string string2) {

    bool substringMatch = true;
    //    std::cout << "THIS IS IN LEVENSHETIN FUNCTION:  " << userInputDefaultSB << std::endl;
    //    std::cout << "THIS IS IN LEVENSHETIN FUNCTION:  " << userInputSecondSB << std::endl;
    std::transform(string1.begin(), string1.end(), string1.begin(), ::tolower);
    std::transform(string2.begin(), string2.end(), string2.begin(), ::tolower);
    //    remove_if(string1.begin(), string1.end(), isspace);
    //    remove_if(string2.begin(), string2.end(), isspace);

    int substitutionCost = 0;
    int substitution = 0;
    int insert = 0;
    int del = 0;
    int count = 0;

    int m = string1.length();
    int n = string2.length();

    int d [m + 1][n + 1]; //2D array

    for (int i = 0; i <= m; i++) {
        d[i][0] = i;
    }

    for (int i = 0; i <= n; i++) {
        d[0][i] = i;
    }
    int i;
    int j;

    if (string1 == string2) {
        return -5;
    }
    if ((string1.find(string2) != std::string::npos) && (string1[0] == string2[0])) {
        return -3;
    }

    if (string2.find(string1) != std::string::npos) {
        return -1;
    }

    for (i = 1; i <= m; i++) { //iterating through first string

        for (j = 1; j <= n; j++) { //iterating through second string
            if (string1[i - 1] == string2[j - 1]) {
                d[i][j] = d[i - 1][j - 1];
            } else {
                //   substringMatch = false;
                substitutionCost = 1;
                d[i][j] = std::min({
                    d[i - 1][j] + 1, //deletion
                    d[i][j - 1] + 1, //insertion
                    d[i - 1][j - 1] + substitutionCost //substitution
                });
            }
            count++;
        }
    }
    return d[m][n];
}

void autoComplete(std::string & streetName) {

    int levenDist = 100;
    int bestScore = 100;
    int secondBestScore = 101;
    int thirdBestScore = 102;
    int fourthBestScore = 103;
    std::string bestString;
    std::string secondBestString;
    std::string thirdBestString;
    std::string fourthBestString;
    int streetID;
    //    std::cout << "THIS IS IN AUTOCOMPLETERED FUNCTION:  " << userInputDefaultSB << std::endl;
    //    std::cout << "THIS IS IN AUTOCOMPLETERED FUNCTION:  " << userInputSecondSB << std::endl;
    for (int i = 0; i < map->combinedNames_from_ID.size(); i++) {
        std::string streetToCompare = map->combinedNames_from_ID[i];
        if (streetToCompare != "<unknown>") {
            levenDist = computeLenvenshteinDistance(streetToCompare, streetName);
            if ((levenDist == -1) && (bestScore == -1) && (streetToCompare != bestString)) {
                if (streetToCompare.length() <= bestString.length()) {
                    fourthBestScore = thirdBestScore;
                    thirdBestScore = secondBestScore;
                    secondBestScore = bestScore;
                    bestScore = levenDist;

                    fourthBestString = thirdBestString;
                    thirdBestString = secondBestString;
                    secondBestString = bestString;
                    bestString = streetToCompare;
                } else {
                    fourthBestScore = thirdBestScore;
                    thirdBestScore = secondBestScore;
                    secondBestScore = levenDist;

                    fourthBestString = thirdBestString;
                    thirdBestString = secondBestString;
                    secondBestString = streetToCompare;
                }
            } else if ((levenDist == bestScore) && (levenDist != -1) && (bestScore != -1)) {
                fourthBestScore = thirdBestScore;
                thirdBestScore = secondBestScore;
                secondBestScore = bestScore;
                bestScore = levenDist;

                fourthBestString = thirdBestString;
                thirdBestString = secondBestString;
                secondBestString = bestString;
                bestString = streetToCompare;
            } else {
                if ((levenDist < bestScore) && (levenDist < secondBestScore) && (levenDist < thirdBestScore) && (levenDist < fourthBestScore)) {
                    fourthBestScore = thirdBestScore;
                    thirdBestScore = secondBestScore;
                    secondBestScore = bestScore;
                    bestScore = levenDist;

                    fourthBestString = thirdBestString;
                    thirdBestString = secondBestString;
                    secondBestString = bestString;
                    bestString = streetToCompare;
                } else if ((levenDist > bestScore) && (levenDist < secondBestScore) && (levenDist < thirdBestScore) && (levenDist < fourthBestScore)) {
                    fourthBestScore = thirdBestScore;
                    thirdBestScore = secondBestScore;
                    secondBestScore = levenDist;

                    fourthBestString = thirdBestString;
                    thirdBestString = secondBestString;
                    secondBestString = streetToCompare;
                } else if ((levenDist > bestScore) && (levenDist > secondBestScore) && (levenDist < thirdBestScore) && (levenDist < fourthBestScore)) {
                    fourthBestScore = thirdBestScore;
                    thirdBestScore = levenDist;

                    fourthBestString = thirdBestString;
                    thirdBestString = streetToCompare;
                } else if ((levenDist > bestScore) && (levenDist > secondBestScore) && (levenDist > thirdBestScore) && (levenDist < fourthBestScore)) {
                    fourthBestScore = levenDist;
                    fourthBestString = streetToCompare;
                }
            }
        }
    }
    //    std::cout << "THIS IS IN END OF THE AUTOCOMPLETERED FUNCTION:  " << userInputDefaultSB << std::endl;
    //    std::cout << "THIS IS IN END OF THE AUTOCOMPLETERED FUNCTION:  " << userInputSecondSB << std::endl;
    streetName = bestString;
}

void autoComplete2() {
    int levenDist = 100;
    int bestScore = 100;
    int secondBestScore = 101;
    int thirdBestScore = 102;
    int fourthBestScore = 103;
    std::string bestString;
    std::string secondBestString;
    std::string thirdBestString;
    std::string fourthBestString;

    if (findPath) {
        for (int i = 0; i < map->combinedNames_from_ID.size(); i++) {
            std::string streetToCompare = map->combinedNames_from_ID[i];
            if (streetToCompare != "<unknown>") {
                if (searchFlagDraw2) {
                    levenDist = computeLenvenshteinDistance(streetToCompare, userInputSecondSB);
                } else if (searchFlagDraw) {
                    levenDist = computeLenvenshteinDistance(streetToCompare, userInputDefaultSB);
                }
                if ((levenDist == -1) && (bestScore == -1) && (streetToCompare != bestString)) {
                    if (streetToCompare.length() <= bestString.length()) {
                        fourthBestScore = thirdBestScore;
                        thirdBestScore = secondBestScore;
                        secondBestScore = bestScore;
                        bestScore = levenDist;

                        fourthBestString = thirdBestString;
                        thirdBestString = secondBestString;
                        secondBestString = bestString;
                        bestString = streetToCompare;
                    } else {
                        fourthBestScore = thirdBestScore;
                        thirdBestScore = secondBestScore;
                        secondBestScore = levenDist;

                        fourthBestString = thirdBestString;
                        thirdBestString = secondBestString;
                        secondBestString = streetToCompare;
                    }
                } else if ((levenDist == bestScore) && (levenDist != -1) && (bestScore != -1) && (streetToCompare != bestString)) {
                    fourthBestScore = thirdBestScore;
                    thirdBestScore = secondBestScore;
                    secondBestScore = bestScore;
                    bestScore = levenDist;

                    fourthBestString = thirdBestString;
                    thirdBestString = secondBestString;
                    secondBestString = bestString;
                    bestString = streetToCompare;
                } else {
                    if ((levenDist < bestScore) && (levenDist < secondBestScore) && (levenDist < thirdBestScore) && (levenDist < fourthBestScore)) {
                        fourthBestScore = thirdBestScore;
                        thirdBestScore = secondBestScore;
                        secondBestScore = bestScore;
                        bestScore = levenDist;

                        fourthBestString = thirdBestString;
                        thirdBestString = secondBestString;
                        secondBestString = bestString;
                        bestString = streetToCompare;
                    } else if ((levenDist > bestScore) && (levenDist < secondBestScore) && (levenDist < thirdBestScore) && (levenDist < fourthBestScore)) {
                        fourthBestScore = thirdBestScore;
                        thirdBestScore = secondBestScore;
                        secondBestScore = levenDist;

                        fourthBestString = thirdBestString;
                        thirdBestString = secondBestString;
                        secondBestString = streetToCompare;
                    } else if ((levenDist > bestScore) && (levenDist > secondBestScore) && (levenDist < thirdBestScore) && (levenDist < fourthBestScore)) {
                        fourthBestScore = thirdBestScore;
                        thirdBestScore = levenDist;

                        fourthBestString = thirdBestString;
                        thirdBestString = streetToCompare;
                    } else if ((levenDist > bestScore) && (levenDist > secondBestScore) && (levenDist > thirdBestScore) && (levenDist < fourthBestScore)) {
                        fourthBestScore = levenDist;
                        fourthBestString = streetToCompare;
                    }
                }
            }
        }
    } else {
        for (int i = 0; i < map->streetName_from_streetID.size(); i++) {
            std::string streetToCompare = map->streetName_from_streetID[i];
            if (streetToCompare != "<unknown>") {
                if (searchFlagDraw2) {
                    levenDist = computeLenvenshteinDistance(streetToCompare, userInputSecondSB);
                } else if (searchFlagDraw) {
                    levenDist = computeLenvenshteinDistance(streetToCompare, userInputDefaultSB);
                }
                if ((levenDist == -1) && (bestScore == -1) && (streetToCompare != bestString)) {
                    if (streetToCompare.length() <= bestString.length()) {
                        fourthBestScore = thirdBestScore;
                        thirdBestScore = secondBestScore;
                        secondBestScore = bestScore;
                        bestScore = levenDist;

                        fourthBestString = thirdBestString;
                        thirdBestString = secondBestString;
                        secondBestString = bestString;
                        bestString = streetToCompare;
                    } else {
                        fourthBestScore = thirdBestScore;
                        thirdBestScore = secondBestScore;
                        secondBestScore = levenDist;

                        fourthBestString = thirdBestString;
                        thirdBestString = secondBestString;
                        secondBestString = streetToCompare;
                    }
                } else if ((levenDist == bestScore) && (levenDist != -1) && (bestScore != -1) && (streetToCompare != bestString)) {
                    fourthBestScore = thirdBestScore;
                    thirdBestScore = secondBestScore;
                    secondBestScore = bestScore;
                    bestScore = levenDist;

                    fourthBestString = thirdBestString;
                    thirdBestString = secondBestString;
                    secondBestString = bestString;
                    bestString = streetToCompare;
                } else {
                    if ((levenDist < bestScore) && (levenDist < secondBestScore) && (levenDist < thirdBestScore) && (levenDist < fourthBestScore)) {
                        fourthBestScore = thirdBestScore;
                        thirdBestScore = secondBestScore;
                        secondBestScore = bestScore;
                        bestScore = levenDist;

                        fourthBestString = thirdBestString;
                        thirdBestString = secondBestString;
                        secondBestString = bestString;
                        bestString = streetToCompare;
                    } else if ((levenDist > bestScore) && (levenDist < secondBestScore) && (levenDist < thirdBestScore) && (levenDist < fourthBestScore)) {
                        fourthBestScore = thirdBestScore;
                        thirdBestScore = secondBestScore;
                        secondBestScore = levenDist;

                        fourthBestString = thirdBestString;
                        thirdBestString = secondBestString;
                        secondBestString = streetToCompare;
                    } else if ((levenDist > bestScore) && (levenDist > secondBestScore) && (levenDist < thirdBestScore) && (levenDist < fourthBestScore)) {
                        fourthBestScore = thirdBestScore;
                        thirdBestScore = levenDist;

                        fourthBestString = thirdBestString;
                        thirdBestString = streetToCompare;
                    } else if ((levenDist > bestScore) && (levenDist > secondBestScore) && (levenDist > thirdBestScore) && (levenDist < fourthBestScore)) {
                        fourthBestScore = levenDist;
                        fourthBestString = streetToCompare;
                    }
                }
            }
        }
    }
    firstStreet = bestString;
    closestStrings[0] = bestString;
    closestStrings[1] = secondBestString;
    closestStrings[2] = thirdBestString;
    closestStrings[3] = fourthBestString;

    if (bestScore > 20) {
        closestStrings[0] = "No further suggestions";
    }
    if (secondBestScore > 16) {
        closestStrings[1] = "No further suggestions";
    }
    if (thirdBestScore > 12) {
        closestStrings[2] = "No further suggestions";
    }
    if (thirdBestScore > 8) {
        closestStrings[3] = "No further suggestions";
    }

}

void autoCompleteIntersection() {
    int levenDist = 100;
    int bestScore = 100;
    int secondBestScore = 101;
    int thirdBestScore = 102;
    int fourthBestScore = 103;
    std::string bestString;
    std::string secondBestString;
    std::string thirdBestString;
    std::string fourthBestString;


    std::string secondStreet;
    if (findPath) {
        for (int i = 0; i < map->combinedNames_from_ID.size(); i++) {
            std::string streetToCompare = map->combinedNames_from_ID[i];
            if (streetToCompare != "<unknown>") {
                if (searchFlagDraw2) {
                    levenDist = computeLenvenshteinDistance(streetToCompare, userInputSecondSB);
                } else if (searchFlagDraw) {
                    levenDist = computeLenvenshteinDistance(streetToCompare, userInputDefaultSB);
                }
                if ((levenDist == -1) && (bestScore == -1) && (streetToCompare != bestString)) {
                    if (streetToCompare.length() <= bestString.length()) {
                        fourthBestScore = thirdBestScore;
                        thirdBestScore = secondBestScore;
                        secondBestScore = bestScore;
                        bestScore = levenDist;

                        fourthBestString = thirdBestString;
                        thirdBestString = secondBestString;
                        secondBestString = bestString;
                        bestString = streetToCompare;
                    } else {
                        fourthBestScore = thirdBestScore;
                        thirdBestScore = secondBestScore;
                        secondBestScore = levenDist;

                        fourthBestString = thirdBestString;
                        thirdBestString = secondBestString;
                        secondBestString = streetToCompare;
                    }
                } else if ((levenDist == bestScore) && (levenDist != -1) && (bestScore != -1) && (streetToCompare != bestString)) {
                    fourthBestScore = thirdBestScore;
                    thirdBestScore = secondBestScore;
                    secondBestScore = bestScore;
                    bestScore = levenDist;

                    fourthBestString = thirdBestString;
                    thirdBestString = secondBestString;
                    secondBestString = bestString;
                    bestString = streetToCompare;
                } else {
                    if ((levenDist < bestScore) && (levenDist < secondBestScore) && (levenDist < thirdBestScore) && (levenDist < fourthBestScore)) {
                        fourthBestScore = thirdBestScore;
                        thirdBestScore = secondBestScore;
                        secondBestScore = bestScore;
                        bestScore = levenDist;

                        fourthBestString = thirdBestString;
                        thirdBestString = secondBestString;
                        secondBestString = bestString;
                        bestString = streetToCompare;
                    } else if ((levenDist > bestScore) && (levenDist < secondBestScore) && (levenDist < thirdBestScore) && (levenDist < fourthBestScore)) {
                        fourthBestScore = thirdBestScore;
                        thirdBestScore = secondBestScore;
                        secondBestScore = levenDist;

                        fourthBestString = thirdBestString;
                        thirdBestString = secondBestString;
                        secondBestString = streetToCompare;
                    } else if ((levenDist > bestScore) && (levenDist > secondBestScore) && (levenDist < thirdBestScore) && (levenDist < fourthBestScore)) {
                        fourthBestScore = thirdBestScore;
                        thirdBestScore = levenDist;

                        fourthBestString = thirdBestString;
                        thirdBestString = streetToCompare;
                    } else if ((levenDist > bestScore) && (levenDist > secondBestScore) && (levenDist > thirdBestScore) && (levenDist < fourthBestScore)) {
                        fourthBestScore = levenDist;
                        fourthBestString = streetToCompare;
                    }
                }
            }
        }
    }
    for (int i = 0; i < map->streetName_from_streetID.size(); i++) {
        std::string streetToCompare = map->streetName_from_streetID[i];
        if (streetToCompare != "<unknown>") {

            if (searchFlagDraw) {
                if (userInputDefaultSB.find("&") != std::string::npos) {
                    secondStreet = userInputDefaultSB.substr(userInputDefaultSB.find("&") + 2);
                } else {
                    secondStreet = userInputDefaultSB.substr(userInputDefaultSB.find("and") + 4);
                }
            } else if (searchFlagDraw2) {
                if (userInputSecondSB.find("&") != std::string::npos) {
                    secondStreet = userInputSecondSB.substr(userInputSecondSB.find("&") + 2);
                } else {
                    secondStreet = userInputSecondSB.substr(userInputSecondSB.find("and") + 4);
                }
            }

            levenDist = computeLenvenshteinDistance(streetToCompare, secondStreet);
            if ((levenDist == -1) && (bestScore == -1) && (streetToCompare != bestString)) {
                if (streetToCompare.length() <= bestString.length()) {
                    fourthBestScore = thirdBestScore;
                    thirdBestScore = secondBestScore;
                    secondBestScore = bestScore;
                    bestScore = levenDist;

                    fourthBestString = thirdBestString;
                    thirdBestString = secondBestString;
                    secondBestString = bestString;
                    bestString = streetToCompare;
                } else {
                    fourthBestScore = thirdBestScore;
                    thirdBestScore = secondBestScore;
                    secondBestScore = levenDist;

                    fourthBestString = thirdBestString;
                    thirdBestString = secondBestString;
                    secondBestString = streetToCompare;
                }
            } else if ((levenDist == bestScore) && (levenDist != -1) && (bestScore != -1) && (streetToCompare != bestString)) {
                fourthBestScore = thirdBestScore;
                thirdBestScore = secondBestScore;
                secondBestScore = bestScore;
                bestScore = levenDist;

                fourthBestString = thirdBestString;
                thirdBestString = secondBestString;
                secondBestString = bestString;
                bestString = streetToCompare;
            } else {
                if ((levenDist < bestScore) && (levenDist < secondBestScore) && (levenDist < thirdBestScore) && (levenDist < fourthBestScore)) {
                    fourthBestScore = thirdBestScore;
                    thirdBestScore = secondBestScore;
                    secondBestScore = bestScore;
                    bestScore = levenDist;

                    fourthBestString = thirdBestString;
                    thirdBestString = secondBestString;
                    secondBestString = bestString;
                    bestString = streetToCompare;
                } else if ((levenDist > bestScore) && (levenDist < secondBestScore) && (levenDist < thirdBestScore) && (levenDist < fourthBestScore)) {
                    fourthBestScore = thirdBestScore;
                    thirdBestScore = secondBestScore;
                    secondBestScore = levenDist;

                    fourthBestString = thirdBestString;
                    thirdBestString = secondBestString;
                    secondBestString = streetToCompare;
                } else if ((levenDist > bestScore) && (levenDist > secondBestScore) && (levenDist < thirdBestScore) && (levenDist < fourthBestScore)) {
                    fourthBestScore = thirdBestScore;
                    thirdBestScore = levenDist;

                    fourthBestString = thirdBestString;
                    thirdBestString = streetToCompare;
                } else if ((levenDist > bestScore) && (levenDist > secondBestScore) && (levenDist > thirdBestScore) && (levenDist < fourthBestScore)) {
                    fourthBestScore = levenDist;
                    fourthBestString = streetToCompare;
                }
            }
        }
    }

    closestStrings[0] = firstStreet + " & " + bestString;
    closestStrings[1] = firstStreet + " & " + secondBestString;
    closestStrings[2] = firstStreet + " & " + thirdBestString;
    closestStrings[3] = firstStreet + " & " + fourthBestString;



    if (bestScore > 20) {
        closestStrings[0] = "No further suggestions";
    }
    if (secondBestScore > 16) {
        closestStrings[1] = "No further suggestions";
    }
    if (thirdBestScore > 12) {
        closestStrings[2] = "No further suggestions";
    }
    if (thirdBestScore > 8) {
        closestStrings[3] = "No further suggestions";
    }

}

int checkQueries() {
    std::cout << "ENTERED checkQueries" << std::endl;

    bool bothQueriesAreIntersections = false;
    bool queryOneisPOI = false;
    bool queryTwoisPOI = false;



    std::string query1_Street1 = userInputDefaultSB.substr(0, userInputDefaultSB.find(" &"));
    std::cout << "WS" << query1_Street1 << "WS" << std::endl;
    std::string query1_Street2 = userInputDefaultSB.substr(userInputDefaultSB.find("&") + 2);
    std::cout << "WS" << query1_Street2 << "WS" << std::endl;

    streetName1 = query1_Street1;
    streetName2 = query1_Street2;

    std::string query2_Street1 = userInputSecondSB.substr(0, userInputSecondSB.find(" &"));
    std::cout << "WS" << query2_Street1 << "WS" << std::endl;
    std::string query2_Street2 = userInputSecondSB.substr(userInputSecondSB.find("&") + 2);
    std::cout << "WS" << query2_Street2 << "WS" << std::endl;

    streetName3 = query2_Street1;
    streetName4 = query2_Street2;

    std::vector <unsigned> queryOneInter = find_intersection_ids_from_street_names(query1_Street1, query1_Street2);
    std::vector <unsigned> queryTwoInter = find_intersection_ids_from_street_names(query2_Street1, query2_Street2);

    std::vector <std::string> combined = map->combinedNames_from_ID;

    if ((std::find(combined.begin(), combined.end(), userInputSecondSB) != combined.end()) && queryOneInter.size() > 0) {
        std::cout << "Found POI in Second Search Bar" << std::endl;
        return 2; //Found POI in Second Search Bar
    } else if (std::find(combined.begin(), combined.end(), userInputDefaultSB) != combined.end() && queryTwoInter.size() > 0) {
        std::cout << "Found POI in First Search Bar" << std::endl;
        return 3; //Found POI in First Search Bar
    } else if (queryOneInter.size() > 0 && queryTwoInter.size() > 0) {
        std::cout << "Both are valid intersections" << std::endl;
        return 4; //Both are valid intersections 
    } else if (std::find(combined.begin(), combined.end(), userInputDefaultSB) != combined.end() &&
            (std::find(combined.begin(), combined.end(), userInputSecondSB) != combined.end())) {
        std::cout << "Invalid input; 2 POIs entered" << std::endl;
        bothPOIsBad = true;
        return 6; //TWO POIS
    } else if ((std::find(combined.begin(), combined.end(), userInputSecondSB) == combined.end())
            && (std::find(combined.begin(), combined.end(), userInputDefaultSB) == combined.end()) && queryOneInter.size() == 0 && queryTwoInter.size() == 0) {
        std::cout << "Two Invalid Intersections" << std::endl;
        bothIntersecInvalid = true;
        return 7; //Two Invalid Intersections
    } else if (queryOneInter.size() == 0 && queryTwoInter.size() > 0) {
        std::cout << "Starting Intersection is Invalid" << std::endl;
        firstInterBad = true;
        return 8; //First Intersection Invalid
    } else if (queryOneInter.size() > 0 && queryTwoInter.size() == 0) {
        std::cout << "Destination Intersection is Invalid" << std::endl;
        secondInterBad = true;
        return 9; //Second Intersection Invalid
    } else if (((std::find(combined.begin(), combined.end(), userInputSecondSB) == combined.end()) && queryOneInter.size() > 0)) {
        std::cout << "POI entered in in box 2 is invalid" << std::endl;
        destinPOIbad = true;
        return 10;
    } else if ((std::find(combined.begin(), combined.end(), userInputDefaultSB) != combined.end() && queryTwoInter.size() > 0)) {
        std::cout << "POI entered in in box 1 is invalid" << std::endl;
        startPOIbad = true;
        return 11;
    }
    if (bothPOIsBad)
        std::cout << "Boolean becomes true on enter" << std::endl;
}

void draw_findPath_errorBox() {
    float x1 = get_visible_world().left();
    float y1 = get_visible_world().bottom();
    float x2 = get_visible_world().right();
    float y2 = y1 + get_visible_world().get_height() / 4;
    //float x2 = x1 + get_visible_world().get_width() / 4;
    //float y2 = y1 + get_visible_world().get_height() / 4;

    t_bound_box box = t_bound_box(x1, y1, x2, y2);
    setcolor(BLACK);
    fillrect(x1, y1, x2, y2 - get_visible_world().get_height() / 12);

    if (findPath) {
        setfontsize(12);
        setcolor(GREEN);
        settextrotation(0);

        if (bothPOIsBad) {
            drawtext_in(box, "Invalid input; 2 POIs entered"); //TWO POIS
        } else if (bothIntersecInvalid) {
            drawtext_in(box, "Two Invalid Intersections"); //Both intersections are invalid
        } else if (firstInterBad) {
            drawtext_in(box, "First Intersection is Invalid"); //First Intersection Invalid
        } else if (secondInterBad) {
            drawtext_in(box, "Second Intersection is Invalid"); //Second Intersection Invalid
        } else if (destinPOIbad) {
            drawtext_in(box, "POI entered as destination is invalid"); //Invalid End POI
        } else if (startPOIbad) {
            drawtext_in(box, "POI entered as starting location is invalid"); //Invalid start POI
        }

    }

}

void draw_path_found() {

    float minX = INFINITY;
    float maxX = -INFINITY;
    float minY = INFINITY;
    float maxY = -INFINITY;


    std::vector<unsigned> localPath;
    if (foundClickCreatedPath)
        localPath = clickCreatedPath;
    else
        localPath = path;

    Surface startingMarker = load_png_from_file("libstreetmap/resources/markerPin.png");
    Surface finalMarker = load_png_from_file("libstreetmap/resources/destinationNew.png");

    for (unsigned i = 0; i < localPath.size(); ++i) {//loop through all segID

        StreetSegmentInfo segment = map -> query_streetSegInfo_from_segID(localPath[i]); //get one segment
        std::string streetName = map -> streetName_from_streetID[segment.streetID];
        unsigned numberOfCurves = segment.curvePointCount;

        float x1 = (map -> pos_from_intersectionID[segment.from]).lon() * cos(average_latitude);
        float x2 = (map -> pos_from_intersectionID[segment.to]).lon() * cos(average_latitude);
        float y1 = (map -> pos_from_intersectionID[segment.from]).lat();
        float y2 = (map -> pos_from_intersectionID[segment.to]).lat();

        if (x1 < minX)
            minX = x1;
        if (x2 < minX)
            minX = x2;

        if (y1 < minY)
            minY = y1;
        if (y2 < minY)
            minY = y2;

        if (x1 > maxX)
            maxX = x1;
        if (x2 > maxX)
            maxX = x2;

        if (y1 > maxY)
            maxY = y1;
        if (y2 > maxY)
            maxY = y2;


        //if no curve points, compute distance between lat/lon both both ends of segment
        if (numberOfCurves == 0) {

            LatLon startPoint = map -> pos_from_intersectionID[segment.from];
            LatLon endPoint = map -> pos_from_intersectionID[segment.to];

            float x1 = longitude_to_cartesian(startPoint.lon(), average_latitude);
            float y1 = latitude_to_cartesian(startPoint.lat());
            float x2 = longitude_to_cartesian(endPoint.lon(), average_latitude);
            float y2 = latitude_to_cartesian(endPoint.lat());

            if (segmentInMapView(x1, x2, y1, y2)) {
                setcolor(BLUE);
                drawline(x1, y1, x2, y2);


            }
        } else {

            //Drawing a line between each curve point in vector
            std::vector<LatLon> curvePoints = map -> curvePoints_from_segmentID[localPath[i]];
            for (unsigned j = 0; j < numberOfCurves - 1; ++j) {

                LatLon curvePoint = curvePoints[j];
                LatLon curvePoint2 = curvePoints[j + 1];

                float x1 = longitude_to_cartesian(curvePoint.lon(), average_latitude);
                float y1 = latitude_to_cartesian(curvePoint.lat());
                float x2 = longitude_to_cartesian(curvePoint2.lon(), average_latitude);
                float y2 = latitude_to_cartesian(curvePoint2.lat());
                //    if (segmentInMapView(x1, x2, y1, y2)) {
                setcolor(BLUE);
                drawline(x1, y1, x2, y2);
                //    }


            }

            LatLon startPoint = map -> pos_from_intersectionID[segment.from];
            LatLon endPoint = map -> pos_from_intersectionID[segment.to];

            float x1 = longitude_to_cartesian(startPoint.lon(), average_latitude); //drawing first point to first curve point
            float y1 = latitude_to_cartesian(startPoint.lat());
            float x2 = longitude_to_cartesian(curvePoints[0].lon(), average_latitude);
            float y2 = latitude_to_cartesian(curvePoints[0].lat());
            setcolor(BLUE);
            drawline(x1, y1, x2, y2);

            float x3 = longitude_to_cartesian(curvePoints[numberOfCurves - 1].lon(), average_latitude); //drawing last curve point to last point
            float y3 = latitude_to_cartesian(curvePoints[numberOfCurves - 1].lat());
            float x4 = longitude_to_cartesian(endPoint.lon(), average_latitude);
            float y4 = latitude_to_cartesian(endPoint.lat());
            //   if (segmentInMapView(x3, x4, y3, y4)) {
            setcolor(BLUE);
            drawline(x3, y3, x4, y4);


            //  }


        }

    }

    if (!foundClickCreatedPath) {
        StreetSegmentInfo segment1 = map -> query_streetSegInfo_from_segID(path[0]);
        StreetSegmentInfo segment2 = map -> query_streetSegInfo_from_segID(path[1]);

        StreetSegmentInfo segmentLast = map -> query_streetSegInfo_from_segID(path[path.size() - 1]);
        StreetSegmentInfo segmentSecondLast = map -> query_streetSegInfo_from_segID(path[path.size() - 2]);

        LatLon startingPos;
        LatLon endingPos;

        if (segment1.to == segment2.from || segment1.to == segment2.to) {
            startingPos = map -> pos_from_intersectionID[segment1.to];
        } else if (segment1.from == segment2.from || segment1.from == segment2.to) {
            startingPos = map -> pos_from_intersectionID[segment1.from];
        }

        if (segmentLast.to == segmentSecondLast.from || segmentLast.to == segmentSecondLast.to) {
            endingPos = map -> pos_from_intersectionID[segmentLast.from];
        } else if (segmentLast.from == segmentSecondLast.from || segmentLast.from == segmentSecondLast.to) {
            endingPos = map -> pos_from_intersectionID[segmentLast.to];
        }


        float startX = startingPos.lon() * cos(average_latitude);
        float startY = startingPos.lat();

        float endX = endingPos.lon() * cos(average_latitude);
        float endY = endingPos.lat();

        set_coordinate_system(GL_WORLD);
        draw_surface(startingMarker, startX, startY);
        draw_surface(finalMarker, endX, endY);


    }

    float xOffSet = get_visible_world().get_width() / 3;

    //set_visible_world(minX - xOffSet, minY - 0.001, maxX + 0.001, maxY + 0.001);

    //drawTravelInstructions = true;


    //draw_screen();

}

void draw_travel_instructions() {


    //set_visible_world(minX - 0.001 - 0.001, minY - 0.001, maxX + 0.001, maxY + 0.001);

    std::vector<unsigned> localPath;
    if (foundClickCreatedPath)
        localPath = clickCreatedPath;
    else
        localPath = path;



    float borderOffSet = get_visible_world().get_width() / 40;
    float x1 = get_visible_world().left() + borderOffSet;
    float y1 = get_visible_world().bottom() + borderOffSet;
    float x2 = get_visible_world().left() + (get_visible_world().get_width() / 2.5);
    float y2 = get_visible_world().top() - (get_visible_world().get_height() / 23) - (get_visible_world().get_height() / 14.88) - (get_visible_world().get_height() / 14.88);

    setcolor(t_color(255, 255, 255, 255 / 1.5)); // 50% transparent
    //setcolor(LIGHTGREY);
    fillrect(x1, y1, x2, y2);

    float numBoxes = travelInstructions.size();

    //Create initial Box with title 
    float yTitleHeight = ((y2 - y1) / numBoxes) * (numBoxes * 0.25);
    setcolor(RED);


    StreetSegmentInfo firstSegment = map -> query_streetSegInfo_from_segID(localPath[0]);
    std::string startingIntersection = map -> intersectionNames_from_IDs[firstSegment.from];
    std::string title1 = "Search for an intersection or street" + startingIntersection;

    StreetSegmentInfo lastSegment = map -> query_streetSegInfo_from_segID(localPath[localPath.size() - 1]);
    std::string lastIntersection = map -> intersectionNames_from_IDs[lastSegment.to];
    std::string title2 = "Search for an intersection or street" + lastIntersection;

    while (title1.length() != title2.length()) {
        if (title1.length() < title2.length()) {
            title1 += " ";
        } else if (title2.length() < title1.length()) {
            title2 += " ";
        }
    }

    std::cout << "title1: " << title1.length() << "\ttitle2: " << title2.length() << std::endl;


    //drawtext_in(box, "From " + startingIntersection + " to " + lastIntersection);

    setcolor(t_color(128, 128, 128, 255 / 1.5));
    fillrect(x1, y2 - yTitleHeight / 2, x2, y2);
    setfontsize(15);
    t_bound_box box(x1, y2 - yTitleHeight / 2, x2, y2);
    setcolor(BLACK);
    drawtext_in(box, title1);

    y2 = y2 - yTitleHeight / 2;
    setcolor(t_color(128, 128, 128, 255 / 1.5));
    setfontsize(15);
    fillrect(x1, y2 - yTitleHeight / 2, x2, y2);
    t_bound_box box2(x1, y2 - yTitleHeight / 2, x2, y2);
    setcolor(BLACK);
    drawtext_in(box2, title2);



    std::cout << "From " << startingIntersection << " to " << lastIntersection << std::endl;


    y2 = y2 - yTitleHeight;


    float yOffset = (y2 - y1) / numBoxes;


    for (unsigned i = 0; i < numBoxes; i++) {
        //y1 -= y2 - ((y2 - y1) / 10);
        setlinewidth(8);
        setcolor(BLACK);
        drawrect(x1, y2 - yOffset * (i + 1), x2, y2 - yOffset * i);
        t_bound_box box3(x1, y2 - yOffset * (i + 1), x2, y2 - yOffset * i);
        travelInstruction_text_size(travelInstructions[i]);
        setcolor(BLACK);
        drawtext_in(box3, travelInstructions[i]);
    }


    //draw_screen();
}

void travelInstruction_text_size(std::string travelString) {
    if (travelString.length() < 65) {
        setfontsize(10);
    } else if (travelString.length() > 65) {
        setfontsize(10);
    }

}