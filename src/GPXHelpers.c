//Luke Fischer - 1061800

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "GPXParser.h"
#include "GPXHelpers.h"
#include "LinkedListAPI.h"
#include <math.h>

GPXData * createGPXData(){
    GPXData *otherDataStruct = malloc(sizeof(GPXData) + 500);
    strcpy(otherDataStruct -> name, "n/a");
    strcpy(otherDataStruct -> value, "n/a");
    return otherDataStruct;
}

Waypoint * createWaypoint(){
    Waypoint * waypointStruct = malloc(sizeof(Waypoint));
    waypointStruct -> name = malloc(sizeof(char) + 100);
    waypointStruct -> latitude = 0.0;
    waypointStruct -> longitude = 0.0;
    waypointStruct -> otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

    GPXData * gpxDataStruct = createGPXData();
    insertBack(waypointStruct -> otherData, gpxDataStruct);
    return waypointStruct;
}

//Parse waypoint
Waypoint * constructWaypoint(Waypoint *waypoint, GPXdoc *gpx, xmlNode *wptHead){
    bool hasOtherData = false;
    xmlNode *node;
    node = wptHead -> children;

    //Initialize lists and its struct
    waypoint -> otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

    while(node != NULL){
        if(strcmp((char *)node -> name, "name") == 0){
            //Input waypoint name into struct
            waypoint -> name = malloc(sizeof(char) * strlen((char*)node -> children -> content) + 1);
            strcpy(waypoint -> name, (char*)node -> children -> content);
            break;
        }
        node = node -> next;
    }
    // Iterate through every attribute of the current node
    xmlAttr *attr = NULL;
    for (attr = wptHead->properties; attr != NULL; attr = attr->next){
        xmlNode *value = attr->children;
        char *attrName = (char *)attr->name;
        char *cont = (char *)(value->content);
        
        if(strcmp(attrName, "lat") == 0){
            waypoint -> latitude = atof(cont);
        }
        if(strcmp(attrName, "lon") == 0){
            waypoint -> longitude = atof(cont);
        }
    }
    if(hasOtherData == false){
        GPXData * otherData = createGPXData();
        insertBack(waypoint -> otherData, otherData);
    }
    return waypoint;
}

//Parse route
Route * constructRoute(Route *route, GPXdoc *gpx, xmlNode *routeHead){
    bool hasWaypoint = false;
    bool hasOtherData = false;
    bool hasRouteName = false;
    xmlNode *node;
    xmlNode *child;
    
    node = routeHead -> children;
    route -> waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    route -> otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

    while(node != NULL){
        if(strcmp((char*)node -> name, "name") == 0){
            //Input route name into struct
            hasRouteName = true;
            route -> name = malloc(sizeof(char) * strlen((char*)node -> children -> content) + 1);
            strcpy(route -> name, (char*)node -> children -> content);
        }
        if(strcmp((char*)node -> name, "desc") == 0){
            //Input route name into struct
            hasOtherData = true;
            GPXData *otherDataStruct = malloc(sizeof(GPXData) + strlen((char*)node -> children -> content) + 50 * sizeof(char));
            strcpy(otherDataStruct -> name, "desc");
            strcpy(otherDataStruct -> value, (char*)node -> children -> content);
            insertBack(route -> otherData, otherDataStruct);

        }
        if(strcmp((char*)node -> name, "rtept") == 0){
            hasWaypoint = true;
            Waypoint *waypoint = malloc(sizeof(Waypoint));
            waypoint -> otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
            GPXData *otherData = createGPXData();
            waypoint -> name = NULL;
            child = node -> children;
            while(child != NULL){
                if(strcmp((char*)child -> name, "name") == 0){
                    //Input waypoint name into struct
                    waypoint -> name = malloc(sizeof(char) * strlen((char*)child -> children -> content) + 50);
                    strcpy(waypoint -> name, (char*)child -> children -> content);
                }
                child = child -> next;
            }
            //If waypoint name is undefined - initialize
            if(waypoint -> name == NULL){
                waypoint -> name = malloc(sizeof(char) * strlen("n/a") + 50);
                strcpy(waypoint -> name, "n/a");
            }
            xmlAttr *attr = NULL;

            for (attr = node->properties; attr != NULL; attr = attr->next){
                xmlNode *value = attr->children;
                char *attrName = (char *)attr->name;
                char *cont = (char *)(value->content);
                if(strcmp(attrName, "lat") == 0){
                    hasWaypoint = true;
                    waypoint -> latitude = atof(cont);
                }
                if(strcmp(attrName, "lon") == 0){
                    waypoint -> longitude = atof(cont);
                }
            }
            insertBack(waypoint -> otherData, otherData);
            insertBack(route -> waypoints, waypoint);
        }
        node = node -> next;
    }
    if(hasWaypoint == false){
        //Initialze empty waypoint list
        Waypoint *waypointStruct = createWaypoint();
        insertBack(route -> waypoints, waypointStruct);
    }
    if(hasOtherData == false){
        GPXData *otherDataStruct = createGPXData();
        insertBack(route -> otherData, otherDataStruct);
    }
    if(hasRouteName == false){
        route -> name = malloc(sizeof(char) * 20);
        strcpy(route -> name, "n/a");
    }
    return route;
}

