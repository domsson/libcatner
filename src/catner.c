#include <stdio.h>
#include <string.h>
#include <libxml/xmlstring.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "bmecat.h"
#include "catner.h"

xmlNodePtr libcatner_add_child(const xmlNodePtr parent, const xmlChar *name, 
		const xmlChar *value)
{
	// Create empty (non-text) or text node, depending on value
	return value == NULL ? xmlNewChild(parent, NULL, name, NULL) :
		xmlNewTextChild(parent, NULL, name, value);
}

/*
 * Searches the parent node for the first child that matches the given `name` 
 * and, if given, text content `value`. Returns the child node found or NULL.
 * If `create` is `1`, the child node will be created if it wasn't found.
 */
xmlNodePtr libcatner_get_child(const xmlNodePtr parent, const xmlChar *name, 
		const xmlChar *value, int create)
{
	// Iterate all child nodes of parent
	xmlNodePtr child = NULL;
	for (child = parent->children; child; child = child->next)
	{
		// Node name mismatch, therefore we continue
		if (xmlStrcmp(child->name, name) != 0)
		{
			continue;
		}
		
		// No node content value given, therefore we're done
		if (value == NULL)
		{
			return child;
		}

		// Node content value given, check if it matches
		if (xmlStrcmp(xmlNodeGetContent(child), value) == 0)
		{
			return child;
		}
	}

	// No matching node found - either create it or return NULL
	return create ? libcatner_add_child(parent, name, value) : NULL;
}

/*
 * Find and return the next sibling node of the same name and return it. 
 * If no further sibling node of the same type exists, NULL is returned.
 */
xmlNodePtr libcatner_next_node(const xmlNodePtr node)
{
	xmlNodePtr current = NULL;
	for (current = node->next; current; current = current->next)
	{
		if (xmlStrcmp(node->name, current->name) == 0)
		{
			return current;
		}
	}
	return NULL;
}

/*
 * Search the parent node for (direct) child nodes with the given name and 
 * return the number of matching nodes found. If value is given (not NULL), 
 * only child nodes with matching text content will be counted.
 */
size_t libcatner_num_children(const xmlNodePtr parent, const xmlChar *name, 
		const xmlChar *value)
{
	size_t num = 0;

	// Iterate all child nodes of parent
	xmlNodePtr child = parent->children;
	while (child != NULL)
	{
		// Does the child node's name match?
		if (xmlStrcmp(child->name, name) == 0)
		{
			// No node content value given, so we count this
			if (value == NULL)
			{
				++num;
			}

			// Node content value given, count if it matches
			else if (xmlStrcmp(xmlNodeGetContent(child), value) == 0)
			{
				++num;
			}
		}
		child = child->next;
	}
	return num;
}

/*
 * Searches parent for child nodes of the given name and returns the `n`th 
 * of those (0-based, so `1` would find the second), if there are that many. 
 * Otherwise returns NULL.
 */
xmlNodePtr libcatner_get_child_at(const xmlNodePtr parent, const xmlChar *name, size_t n)
{
	// We count how many matches we've found (0-based) 
	size_t cur = 0;

	// Iterate all children of parent and check if they match `name`
	xmlNodePtr child = parent->children;
	while (child != NULL)
	{
		if (xmlStrcmp(child->name, BAD_CAST name) == 0)
		{
			// Is this the `n`th item of this type we've found?
			if (cur == n)
			{
				return child;
			}

			// Otherwise, continue to search
			++cur;
		}
		child = child->next;
	}
	
	// Couldn't find `n` number of matching elements (maybe even none)
	return NULL;
}

/*
 * Add the BMEcat root node to the given document and return it.
 * This function does not check for exisiting root elements.
 */
xmlNodePtr libcatner_add_root(const xmlDocPtr doc)
{
	xmlNodePtr root = xmlNewNode(NULL, BMECAT_NODE_ROOT);
	xmlNewProp(root, BAD_CAST "version", BMECAT_VERSION);
	xmlNewProp(root, BAD_CAST "xmlns",   BMECAT_NAMESPACE);
	xmlDocSetRootElement(doc, root);

	return root;
}

/*
 * Returns the BMEcat root node for the given document, or NULL if the document 
 * has no root node or the root node doesn't match BMECAT_NODE_ROOT ("BMECAT").
 * If create is `1`, the root node will be created if it doesn't exist yet.
 */
