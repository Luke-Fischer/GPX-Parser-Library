//Luke Fischer - 1061800

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "GPXParser.h"
#include "GPXHelpers.h"
#include "LinkedListAPI.h"
#include <stdbool.h>
#include <math.h>

/* Public API - main */

/** Function to create an GPX object based on the contents of an GPX file.
 *@pre File name cannot be an empty string or NULL.
       File represented by this name must exist and must be readable.
 *@post Either:
        A valid GPXdoc has been created and its address was returned
		or 
		An error occurred, and NULL was returned
 *@return the pointer to the new struct or NULL
 *@param fileName - a string containing the name of the GPX file
**/

GPXdoc * createGPXdoc(char* fileName){
    if(fileName == NULL){
        return NULL;
    }

    //this initialize the library
    LIBXML_TEST_VERSION

    //char *gpxString;
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;

    //parse the file and get the DOM (tree)
    doc = xmlReadFile(fileName, NULL, 0);

    //Check for xml file validity
    if (doc == NULL) {
        //printf("error: could not parse file %s\n", fileName);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }

    //Malloc space for GPXdoc
    GPXdoc *gpx = malloc(sizeof(GPXdoc));

    //Initialize lists for waypoints, tracks and routes
    gpx -> waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    gpx -> tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);
    gpx -> routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    

    
    //Get the root element node 
    root_element = xmlDocGetRootElement(doc);

    //Parse xmlDoc tree into a GPXdoc
    constructDoc(gpx, root_element);

    //Print out gpx doc
    //gpxString = GPXdocToString(gpx);
    //printf("%s\n", gpxString);
    //free(gpxString);
    //deleteGPXdoc(gpx);

    //free the document
    xmlFreeDoc(doc);

    //Free the global variables that may have been allocated by the parser.
    xmlCleanupParser();

    return gpx;
}

/** Function to create a string representation of an GPX object.
 *@pre GPX object exists, is not null, and is valid
 *@post GPX has not been modified in any way, and a string representing the GPX contents has been created
 *@return a string contaning a humanly readable representation of an GPX object
 *@param obj - a pointer to an GPX struct
**/
char* GPXdocToString(GPXdoc* doc){
    char doubleBuffer[50];
    char *gpxString;
    char *tmp;
    char *wptStr;
    char *routeStr;
    char *trackStr;

    //toString for namespace
    gpxString = (char *)malloc(sizeof(char) * ((strlen("Namespace: ")) + strlen(doc -> namespace) + 1));
    if(gpxString == NULL){
        return NULL;
    }
    strcpy(gpxString, "Namespace: ");
    strcat(gpxString, doc -> namespace);

    //toString for creator
    tmp = realloc(gpxString, sizeof(char) * ((strlen(" Creator: ")) + strlen(doc -> creator) + 1) + strlen(gpxString) + 1);

    if(tmp == NULL){
        return NULL;
    }

    //Concatenat creator onto gpx string
    gpxString = tmp;
    strcat(gpxString, " Creator: ");
    strcat(gpxString, doc -> creator);

    //Cast (double) version into a string "doubleBuffer"
    sprintf(doubleBuffer, "%g", doc -> version);

    //Realloc enough space for doc -> version to be concatenated onto gpx string
    tmp = realloc(gpxString, sizeof(char) * ((strlen(" Version: ")) + strlen(doubleBuffer) + 1 + strlen(gpxString) + 1));

    if(tmp == NULL){
        return NULL;
    }
    gpxString = tmp;
    strcat(gpxString, " Version: ");
    strcat(gpxString, doubleBuffer);

    wptStr = toString(doc -> waypoints);
    routeStr = toString(doc -> routes);
    trackStr = toString(doc -> tracks);

    tmp = realloc(gpxString, sizeof(char) * (strlen(wptStr) + strlen(routeStr) + strlen(trackStr)) + 1000);
    gpxString = tmp;

    strcat(gpxString, "\n");
    strcat(gpxString, wptStr);
    strcat(gpxString, routeStr);
    strcat(gpxString, trackStr);

    free(wptStr);
    free(routeStr);
    free(trackStr);
   
    return gpxString;
}

