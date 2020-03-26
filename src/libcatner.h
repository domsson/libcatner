#ifndef LIBCATNER_H
#define LIBCATNER_H

#include <libxml/xmlstring.h>
#include <libxml/tree.h>

// Name & version
#define LIBCATNER_NAME "catner"
#define LIBCATNER_VER_MAJOR 0
#define LIBCATNER_VER_MINOR 1
#define LIBCATNER_VER_PATCH 0

// XML file settings
#define LIBCATNER_XML_VERSION  "1.0"
#define LIBCATNER_XML_ENCODING "utf-8"

// Default values for various BMEcat entries
#define LIBCATNER_DEF_IMAGE_MIME   "image/jpg"
#define LIBCATNER_DEF_UNIT_CODE    "PCE"
#define LIBCATNER_DEF_UNIT_FACTOR  "1"
#define LIBCATNER_DEF_FEATURE_UNIT "00"
#define LIBCATNER_FEATURE_WEIGHT   "kloeckner_weight"

// Misc
#define LIBCATNER_STDOUT_FILE "-"

// Errors
#define LIBCATNER_ERR_NONE               0
#define LIBCATNER_ERR_OTHER             -1
#define LIBCATNER_ERR_OUT_OF_MEMORY     -2
#define LIBCATNER_ERR_ALREADY_EXISTS    -3
#define LIBCATNER_ERR_INVALID_VALUE     -4
#define LIBCATNER_ERR_NO_SUCH_AID      -10
#define LIBCATNER_ERR_NO_SUCH_FID      -11
#define LIBCATNER_ERR_NO_SUCH_VID      -12
#define LIBCATNER_ERR_NO_SUCH_NODE     -13
#define LIBCATNER_ERR_NO_SEL_ARTICLE   -20
#define LIBCATNER_ERR_NO_SEL_FEATURE   -21
#define LIBCATNER_ERR_NO_SEL_VARIANT   -22
#define LIBCATNER_ERR_NO_SEL_IMAGE     -23
#define LIBCATNER_ERR_NO_SEL_UNIT      -24

// BMEcat attributes
#define BMECAT_VERSION   BAD_CAST "2005"
#define BMECAT_NAMESPACE BAD_CAST "http://www.bmecat.org/bmecat/2005.1"

// BMEcat elements/nodes
#define BMECAT_NODE_ROOT                BAD_CAST "BMECAT"
#define BMECAT_NODE_HEADER              BAD_CAST "HEADER"
#define BMECAT_NODE_CATALOG             BAD_CAST "CATALOG"
#define BMECAT_NODE_LOCALE              BAD_CAST "LOCALE"
#define BMECAT_NODE_TERRITORY           BAD_CAST "TERRITORY"
#define BMECAT_NODE_GENERATOR           BAD_CAST "GENERATOR_INFO"
#define BMECAT_NODE_ARTICLES            BAD_CAST "T_NEW_CATALOG"
#define BMECAT_NODE_ARTICLE             BAD_CAST "ARTICLE"
#define BMECAT_NODE_ARTICLE_ID          BAD_CAST "SUPPLIER_AID"
#define BMECAT_NODE_ARTICLE_DETAILS     BAD_CAST "ARTICLE_DETAILS"
#define BMECAT_NODE_ARTICLE_TITLE       BAD_CAST "DESCRIPTION_SHORT"
#define BMECAT_NODE_ARTICLE_DESCR       BAD_CAST "DESCRIPTION_LONG"
#define BMECAT_NODE_ARTICLE_UNITS       BAD_CAST "ARTICLE_ORDER_DETAILS"
#define BMECAT_NODE_ARTICLE_MAIN_UNIT   BAD_CAST "ORDER_UNIT"
#define BMECAT_NODE_ARTICLE_ALT_UNIT    BAD_CAST "ALTERNATIVE_UNIT"
#define BMECAT_NODE_ARTICLE_UNIT_CODE   BAD_CAST "ALTERNATIVE_UNIT_CODE"
#define BMECAT_NODE_ARTICLE_UNIT_FACTOR BAD_CAST "ALTERNATIVE_UNIT_FACTOR"
#define BMECAT_NODE_ARTICLE_CATEGORY    BAD_CAST "ARTICLE_REFERENCE"
#define BMECAT_NODE_ARTICLE_CATEGORY_ID BAD_CAST "CATALOG_ID"
#define BMECAT_NODE_ARTICLE_IMAGES      BAD_CAST "MIME_INFO"
#define BMECAT_NODE_ARTICLE_IMAGE       BAD_CAST "MIME"
#define BMECAT_NODE_ARTICLE_IMAGE_MIME  BAD_CAST "MIME_TYPE"
#define BMECAT_NODE_ARTICLE_IMAGE_PATH  BAD_CAST "MIME_SOURCE"
#define BMECAT_NODE_FEATURES            BAD_CAST "ARTICLE_FEATURES"
#define BMECAT_NODE_FEATURE             BAD_CAST "FEATURE"
#define BMECAT_NODE_FEATURE_ID          BAD_CAST "FID"
#define BMECAT_NODE_FEATURE_NAME        BAD_CAST "FNAME"
#define BMECAT_NODE_FEATURE_ORDER       BAD_CAST "FORDER"
#define BMECAT_NODE_FEATURE_DESCR       BAD_CAST "FDESCR"
#define BMECAT_NODE_FEATURE_UNIT        BAD_CAST "FUNIT"
#define BMECAT_NODE_FEATURE_VALUE       BAD_CAST "FVALUE"
#define BMECAT_NODE_VARIANTS            BAD_CAST "VARIANTS"
#define BMECAT_NODE_VARIANT             BAD_CAST "VARIANT"
#define BMECAT_NODE_VARIANT_ID          BAD_CAST "SUPPLIER_AID_SUPPLEMENT"
#define BMECAT_NODE_VARIANT_VALUE       BAD_CAST "FVALUE"

