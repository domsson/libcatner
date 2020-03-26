#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include "libcatner.h"

/*
 * Add a node with the given `name` to the given parent node. If `value` is 
 * given, a text node will be created, otherwise a regular node. 
 * Returns a reference to the newly created node. 
 */
static inline xmlNodePtr libcatner_add_child(const xmlNodePtr parent, const xmlChar *name, 
		const xmlChar *value)
{
	// Create empty (non-text) or text node, depending on value
	return value == NULL ? xmlNewChild(parent, NULL, name, NULL) :
		xmlNewTextChild(parent, NULL, name, value);
}

int libcatner_cmp_content(const xmlNodePtr node, const xmlChar *value)
{
	xmlChar *content = xmlNodeGetContent(node);
	int matches = (xmlStrcmp(content, value) == 0);
	xmlFree(content);
	return matches;
}

/*
 * Searches the parent node for the first child that matches the given `name` 
 * and, if given, text content `value`. Returns the child node found or NULL.
 * If `create` is `1`, the child node will be created if it wasn't found.
 */
xmlNodePtr libcatner_get_child(const xmlNodePtr parent, const xmlChar *name, 
		const xmlChar *value, int add)
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
		if (libcatner_cmp_content(child, value))
		{
			return child;
		}
	}

	// No matching node found - either create it or return NULL
	return add ? libcatner_add_child(parent, name, value) : NULL;
}

/*
 * Sets the content of the parent's first child node with the given name to
 * the given value, if such a node can be found, otherwise nothing is done.
 * Returns 0 on success, -1 if no suitable child node was found.
 */
int libcatner_set_child(const xmlNodePtr parent, const xmlChar *name, 
		const xmlChar *value, int add)
{
	xmlNodePtr child = libcatner_get_child(parent, name, NULL, 0);

	if (child == NULL)
	{
		// No child, but we're supposed to add it
		if (add)
		{
			child = libcatner_add_child(parent, name, value);
			return 0;
		}
		// No child and we're not supposed to add it
		return -1;
	}

	xmlNodeSetContent(child, BAD_CAST value);
	return 0;
}

/*
 * Deletes and frees the given node. 
 */
static inline int libcatner_del_node(const xmlNodePtr node)
{
	if (node == NULL)
	{
		return -1;
	}

	xmlUnlinkNode(node);
	xmlFreeNode(node);
	return 0;
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
	xmlNodePtr child = NULL; 
	for (child = parent->children; child; child = child->next)
	{
		// Node name mismatch, therefore we continue
		if (xmlStrcmp(child->name, name) != 0)
		{
			continue;
		}
		
		// No node content value given, so we count this
		if (value == NULL)
		{
			++num;
		}

		// Node content value given, count if it matches
		else if (libcatner_cmp_content(child, value))
		{
			++num;
		}
	}
	return num;
}

/*
 * Searches parent for child nodes of the given name and returns the `n`th 
 * of those (0-based, so `1` would find the second), if there are that many. 
 * Otherwise returns NULL.
 */