/** Function to delete doc content and free all the memory.
 *@pre GPX object exists, is not null, and has not been freed
 *@post GPX object had been freed
 *@return none
 *@param obj - a pointer to an GPX struct
**/
void deleteGPXdoc(GPXdoc* doc){
    if(doc != NULL){

        //Free doc string
        free(doc -> creator);

        //Free waypoint string
        freeList(doc -> waypoints);
        freeList(doc -> routes);
        freeList(doc -> tracks);

        free(doc);
    }
}

void deleteGpxData( void* data){
    if(data != NULL){
        GPXData* otherData = (GPXData*)data;
        free(otherData);
    }
}
char* gpxDataToString( void* data){
    char *str;
    if(data != NULL){
        GPXData *otherData = (GPXData*)data;
        str = malloc(sizeof(char) * strlen(otherData -> value) + 100);
        sprintf(str, "%s: value: %s\n",otherData -> name, otherData -> value);     
    }
    return str;
}
int compareGpxData(const void *first, const void *second){
    return 0;
}

void deleteTrack(void* data){
    if(data != NULL){
        Track* track = (Track*)data;
        TrackSegment *trackSegmentStruct;
        trackSegmentStruct = getFromBack(track -> segments);
        if(track -> segments != NULL){
            freeList(track -> segments);
        }
        if(track-> otherData != NULL){
            freeList(track -> otherData);
        }
        freeList(trackSegmentStruct -> waypoints);
        free(trackSegmentStruct);
        free(track -> name);
        free(track);
    }

}
char* trackToString(void* data){
    char *str;
    if(data != NULL){
        Track *trackStruct = (Track*)data;
        TrackSegment *trackSegmentStruct; 
        trackSegmentStruct = getFromBack(trackStruct -> segments);

        char *otherStr = toString(trackStruct -> otherData);
        char *wptStr = toString(trackSegmentStruct -> waypoints);
        str = malloc((sizeof(char) + strlen(trackStruct -> name) + strlen(wptStr) + strlen(otherStr)) + 100);
        sprintf(str, "Name: %s\nOther data: %s\n%s", trackStruct -> name, otherStr, wptStr);
        free(wptStr);
        free(otherStr);
    }
    return str;

}
int compareTracks(const void *first, const void *second){
    return 0;
}

void deleteTrackSegment(void* data){
    if(data != NULL){

        /*if(trackSegment -> waypoints != NULL){
            freeList(trackSegment -> waypoints);
        }*/
       // free(trackSegment);
    }
}
char* trackSegmentToString(void* data){
    return NULL;
}
int compareTrackSegments(const void *first, const void *second){
    return 0;
}

void deleteWaypoint(void* data){
    if(data != NULL){
        Waypoint* waypoint = (Waypoint*)data;
        if(waypoint -> otherData != NULL){
            freeList(waypoint -> otherData);
        }
        free(waypoint -> name);
        free(waypoint);
    }
}
char* waypointToString( void* data){
    char *str;
    char *dataStr;
    if(data != NULL){
        Waypoint *waypoint = (Waypoint*)data;
        dataStr = toString(waypoint -> otherData);
        str = malloc(sizeof(char) * strlen(waypoint -> name) + 100);
        sprintf(str, "Name: %s\n    Lat: %f\n    Lon: %f\n       %s\n", waypoint -> name, waypoint -> latitude, waypoint -> longitude, dataStr);
        free(dataStr);
    }
    return str;
}
int compareWaypoints(const void *first, const void *second){
    return 0;
}

void deleteRoute(void* data){
    if(data != NULL){
        Route* route = (Route*)data;
        if(route -> waypoints != NULL){
            freeList(route -> waypoints);
        }
        if(route -> otherData != NULL){
            freeList(route -> otherData);
        }
        free(route -> name);
        free(route);
    }
}
char* routeToString(void* data){
    char *str;
    if(data != NULL){
        Route *route = (Route*)data;
        char *wptStr = toString(route -> waypoints);
        char *otherStr = toString(route -> otherData);
        str = malloc((sizeof(char) + strlen(route -> name) + strlen(wptStr) + strlen(otherStr)) + 100);
        sprintf(str, "Name: %s\nOther data: %s\n%s", route -> name, otherStr, wptStr);
        free(wptStr);
        free(otherStr);
    }
    return str;
}
int compareRoutes(const void *first, const void *second){
    return 0;
}