//Parse route
Track * constructTrack(Track *track, GPXdoc *gpx, xmlNode *trackHead){
    bool hasTrackName = false;
    bool hasOtherData = false;
    bool hasTrackSegments = false;
    bool waypointHasName = false;
    xmlNode *node;
    xmlNode *child;
    xmlNode *wayName;
    node = trackHead -> children;
    
    track -> segments = initializeList(&trackSegmentToString, &deleteTrackSegment, &compareTrackSegments);
    track -> otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    
    while(node != NULL){
        if(strcmp((char*)node -> name, "name") == 0){
            //Input route name into struct
            hasTrackName = true;
            track -> name = malloc(sizeof(char) * strlen((char*)node -> children -> content) + 50);
            strcpy(track -> name, (char*)node -> children -> content);
        }
        if(strcmp((char*)node -> name, "desc") == 0){
            //Input route name into struct
            hasOtherData = true;
            GPXData *otherDataStruct = malloc(sizeof(GPXData) + strlen((char*)node -> children -> content) + 50 * sizeof(char));
            strcpy(otherDataStruct -> name, "desc");
            strcpy(otherDataStruct -> value, (char*)node -> children -> content);
            insertBack(track -> otherData, otherDataStruct);

        }
        if(strcmp((char*)node -> name, "trkseg") == 0){
            hasTrackSegments = true;
            TrackSegment *segmentStruct = malloc(sizeof(TrackSegment));
            segmentStruct -> waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
            child = node -> children;
            while(child != NULL){
                if(strcmp((char*)child -> name, "trkpt") == 0){
                    waypointHasName = false;
                    Waypoint *waypoint = malloc(sizeof(Waypoint));
                    waypoint -> otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
                    //Find name of waypoint
                    wayName = child -> children;
                    while(wayName != NULL){
                        if(strcmp((char*)wayName -> name, "name") == 0){
                            waypoint -> name = malloc(sizeof(char) * strlen((char*)wayName -> name) + 50);
                            strcpy(waypoint -> name, (char*)wayName -> children -> content);
                            waypointHasName = true;
                        }
                        if(strcmp((char*)wayName -> name, "ele") == 0){
                            GPXData *waypointOtherData = malloc(sizeof(GPXData) + 200);
                            strcpy(waypointOtherData -> name, "ele");
                            strcpy(waypointOtherData -> value, (char*)(wayName -> children -> content));
                            insertBack(waypoint -> otherData, waypointOtherData);
                        }
                        wayName = wayName -> next;
                    }
                    if(waypointHasName == false){
                        waypoint -> name = malloc(sizeof(char) * strlen("n/a") + 20);
                        strcpy(waypoint -> name, "n/a");
                    }
                    GPXData *data = malloc(sizeof(GPXData) + 50);
                    strcpy(data -> name, "n/a");
                    strcpy(data -> value, "n/a");
                    insertBack(waypoint -> otherData, data);

                    //Input waypoint name into struct
                    xmlAttr *attr = NULL;
                    for (attr = child->properties; attr != NULL; attr = attr->next){
                        xmlNode *value = attr->children;
                        char *attrName = (char *)attr->name;
                        char *cont = (char *)(value->content);
                        if(strcmp(attrName, "lat") == 0){
                            waypoint -> latitude = atof(cont);
                        }
                        if(strcmp(attrName, "lon") == 0){
                            waypoint -> longitude = atof(cont);
                        }
                    }
                    insertBack(segmentStruct -> waypoints, waypoint);
                }
                child = child -> next;
            }
            insertBack(track -> segments, segmentStruct);
        }
        node = node -> next;
    }
    if(hasTrackName == false){
        track -> name = malloc(sizeof(char) * 20);
        strcpy(track -> name, "n/a");
    }
    if(hasOtherData == false){
        GPXData *otherData = createGPXData();
        insertBack(track -> otherData, otherData);
    }
    if(hasTrackSegments == false){
        TrackSegment *segment = malloc(sizeof(TrackSegment));
        Waypoint *waypoint = createWaypoint();
        insertBack(segment -> waypoints, waypoint);
        insertBack(track -> segments, segment);
    }
    return track;
}

