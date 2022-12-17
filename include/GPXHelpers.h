//Luke Fischer - 1061800

#ifndef GPX_PARSER_HELPERS_H
#define GPX_PARSER_HELPERS_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlschemastypes.h>
#include "LinkedListAPI.h"

GPXData * createGPXData();
Waypoint * createWaypoint();
void constructDoc(GPXdoc *, xmlNode *);
Waypoint * constructWaypoint(Waypoint *, GPXdoc *, xmlNode *);
Route * constructRoute(Route *, GPXdoc *, xmlNode *);
Track * constructTrack(Track *, GPXdoc *, xmlNode *);
char * printFunc(void *toBePrinted);
int compareFunc(const void *first, const void *second);
void deleteFunc(void *toBeDeleted);
int numWaypointData(List *);
int numRouteData(List *);
int numGPXData(List *);
int numTrackData(List *);
int numTrackSegments(List *);
void writeWaypoints(List *, xmlNodePtr);
void writeRoutes(List *, xmlNodePtr);
void writeGPXData(List *, xmlNodePtr);
void writeTracks(List *, xmlNodePtr);
bool validateGPXstruct(GPXdoc *);
bool validateGPXdata(List *);
bool validateWaypoints(List *);
bool validateRoutes(List *);
bool validateTracks(List *);
bool validateSegments(List *);
float getHaversine(Waypoint *, Waypoint *);
void deleteDummy(void *);
float getHaversineVariant(Waypoint *, float, float);
#endif