//Total number of waypoints in the GPX file
int getNumWaypoints(const GPXdoc* doc){
    if(doc == NULL){
        return 0;
    }
    int numWaypoints = getLength(doc -> waypoints);
    return numWaypoints;
}

//Total number of routes in the GPX file
int getNumRoutes(const GPXdoc* doc){
    if(doc == NULL){
        return 0;
    }
    int numRoutes = getLength(doc -> routes);
    return numRoutes;
}

//Total number of tracks in the GPX file
int getNumTracks(const GPXdoc* doc){
    if(doc == NULL){
        return 0;
    }
    int numTracks = getLength(doc -> tracks);
    return numTracks;
}

//Total number of track segments
int getNumSegments(const GPXdoc* doc){
    int numSegments = 0;
    if(doc == NULL){
        return 0;
    }
    List *trackList = doc -> tracks;

	ListIterator iter = createIterator(trackList);
    void *elem;
	while ((elem = nextElement(&iter)) != NULL){
		Track* trackStruct = (Track*)elem;
        numSegments = numSegments + getLength(trackStruct -> segments);
    }
    return numSegments;
}

int getNumGPXData(const GPXdoc* doc){
    if(doc == NULL){
        return 0;
    }
    int numData = 0;

    numData += numWaypointData(doc -> waypoints);
    numData += numRouteData(doc -> routes);
    numData += numTrackData(doc -> tracks);

    return numData;
}

//Exctract number of data from waypoint struct
int numWaypointData(List *list){
    //Get GPX count for waypoints
    int numData = 0;
	ListIterator listIter = createIterator(list);
    void *elem;
	while ((elem = nextElement(&listIter)) != NULL){
		Waypoint* waypointStruct = (Waypoint*)elem;
        numData += numGPXData(waypointStruct -> otherData);
        if(strcmp(waypointStruct -> name, "n/a") != 0){
            numData++;
        }
    }
    return numData;
}

//Extract number of data from route struct
int numRouteData(List *list){
    int numData = 0;
	ListIterator listIter = createIterator(list);
    void *elem;
	while ((elem = nextElement(&listIter)) != NULL){
		Route* routeStruct = (Route*)elem;
        numData += numGPXData(routeStruct -> otherData);
        numData += numWaypointData(routeStruct -> waypoints);
        if(strcmp(routeStruct -> name, "n/a") != 0){
            numData++;
        }
    }
    return numData;
}

//Extract number of data from GPX struct
int numGPXData(List *list){
    int numData = 0;
	ListIterator listIter = createIterator(list);
    void *elem;
	while ((elem = nextElement(&listIter)) != NULL){
		GPXData* gpxStruct = (GPXData*)elem;
        if(strcmp(gpxStruct -> name, "n/a") != 0){
            numData++;
        }
    }
    return numData;
}

int numTrackData(List *list){
    int numData = 0;
	ListIterator listIter = createIterator(list);
    void *elem;
	while ((elem = nextElement(&listIter)) != NULL){
		Track* trackStruct = (Track*)elem;
        numData += numGPXData(trackStruct -> otherData);
        numData += numTrackSegments(trackStruct -> segments);
        if(strcmp(trackStruct -> name, "n/a") != 0){
            numData++;
        }
    }
    return numData;
}

int numTrackSegments(List *list){
    int numData = 0;
	ListIterator listIter = createIterator(list);
    void *elem;
	while ((elem = nextElement(&listIter)) != NULL){
		TrackSegment* trackSegmentStruct = (TrackSegment*)elem;
        numData += numWaypointData(trackSegmentStruct -> waypoints);
    }
    return numData;
}

//Get specified waypoint
Waypoint* getWaypoint(const GPXdoc* doc, char* name){
    if(doc == NULL || name == NULL){
        return NULL;
    }
	ListIterator listIter = createIterator(doc -> waypoints);
    void *elem;
	while ((elem = nextElement(&listIter)) != NULL){
		Waypoint* waypointStruct = (Waypoint*)elem;
        if(strcmp(waypointStruct -> name, name) == 0){
            return waypointStruct;
        }
    }
    return NULL;
}