xmlNodePtr libcatner_get_root(const xmlDocPtr doc, int create)
{
	xmlNodePtr root = xmlDocGetRootElement(doc);
	
	// There is no root element
	if (root == NULL)
	{
		// Create it, if requested, otherwise return NULL
		return create ? libcatner_add_root(doc) : NULL;
	}

	// A BMEcat root element exists, return it
	if (xmlStrcmp(root->name, BMECAT_NODE_ROOT) == 0)
	{
		return root;
	}

	// A root element exists, but isn't the BMEcat root element
	return NULL;
}

/*
 * Returns the HEADER node (BMECAT_NODE_HEADER), if present, otherwise NULL.
 * If create is `1`, the header node will be created if it doesn't exist yet.
 */
xmlNodePtr libcatner_get_header(const xmlNodePtr root, int create)
{
	return libcatner_get_child(root, BMECAT_NODE_HEADER, NULL, create);
}

/*
 * Returns the node containing all ARTICLE nodes ("T_NEW_CATALOG").
 * If create is `1`, the articles node will be created if it doesn't exist yet.
 */
xmlNodePtr libcatner_get_articles(const xmlNodePtr root, int create)
{
	return libcatner_get_child(root, BMECAT_NODE_ARTICLES, NULL, create);
}

/*
 * Returns the CATALOG node within the given node, if present, otherwise NULL.
 * If create is `1`, the catalog node will be created if it doesn't exist yet.
 */
xmlNodePtr libcatner_get_catalog(xmlNodePtr header, int create)
{
	return libcatner_get_child(header, BMECAT_NODE_CATALOG, NULL, create);
}

/*
 * Given the T_NEW_CATALOG parent node, searches all ARTICLE child nodes for 
 * one that has a matching article ID (SUPPLIER_ID) and returns it. If no 
 * matching article node could be found, NULL will be returned.
 */
xmlNodePtr libcatner_get_article(const xmlNodePtr articles, const xmlChar *aid)
{
	// Iterate all articles
	xmlNodePtr article = articles->children;
	while (article != NULL)
	{
		// Find the article's SUPPLIER_AID element with the given aid value
		if (libcatner_get_child(article, BMECAT_NODE_ARTICLE_ID, aid, 0) != NULL)
		{
			// If found, return this article
			return article;
		}

		// Otherwise, check the next article
		article = article->next;
	}

	// No matching article found
	return NULL;
}

/*
 * Given an ARTICLE node, finds and returns the FEATURE node with the given FID 
 * or NULL if no matching feature exists within the article.
 */
xmlNodePtr libcatner_get_feature(const xmlNodePtr article, const xmlChar *fid)
{
	// Find the ARTICLE_FEATURES node, which holds all features
	xmlNodePtr features = libcatner_get_child(article, BMECAT_NODE_FEATURES, NULL, 0);

	// If ARTICLE_FEATURES doesn't exist, then no FEATUREs can exist
	if (features == NULL)
	{
		return NULL;
	}

	xmlNodePtr child = features->children;
	while (child != NULL)
	{
		// We are only interested in FEATURE nodes
		if (xmlStrcmp(child->name, BMECAT_NODE_FEATURE) == 0)
		{
			// If this FEATURE has a FID and its content matches fid, we're done
			if (libcatner_get_child(child, BMECAT_NODE_FEATURE_ID, fid, 0) != NULL)
			{
				return child;
			}
		}
		// Continue with next child node
		child = child->next;
	}

	// No matching node found
	return NULL;
}

/*
 * Set the LOCALE to the given value, overwriting the existing value if any.
 * If the node didn't exist yet, it will be created.
 * Returns 0 on success, -1 on error.
 *
 * The LOCALE node tells the processing software (the shop) what language the 
 * information in the file is in. Valid values are, for example, "EN" or "DE". 
 * 
 * TODO - should we make sure `value` is uppercase?
 *      - should we trim whitespace from `value`?
 */
int catner_set_locale(catner_state_s *cs, const char *value)
{
	// Valid LOCALE values should be two uppercase ASCII letters
	if (xmlStrlen(BAD_CAST value) != 2)
	{
		return -1;
	}
	
	// Get the LOCALE node within CATALOG; create it if not present
	xmlNodePtr locale = libcatner_get_child(cs->catalog, BMECAT_NODE_LOCALE, NULL, 1);

	// Set the LOCALE's text node content to the given string
	xmlNodeSetContent(locale, BAD_CAST value);
	return 0;
}

