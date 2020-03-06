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
#define CATNER_DEF_IMAGE_MIME   "image/jpg"
#define CATNER_DEF_UNIT_CODE    "PCE"
#define CATNER_DEF_UNIT_FACTOR  "1"
#define CATNER_DEF_FEATURE_UNIT "00"

// Misc
#define CATNER_STDOUT_FILE "-"

/*
 * Data structures
 */

struct catner_state;

typedef struct catner_state catner_state_s;

struct catner_state
{
	char *path;		// Path to XML file, if document was loaded from one
	
	xmlDocPtr  doc;		// XML document pointer
	xmlNodePtr root;	// Pointer to BMECAT node
	xmlNodePtr header;	// Pointer to HEADER
	xmlNodePtr catalog;	// Pointer to CATALOG node
	xmlNodePtr generator;	// Pointer to GENERATOR node
	xmlNodePtr articles;	// Pointer to T_NEW_CATALOG node

	xmlNodePtr _curr_article;	// For iterating purposes
	xmlNodePtr _curr_feature;	// For iterating purposes
	xmlNodePtr _curr_variant;	// For iterating purposes
};

#endif