//Get specified track
Track* getTrack(const GPXdoc* doc, char* name){
    if(doc == NULL || name == NULL){
        return NULL;
    }
	ListIterator listIter = createIterator(doc -> tracks);
    void *elem;
	while ((elem = nextElement(&listIter)) != NULL){
		Track* trackStruct = (Track*)elem;
        if(strcmp(trackStruct -> name, name) == 0){
            return trackStruct;
        }
    }
    return NULL;
}

//Get specified route
Route* getRoute(const GPXdoc* doc, char* name){
    if(doc == NULL || name == NULL){
        return NULL;
    }
	ListIterator listIter = createIterator(doc -> routes);
    void *elem;
	while ((elem = nextElement(&listIter)) != NULL){
		Route* routeStruct = (Route*)elem;
        if(strcmp(routeStruct -> name, name) == 0){
            return routeStruct;
        }
    }
    return NULL;
}

/***Assignment 2 Module 1****/

GPXdoc* createValidGPXdoc(char* fileName, char* gpxSchemaFile){
    if(fileName == NULL || gpxSchemaFile == NULL){
        return NULL;
    }
    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxtParse;
    //char *gpxString;
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;

    //this initialize the library
    LIBXML_TEST_VERSION

    //parse the file and get the DOM (tree)
    doc = xmlReadFile(fileName, NULL, 0);

    //Check for xml file validity
    if (doc == NULL) {
        //printf("error: could not parse file %s\n", fileName);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }
    xmlLineNumbersDefault(1);
    ctxtParse = xmlSchemaNewParserCtxt(gpxSchemaFile);
    schema = xmlSchemaParse(ctxtParse);

    xmlSchemaValidCtxtPtr ctxt;
    int ret;
    ctxt = xmlSchemaNewValidCtxt(schema);
    ret = xmlSchemaValidateDoc(ctxt, doc);
    xmlSchemaFreeValidCtxt(ctxt);
    if(ret != 0){
        xmlSchemaFreeParserCtxt(ctxtParse);
        //free the document
        xmlFreeDoc(doc);

        if(schema != NULL){
            xmlSchemaFree(schema);
        }

        //Free the global variables that may have been allocated by the parser.
        xmlSchemaCleanupTypes();
        xmlCleanupParser();
        xmlMemoryDump();
        return NULL;
    }
    

    //Malloc space for GPXdoc
    GPXdoc *gpx = malloc(sizeof(GPXdoc));

    //Initialize lists for waypoints, tracks and routes
    gpx -> waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    gpx -> tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);
    gpx -> routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    
    //Get the root element node 
    root_element = xmlDocGetRootElement(doc);
    
    //Parse xmlDoc tree into a GPXdoc
    constructDoc(gpx, root_element);

    //Print out gpx doc
    //gpxString = GPXdocToString(gpx);
    //printf("%s\n", gpxString);
    //free(gpxString);
    //deleteGPXdoc(gpx);
    
    xmlSchemaFreeParserCtxt(ctxtParse);
    //free the document
    xmlFreeDoc(doc);

    if(schema != NULL){
        xmlSchemaFree(schema);
    }

    //Free the global variables that may have been allocated by the parser.
    xmlSchemaCleanupTypes();
    xmlCleanupParser();
    xmlMemoryDump();

    return gpx; 
}

