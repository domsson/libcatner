#ifndef LIBCATNER_H
#define LIBCATNER_H

// Name & version
#define CATNER_NAME "catner"
#define CATNER_VER_MAJOR 0
#define CATNER_VER_MINOR 1
#define CATNER_VER_PATCH 0

// XML file settings
#define CATNER_XML_VERSION  "1.0"
#define CATNER_XML_ENCODING "utf-8"

// Default values for various BMEcat entries
#define CATNER_DEF_IMAGE_MIME "image/jpg"

/*
 * Data structures
 */

struct catner_state;

typedef struct catner_state catner_state_s;

struct catner_state
{
	xmlDocPtr  tree;
	xmlNodePtr root;
	xmlNodePtr header;
	xmlNodePtr catalog;
	xmlNodePtr generator;
	xmlNodePtr articles;
};

#endif