/*
 * Add a TERRITORY with the given value.
 * Returns 0 on success, -1 on error.
 *
 * TERRITORY nodes tell the processing software (the shop) what regions the 
 * products in this BMEcat file can be shipped to. Examples: "DE", "AT".
 * There can be multiple TERRITORY nodes, but each has to have a unique value.
 *
 * TODO - should we make sure `value` is uppercase?
 *      - should we trim whitespace from `value`?
 */
int catner_add_territory(catner_state_s *cs, const char *value)
{
	// Valid TERRITORY values should be two uppercase ASCII letters
	if (xmlStrlen(BAD_CAST value) != 2)
	{
		return -1;
	}

	// Find or create the TERRITORY node with the given value
	xmlNodePtr t = libcatner_get_child(cs->catalog, BMECAT_NODE_TERRITORY, BAD_CAST value, 1);
	return t == NULL ? -1 : 0;
}

/*
 * Set the GENERATOR_INFO node to the given value.
 *
 * This node is optional and gives information on the software that 
 * was used to generate the BMEcat file.
 */
int catner_set_generator(catner_state_s *cs, const char *value)
{
	if (cs->generator == NULL)
	{
		cs->generator = xmlNewTextChild(cs->header, NULL, BMECAT_NODE_GENERATOR, BAD_CAST value);
		return 0;
	}

	xmlNodeSetContent(cs->generator, BAD_CAST value);
	return 0;
}

/*
 * Sets the title (DESCRIPTION_SHORT) of the article with the given AID. 
 * Returns -1 if no matching article could be found, otherwise 0.
 */
int catner_set_article_title(catner_state_s *cs, const char *aid, const char *title)
{
	xmlNodePtr article = libcatner_get_article(cs->articles, BAD_CAST aid);
	if (article == NULL)
	{
		return -1;
	}

	// Find or create the ARTICLE_DETAILS node within this ARTICLE
	xmlNodePtr details = libcatner_get_child(article, BMECAT_NODE_ARTICLE_DETAILS, NULL, 1);

	// Find or create the DESCRIPTION_SHORT node within ARTICLE_DETAILS
	xmlNodePtr t = libcatner_get_child(details, BMECAT_NODE_ARTICLE_TITLE, NULL, 1);
	
	// Set the text of the title node accordingly
	xmlNodeSetContent(t, BAD_CAST title);
	return 0;
}

/*
 * Sets the description (DESCRIPTION_LONG) of the article with the given AID. 
 * Returns -1 if no matching article could be found, otherwise 0.
 */
int catner_set_article_descr(catner_state_s *cs, const char *aid, const char *descr)
{
	xmlNodePtr article = libcatner_get_article(cs->articles, BAD_CAST aid);
	if (article == NULL)
	{
		return -1;
	}

	// Find or create the ARTICLE_DETAILS node within this ARTICLE
	xmlNodePtr details = libcatner_get_child(article, BMECAT_NODE_ARTICLE_DETAILS, NULL, 1);

	// Find or create the DESCRIPTION_SHORT node within ARTICLE_DETAILS
	xmlNodePtr d = libcatner_get_child(details, BMECAT_NODE_ARTICLE_DESCR, NULL, 1);
	
	// Set the text of the title node accordingly
	xmlNodeSetContent(d, BAD_CAST descr);
	return 0;
}

/*
 * Add a new article with the given ID (SUPPLIER_AID), title and description.
 * If an article with the given ID already exists, this function returns 1.
 * Otherwise, the article will be created and the function returns 0.
 */
int catner_add_article(catner_state_s *cs, const char *aid, const char *title, const char *descr)
{
	// Find the article with the given AID
	if (libcatner_get_article(cs->articles, BAD_CAST aid) != NULL)
	{
		return 1;
	}

	// Create ARTICLE node with SUPPLIER_AID and ARTICLE_DETAILS child nodes
	xmlNodePtr article = xmlNewChild(cs->articles, NULL, BMECAT_NODE_ARTICLE, NULL);
	xmlNewTextChild(article, NULL, BMECAT_NODE_ARTICLE_ID, BAD_CAST aid);
	xmlNodePtr details = xmlNewChild(article, NULL, BMECAT_NODE_ARTICLE_DETAILS, NULL);

	// Possibly add DESCRIPTION_SHORT child node to ARTICLE_DETAILS
	if (title != NULL)
	{
		xmlNewTextChild(details, NULL, BMECAT_NODE_ARTICLE_TITLE, BAD_CAST title);
	}
	// Possibly add DESCRIPTION_LONG child node to ARTICLE_DETAILS
	if (descr != NULL)
	{
		xmlNewTextChild(details, NULL, BMECAT_NODE_ARTICLE_DESCR, BAD_CAST descr);
	}
	
	return 0;
}