bool writeGPXdoc(GPXdoc* doc, char* fileName){
    if(doc == NULL || fileName == NULL){
        return false;
    }
    xmlNsPtr NsPtr = NULL;
    char versionTemp[30];
    xmlDocPtr docPtr = NULL;       /* document pointer */
    xmlNodePtr root_node = NULL;

    LIBXML_TEST_VERSION

    //Cast (double) version into a string "doubleBuffer"
    sprintf(versionTemp, "%g", doc -> version);

    //Create root node for gpx doc
    docPtr = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "gpx");
    
    //Set gpx attributes
    xmlNewProp(root_node, BAD_CAST "version", BAD_CAST versionTemp);
    xmlNewProp(root_node, BAD_CAST "creator", BAD_CAST doc -> creator);

    NsPtr = xmlNewNs(root_node, (const xmlChar *)doc -> namespace, NULL);
    xmlSetNs(root_node, NsPtr);

    xmlDocSetRootElement(docPtr, root_node);

     /*****WAYPOINTS******/
    writeWaypoints(doc -> waypoints, root_node);

    /******ROUTES*****/
    writeRoutes(doc -> routes, root_node);

    /****TRACKS*****/
    writeTracks(doc -> tracks, root_node);
    /* 
     * Dumping document to stdio or file
     */
    int ret = 0;
    ret = xmlSaveFormatFileEnc(fileName, docPtr, "UTF-8", 1);

    //Writing failed
    if(ret == -1){
        xmlFreeDoc(docPtr);

        xmlCleanupParser();

        xmlMemoryDump();
        return false;
    }

    xmlFreeDoc(docPtr);

    xmlCleanupParser();

    xmlMemoryDump();

    return true;
}

bool validateGPXDoc(GPXdoc* doc, char* gpxSchemaFile){
    if(doc == NULL || gpxSchemaFile == NULL){
        return false;
    }

    /****PART 1 VALIDATION*****/
    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxtParser; 
    xmlNsPtr NsPtr = NULL;
    char versionTemp[30];
    xmlDocPtr docPtr = NULL;       /* document pointer */
    xmlNodePtr root_node = NULL;

    LIBXML_TEST_VERSION

    //Cast (double) version into a string "doubleBuffer"
    sprintf(versionTemp, "%g", doc -> version);

    //Create root node for gpx doc
    docPtr = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "gpx");
    
    //Set gpx attributes
    xmlNewProp(root_node, BAD_CAST "version", BAD_CAST versionTemp);
    xmlNewProp(root_node, BAD_CAST "creator", BAD_CAST doc -> creator);

    NsPtr = xmlNewNs(root_node, (const xmlChar *)doc -> namespace, NULL);
    xmlSetNs(root_node, NsPtr);

    xmlDocSetRootElement(docPtr, root_node);

     /*****WAYPOINTS******/
    writeWaypoints(doc -> waypoints, root_node);

    /******ROUTES*****/
    writeRoutes(doc -> routes, root_node);

    /****TRACKS*****/
    writeTracks(doc -> tracks, root_node);

    LIBXML_TEST_VERSION


    //Check for xml file validity
    if (docPtr == NULL) {
        xmlFreeDoc(docPtr);
        xmlCleanupParser();
        return false;
    }
    xmlLineNumbersDefault(1);
    ctxtParser = xmlSchemaNewParserCtxt(gpxSchemaFile);

    schema = xmlSchemaParse(ctxtParser);
    xmlSchemaValidCtxtPtr ctxt;
    int ret;
    ctxt = xmlSchemaNewValidCtxt(schema);
    ret = xmlSchemaValidateDoc(ctxt, docPtr);
    xmlSchemaFreeValidCtxt(ctxt);
    if(ret != 0){
        xmlSchemaFreeParserCtxt(ctxtParser);
        //free the document
        xmlFreeDoc(docPtr);

        if(schema != NULL){
            xmlSchemaFree(schema);
        }

        //Free the global variables that may have been allocated by the parser.
        xmlSchemaCleanupTypes();
        xmlCleanupParser();
        xmlMemoryDump();
        return NULL;
    }


    xmlSchemaFreeParserCtxt(ctxtParser);
    //free the document
    xmlFreeDoc(docPtr);

    if(schema != NULL){
        xmlSchemaFree(schema);
    }

    //Free the global variables that may have been allocated by the parser.
    xmlSchemaCleanupTypes();
    xmlCleanupParser();
    xmlMemoryDump();

    /*****PART 2 VALIDATION******/
    if(validateGPXstruct(doc) == false){
        return false;
    }
    if(validateWaypoints(doc -> waypoints) == false){
        return false;
    }
    if(validateRoutes(doc -> routes) == false){
        return false;
    }
    if(validateTracks(doc -> tracks) == false){
        return false;
    }
    return true;
}