void constructDoc(GPXdoc * gpx, xmlNode * a_node){
    xmlNode *cur_node = NULL;

    for (cur_node = a_node; cur_node != NULL; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
           // printf("node type: Element, name: %s\n", cur_node->name);

            //Parse for routes
            if(strcmp((char*)cur_node -> name, "rte") == 0){
                Route *routeStruct = malloc(sizeof(Route));
                routeStruct = constructRoute(routeStruct, gpx, cur_node);
                insertBack(gpx -> routes, routeStruct);
            }

            //Parse for waypoints
            if(strcmp((char*)cur_node -> name, "wpt") == 0){
                Waypoint *waypointStruct = malloc(sizeof(Waypoint));
                waypointStruct = constructWaypoint(waypointStruct, gpx, cur_node);
                insertBack(gpx -> waypoints, waypointStruct);
                
            }
            //Parse for Tracks
            if(strcmp((char*)cur_node -> name, "trk") == 0){
                Track *trackStruct = malloc(sizeof(Track));
                trackStruct = constructTrack(trackStruct, gpx, cur_node);
                insertBack(gpx -> tracks, trackStruct);
            }
        }
        // Uncomment the code below if you want to see the content of every node.

        if (cur_node->content != NULL ){
           // printf("  content: %s\n",cur_node->content);
        }

        // Iterate through every attribute of the current node
        xmlAttr *attr = NULL;
        for (attr = cur_node->properties; attr != NULL; attr = attr->next)
        {
            xmlNode *value = attr->children;
            char *attrName = (char *)attr->name;
            char *cont = (char *)(value->content);
            //printf("\tattribute name: %s, attribute value = %s\n", attrName, cont);

            //Extract namespace, version, and creator from xml
            if(strcmp(attrName, "schemaLocation") == 0){
                for(int i = 0; i < strlen(cont); i++){
                    if(cont[i] == ' '){
                        cont[i] = '\0';
                        break;
                    }
                }
                strcpy(gpx -> namespace, cont);
            }
            if(strcmp(attrName, "version") == 0){
                gpx -> version = atof(cont);
            }
            if(strcmp(attrName, "creator") == 0){
                gpx -> creator = malloc(sizeof(char) * strlen(cont) + 1);
                strcpy(gpx -> creator, cont);
            }

        }
        constructDoc(gpx, cur_node->children);
    }
}

void writeWaypoints(List * waypoints, xmlNodePtr root_node){
    xmlNodePtr node1 = NULL;
    ListIterator listIter = createIterator(waypoints);
    void *elem;
	while ((elem = nextElement(&listIter)) != NULL){
        char castLat[15];
        char castLon[15];
		Waypoint* waypointStruct = (Waypoint*)elem;
        node1 = xmlNewChild(root_node, NULL, BAD_CAST "wpt", BAD_CAST NULL);
        if(strcmp(waypointStruct -> name, "n/a") != 0){
            xmlNewChild(node1, NULL, BAD_CAST "name", BAD_CAST waypointStruct -> name);
        }
        snprintf(castLat, 15, "%.6f", waypointStruct -> latitude);
        snprintf(castLon, 15, "%.6f", waypointStruct -> longitude);
        xmlNewProp(node1, BAD_CAST "lat", BAD_CAST castLat);
        xmlNewProp(node1, BAD_CAST "lon", BAD_CAST castLon);
        if(numGPXData(waypointStruct -> otherData) != 0){
            writeGPXData(waypointStruct -> otherData, node1);
        }
    }
}

