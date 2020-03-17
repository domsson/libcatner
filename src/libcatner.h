#ifndef LIBCATNER_H
#define LIBCATNER_H

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

// Misc
#define LIBCATNER_STDOUT_FILE "-"

// Errors
#define LIBCATNER_ERR_NONE              0
#define LIBCATNER_ERR_OTHER            -1
#define LIBCATNER_ERR_NO_SEL_ARTICLE   -2
#define LIBCATNER_ERR_NO_SEL_FEATURE   -3
#define LIBCATNER_ERR_NO_SEL_VARIANT   -4
#define LIBCATNER_ERR_NO_SEL_IMAGE     -5
#define LIBCATNER_ERR_NO_SEL_UNIT      -6

/*
 * Data structures
 */

struct catner_state
{
	char *path;		// Path to XML file, if doc was loaded from one
	
	xmlDocPtr  doc;		// XML document pointer
	xmlNodePtr root;	// Pointer to BMECAT node
	xmlNodePtr header;	// Pointer to HEADER
	xmlNodePtr catalog;	// Pointer to CATALOG node
	xmlNodePtr generator;	// Pointer to GENERATOR node
	xmlNodePtr articles;	// Pointer to T_NEW_CATALOG node

	xmlNodePtr _curr_article;	// For iterating purposes
	xmlNodePtr _curr_feature;	// ...
	xmlNodePtr _curr_variant;
	xmlNodePtr _curr_image;
	xmlNodePtr _curr_unit;

	int error;              // Last error that occured
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

/*
 * Deleting elements
 */

void catner_del_generator(catner_state_s *cs);
void catner_del_territory(catner_state_s *cs, const char *value);

void catner_del_article(catner_state_s *cs, const char *aid);
void catner_del_article_category(catner_state_s *cs, const char *aid, const char *cid);
void catner_del_article_image(catner_state_s *cs, const char *aid, const char *path);
void catner_del_feature(catner_state_s *cs, const char *aid, const char *fid);
void catner_del_article_feature_variant(catner_state_s *cs, const char *aid, const char *fid, const char *vid);

/*
 * Counting elements
 */

size_t catner_num_territories(catner_state_s *cs);
size_t catner_num_articles(catner_state_s *cs);
size_t catner_num_article_categories(catner_state_s *cs, const char *aid);
size_t catner_num_article_features(catner_state_s *cs, const char *aid);
size_t catner_num_article_feature_variants(catner_state_s *cs, const char *aid, const char *fid);

/*
 * Selecting active elements
 */

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