/***Assignment 2 Module 2*****/
float round10(float len){
    float a = len;
    int x = (a+5)/10;
    x = x*10;
    return (float) x;
}

float getRouteLen(const Route *rt){
    float routeDist = 0;
    if(rt == NULL){
        return 0;
    }
    ListIterator listIter1 = createIterator(rt -> waypoints);
    ListIterator listIter2 = createIterator(rt -> waypoints);
    void *elem1;
    void *elem2;

    //Skip first element
    nextElement(&listIter2);

    while ((elem2 = nextElement(&listIter2)) != NULL){
        elem1 = nextElement(&listIter1);
        Waypoint* waypointStruct1 = (Waypoint*)elem1;
        Waypoint* waypointStruct2 = (Waypoint*)elem2;
        routeDist += getHaversine(waypointStruct1, waypointStruct2);
        
    }
    return routeDist;
}

float getTrackLen(const Track *tr){
    Waypoint * firstWaypoint = NULL;
    Waypoint * lastWaypoint = NULL;
    bool newSeg = false;
    float trackDist = 0;
    if(tr == NULL){
        return 0;
    }
    ListIterator listIterSeg = createIterator(tr -> segments);
    void *elemSeg;
    while((elemSeg = nextElement(&listIterSeg)) != NULL){
        TrackSegment* segment = (TrackSegment*)elemSeg;

        ListIterator listIterWay1 = createIterator(segment -> waypoints);
        ListIterator listIterWay2 = createIterator(segment -> waypoints);
        void *elemWay1;
        void *elemWay2;

        //Skip and extract first element
        elemWay1 = nextElement(&listIterWay2);
        firstWaypoint = (Waypoint*)elemWay1;
        if(newSeg == true){
            trackDist += getHaversine(lastWaypoint, firstWaypoint);
            newSeg = false;
        }
        while((elemWay2 = nextElement(&listIterWay2)) != NULL){
            elemWay1 = nextElement(&listIterWay1);
            Waypoint* waypointStruct1 = (Waypoint*)elemWay1;
            Waypoint* waypointStruct2 = (Waypoint*)elemWay2;
            trackDist += getHaversine(waypointStruct1, waypointStruct2);
        }
        //Extract last waypoint of track segment
        elemWay1 = nextElement(&listIterWay1);
        lastWaypoint = (Waypoint*)elemWay1;
        newSeg = true;
    }
    return trackDist;
}

int numRoutesWithLength(const GPXdoc* doc, float len, float delta){
    int numRoutes = 0;
    float tempDist = 0;
    if(doc == NULL || len < 0 || delta < 0){
       return 0;
    }
    ListIterator listIter = createIterator(doc -> routes);
    void *elem;

    while ((elem = nextElement(&listIter)) != NULL){
        Route* routeStruct = (Route*)elem;
        tempDist = getRouteLen(routeStruct);
        if(abs(tempDist - len) <= delta){
            numRoutes++;
        } 
    }
    return numRoutes;
}

int numTracksWithLength(const GPXdoc* doc, float len, float delta){
    int numTracks = 0;
    float tempDist = 0;
    if(doc == NULL || len < 0 || delta < 0){
        return 0;
    }
    ListIterator listIter = createIterator(doc -> tracks);
    void *elem;

    while ((elem = nextElement(&listIter)) != NULL){
        Track* trackStruct = (Track*)elem;
        tempDist = getTrackLen(trackStruct);
        if(abs(tempDist - len) <= delta){
            numTracks++;
        } 
    }
    return numTracks;
}

bool isLoopRoute(const Route* route, float delta){
    Waypoint * firstWaypoint = NULL;
    Waypoint * lastWaypoint = NULL;
    float distance = 0;

    if(route == NULL || delta < 0){
        return false;
    }
    if(getLength(route -> waypoints) < 4){
        return false;
    }
    firstWaypoint = getFromFront(route -> waypoints);
    lastWaypoint = getFromBack(route -> waypoints);

    //Calculate distance between points
    distance = getHaversine(firstWaypoint, lastWaypoint);

    if(distance < delta){
        return true;
    }
    return false;
}