// Returns 0 on success, 1 if image was already present, -1 on error
int catner_add_article_image(catner_state_s *cs, const char *aid, const char *mime, const char *path)
{
	xmlNodePtr article = libcatner_get_article(cs->articles, BAD_CAST aid);
	if (article == NULL)
	{
		return -1;
	}
	
	xmlNodePtr images = libcatner_get_child(article, BMECAT_NODE_ARTICLE_IMAGES, NULL, 1);

	// See if there is already an image with that path in this ARTICLE
	xmlNodePtr image  = images->children;
	while (image != NULL)
	{
		// Check if this MIME node has a MIME_SOURCE node with the given `path` value
		if (libcatner_get_child(image, BMECAT_NODE_ARTICLE_IMAGE_PATH, BAD_CAST path, 0) != NULL)
		{
			// If so, this image already exists, we're done
			return 1;
		}

		// Try the next image
		image = image->next;
	}

	// No such image present yet, let's create and return it
	image = xmlNewChild(images, NULL, BMECAT_NODE_ARTICLE_IMAGE, NULL);
	// TODO use xmlNewChild() or xmlNewTextChild()? Might need to ask kloeckner
	xmlNewTextChild(image, NULL, BMECAT_NODE_ARTICLE_IMAGE_MIME, BAD_CAST mime);
	xmlNewTextChild(image, NULL, BMECAT_NODE_ARTICLE_IMAGE_PATH, BAD_CAST path);

	return 0;
}

/*
 * TODO contract/workings of this function: should we automatically set the
 *      main unit if there wasn't one? shouldn't we strictly adhere to the 
 *      user's request? Also, it doesn't technically make much sense to have 
 *      a main unit that has a factor other than "1" ("1.0", "1.00", ...),
 *      so should we check for that? etc etc etc
 */
int catner_add_article_unit(catner_state_s *cs, const char *aid, const char *code, const char *factor, int main)
{
	xmlNodePtr article = libcatner_get_article(cs->articles, BAD_CAST aid);
	if (article == NULL)
	{
		return -1;
	}

	// Construct unit factor string based on user input and default value
	const char *f = factor ? factor : CATNER_DEF_UNIT_FACTOR;

	// Find the ARTICLE_ORDER_DETAILS node, which holds all units
	xmlNodePtr details = libcatner_get_child(article, BMECAT_NODE_ARTICLE_UNITS, NULL, 1);

	// Iterate ARTICLE_ORDER_DETAILS' children to find ALTERNATIVE_UNIT nodes
	xmlNodePtr child = details->children;
	xmlNodePtr alt_unit = NULL;
	while (child != NULL)
	{
		// We're only interested in ALTERNATIVE_UNIT nodes
		if (xmlStrcmp(child->name, BMECAT_NODE_ARTICLE_ALT_UNIT) == 0)
		{
			// Check if this ALTERNATIVE_UNIT has the unit code we're looking for
			if (libcatner_get_child(child, BMECAT_NODE_ARTICLE_UNIT_CODE, BAD_CAST code, 0) != NULL)
			{
				// If so, remember this node and stop iterating
				alt_unit = child;
				break;
			}
		}

		child = child->next;
	}

	xmlNodePtr main_unit = libcatner_get_child(details, BMECAT_NODE_ARTICLE_UNIT, NULL, 0);

	// No ORDER_UNIT (main unit) present yet, let's add it
	if (main_unit == NULL)
	{
		main_unit = xmlNewTextChild(details, NULL, BMECAT_NODE_ARTICLE_UNIT, BAD_CAST code);
	}

	// ORDER_UNIT already present, let's update it if so requested 
	if (main)
	{
		xmlNodeSetContent(main_unit, (xmlChar*) code);
	}

	// ALTERNATIVE_UNIT node wasn't present for this unit code, we'll add it now
	if (alt_unit == NULL)
	{
		alt_unit = xmlNewChild(details, NULL, BMECAT_NODE_ARTICLE_ALT_UNIT, NULL);
		xmlNewTextChild(alt_unit, NULL, BMECAT_NODE_ARTICLE_UNIT_CODE, BAD_CAST code);
		xmlNewTextChild(alt_unit, NULL, BMECAT_NODE_ARTICLE_UNIT_FACTOR, BAD_CAST f);
	}
	
	// ALTERNATIVE_UNIT was present, we'll just update it
	else
	{
		xmlNodePtr unit_factor = libcatner_get_child(alt_unit, BMECAT_NODE_ARTICLE_UNIT_FACTOR, NULL, 0);
		xmlNodeSetContent(unit_factor, BAD_CAST f);
	}

	return 0;
}