void writeRoutes(List * routes, xmlNodePtr root_node){
    xmlNodePtr node1, node2 = NULL;
    ListIterator listIter = createIterator(routes);
    void *elem;
	while ((elem = nextElement(&listIter)) != NULL){
		Route* routeStruct = (Route*)elem;
        node1 = xmlNewChild(root_node, NULL, BAD_CAST "rte", BAD_CAST NULL);
        if(strcmp(routeStruct -> name, "n/a") != 0){
            node2 = xmlNewChild(node1, NULL, BAD_CAST "name", BAD_CAST routeStruct -> name);
        }
        if(numGPXData(routeStruct -> otherData) != 0){
            writeGPXData(routeStruct -> otherData, node1);
        }
        
        ListIterator listIterWaypoints = createIterator(routeStruct -> waypoints);
        void *elemWaypoint;
        while ((elemWaypoint = nextElement(&listIterWaypoints)) != NULL){
            char castLat[15];
            char castLon[15];
            Waypoint* waypointStruct = (Waypoint*)elemWaypoint;
            node2 = xmlNewChild(node1, NULL, BAD_CAST "rtept", BAD_CAST NULL);
            if(strcmp(waypointStruct -> name, "n/a") != 0){
                xmlNewChild(node2, NULL, BAD_CAST "name", BAD_CAST waypointStruct -> name);
            }
            snprintf(castLat, 15, "%.6f", waypointStruct -> latitude);
            snprintf(castLon, 15, "%.6f", waypointStruct -> longitude);
            xmlNewProp(node2, BAD_CAST "lat", BAD_CAST castLat);
            xmlNewProp(node2, BAD_CAST "lon", BAD_CAST castLon);
            if(numGPXData(waypointStruct -> otherData) != 0){
                writeGPXData(waypointStruct -> otherData, node2);
            }
        }
    }
}

void writeGPXData(List * GPXDataList, xmlNodePtr root_node){
    ListIterator listIter = createIterator(GPXDataList);
    void *elem;
	while ((elem = nextElement(&listIter)) != NULL){
		GPXData* otherData = (GPXData*)elem;
        if(strcmp(otherData -> name, "n/a") != 0){
            xmlNewChild(root_node, NULL, BAD_CAST otherData -> name, BAD_CAST otherData -> value);
        }
    }
}

void writeTracks(List * tracks, xmlNodePtr root_node){
    xmlNodePtr node1, node2, node3 = NULL;
    ListIterator listIter = createIterator(tracks);
    void *elem;
	while ((elem = nextElement(&listIter)) != NULL){
		Track* trackStruct = (Track*)elem;
        node1 = xmlNewChild(root_node, NULL, BAD_CAST "trk", BAD_CAST NULL);
        if(strcmp(trackStruct -> name, "n/a") != 0){
            node2 = xmlNewChild(node1, NULL, BAD_CAST "name", BAD_CAST trackStruct -> name);
        }
        if(numGPXData(trackStruct -> otherData) != 0){
            writeGPXData(trackStruct -> otherData, node1);
        }
        ListIterator listIterTrackSegments = createIterator(trackStruct -> segments);
        void *elemTrackSeg;
        //Traverse through track segments
        while ((elemTrackSeg = nextElement(&listIterTrackSegments)) != NULL){
            node2 = xmlNewChild(node1, NULL, BAD_CAST "trkseg", BAD_CAST NULL);
            TrackSegment* trackSeg = (TrackSegment*)elemTrackSeg;
            ListIterator listIterWaypoints = createIterator(trackSeg -> waypoints);
            void *elemWaypoint;
            //Traverse through waypoints
            while ((elemWaypoint = nextElement(&listIterWaypoints)) != NULL){
                char castLat[15];
                char castLon[15];
                Waypoint* waypointStruct = (Waypoint*)elemWaypoint;
                node3 = xmlNewChild(node2, NULL, BAD_CAST "trkpt", BAD_CAST NULL);
                if(strcmp(waypointStruct -> name, "n/a") != 0){
                    xmlNewChild(node3, NULL, BAD_CAST "name", BAD_CAST waypointStruct -> name);
                }
                snprintf(castLat, 15, "%.6f", waypointStruct -> latitude);
                snprintf(castLon, 15, "%.6f", waypointStruct -> longitude);
                xmlNewProp(node3, BAD_CAST "lat", BAD_CAST castLat);
                xmlNewProp(node3, BAD_CAST "lon", BAD_CAST castLon);
                if(numGPXData(waypointStruct -> otherData) != 0){
                    writeGPXData(waypointStruct -> otherData, node3);
                }   
            }
        }
    }
}

bool validateGPXstruct(GPXdoc * doc){
    /*if(strcmp(doc -> namespace, "\0") == 0){
        return false;
    }*/
    if(doc -> version == 0){
        return false;
    }
    if((strcmp(doc -> creator, "\0") == 0) || (doc -> creator == NULL)){
        return false;
    }
    if(doc -> waypoints == NULL){
        return false;
    }
    if(doc -> routes == NULL){
        return false;
    }
    if(doc -> tracks == NULL){
        return false;
    }
    return true;
}