/*
 * Data structures
 */

struct catner_state
{
	int error;              // Last error that occured
	char *path;		// Path to XML file, if doc was loaded from one
	
	xmlDocPtr  doc;		// XML document pointer
	xmlNodePtr root;	// Pointer to BMECAT node
	xmlNodePtr header;	// Pointer to HEADER
	xmlNodePtr catalog;	// Pointer to CATALOG node
	xmlNodePtr generator;	// Pointer to GENERATOR node
	xmlNodePtr articles;	// Pointer to T_NEW_CATALOG node

	xmlNodePtr _curr_article;	// Selected article
	xmlNodePtr _curr_feature;	// Selected features
	xmlNodePtr _curr_variant;	// Selected variant
	xmlNodePtr _curr_image;		// Selected article image
	xmlNodePtr _curr_unit;		// Selected article unit
};

typedef struct catner_state catner_state_s;

/*
 * Adding elements
 */

int catner_add_generator(catner_state_s *cs, const char *value);
int catner_add_territory(catner_state_s *cs, const char *value);

int catner_add_article(catner_state_s *cs, const char *aid, const char *title, const char *descr);
int catner_add_article_unit(catner_state_s *cs, const char *aid, const char *code, const char *factor, int main);
int catner_add_article_image(catner_state_s *cs, const char *aid, const char *mime, const char *path);
int catner_add_article_category(catner_state_s *cs, const char *aid, const char *value);
int catner_add_feature(catner_state_s *cs, const char *aid, const char *fid, const char *name, const char *descr, const char *unit, const char *value);
int catner_add_variant(catner_state_s *cs, const char *aid, const char *fid, const char *vid, const char *value);
int catner_add_weight_feature(catner_state_s *cs, const char *aid, const char *value);
int catner_add_weight_variant(catner_state_s *cs, const char *aid, const char *vid, const char *value);

/*
 * Setting element content
 */

int catner_set_locale(catner_state_s *cs, const char *value);
int catner_set_generator(catner_state_s *cs, const char *value);

int catner_set_article_id(catner_state_s *cs, const char *aid, const char *value);
int catner_set_article_title(catner_state_s *cs, const char *aid, const char *value);
int catner_set_article_descr(catner_state_s *cs, const char *aid, const char *value);
int catner_set_feature_id(catner_state_s *cs, const char *aid, const char *fid, const char *value);
int catner_set_feature_name(catner_state_s *cs, const char *aid, const char *fid, const char *value);
int catner_set_feature_descr(catner_state_s *cs, const char *aid, const char *fid, const char *value);
int catner_set_feature_value(catner_state_s *cs, const char *aid, const char *fid, const char *value);
int catner_set_feature_unit(catner_state_s *cs, const char *aid, const char *fid, const char *value);
int catner_set_variant_value(catner_state_s *cs, const char *aid, const char *fid, const char *vid, const char *value);
int catner_set_weight_variant(catner_state_s *cs, const char *aid, const char *vid, const char *value);