/*
 * TODO update return value documentation
 *
 * Adds the given category id to the article with the given `aid` and returns 
 * the associated ARTICLE_REFERENCE node. If there is no article with the given 
 * `aid`, this function returns NULL. If the article already had the cateogry 
 * added, the function simply returns a reference to it.
 */
int catner_add_article_category(catner_state_s *cs, const char *aid, const char *cat)
{
	xmlNodePtr article = libcatner_get_article(cs->articles, BAD_CAST aid);
	if (article == NULL)
	{
		return -1;
	}

	xmlNodePtr child = article->children;
	while (child != NULL)
	{
		// We are only interested in ARTICLE_REFERENCE nodes
		if (xmlStrcmp(child->name, BAD_CAST BMECAT_NODE_ARTICLE_CATEGORY) == 0)
		{
			if (libcatner_get_child(child, BMECAT_NODE_ARTICLE_CATEGORY_ID, BAD_CAST cat, 0) != NULL)
			{
				// Already exists
				return 1;
			}
		}
		
		// No match, check next node
		child = child->next;
	}

	// No such category present yet, let's add it
	xmlNodePtr category = xmlNewChild(article, NULL, BMECAT_NODE_ARTICLE_CATEGORY, NULL);
	xmlNewTextChild(category, NULL, BMECAT_NODE_ARTICLE_CATEGORY_ID, BAD_CAST cat);

	return 0;
}

size_t catner_num_articles(catner_state_s *cs)
{
	return libcatner_num_children(cs->articles, BMECAT_NODE_ARTICLE, NULL);
}

size_t catner_num_article_features(catner_state_s *cs, const char *aid)
{
	xmlNodePtr article = libcatner_get_article(cs->articles, BAD_CAST aid);
	if (article == NULL)
	{
		return 0;
	}

	xmlNodePtr features = libcatner_get_child(article, BMECAT_NODE_FEATURES, NULL, 0);
	if (features == NULL)
	{
		return 0;
	}

	return libcatner_num_children(features, BMECAT_NODE_FEATURE, NULL);
}

size_t catner_num_article_feature_variants(catner_state_s *cs, const char *aid, const char *fid)
{
	xmlNodePtr article = libcatner_get_article(cs->articles, BAD_CAST aid);
	if (article == NULL)
	{
		return 0;
	}

	xmlNodePtr feature = libcatner_get_feature(article, BAD_CAST fid);
	if (feature == NULL)
	{
		return 0;
	}

	xmlNodePtr variants = libcatner_get_child(feature, BMECAT_NODE_VARIANTS, NULL, 0);
	if (variants == NULL)
	{
		return 0;
	}

	return libcatner_num_children(variants, BMECAT_NODE_VARIANT, NULL);
}

/*
 * TODO if feature already exists, should be return indication?
 *      should we update the feature with the given name, desc, unit, value?
 * TODO what values are/should be optional? should we use the same value
 *      for name and descr or leave it up to the user?
 */