bool validateGPXdata(List * dataList){
    ListIterator listIter = createIterator(dataList);
    void *elem;
	while ((elem = nextElement(&listIter)) != NULL){
		GPXData* data = (GPXData*)elem;
        if(strcmp(data -> name, "\0") == 0){
            return false;
        }
        if(strcmp(data -> value, "\0") == 0){
            return false;
        }
    }
    return true;
}

bool validateWaypoints(List * waypoints){
    ListIterator listIter = createIterator(waypoints);
    void *elem;
	while ((elem = nextElement(&listIter)) != NULL){
		Waypoint* waypointStruct = (Waypoint*)elem;
        if(waypointStruct -> name == NULL){
            return false;
        }
        if(waypointStruct -> latitude == 0){
            return false;
        }
        if(waypointStruct -> longitude == 0){
            return false;
        }
        if(waypointStruct -> otherData == NULL){
            return false;
        }
        if(validateGPXdata(waypointStruct -> otherData) == false){
            return false;
        }
    }
    return true;
}

bool validateRoutes(List * routes){
    ListIterator listIter = createIterator(routes);
    void *elem;
	while ((elem = nextElement(&listIter)) != NULL){
		Route* routeStruct = (Route*)elem;
        if(routeStruct -> name == NULL){
            return false;
        }
        if(routeStruct -> waypoints == NULL){
            return false;
        }
        if(routeStruct -> otherData == NULL){
            return false;
        }
        if(validateWaypoints(routeStruct -> waypoints) == false){
            return false;
        }
        if(validateGPXdata(routeStruct -> otherData) == false){
            return false;
        }
    }
    return true;
}

bool validateTracks(List * tracks){
    ListIterator listIter = createIterator(tracks);
    void *elem;
	while ((elem = nextElement(&listIter)) != NULL){
		Track* trackStruct = (Track*)elem;
        if(trackStruct -> name == NULL){
            return false;
        }
        if(trackStruct -> segments == NULL){
            return false;
        }
        if(trackStruct -> otherData == NULL){
            return false;
        }
        if(validateSegments(trackStruct -> segments) == false){
            return false;
        }
    }
    return true;
}

bool validateSegments(List * segments){
    ListIterator listIter = createIterator(segments);
    void *elem;
	while ((elem = nextElement(&listIter)) != NULL){
		TrackSegment* segment = (TrackSegment*)elem;
        if(segment -> waypoints == NULL){
            return false;
        }
        if(validateWaypoints(segment -> waypoints) == false){
            return false;
        }
    }
    return true;
}

float getHaversine(Waypoint * waypoint1, Waypoint * waypoint2){
    float lat1 = waypoint1 -> latitude;
    float lat2 = waypoint2 -> latitude;
    float lon1 = waypoint1 -> longitude;
    float lon2 = waypoint2 -> longitude;

    //Haversine formula
    const float R = 6371e3;
    const float rad1 = lat1 * M_PI/180; 
    const float rad2 = lat2 * M_PI/180;
    const float dist1 = (lat2-lat1) * M_PI/180;
    const float dist2 = (lon2-lon1) * M_PI/180;

    const float a = sin(dist1/2) * sin(dist1/2) +
            cos(rad1) * cos(rad2) *
            sin(dist2/2) * sin(dist2/2);
    const float c = 2 * atan2(sqrt(a), sqrt(1-a));

    const float d = R * c;

    return d;
}

float getHaversineVariant(Waypoint * waypoint1, float lat, float lon){
    float lat1 = waypoint1 -> latitude;
    float lat2 = lat;
    float lon1 = waypoint1 -> longitude;
    float lon2 = lon;

    //Haversine formula
    const float R = 6371e3;
    const float rad1 = lat1 * M_PI/180; 
    const float rad2 = lat2 * M_PI/180;
    const float dist1 = (lat2-lat1) * M_PI/180;
    const float dist2 = (lon2-lon1) * M_PI/180;

    const float a = sin(dist1/2) * sin(dist1/2) +
            cos(rad1) * cos(rad2) *
            sin(dist2/2) * sin(dist2/2);
    const float c = 2 * atan2(sqrt(a), sqrt(1-a));

    const float d = R * c;

    return d;
}