bool isLoopTrack(const Track *tr, float delta){
    bool hasFour = false;
    Waypoint * firstWaypoint = NULL;
    Waypoint * lastWaypoint = NULL;
    float distance = 0;

    if(tr == NULL || delta < 0){
        return false;
    }

    TrackSegment * firstSegment = getFromFront(tr -> segments);
    firstWaypoint = getFromFront(firstSegment -> waypoints);
    TrackSegment * lastSegment = getFromBack(tr -> segments);
    lastWaypoint = getFromBack(lastSegment -> waypoints);

    ListIterator listIterSeg = createIterator(tr -> segments);
    void *elemSeg;
    //Iterate through segments
    while((elemSeg = nextElement(&listIterSeg)) != NULL){
        TrackSegment* segment = (TrackSegment*)elemSeg;
        //Check if segment -> waypoint has atleast 4 points
        if(getLength(segment -> waypoints) >= 4){
            hasFour = true;
        }
    }
    if(hasFour == false){
        return false;
    }

    //Calculate distance between points
    distance = getHaversine(firstWaypoint, lastWaypoint);

    if(distance < delta){
        return true;
    }
    return false;
}

List* getRoutesBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta){
    Waypoint * firstWaypoint = NULL;
    Waypoint * lastWaypoint = NULL;
    float srcDist = 0;
    float destDist = 0;
    if(doc == NULL){
        return NULL;
    }
    List * routesBetween = initializeList(&routeToString, &deleteDummy, &compareRoutes);
    ListIterator listIter = createIterator(doc -> routes);
    void *elem;

    while ((elem = nextElement(&listIter)) != NULL){
        Route* routeStruct = (Route*)elem;
        firstWaypoint = getFromFront(routeStruct -> waypoints);
        lastWaypoint = getFromBack(routeStruct -> waypoints);
        srcDist = getHaversineVariant(firstWaypoint, sourceLat, sourceLong);
        destDist = getHaversineVariant(lastWaypoint, destLat, destLong);
        if((srcDist <= delta) && (destDist <= delta)){
            insertBack(routesBetween, routeStruct);
        }
    }
    if(getLength(routesBetween) > 0){
        return routesBetween;
    }
    else{
        free(routesBetween);
        return NULL;
    }
}

void deleteDummy(void* data){
    
}

List* getTracksBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta){
    Waypoint * firstWaypoint = NULL;
    Waypoint * lastWaypoint = NULL; 
    float srcDist = 0;
    float destDist = 0;

    if(doc == NULL){
        return NULL;
    }
    List * tracksBetween = initializeList(&trackToString, &deleteDummy, &compareTracks);

    ListIterator listIter = createIterator(doc -> tracks);
    void *elem;

    while ((elem = nextElement(&listIter)) != NULL){
        Track* trackStruct = (Track*)elem;

        //Get first point of current track
        TrackSegment * firstSegment = getFromFront(trackStruct -> segments);
        firstWaypoint = getFromFront(firstSegment -> waypoints);

        //Get last point of current track
        TrackSegment * lastSegment = getFromBack(trackStruct -> segments);
        lastWaypoint = getFromBack(lastSegment -> waypoints);

        srcDist = getHaversineVariant(firstWaypoint, sourceLat, sourceLong);
        destDist = getHaversineVariant(lastWaypoint, destLat, destLong);
        if((srcDist <= delta) && (destDist <= delta)){
            insertBack(tracksBetween, trackStruct);
        }
    }
    if(getLength(tracksBetween) > 0){
        return tracksBetween;
    }
    else{
        free(tracksBetween);
        return NULL;
    }
}

/*****MODULE 3******/
char* routeToJSON(const Route *rt){
    char *jsonString = malloc(sizeof(char) * 300);
    if(rt == NULL){
        strcpy(jsonString, "{}");
        return jsonString;
    }
    char name[100];
    char isLoopStr[10];

    int numPoints = getLength(rt -> waypoints);

    bool isLoop = isLoopRoute(rt, 10);

    float len = getRouteLen(rt);
    len = round10(len);

    if(isLoop == true){
        strcpy(isLoopStr, "true");
    }
    else{
        strcpy(isLoopStr, "false");
    }

    //Extract route data
    if(strcmp(rt -> name, "n/a") == 0){
        strcpy(name, "None");
    }
    else{
        strcpy(name, rt -> name);
    }

    //Enter into json format
    sprintf(jsonString, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s}", name, numPoints, len, isLoopStr);

    return jsonString;
}