int catner_add_article_feature(catner_state_s *cs, const char *aid, const char *fid, 
		const char *name, const char *descr, const char *unit, const char *value)
{
	// Find the ARTICLE node with the given AID
	xmlNodePtr article = libcatner_get_article(cs->articles, BAD_CAST aid);
	
	// Article doesn't exist, that's an error
	if (article == NULL)
	{
		return -1;
	}

	// Find the FEATURE node with the given FID
	xmlNodePtr feature = libcatner_get_feature(article, BAD_CAST fid);
	
	// Feature already exists, we're done
	if (feature != NULL)
	{
		return 1;
	}

	// Figure out the number of existing FEATUREs and make it a string
	size_t num_features = catner_num_article_features(cs, aid);
	char o[8];
	snprintf(o, 8, "%zu", num_features + 1);

	const char *d = descr ? descr : name;
	const char *u = unit  ? unit  : CATNER_DEF_FEATURE_UNIT;

	// Find or create ARTICLE_FEATURES node
	xmlNodePtr features = libcatner_get_child(article, BMECAT_NODE_FEATURES, NULL, 1);
	feature = xmlNewChild(features, NULL, BMECAT_NODE_FEATURE, NULL);
	xmlNewTextChild(feature, NULL, BMECAT_NODE_FEATURE_ID,    BAD_CAST fid);
	xmlNewTextChild(feature, NULL, BMECAT_NODE_FEATURE_NAME,  BAD_CAST name);
	xmlNewTextChild(feature, NULL, BMECAT_NODE_FEATURE_DESCR, BAD_CAST d);
	xmlNewTextChild(feature, NULL, BMECAT_NODE_FEATURE_UNIT,  BAD_CAST u);
	xmlNewTextChild(feature, NULL, BMECAT_NODE_FEATURE_ORDER, BAD_CAST o);

	if (value != NULL)
	{
		xmlNewTextChild(feature, NULL, BMECAT_NODE_FEATURE_VALUE, BAD_CAST value);
	}

	return 0;
}

// TODO test this
int catner_add_article_feature_variant(catner_state_s *cs, const char *aid, const char *fid,
		const char *vid, const char *value)
{
	xmlNodePtr article = libcatner_get_article(cs->articles, BAD_CAST aid);
	
	// Article doesn't exist, that's an error
	if (article == NULL)
	{
		return -1;
	}

	xmlNodePtr feature = libcatner_get_feature(article, BAD_CAST fid);

	// Feature doesn't exist, that's an error
	if (feature == NULL)
	{
		return -1;
	}

	// Find or create VARIANTS node
	xmlNodePtr variants = libcatner_get_child(feature, BMECAT_NODE_VARIANTS, NULL, 1);
	
	// Iterate over all VARIANT child nodes
	xmlNodePtr child = variants->children;
	while (child != NULL)
	{
		// We are only interested in VARIANT nodes
		if (xmlStrcmp(child->name, BMECAT_NODE_VARIANT) == 0)
		{
			// If this VARIANT has a VID and its content matches vid, we're done
			if (libcatner_get_child(child, BMECAT_NODE_VARIANT_ID, BAD_CAST vid, 0) != NULL)
			{
				return 1;
			}
		}

		child = child->next;
	}
	
	// VARIANT does not yet exist, let's create it
	xmlNodePtr variant = xmlNewChild(variants, NULL, BMECAT_NODE_VARIANT, NULL);
	xmlNewTextChild(variant, NULL, BMECAT_NODE_VARIANT_ID, BAD_CAST vid);
	xmlNewTextChild(variant, NULL, BMECAT_NODE_VARIANT_VALUE, BAD_CAST value);

	return 0;
}

int catner_write_xml(catner_state_s *cs, const char *path)
{
	return xmlSaveFormatFileEnc(path, cs->doc, CATNER_XML_ENCODING, 1);
}

int catner_print_xml(catner_state_s *cs)
{
	return xmlSaveFormatFileEnc(CATNER_STDOUT_FILE, cs->doc, CATNER_XML_ENCODING, 1);
}

/*
 * Write the document back to the file it was originally loaded from.
 */
int catner_save(catner_state_s *cs)
{
	if (cs->path == NULL)
	{
		return -1;
	}

	return catner_write_xml(cs, cs->path);
}

catner_state_s *catner_init()
{
	catner_state_s *state = malloc(sizeof(catner_state_s));
	catner_state_s empty_state = { 0 };
	*state = empty_state;

	state->doc      = xmlNewDoc(BAD_CAST CATNER_XML_VERSION);
	state->root     = libcatner_get_root(state->doc, 1);
	state->header   = libcatner_get_header(state->root, 1);
	state->articles = libcatner_get_articles(state->root, 1);
	state->catalog  = libcatner_get_catalog(state->header, 1);

	return state;
}

/*
 * TODO implement
 */