/*
 * Getting element content
 */

size_t catner_get_locale(catner_state_s *cs, char *buf, size_t len);
size_t catner_get_generator(catner_state_s *cs, char *buf, size_t len);
size_t catner_get_territories(catner_state_s *cs, char *buf, size_t len);

size_t catner_get_article_aid(catner_state_s *cs, char *buf, size_t len);
size_t catner_get_article_title(catner_state_s *cs, const char *aid, char *buf, size_t len);
size_t catner_get_article_descr(catner_state_s *cs, const char *aid, char *buf, size_t len);
size_t catner_get_article_categories(catner_state_s *cs, const char *aid, char *buf, size_t len);

size_t catner_get_sel_article_id(catner_state_s *cs, char *buf, size_t len);
size_t catner_get_sel_feature_id(catner_state_s *cs, char *buf, size_t len);
size_t catner_get_sel_variant_id(catner_state_s *cs, char *buf, size_t len);

/*
 * Deleting elements
 */

int catner_del_generator(catner_state_s *cs);
int catner_del_territory(catner_state_s *cs, const char *value);

int catner_del_article(catner_state_s *cs, const char *aid);
int catner_del_article_image(catner_state_s *cs, const char *aid, const char *path);
int catner_del_article_category(catner_state_s *cs, const char *aid, const char *cid);
int catner_del_feature(catner_state_s *cs, const char *aid, const char *fid);
int catner_del_variant(catner_state_s *cs, const char *aid, const char *fid, const char *vid);
int catner_del_weight_feature(catner_state_s *cs, const char *aid);
int catner_del_weight_variant(catner_state_s *cs, const char *aid, const char *vid);

/*
 * Checking for elements
 */

//int catner_has_article_title(catner_state_s *cs, const char *aid);
//int catner_has_article_descr(catner_state_s *cs, const char *aid);
//int catner_has_article_images(catner_state_s *cs, const char *aid);
//int catner_has_article_categories(catner_state_s *cs, const char *aid);

/*
 * Counting elements
 */

size_t catner_num_territories(catner_state_s *cs);
size_t catner_num_articles(catner_state_s *cs);
size_t catner_num_article_categories(catner_state_s *cs, const char *aid);
//size_t catner_num_article_images(catner_state_s *cs, const char *aid);
size_t catner_num_features(catner_state_s *cs, const char *aid);
size_t catner_num_variants(catner_state_s *cs, const char *aid, const char *fid);

/*
 * Selecting active elements
 */

int catner_sel_article(catner_state_s *cs, const char *aid);
int catner_sel_feature(catner_state_s *cs, const char *fid);
//int catner_sel_variant(catner_state_s *cs, const char *vid);
//int catner_sel_image(catner_state_s *cs, const char *path);
//int catner_sel_unit(catner_state_s *cs, const char *unit);

int catner_sel_first_article(catner_state_s *cs);
int catner_sel_first_feature(catner_state_s *cs);
int catner_sel_first_variant(catner_state_s *cs);
int catner_sel_first_image(catner_state_s *cs);
int catner_sel_first_unit(catner_state_s *cs);

int catner_sel_next_article(catner_state_s *cs);
int catner_sel_next_feature(catner_state_s *cs);
int catner_sel_next_variant(catner_state_s *cs);
int catner_sel_next_image(catner_state_s *cs);
int catner_sel_next_unit(catner_state_s *cs);

/*
 * Output
 */

int catner_write_xml(catner_state_s *cs, const char *path);
int catner_print_xml(catner_state_s *cs);
int catner_save(catner_state_s *cs);

/*
 * Initialization
 */

catner_state_s *catner_init();
catner_state_s *catner_load(const char *path, int amend);

/*
 * Free, Debug, etc
 */

void catner_free(catner_state_s *cs);
int catner_last_error(catner_state_s *cs);

#endif