char* trackToJSON(const Track *tr){
    char *jsonString = malloc(sizeof(char) * 300);
    if(tr == NULL){
        strcpy(jsonString, "{}");
        return jsonString;
    }
    char name[100];
    char isLoopStr[10];

    bool isLoop = isLoopTrack(tr, 10);

    float len = getTrackLen(tr);
    len = round10(len);

    if(isLoop == true){
        strcpy(isLoopStr, "true");
    }
    else{
        strcpy(isLoopStr, "false");
    }

    if(strcmp(tr -> name, "n/a") == 0){
        strcpy(name, "None");
    }
    else{
        strcpy(name, tr -> name);
    }
    //Enter into json format
    sprintf(jsonString, "{\"name\":\"%s\",\"len\":%.1f,\"loop\":%s}", name, len, isLoopStr);

    return jsonString;
}

char* routeListToJSON(const List *list){
    //Cast list
    List * castList = (List *)list;
    int count = 0;
    char *jsonRouteStr = malloc(sizeof(char) * 100);

    if(list == NULL){
        strcpy(jsonRouteStr, "[]");
        return jsonRouteStr;
    }
    char *tempJson;
    ListIterator listIter = createIterator(castList);
    void *elem;

    strcpy(jsonRouteStr, "[");
    while ((elem = nextElement(&listIter)) != NULL){
        if (count != 0){
            strcat(jsonRouteStr, ",");
        }
        Route* routeStruct = (Route*)elem;
        tempJson = routeToJSON(routeStruct);
        char *tmp = realloc(jsonRouteStr, sizeof(char) * (strlen(tempJson) + strlen(jsonRouteStr) +20));
        jsonRouteStr = tmp;

        strcat(jsonRouteStr, tempJson);
        free(tempJson);
        count++;
    }
    strcat(jsonRouteStr, "]");

    return jsonRouteStr;
}

char* trackListToJSON(const List *list){
    int count = 0;
    //Cast list
    List * castList = (List *)list;

    char *jsonTrackStr = malloc(sizeof(char) * 100);

    if(list == NULL){
        strcpy(jsonTrackStr, "[]");
        return jsonTrackStr;
    }
    char *tempJson;
    ListIterator listIter = createIterator(castList);
    void *elem;

    strcpy(jsonTrackStr, "[");
    while ((elem = nextElement(&listIter)) != NULL){
        if(count != 0){
            strcat(jsonTrackStr, ",");
        }
        Track* trackStruct = (Track*)elem;
        tempJson = trackToJSON(trackStruct);
        char *tmp = realloc(jsonTrackStr, sizeof(char) * (strlen(tempJson) + strlen(jsonTrackStr) +20));
        jsonTrackStr = tmp;

        strcat(jsonTrackStr, tempJson);
        free(tempJson);
        count++;
    }
    strcat(jsonTrackStr, "]");

    return jsonTrackStr;
}

char* GPXtoJSON(const GPXdoc* gpx){
    char *jsonString = malloc(sizeof(char) * 300);
    if(gpx == NULL){
        strcpy(jsonString, "{}");
        return jsonString;
    }

    //Init statistical values
    int numWaypoints = getNumWaypoints(gpx);
    int numRoutes = getNumRoutes(gpx);
    int numTracks = getNumTracks(gpx);

    //Enter into json format
    sprintf(jsonString, "{\"version\":%.1lf,\"creator\":\"%s\",\"numWaypoints\":%d,\"numRoutes\":%d,\"numTracks\":%d}", gpx -> version, gpx -> creator, numWaypoints, numRoutes, numTracks);

    return jsonString;
}

void addWaypoint(Route *rt, Waypoint *pt){

}

void addRoute(GPXdoc* doc, Route* rt){

}

GPXdoc* JSONtoGPX(const char* gpxString){
    return NULL;
}

Waypoint* JSONtoWaypoint(const char* gpxString){
    return NULL;
}

Route* JSONtoRoute(const char* gpxString){
    return NULL;
}