catner_state_s *catner_load(const char *path, int amend)
{
	catner_state_s *state = malloc(sizeof(catner_state_s));
	catner_state_s empty_state = { 0 };
	*state = empty_state;

	// Load the XML file
	state->doc  = xmlReadFile(path, NULL, 0);
	state->path = strdup(path);

	// Find (or possibly create) the BMECAT node
	state->root   = libcatner_get_root(state->doc, amend);
	if (state->root == NULL)
	{
		return NULL;
	}

	// Find (or possibly create) the HEADER node
	state->header = libcatner_get_header(state->root, amend);
	if (state->header == NULL)
	{
		return NULL;
	}

	// Find (or possibly create) the T_NEW_CATALOG node
	state->articles = libcatner_get_articles(state->root, amend);
	if (state->articles == NULL)
	{
		return NULL;
	}

	// Find (or possibly create) the CATALOG node
	state->catalog = libcatner_get_catalog(state->header, amend);
	if (state->catalog == NULL)
	{
		return NULL;
	}
	
	return state;
}

/*
 * TODO implement
 */
void catner_free(catner_state_s *cs)
{
	xmlFreeDoc(cs->doc);
	xmlCleanupParser();
	free(cs->path);
	free(cs);
	return;
}

int main(int argc, char **argv)
{
	LIBXML_TEST_VERSION;

	catner_state_s *cs = catner_init();
	catner_set_generator(cs, "not great"); // This should not show up
	catner_set_generator(cs, CATNER_NAME);
	catner_set_locale(cs, "DE"); // This should not show up
	catner_set_locale(cs, "EN");
	catner_add_territory(cs, "DE");
	catner_add_territory(cs, "AT");
	catner_add_territory(cs, "DE"); // This should not show up
	catner_add_article(cs, "SRTS62", "Sicherheitsroststufe ECO X12", "Total preiswerte Stufe, mach die mal jetzt rein da, komm, mach.");
	catner_add_article(cs, "SRTS63", "Sicherheitsroststufe Schlingenhorst", "Super geile Stufe, die sogar mit Schlappen zu besteigen ist!");
	catner_add_article(cs, "SRTS63", "This should not exist.", "Because SRTS63 has been added before!"); // This should not show up
	catner_add_article_image(cs, "SRTS63", "image/jpg", "images/srts63-1.jpg");
	catner_add_article_image(cs, "SRTS63", "image/jpg", "images/srts63-2.jpg");
	catner_add_article_image(cs, "SRTS63", "image/jpg", "images/srts63-2.jpg"); // This should not show up
	catner_add_article_category(cs, "SRTS63", "10010000");
	catner_add_article_category(cs, "SRTS63", "10020000");
	catner_add_article_category(cs, "SRTS63", "10020000"); // This should not show up
	catner_add_article_unit(cs, "SRTS63", "PCE", NULL, 1);
	catner_add_article_unit(cs, "SRTS63", "PCE", "1", 1);
	catner_add_article_unit(cs, "SRTS63", "MTR", "6", 1);
	catner_add_article_feature(cs, "SRTS63", "f_breite", "Breite",   "Breite (mm)", NULL, "Success");
	catner_add_article_feature(cs, "SRTS63", "f_breite", "Breite 2", "Breite (mm)", NULL, "Failure"); // This should not show up
	catner_add_article_feature(cs, "SRTS63", "f_laenge", "Laenge",   "Laenge (mm)", NULL, "Success");
	catner_add_article_feature_variant(cs, "SRTS63", "f_breite", "01", "400");
	catner_add_article_feature_variant(cs, "SRTS63", "f_breite", "01", "999"); // This should not show up
	catner_add_article_feature_variant(cs, "SRTS63", "f_laenge", "01", "1200");
	catner_add_article_feature_variant(cs, "SRTS63", "f_breite", "02", "400");
	catner_add_article_feature_variant(cs, "SRTS63", "f_laenge", "02", "1500");

	catner_print_xml(cs);

	/*
	catner_write_xml(cs, "/home/julien/workspace/catner/xml/test.xml");
	catner_state_s *cs2 = catner_load("/home/julien/workspace/catner/xml/test.xml", 0);
	catner_set_generator(cs2, "THIS HAS CHANGED, COOL!");
	catner_save(cs2);
	catner_free(cs2);
	*/

	catner_free(cs);
}