xmlNodePtr libcatner_get_child_at(const xmlNodePtr parent, const xmlChar *name, 
		size_t n)
{
	// We count how many matches we've found (0-based) 
	size_t cur = 0;

	// Iterate all children of parent and check if they match `name`
	xmlNodePtr child = NULL;
	for (child = parent->children; child; child = child->next)
	{
		if (xmlStrcmp(child->name, BAD_CAST name) != 0)
		{
			continue;
		}

		// Is this the `n`th item of this type we've found?
		if (cur == n)
		{
			return child;
		}

		// Otherwise, continue to search
		++cur;
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
static inline xmlNodePtr libcatner_get_header(const xmlNodePtr root, int create)
{
	return libcatner_get_child(root, BMECAT_NODE_HEADER, NULL, create);
}

/*
 * Returns the node containing all ARTICLE nodes ("T_NEW_CATALOG").
 * If create is `1`, the articles node will be created if it doesn't exist yet.
 */
static inline xmlNodePtr libcatner_get_articles(const xmlNodePtr root, int create)
{
	return libcatner_get_child(root, BMECAT_NODE_ARTICLES, NULL, create);
}

/*
 * Returns the CATALOG node within the given node, if present, otherwise NULL.
 * If create is `1`, the catalog node will be created if it doesn't exist yet.
 */
static inline xmlNodePtr libcatner_get_catalog(xmlNodePtr header, int create)
{
	return libcatner_get_child(header, BMECAT_NODE_CATALOG, NULL, create);
}

/*
 * Returns the GENERATOR_INFO node from the HEADER, if present, otherwise NULL.
 * If create is `1`, the generator node will be created if it didn't exist yet.
 */
static inline xmlNodePtr libcatner_get_generator(xmlNodePtr header, int create)
{
	return libcatner_get_child(header, BMECAT_NODE_GENERATOR, BAD_CAST "", create);
}

/*
 * Given the T_NEW_CATALOG parent node, searches all ARTICLE child nodes for 
 * one that has a matching article ID (SUPPLIER_ID) and returns it. If no 
 * matching article node could be found, NULL will be returned.
 */
xmlNodePtr libcatner_get_article(const xmlNodePtr articles, const xmlChar *aid)
{
	// Iterate all articles
	xmlNodePtr article = NULL;
	for (article = articles->children; article; article = article->next)
	{
		// Find the article's SUPPLIER_AID element with the given aid value
		if (libcatner_get_child(article, BMECAT_NODE_ARTICLE_ID, aid, 0))
		{
			// If found, return this article
			return article;
		}
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

	xmlNodePtr child = NULL;
	for (child = features->children; child; child = child->next)
	{
		// We are only interested in FEATURE nodes
		if (xmlStrcmp(child->name, BMECAT_NODE_FEATURE) != 0)
		{
			continue;
		}
		
		// If this FEATURE has a FID and its content matches fid, we're done
		if (libcatner_get_child(child, BMECAT_NODE_FEATURE_ID, fid, 0))
		{
			return child;
		}
	}

	// No matching node found
	return NULL;
}

xmlNodePtr libcatner_get_variant(const xmlNodePtr feature, const xmlChar *vid)
{
	// Find the VARIANTS node, which holds all variants
	xmlNodePtr variants = libcatner_get_child(feature, BMECAT_NODE_VARIANTS, NULL, 0);

	// If VARIANTS doesn't exist, then no variants can exists
	if (variants == NULL)
	{
		return NULL;
	}

	xmlNodePtr child = NULL;
	for (child = variants->children; child; child = child->next)
	{
		// We are only interested in VARIANT nodes
		if (xmlStrcmp(child->name, BMECAT_NODE_VARIANT) != 0)
		{
			continue;
		}

		// If this VARIANT has a SUPPLIED_AID_SUPPLEMENT and its content matches VID, we're done
		if (libcatner_get_child(child, BMECAT_NODE_VARIANT_ID, vid, 0))
		{
			return child;
		}
	}

	// No matching node found
	return NULL;
}

/*
 * Extract the content from the given node and copy it into the given buffer. 
 * If the buffer isn't big enough to hold the content, truncation will happen. 
 * Returns the number of bytes (including the terminating null-byte), that was 
 * or would've been required to copy the entire content, or 0 on error.
 */
size_t libcatner_cpy_content(xmlNodePtr node, char *buf, size_t len)
{
	// Return empty buffer if node is NULL 
	if (node == NULL)
	{
		buf[0] = '\0';
		return 0;
	}

	// Fetch the content string
	xmlChar *content = xmlNodeGetContent(node);
	
	// Return empty buffer if node has no content
	if (content == NULL)
	{
		buf[0] = '\0';
		return 0;
	}

	// Figure out the length of the string (add one for 0-terminator)
	int content_len = xmlStrlen(content) + 1;

	// Figure out the max length we can copy
	size_t copy_len = content_len > len ? len : content_len;

	// Copy everything over (that fits in the buffer)
	strncpy(buf, (char *) content, copy_len);

	// Make sure the buffer is 0-terminated
	buf[copy_len - 1] = '\0';

	// Free the content string we received from libxml
	xmlFree(content);

	// Return the buffer size that was needed to copy everything (including 
	// the terminating null-byte) which might be less than what was used in 
	// case the provided buffer wasn't sufficient
	return content_len;
}

size_t libcatner_num_features(xmlNodePtr article)
{
	xmlNodePtr features = libcatner_get_child(article, BMECAT_NODE_FEATURES, NULL, 0);
	if (features == NULL)
	{
		return 0;
	}

	return libcatner_num_children(features, BMECAT_NODE_FEATURE, NULL);
}

size_t libcatner_num_variants(xmlNodePtr feature)
{
	xmlNodePtr variants = libcatner_get_child(feature, BMECAT_NODE_VARIANTS, NULL, 0);
	if (variants == NULL)
	{
		return 0;
	}

	return libcatner_num_children(variants, BMECAT_NODE_VARIANT, NULL);
}

/*
 * TODO documentation
 */
int libcatner_fix_feature_order(xmlNodePtr article)
{
	// Find the ARTICLE_FEATUERS node containing all features
	xmlNodePtr features = libcatner_get_child(article, BMECAT_NODE_FEATURES, NULL, 0);

	// Article has no features yet
	if (features == NULL)
	{
		return -1;
	}

	xmlNodePtr feature = libcatner_get_child(features, BMECAT_NODE_FEATURE, NULL, 0);
	char order[8];
	order[0] = '\0';

	for (size_t i = 1; feature; feature = libcatner_next_node(feature))
	{
		snprintf(order, 8, "%zu", i);
		// Update the node (create it if it didn't exist yet)
		libcatner_set_child(feature, BMECAT_NODE_FEATURE_ORDER, BAD_CAST order, 1);
		++i;
	}
	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  PUBLIC API                                                               //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//
// ADD
//

/*
 * Add the GENERATOR_INFO node and set it to the given `value`.
 * If the node already exists, it will not be changed and -1 is returned.
 * Otherwise, the node will be created and the function returns 0.
 */
int catner_add_generator(catner_state_s *cs, const char *value)
{
	if (cs->generator)
	{
		// Already exists
		cs->error = LIBCATNER_ERR_ALREADY_EXISTS;
		return -1;
	}

	libcatner_add_child(cs->header, BMECAT_NODE_GENERATOR, BAD_CAST value);
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
		cs->error = LIBCATNER_ERR_INVALID_VALUE;
		return -1;
	}

	// Find or create the TERRITORY node with the given value
	xmlNodePtr t = libcatner_get_child(cs->catalog, BMECAT_NODE_TERRITORY, BAD_CAST value, 1);
	
	// Couldn't find nor create the TERRITORY node, no idea why
	if (t == NULL)
	{
		cs->error = LIBCATNER_ERR_OTHER;
		return -1;
	}
	
	return 0;
}

/*
 * Add a new article with the given ID (SUPPLIER_AID), title and description.
 * If an article with the given ID already exists, this function returns -1.
 * Otherwise, the article will be created and the function returns 0.
 * On error (for example, aid is the empty string), -1 will be returned.
 */
int catner_add_article(catner_state_s *cs, const char *aid, const char *title, const char *descr)
{
	if (xmlStrlen(BAD_CAST aid) == 0)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_AID;
		return -1;
	}

	// Check if an article with the given AID already exists
	if (libcatner_get_article(cs->articles, BAD_CAST aid) != NULL)
	{
		cs->error = LIBCATNER_ERR_ALREADY_EXISTS;
		return -1;
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

/*
 * TODO documentation
 */
int catner_add_article_image(catner_state_s *cs, const char *aid, const char *mime, const char *path)
{
	xmlNodePtr article = aid ? libcatner_get_article(cs->articles, BAD_CAST aid) :
		cs->_curr_article;

	if (article == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_AID;
		return -1;
	}
	
	// Find or create the MIME_INFO (image container) node for this article
	xmlNodePtr images = libcatner_get_child(article, BMECAT_NODE_ARTICLE_IMAGES, NULL, 1);

	// See if there is already an image with that path present
	xmlNodePtr image = NULL;
	for (image = images->children; image; image = image->next)
	{
		// Check if this MIME node has a MIME_SOURCE node with the given `path` value
		if (libcatner_get_child(image, BMECAT_NODE_ARTICLE_IMAGE_PATH, BAD_CAST path, 0))
		{
			// If so, this image already exists, we're done
			cs->error = LIBCATNER_ERR_ALREADY_EXISTS;
			return -1;
		}
	}

	// No such image present yet, let's create and return it
	image = xmlNewChild(images, NULL, BMECAT_NODE_ARTICLE_IMAGE, NULL);
	// TODO use xmlNewChild() or xmlNewTextChild()? 
	xmlNewTextChild(image, NULL, BMECAT_NODE_ARTICLE_IMAGE_MIME, BAD_CAST mime);
	xmlNewTextChild(image, NULL, BMECAT_NODE_ARTICLE_IMAGE_PATH, BAD_CAST path);

	return 0;
}

/*
 * Adds a new alternative unit to the given article. If the article didn't have 
 * a main unit set before, this unit will also be set as such. If the unit code 
 * or factor aren't given, sensible defaults ("PCE" and "1") will be used. When 
 * `main` is `1`, the current main unit (if any) will be updated with this one.
 *
 * TODO - it doesn't technically make much sense to have a main unit that has 
 *        a factor other than "1" ("1.0", "1.00", ...), let's handle that
 *      - we should consider taking the factor as a double, then converting it
 *      - currently, calling this function with a unit CODE that already exists,
 *        it will override the factor for that unit; instead, it should return 
 *        -1 and set error to LIBCATNER_ERR_ALREADY_EXISTS; however, before we
 *        change this, we should write catner_set_article_unit() so that there
 *        is a way to update an existing unit...
 */
int catner_add_article_unit(catner_state_s *cs, const char *aid, 
		const char *code, const char *factor, int main)
{
	xmlNodePtr article = aid ? libcatner_get_article(cs->articles, BAD_CAST aid) :
		cs->_curr_article;

	if (article == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_AID;
		return -1;
	}

	// Construct unit factor string based on user input and default value
	const char *c = code   ? code   : LIBCATNER_DEF_UNIT_CODE;
	const char *f = factor ? factor : LIBCATNER_DEF_UNIT_FACTOR;

	// Find the ARTICLE_ORDER_DETAILS node, which holds all units
	xmlNodePtr details = libcatner_get_child(article, BMECAT_NODE_ARTICLE_UNITS, NULL, 1);

	// Iterate ARTICLE_ORDER_DETAILS' children to find ALTERNATIVE_UNIT nodes
	xmlNodePtr alt_unit = NULL;
	xmlNodePtr child = details->children;
	for (child = details->children; child; child = child->next)
	{
		// We're only interested in ALTERNATIVE_UNIT nodes
		if (xmlStrcmp(child->name, BMECAT_NODE_ARTICLE_ALT_UNIT) != 0)
		{
			continue;
		}
		
		// Check if this ALTERNATIVE_UNIT has the unit code we're looking for
		if (libcatner_get_child(child, BMECAT_NODE_ARTICLE_UNIT_CODE, BAD_CAST c, 0))
		{
			// If so, remember this node and stop iterating
			alt_unit = child;
			break;
		}
	}

	xmlNodePtr main_unit = libcatner_get_child(details, BMECAT_NODE_ARTICLE_MAIN_UNIT, NULL, 0);

	// No ORDER_UNIT (main unit) present yet, let's add it
	if (main_unit == NULL)
	{
		main_unit = xmlNewTextChild(details, NULL, BMECAT_NODE_ARTICLE_MAIN_UNIT, BAD_CAST c);
	}

	// ORDER_UNIT (main unit) present; let's update the main unit, if so requested 
	else if (main)
	{
		xmlNodeSetContent(main_unit, BAD_CAST c);
	}

	// ALTERNATIVE_UNIT node wasn't present for this unit code, we'll add it now
	if (alt_unit == NULL)
	{
		alt_unit = xmlNewChild(details, NULL, BMECAT_NODE_ARTICLE_ALT_UNIT, NULL);
		xmlNewTextChild(alt_unit, NULL, BMECAT_NODE_ARTICLE_UNIT_CODE,   BAD_CAST c);
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
 * Adds the given category ID to the article with the ID `aid` and returns 0.
 * If there is no article with the given `aid` or if the article already has 
 * the given category associated with it, this function returns -1.
 */
int catner_add_article_category(catner_state_s *cs, const char *aid, const char *value)
{
	xmlNodePtr article = aid ? libcatner_get_article(cs->articles, BAD_CAST aid) :
		cs->_curr_article;

	if (article == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_AID;
		return -1;
	}

	xmlNodePtr child = NULL;
	for (child = article->children; child; child = child->next)
	{
		// We are only interested in ARTICLE_REFERENCE nodes
		if (xmlStrcmp(child->name, BMECAT_NODE_ARTICLE_CATEGORY) != 0)
		{
			continue;
		}
	
		if (libcatner_get_child(child, BMECAT_NODE_ARTICLE_CATEGORY_ID, BAD_CAST value, 0))
		{
			// Already exists
			cs->error = LIBCATNER_ERR_ALREADY_EXISTS;
			return -1;
		}
	}

	// No such category present yet, let's add it
	xmlNodePtr cat = xmlNewChild(article, NULL, BMECAT_NODE_ARTICLE_CATEGORY, NULL);
	xmlNewTextChild(cat, NULL, BMECAT_NODE_ARTICLE_CATEGORY_ID, BAD_CAST value);

	return 0;
}

/*
 * TODO documentation
 */
int catner_add_feature(catner_state_s *cs, const char *aid, const char *fid, 
		const char *name, const char *descr, const char *unit, const char *value)
{
	// Find the ARTICLE node with the given AID
	xmlNodePtr article = aid ? libcatner_get_article(cs->articles, BAD_CAST aid) : 
		cs->_curr_article;
	
	// Article doesn't exist, that's an error
	if (article == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_AID;
		return -1;
	}

	// See if a FEATURE node with the given FID already exists
	xmlNodePtr feature = libcatner_get_feature(article, BAD_CAST fid);
	
	// Feature already exists, we're done
	if (feature != NULL)
	{
		cs->error = LIBCATNER_ERR_ALREADY_EXISTS;
		return -1;
	}

	// Figure out the number of existing FEATUREs and make it a string
	size_t num_features = libcatner_num_features(article);
	char order[8];
	snprintf(order, 8, "%zu", num_features + 1);

	xmlNodePtr features = libcatner_get_child(article, BMECAT_NODE_FEATURES, NULL, 1);
	feature = xmlNewChild(features, NULL, BMECAT_NODE_FEATURE, NULL);
	xmlNewTextChild(feature, NULL, BMECAT_NODE_FEATURE_ID, BAD_CAST fid);
	xmlNewTextChild(feature, NULL, BMECAT_NODE_FEATURE_ORDER, BAD_CAST order);

	if (name)
		xmlNewTextChild(feature, NULL, BMECAT_NODE_FEATURE_NAME,  BAD_CAST name);
	
	if (descr)
		xmlNewTextChild(feature, NULL, BMECAT_NODE_FEATURE_DESCR, BAD_CAST descr);
	
	if (unit)
		xmlNewTextChild(feature, NULL, BMECAT_NODE_FEATURE_UNIT,  BAD_CAST unit);
	
	if (value)
		xmlNewTextChild(feature, NULL, BMECAT_NODE_FEATURE_VALUE, BAD_CAST value);

	return 0;
}

/*
 * TODO - add documentation
 *      - is it a problem that catner_add_feature() will add the FORDER node?
 */
int catner_add_weight_feature(catner_state_s *cs, const char *aid, const char *value)
{
	return catner_add_feature(cs, aid, LIBCATNER_FEATURE_WEIGHT, 
			LIBCATNER_FEATURE_WEIGHT, NULL, NULL, NULL);
}

/*
 * TODO - documentation
 */
int catner_add_variant(catner_state_s *cs, const char *aid, 
		const char *fid, const char *vid, const char *value)
{
	xmlNodePtr article = aid ? libcatner_get_article(cs->articles, BAD_CAST aid) :
		cs->_curr_article;
	
	// Article doesn't exist, that's an error
	if (article == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_AID;
		return -1;
	}

	xmlNodePtr feature = fid ? libcatner_get_feature(article, BAD_CAST fid) :
		cs->_curr_feature;

	// Feature doesn't exist, that's an error
	if (feature == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_FID;
		return -1;
	}

	// Find or create VARIANTS node
	xmlNodePtr variants = libcatner_get_child(feature, BMECAT_NODE_VARIANTS, NULL, 1);
	
	// Iterate over all VARIANT child nodes
	xmlNodePtr child = NULL;
	for (child = variants->children; child; child = child->next)
	{
		// We are only interested in VARIANT nodes
		if (xmlStrcmp(child->name, BMECAT_NODE_VARIANT) != 0)
		{
			continue;
		}
	
		// If this VARIANT has a VID and its content matches vid, we're done
		if (libcatner_get_child(child, BMECAT_NODE_VARIANT_ID, BAD_CAST vid, 0))
		{
			cs->error = LIBCATNER_ERR_ALREADY_EXISTS;
			return -1;
		}
	}

	// Features with variants should not have a FVALUE node themselves
	xmlNodePtr fvalue = libcatner_get_child(feature, BMECAT_NODE_FEATURE_VALUE, NULL, 0);
	if (fvalue)
	{
		// ... so if there is one, we'll remove it
		libcatner_del_node(fvalue);
	}
	
	// VARIANT does not yet exist, let's create it
	xmlNodePtr variant = xmlNewChild(variants, NULL, BMECAT_NODE_VARIANT, NULL);
	xmlNewTextChild(variant, NULL, BMECAT_NODE_VARIANT_ID,    BAD_CAST vid);
	xmlNewTextChild(variant, NULL, BMECAT_NODE_VARIANT_VALUE, BAD_CAST value);

	return 0;
}

int catner_add_weight_variant(catner_state_s *cs, const char *aid, const char *vid, const char *value)
{
	return catner_add_variant(cs, aid, LIBCATNER_FEATURE_WEIGHT, vid, value);
}

//
// SET
// 

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
		cs->error = LIBCATNER_ERR_INVALID_VALUE;
		return -1;
	}

	libcatner_set_child(cs->catalog, BMECAT_NODE_LOCALE, BAD_CAST value, 1);
	return 0;
}

/*
 * Set the GENERATOR_INFO node to the given value. If the node didn't exist
 * yet, it will be created.
 *
 * This node is optional and gives information on the software that 
 * was used to generate the BMEcat file.
 */
int catner_set_generator(catner_state_s *cs, const char *value)
{
	if (cs->generator == NULL)
	{
		cs->generator = libcatner_add_child(cs->header, BMECAT_NODE_GENERATOR, BAD_CAST value);
		return 0;
	}

	xmlNodeSetContent(cs->generator, BAD_CAST value);
	return 0;
}

int catner_set_article_id(catner_state_s *cs, const char *aid, const char *value)
{
	if (xmlStrlen(BAD_CAST value) == 0)
	{
		cs->error = LIBCATNER_ERR_INVALID_VALUE;
		return -1;
	}

	xmlNodePtr article = aid ? libcatner_get_article(cs->articles, BAD_CAST aid) :
		cs->_curr_article;

	if (article == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_AID;
		return -1;
	}

	return libcatner_set_child(article, BMECAT_NODE_ARTICLE_ID, BAD_CAST value, 0);
}

/*
 * Sets the title (DESCRIPTION_SHORT) of the article with the given AID. 
 * Returns -1 if no matching article could be found, otherwise 0.
 */
int catner_set_article_title(catner_state_s *cs, const char *aid, const char *value)
{
	xmlNodePtr article = aid ? libcatner_get_article(cs->articles, BAD_CAST aid) :
		cs->_curr_article;

	if (article == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_AID;
		return -1;
	}

	// Find or create the ARTICLE_DETAILS node within this ARTICLE
	xmlNodePtr details = libcatner_get_child(article, BMECAT_NODE_ARTICLE_DETAILS, NULL, 1);

	// Find or create the DESCRIPTION_SHORT node within ARTICLE_DETAILS
	xmlNodePtr title = libcatner_get_child(details, BMECAT_NODE_ARTICLE_TITLE, NULL, 1);
	
	// Set the text of the title node accordingly
	xmlNodeSetContent(title, BAD_CAST value);
	return 0;
}

/*
 * Sets the description (DESCRIPTION_LONG) of the article with the given AID. 
 * Returns -1 if no matching article could be found, otherwise 0.
 */
int catner_set_article_descr(catner_state_s *cs, const char *aid, const char *value)
{
	xmlNodePtr article = aid ? libcatner_get_article(cs->articles, BAD_CAST aid) 
		: cs->_curr_article;

	if (article == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_AID;
		return -1;
	}

	// Find or create the ARTICLE_DETAILS node within this ARTICLE
	xmlNodePtr details = libcatner_get_child(article, BMECAT_NODE_ARTICLE_DETAILS, NULL, 1);

	// Find or create the DESCRIPTION_SHORT node within ARTICLE_DETAILS
	xmlNodePtr descr = libcatner_get_child(details, BMECAT_NODE_ARTICLE_DESCR, NULL, 1);
	
	// Set the text of the title node accordingly
	xmlNodeSetContent(descr, BAD_CAST value);
	return 0;
}

int catner_set_feature_prop(catner_state_s *cs, const char *aid, const char *fid, 
		const char *prop, const char *value, int add)
{
	xmlNodePtr article = aid ? libcatner_get_article(cs->articles, BAD_CAST aid) 
		: cs->_curr_article;

	if (article == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_AID;
		return -1;
	}

	xmlNodePtr feature = fid ? libcatner_get_feature(article, BAD_CAST fid) :
		cs->_curr_feature;

	if (feature == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_FID;
		return -1;
	}

	return libcatner_set_child(feature, BAD_CAST prop, BAD_CAST value, add);
}

int catner_set_feature_id(catner_state_s *cs, const char *aid, const char *fid, const char *value)
{
	const char* prop = (char *) BMECAT_NODE_FEATURE_ID;
	return catner_set_feature_prop(cs, aid, fid, prop, value, 0);
}

int catner_set_feature_name(catner_state_s *cs, const char *aid, const char *fid, const char *value)
{
	const char* prop = (char *) BMECAT_NODE_FEATURE_NAME;
	return catner_set_feature_prop(cs, aid, fid, prop, value, 1);
}

int catner_set_feature_descr(catner_state_s *cs, const char *aid, const char *fid, const char *value)
{
	const char* prop = (char *) BMECAT_NODE_FEATURE_DESCR;
	return catner_set_feature_prop(cs, aid, fid, prop, value, 1);
}

// TODO there might not be a FVALUE element yet! If so, we have to create it
int catner_set_feature_value(catner_state_s *cs, const char *aid, const char *fid, const char *value)
{
	const char* prop = (char *) BMECAT_NODE_FEATURE_VALUE;
	return catner_set_feature_prop(cs, aid, fid, prop, value, 1);
}

int catner_set_feature_unit(catner_state_s *cs, const char *aid, const char *fid, const char *value)
{
	const char *v = xmlStrlen(BAD_CAST value) ? value : LIBCATNER_DEF_FEATURE_UNIT;
	const char* prop = (char *) BMECAT_NODE_FEATURE_UNIT;
	return catner_set_feature_prop(cs, aid, fid, prop, v, 1);
}

int catner_set_variant_value(catner_state_s *cs, const char *aid, const char *fid, const char *vid, const char *value)
{
	xmlNodePtr article = aid ? libcatner_get_article(cs->articles, BAD_CAST aid) : 
		cs->_curr_article;

	if (article == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_AID;
		return -1;
	}

	xmlNodePtr feature = fid ? libcatner_get_feature(article, BAD_CAST fid) :
		cs->_curr_feature;

	if (feature == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_FID;
		return -1;
	}

	xmlNodePtr variant = vid ? libcatner_get_variant(feature, BAD_CAST vid) : 
		cs->_curr_variant;

	if (variant == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_VID;
		return -1;
	}

	return libcatner_set_child(variant, BMECAT_NODE_VARIANT_VALUE, BAD_CAST value, 0);
}

int catner_set_weight_variant(catner_state_s *cs, const char *aid, const char *vid, const char *value)
{
	return catner_set_variant_value(cs, aid, LIBCATNER_FEATURE_WEIGHT, vid, value);
}

//
// GET
//

size_t catner_get_locale(catner_state_s *cs, char *buf, size_t len)
{
	xmlNodePtr locale = libcatner_get_child(cs->catalog, BMECAT_NODE_LOCALE, NULL, 0);
	return libcatner_cpy_content(locale, buf, len);
}

size_t catner_get_generator(catner_state_s *cs, char *buf, size_t len)
{
	return libcatner_cpy_content(cs->generator, buf, len);
}

size_t catner_get_article_aid(catner_state_s *cs, char *buf, size_t len)
{
	if (cs->_curr_article == NULL)
	{
		return libcatner_cpy_content(NULL, buf, len);
	}

	xmlNodePtr aid = libcatner_get_child(cs->_curr_article, BMECAT_NODE_ARTICLE_ID, NULL, 0);
	return libcatner_cpy_content(aid, buf, len);
}

size_t catner_get_article_title(catner_state_s *cs, const char *aid, char *buf, size_t len)
{
	xmlNodePtr article = aid ? libcatner_get_article(cs->articles, BAD_CAST aid) : 
		cs->_curr_article;

	if (article == NULL)
	{
		return libcatner_cpy_content(NULL, buf, len);
	}

	xmlNodePtr title = libcatner_get_child(article, BMECAT_NODE_ARTICLE_TITLE, NULL, 0);
	return libcatner_cpy_content(title, buf, len);
}

size_t catner_get_article_descr(catner_state_s *cs, const char *aid, char *buf, size_t len)
{
	xmlNodePtr article = aid ? libcatner_get_article(cs->articles, BAD_CAST aid) :
		cs->_curr_article;

	if (article == NULL)
	{
		return libcatner_cpy_content(NULL, buf, len);
	}

	xmlNodePtr descr = libcatner_get_child(article, BMECAT_NODE_ARTICLE_DESCR, NULL, 0);
	return libcatner_cpy_content(descr, buf, len);
}

size_t catner_get_territories(catner_state_s *cs, char *buf, size_t len)
{
	buf[0] = '\0';

	const char *comma = ",";
	size_t cur_len = 0;
	size_t req_len = 0;
	xmlNodePtr t = libcatner_get_child(cs->catalog, BMECAT_NODE_TERRITORY, NULL, 0);

	// TODO xmlFree(t_str)
	for (; t; t = libcatner_next_node(t))
	{
		char *t_str = (char *) xmlNodeGetContent(t);
		size_t t_len = strlen(t_str);

		req_len += (t_len + (cur_len != 0));
		if ((req_len + 1) > len)
		{
			continue;
		}
		
		if (cur_len)
		{
			strncat(buf, comma, 1);
		}
		
		strncat(buf, t_str, t_len);
		cur_len = req_len;
	}

	buf[cur_len] = '\0';
	return req_len + 1;
}

size_t catner_get_article_categories(catner_state_s *cs, const char *aid, char *buf, size_t len)
{
	// Make sure buf passes as an empty, 0-terminated string
	buf[0] = '\0';

	xmlNodePtr article = aid ? libcatner_get_article(cs->articles, BAD_CAST aid) :
		cs->_curr_article;
	
	if (article == NULL)
	{
		return 0;
	}

	// Iterate all categories, concat each to the buffer
	const char *comma = ",";
	size_t cur_len = 0;
	size_t req_len = 0;
	xmlNodePtr unit = libcatner_get_child(article, BMECAT_NODE_ARTICLE_CATEGORY, NULL, 0);

	// TODO xmlFree(unit_id_str)
	for (; unit; unit = libcatner_next_node(unit))
	{
		// Get the inner CATALOG_ID node that actually holds the category
		xmlNodePtr unit_id = libcatner_get_child(unit, BMECAT_NODE_ARTICLE_CATEGORY_ID, NULL, 0);
		if (unit_id == NULL)
		{
			continue;
		}
		
		// Extract the actual category ID string
		char *unit_id_str = (char *) xmlNodeGetContent(unit_id);

		// Get the length of the current ID (should be 8, but let's check)
		size_t id_len = strlen(unit_id_str);

		// Update the required buffer lenght (in case it ain't big enough)
		req_len += (id_len + (cur_len != 0));

		// We're not concating if we'd be exeeding the buffer
		if ((req_len + 1) > len)
		{
			// We'll still continue though in order to sum up the required length
			continue;
		}
		
		// Maybe add a comma
		if (cur_len)
		{
			strncat(buf, comma, 1);
		}
		// Definitely add the category ID
		strncat(buf, unit_id_str, id_len);

		// Update the number of characters written (not including '\0')
		cur_len = req_len;
	}
	
	buf[cur_len] = '\0'; // Make sure we're properly terminated
	return req_len + 1; // Include required length (including '\0')
}

size_t catner_get_sel_article_id(catner_state_s *cs, char *buf, size_t len)
{
	if (cs->_curr_article == NULL)
	{
		return libcatner_cpy_content(NULL, buf, len);
	}

	xmlNodePtr aid = libcatner_get_child(cs->_curr_article, BMECAT_NODE_ARTICLE_ID, NULL, 0);
	return libcatner_cpy_content(aid, buf, len);
}

size_t catner_get_sel_feature_id(catner_state_s *cs, char *buf, size_t len)
{
	if (cs->_curr_feature == NULL)
	{
		return libcatner_cpy_content(NULL, buf, len);
	}

	xmlNodePtr fid = libcatner_get_child(cs->_curr_feature, BMECAT_NODE_FEATURE_ID, NULL, 0);
	return libcatner_cpy_content(fid, buf, len);
}

size_t catner_get_sel_variant_id(catner_state_s *cs, char *buf, size_t len)
{
	if (cs->_curr_variant == NULL)
	{
		return libcatner_cpy_content(NULL, buf, len);
	}

	xmlNodePtr vid = libcatner_get_child(cs->_curr_variant, BMECAT_NODE_VARIANT_ID, NULL, 0);
	return libcatner_cpy_content(vid, buf, len);
}

//
// DEL
// 

/*
 * TODO documentation
 */
int catner_del_generator(catner_state_s *cs)
{
	libcatner_del_node(cs->generator);
	return 0;
}

/*
 * TODO documentation
 */
int catner_del_territory(catner_state_s *cs, const char *value)
{
	xmlNodePtr territory = libcatner_get_child(cs->catalog, BMECAT_NODE_TERRITORY, BAD_CAST value, 0);
	if (territory == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_NODE;
		return -1;
	}

	libcatner_del_node(territory);
	return 0;
}

/*
 * TODO documentation
 */
int catner_del_article(catner_state_s *cs, const char *aid)
{
	xmlNodePtr article = aid ? libcatner_get_article(cs->articles, BAD_CAST aid) :
		cs->_curr_article;

	if (article == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_AID;
		return -1;
	}

	// Are we about to delete the currently selected article?
	if (cs->_curr_article == article)
	{
		cs->_curr_article = NULL;
		cs->_curr_feature = NULL;
		cs->_curr_variant = NULL;
		cs->_curr_image   = NULL;
	}

	libcatner_del_node(article);
	return 0;
}

/*
 * TODO documentation
 */
int catner_del_article_category(catner_state_s *cs, const char *aid, const char *cid)
{
	xmlNodePtr article = aid ? libcatner_get_article(cs->articles, BAD_CAST aid) :
		cs->_curr_article;

	if (article == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_AID;
		return -1;
	}

	xmlNodePtr cat = libcatner_get_child(article, BMECAT_NODE_ARTICLE_CATEGORY, NULL, 0);

	for (; cat; cat = libcatner_next_node(cat))
	{
		xmlNodePtr cat_id = libcatner_get_child(cat, BMECAT_NODE_ARTICLE_CATEGORY_ID, BAD_CAST cid, 0);
		if (cat_id == NULL)
		{
			continue;
		}

		libcatner_del_node(cat);
		return 0;
	}
	cs->error = LIBCATNER_ERR_NO_SUCH_NODE;
	return -1;
}

/*
 * TODO documentation
 */
int catner_del_article_image(catner_state_s *cs, const char *aid, const char *path)
{
	xmlNodePtr article = aid ? libcatner_get_article(cs->articles, BAD_CAST aid) :
		cs->_curr_article;

	if (article == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_AID;
		return -1;
	}

	xmlNodePtr images = libcatner_get_child(article, BMECAT_NODE_ARTICLE_IMAGES, NULL, 0);

	if (images == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_NODE;
		return -1;
	}

	xmlNodePtr image = libcatner_get_child(images, BMECAT_NODE_ARTICLE_IMAGE_PATH, BAD_CAST path, 0);

	// Are we about to delete the currently selected image?
	if (cs->_curr_image == image)
	{
		cs->_curr_image = NULL;
	}

	libcatner_del_node(image);
	return 0;
}

/*
 * TODO documentation
 */
int catner_del_feature(catner_state_s *cs, const char *aid, 
		const char *fid)
{
	xmlNodePtr article = aid ? libcatner_get_article(cs->articles, BAD_CAST aid) :
		cs->_curr_article;

	if (article == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_AID;
		return -1;
	}

	xmlNodePtr feature = fid ? libcatner_get_feature(article, BAD_CAST fid) :
		cs->_curr_feature;

	if (feature == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_FID;
		return -1;
	}

	// Are we about to delete the currently selected feature?
	if (cs->_curr_feature == feature)
	{
		cs->_curr_feature = NULL;
		cs->_curr_variant = NULL;
	}

	libcatner_del_node(feature);
	libcatner_fix_feature_order(article);
	return 0;
}

/*
 * TODO documentation
 */
int catner_del_weight_feature(catner_state_s *cs, const char *aid)
{
	return catner_del_feature(cs, aid, LIBCATNER_FEATURE_WEIGHT);
}

/*
 * TODO documentation
 */
int catner_del_variant(catner_state_s *cs, const char *aid, 
		const char *fid, const char *vid)
{
	xmlNodePtr article = aid ? libcatner_get_article(cs->articles, BAD_CAST aid) :
		cs->_curr_article;

	if (article == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_AID;
		return -1;
	}

	xmlNodePtr feature = fid ? libcatner_get_feature(article, BAD_CAST fid) :
		cs->_curr_feature;

	if (feature == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_FID;
		return -1;
	}

	xmlNodePtr variant = vid ? libcatner_get_variant(feature, BAD_CAST vid) : 
		cs->_curr_variant;

	if (variant == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_VID;
		return -1;
	}

	// Are we about to delete the currently selected variant?
	if (cs->_curr_variant == variant)
	{
		cs->_curr_variant = NULL;
	}

	libcatner_del_node(variant);
	return 0;
}

/*
 * TODO documentation
 */
int catner_del_weight_variant(catner_state_s *cs, const char *aid, const char *vid)
{
	return catner_del_variant(cs, aid, LIBCATNER_FEATURE_WEIGHT, vid);
}

//
// NUM
//

size_t catner_num_territories(catner_state_s *cs)
{
	return libcatner_num_children(cs->catalog, BMECAT_NODE_TERRITORY, NULL);
}

size_t catner_num_articles(catner_state_s *cs)
{
	return libcatner_num_children(cs->articles, BMECAT_NODE_ARTICLE, NULL);
}

size_t catner_num_article_categories(catner_state_s *cs, const char *aid)
{
	xmlNodePtr article = aid ? libcatner_get_article(cs->articles, BAD_CAST aid) :
		cs->_curr_article;

	if (article == NULL)
	{
		return 0;
	}

	return libcatner_num_children(article, BMECAT_NODE_ARTICLE_CATEGORY, NULL);
}

size_t catner_num_features(catner_state_s *cs, const char *aid)
{
	xmlNodePtr article = aid ? libcatner_get_article(cs->articles, BAD_CAST aid) :
		cs->_curr_article;

	if (article == NULL)
	{
		return 0;
	}

	return libcatner_num_features(article);
}

size_t catner_num_variants(catner_state_s *cs, const char *aid, const char *fid)
{
	xmlNodePtr article = aid ? libcatner_get_article(cs->articles, BAD_CAST aid) :
		cs->_curr_article;

	if (article == NULL)
	{
		return 0;
	}

	xmlNodePtr feature = fid ? libcatner_get_feature(article, BAD_CAST fid) :
		cs->_curr_feature;

	if (feature == NULL)
	{
		return 0;
	}

	return libcatner_num_variants(feature);
}

//
// SEL
//

/*
 * Set the currently selected article to the one identified by the given
 * `aid`. If the article was found, it will be selected and 0 is returned.
 * Otherwise, the selected article will not be changed and -1 is returned.
 */
int catner_sel_article(catner_state_s *cs, const char *aid)
{
	xmlNodePtr article = libcatner_get_article(cs->articles, BAD_CAST aid);
	if (article == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_NODE;
		return -1;
	}
	
	// Check if this is different from the currently selected article
	if (cs->_curr_article != article)
	{
		// If so, update the reference
		cs->_curr_article = article;

		// We also have to reset feature and variant selection
		cs->_curr_feature = NULL;
		cs->_curr_variant = NULL;
		cs->_curr_image   = NULL;
		cs->_curr_unit    = NULL;
	}

	return 0;
}

/*
 * Set the currently selected article to the first article there is, or NULL 
 * if there is none. If the first article was already selected, this function 
 * does nothing. Otherwise, the current feature and variant selection will be 
 * reset to NULL. Returns 0 if there is now an article selected, otherwise -1.
 */
int catner_sel_first_article(catner_state_s *cs)
{
	// Let's find the first article
	xmlNodePtr first = libcatner_get_child(cs->articles, BMECAT_NODE_ARTICLE, NULL, 0);
	
	// Check if this is different from the currently selected article
	if (cs->_curr_article != first)
	{
		// If so, update the reference
		cs->_curr_article = first;

		// We also have to reset feature and variant selection
		cs->_curr_feature = NULL;
		cs->_curr_variant = NULL;
		cs->_curr_image   = NULL;
		cs->_curr_unit    = NULL;
	}

	if (cs->_curr_article == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_NODE;
		return -1;
	}
	return 0;
}

/*
 * Change the currently selected article to the next one there is, or NULL.
 * If no article was selected, this function will do nothing and return 0.
 * Otherwise, the currently selected feature and variant will be reset, too.
 * Returns 0 if there was another article and it was selected, otherwise -1.
 */
int catner_sel_next_article(catner_state_s *cs)
{
	// If we have no article currently selected, abort
	if (cs->_curr_article == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SEL_ARTICLE;
		return -1;
	}

	// Now we'll advance to the next article
	cs->_curr_article = libcatner_next_node(cs->_curr_article);
	
	// Change of article means we've got to reset selected feature and variant
	cs->_curr_feature = NULL;
	cs->_curr_variant = NULL;
	cs->_curr_image   = NULL;
	cs->_curr_unit    = NULL;

	if (cs->_curr_article == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_NODE;
		return -1;
	}
	return 0;
}

int catner_sel_feature(catner_state_s *cs, const char *fid)
{
	// Can't select a feature if no article selected
	if (cs->_curr_article == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SEL_ARTICLE;
		return -1;
	}

	xmlNodePtr feature = libcatner_get_feature(cs->_curr_article, BAD_CAST fid);

	if (feature == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_NODE;
		return -1;
	}

	// The selected feature has changed?
	if (cs->_curr_feature != feature)
	{
		// Update the feature selection
		cs->_curr_feature = feature;

		// Therefore, also reset the current variant selection
		cs->_curr_variant = NULL;
	}

	return 0;
}

/*
 * Set the currently selected feature to the first one of the currently 
 * selected article, if any. If the first feature was already selected before, 
 * this function does nothing and returns 1. If the selection was changed, 
 * the current variant selection will be reset and the function returns 1 if 
 * there is now a feature selected or 0 if no feature could be selected. 
 */
int catner_sel_first_feature(catner_state_s *cs)
{
	// Can't select a feature if no article selected
	if (cs->_curr_article == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SEL_ARTICLE;
		return -1;
	}
	
	// Find the node containing all features 
	xmlNodePtr features = libcatner_get_child(cs->_curr_article, BMECAT_NODE_FEATURES, NULL, 0);
	if (features == NULL)
	{
		// The selected article doesn't have any features
		cs->error = LIBCATNER_ERR_NO_SUCH_NODE;
		return -1;
	}

	// Find the first feature or NULL if there aren't any
	xmlNodePtr first = libcatner_get_child(features, BMECAT_NODE_FEATURE, NULL, 0);

	// The selected feature has changed?
	if (cs->_curr_feature != first)
	{
		// Update the feature selection
		cs->_curr_feature = first;

		// Therefore, also reset the current variant selection
		cs->_curr_variant = NULL;
	}

	if (cs->_curr_feature == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_NODE;
		return -1;
	}
	return 0;
}

/*
 * TODO documentation
 */
int catner_sel_next_feature(catner_state_s *cs)
{
	// Make sure we currently have a feature selected, otherwise abort
	if (cs->_curr_feature == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SEL_FEATURE;
		return -1;
	}

	// Now we'll advance to the next feature
	cs->_curr_feature = libcatner_next_node(cs->_curr_feature);

	// Change of feature means we've to to reset the selected variant
	cs->_curr_variant = NULL;

	if (cs->_curr_feature == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_NODE;
		return -1;	
	}
	return 0;
}

/*
 * TODO documentation
 */
int catner_sel_first_variant(catner_state_s *cs)
{
	// Check if we have a feature selected, otherwise abort
	if (cs->_curr_feature == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SEL_FEATURE;
		return -1;
	}

	// Find the node containing all variants of this feature
	xmlNodePtr variants = libcatner_get_child(cs->_curr_feature, BMECAT_NODE_VARIANTS, NULL, 0);
	if (variants == NULL)
	{
		// The selected feature doesn't have any variants
		cs->error = LIBCATNER_ERR_NO_SUCH_NODE;
		return -1;
	}

	// Find the first variant
	cs->_curr_variant = libcatner_get_child(variants, BMECAT_NODE_VARIANT, NULL, 0);
		
	if (cs->_curr_variant == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_NODE;
		return -1;
	}
	return 0;
}

/*
 * TODO documentation
 */
int catner_sel_next_variant(catner_state_s *cs)
{
	// Abort right away if we don't currently have a variant selected
	if (cs->_curr_variant == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SEL_VARIANT;
		return -1;
	}

	// Now we'll advance to the next variant
	cs->_curr_variant = libcatner_next_node(cs->_curr_variant);

	if (cs->_curr_variant == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_NODE;
		return -1;
	}
	return 0;
}

/*
 * TODO documentation
 */
int catner_sel_first_image(catner_state_s *cs)
{
	// Abort if no article selected
	if (cs->_curr_article == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SEL_ARTICLE;
		return -1;
	}

	// Get the MIME_INFO node that contains all images
	xmlNodePtr images = libcatner_get_child(cs->_curr_article, BMECAT_NODE_ARTICLE_IMAGES, NULL, 0);
	if (images == NULL)
	{
		// The selected article doesn't have any images
		cs->error = LIBCATNER_ERR_NO_SUCH_NODE;
		return -1;
	}

	// Find the first image
	cs->_curr_image = libcatner_get_child(images, BMECAT_NODE_ARTICLE_IMAGE, NULL, 0);

	if (cs->_curr_image == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_NODE;
		return -1;
	}
	return 0;
}

/*
 * TODO documentation
 */
int catner_sel_next_image(catner_state_s *cs)
{
	// Abort if not currently any image selected
	if (cs->_curr_image == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SEL_IMAGE;
		return -1;
	}

	// Now we'll advance to the next image
	cs->_curr_image = libcatner_next_node(cs->_curr_image);

	if (cs->_curr_image == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_NODE;
		return -1;
	}
	return 0;
}

/*
 * TODO documentation
 */
int catner_sel_first_unit(catner_state_s *cs)
{
	// Abort if no article selected
	if (cs->_curr_article == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SEL_ARTICLE;
		return -1;
	}

	// Get the ARTICLE_ORDER_DETAILS node that contains all (alternative) units
	xmlNodePtr units = libcatner_get_child(cs->_curr_article, BMECAT_NODE_ARTICLE_UNITS, NULL, 0);
	if (units == NULL)
	{
		// No units, we're done
		cs->error = LIBCATNER_ERR_NO_SUCH_NODE;
		return -1;
	}

	// Find the first unit
	cs->_curr_unit = libcatner_get_child(units, BMECAT_NODE_ARTICLE_ALT_UNIT, NULL, 0);
	
	if (cs->_curr_unit == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_NODE;
		return -1;
	}
	return 0;
}

/*
 * TODO documentation
 */
int catner_sel_next_unit(catner_state_s *cs)
{
	// Abort if no unit selected
	if (cs->_curr_unit == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SEL_UNIT;
		return -1;
	}

	// Get the next unit node, if there is one
	cs->_curr_unit = libcatner_next_node(cs->_curr_unit);

	if (cs->_curr_unit == NULL)
	{
		cs->error = LIBCATNER_ERR_NO_SUCH_NODE;
		return -1;
	}
	return 0;
}

//
// INIT / FREE / INPUT / OUTPUT / DEBUG
// 

/*
 * TODO documentation
 */
int catner_write_xml(catner_state_s *cs, const char *path)
{
	return xmlSaveFormatFileEnc(path, cs->doc, LIBCATNER_XML_ENCODING, 1);
}

/*
 * TODO documentation
 */
int catner_print_xml(catner_state_s *cs)
{
	return xmlSaveFormatFileEnc(LIBCATNER_STDOUT_FILE, cs->doc, LIBCATNER_XML_ENCODING, 1);
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

/*
 * Creates and returns a `catner_state_s` struct, which holds a reference to 
 * an XML tree which holds some basic elements required to construct a valid 
 * kloeckner-style BMEcat XML file. Returns NULL if out of memory. 
 */
catner_state_s *catner_init()
{
	catner_state_s *state = malloc(sizeof(catner_state_s));
	if (state == NULL)
	{
		return NULL;
	}
	catner_state_s empty_state = { 0 };
	*state = empty_state;

	state->doc       = xmlNewDoc(BAD_CAST LIBCATNER_XML_VERSION);
	state->root      = libcatner_get_root(state->doc, 1);
	state->header    = libcatner_get_header(state->root, 1);
	state->articles  = libcatner_get_articles(state->root, 1);
	state->catalog   = libcatner_get_catalog(state->header, 1);

	return state;
}

/*
 * Loads the given kloeckner-style BMEcat XML file into memory and returns 
 * a `catner_state_s` struct that allows reading and manipulating the XML. 
 *
 * If `amend` is `1`, required elements that are missing in the file will be
 * added on import. If an empty document is imported, this will create the 
 * basic outline of a kloeckner-style BMEcat file, including the BMECAT, 
 * HEADER, CATALOG and T_NEW_CATALOG nodes. If `amend` is `0` and the imported 
 * file is missing some of the required elements, this function returns `NULL`.
 */
catner_state_s *catner_load(const char *path, int amend)
{
	catner_state_s *state = malloc(sizeof(catner_state_s));
	if (state == NULL)
	{
		return NULL;
	}
	catner_state_s empty_state = { 0 };
	*state = empty_state;

	// Load the XML file
	state->doc  = xmlReadFile(path, NULL, 0);
	state->path = strdup(path);

	// Find (or possibly create) the BMECAT node
	state->root = libcatner_get_root(state->doc, amend);
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

	// Find (but don't create) the optional GENERATOR_INFO node
	state->generator = libcatner_get_generator(state->header, 0);
	
	return state;
}

/*
 * TODO - documentation 
 *      - make absolutely sure this does all we need it to do
 */
void catner_free(catner_state_s *cs)
{
	xmlFreeDoc(cs->doc);
	xmlCleanupParser();
	free(cs->path);
	free(cs);
	return;
}

/*
 * TODO documentation
 */
int catner_last_error(catner_state_s *cs)
{
	int e = cs->error;
	cs->error = LIBCATNER_ERR_NONE;
	return e;
}

