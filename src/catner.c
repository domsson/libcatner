#include <stdio.h>
#include <libxml/tree.h>
#include "bmecat.h"
#include "catner.h"

/*
 * Searches the parent node for the first child that matches the given `name` 
 * and, if given, text content `value`. Returns the child node found or NULL.
 * If `create` is `1`, the child node will be created if it wasn't found.
 */
xmlNodePtr libcatner_get_child(xmlNodePtr parent, const xmlChar *name, const xmlChar *value, int create)
{
	// Iterate all child nodes or parent
	xmlNodePtr child = parent->children;
	while (child != NULL)
	{
		// Does the child node's name match?
		if (xmlStrcmp(child->name, name) == 0)
		{
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
		child = child->next;
	}

	// No matching node found; return NULL or create and return one
	return create ? xmlNewChild(parent, NULL, name, value) : NULL;
}

/*
 * Searches parent for child nodes of the given name and returns the `n`th 
 * of those (0-based, so `1` would find the second), if there are that many. 
 * Otherwise returns NULL.
 */
xmlNodePtr libcatner_get_child_at(xmlNodePtr parent, const xmlChar *name, size_t n)
{
	// We count how many matches we've found (0-based) 
	size_t cur = 0;

	// Iterate all children of parent and check if they match `name`
	xmlNodePtr child = parent->children;
	while (child != NULL)
	{
		if (xmlStrcmp(child->name, name) == 0)
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
 * Set the LOCALE to the given value, overwriting the existing value if any.
 * If the node didn't exist yet, it will be created.
 *
 * The LOCALE node tells the processing software (the shop) what language the 
 * information in the file is in. Valid values are, for example, "EN" or "DE". 
 * 
 * TODO make sure `value` is valid (two letters, all uppercase)
 */
void catner_set_locale(catner_state_s *cs, const xmlChar *value)
{
	// Get the LOCALE node within CATALOG; create it if not present
	xmlNodePtr locale = libcatner_get_child(cs->catalog, BMECAT_NODE_LOCALE, NULL, 1);

	// Set the LOCALE's text node content to the given string
	xmlNodeSetContent(locale, value);
}

/*
 * Add a TERRITORY with the given value.
 *
 * TERRITORY nodes tell the processing software (the shop) what regions the 
 * products in this BMEcat file can be shipped to. Examples: "DE", "AT".
 * There can be multiple TERRITORY nodes, but each has to have a unique value.
 *
 * TODO make sure `value` is valid (two letters, all uppercase)
 */
void catner_add_territory(catner_state_s *cs, const xmlChar *value)
{
	libcatner_get_child(cs->catalog, BMECAT_NODE_TERRITORY, value, 1);
}

/*
 * Set the GENERATOR_INFO node to the given value.
 *
 * This node is optional and gives information on the software that 
 * was used to generate the BMEcat file.
 */
void catner_set_generator(catner_state_s *cs, const xmlChar *value)
{
	if (cs->generator == NULL)
	{
		cs->generator = xmlNewChild(cs->header, NULL, BMECAT_NODE_GENERATOR, value);
		return;
	}

	xmlNodeSetContent(cs->generator, value);
}

xmlNodePtr catner_get_article(catner_state_s *cs, const xmlChar *aid)
{
	// Iterate all articles
	xmlNodePtr article = cs->articles->children;
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
 * TODO return codes?
 */
int catner_set_article_title(catner_state_s *cs, const xmlChar *aid, const xmlChar *title)
{
	xmlNodePtr article = catner_get_article(cs, aid);
	if (article == NULL)
	{
		return -1;
	}

	// Find or create the ARTICLE_DETAILS node within this ARTICLE
	xmlNodePtr details = libcatner_get_child(article, BMECAT_NODE_ARTICLE_DETAILS, NULL, 1);

	// Find or create the DESCRIPTION_SHORT node within ARTICLE_DETAILS
	xmlNodePtr t = libcatner_get_child(details, BMECAT_NODE_ARTICLE_TITLE, NULL, 1);
	
	// Set the text of the title node accordingly
	xmlNodeSetContent(t, title);

	return 1;
}

/*
 * TODO return codes?
 */
int catner_set_article_descr(catner_state_s *cs, const xmlChar *aid, const xmlChar *descr)
{
	xmlNodePtr article = catner_get_article(cs, aid);
	if (article == NULL)
	{
		return -1;
	}

	// Find or create the ARTICLE_DETAILS node within this ARTICLE
	xmlNodePtr details = libcatner_get_child(article, BMECAT_NODE_ARTICLE_DETAILS, NULL, 1);

	// Find or create the DESCRIPTION_SHORT node within ARTICLE_DETAILS
	xmlNodePtr d = libcatner_get_child(details, BMECAT_NODE_ARTICLE_DESCR, NULL, 1);
	
	// Set the text of the title node accordingly
	xmlNodeSetContent(d, descr);

	return 1;
}

// TODO return codes? 1 for already exists, 0 for success, -1 for errors?
int catner_add_article(catner_state_s *cs, const xmlChar *aid, const xmlChar *title, const xmlChar *descr)
{
	if (catner_get_article(cs, aid) != NULL)
	{
		return 1;
	}
	xmlNodePtr article = xmlNewChild(cs->articles, NULL, BMECAT_NODE_ARTICLE, NULL);
	xmlNewChild(article, NULL, BMECAT_NODE_ARTICLE_ID, aid);
	xmlNodePtr details = xmlNewChild(article, NULL, BMECAT_NODE_ARTICLE_DETAILS, NULL);

	if (title != NULL)
	{
		xmlNewChild(details, NULL, BMECAT_NODE_ARTICLE_TITLE, title);
	}
	if (descr != NULL)
	{
		xmlNewChild(details, NULL, BMECAT_NODE_ARTICLE_DESCR, descr);
	}
	return 0;
}

xmlNodePtr catner_add_article_image(catner_state_s *cs, const xmlChar *aid, const xmlChar *mime, const xmlChar *path)
{
	xmlNodePtr article = catner_get_article(cs, aid);
	if (article == NULL)
	{
		return NULL;
	}
	
	xmlNodePtr images = libcatner_get_child(article, BMECAT_NODE_ARTICLE_IMAGES, NULL, 1);

	// See if there is already an image with that path in this ARTICLE
	xmlNodePtr image  = images->children;
	while (image != NULL)
	{
		// Check if this MIME node has a MIME_SOURCE node with the given `path` value
		if (libcatner_get_child(image, BMECAT_NODE_ARTICLE_IMAGE_PATH, path, 0) != NULL)
		{
			// If so, this image already exists, let's return it
			return image;
		}

		// Try the next image
		image = image->next;
	}

	// No such image present yet, let's create and return it
	image = xmlNewChild(images, NULL, BMECAT_NODE_ARTICLE_IMAGE, NULL);
	xmlNewChild(image, NULL, BMECAT_NODE_ARTICLE_IMAGE_MIME, mime);
	xmlNewChild(image, NULL, BMECAT_NODE_ARTICLE_IMAGE_PATH, path);

	return image;
}

/*
 * Adds the given category id to the article with the given `aid` and returns 
 * the associated ARTICLE_REFERENCE node. If there is no article with the given 
 * `aid`, this function returns NULL. If the article already had the cateogry 
 * added, the function simply returns a reference to it.
 */
xmlNodePtr catner_add_article_category(catner_state_s *cs, const xmlChar *aid, const xmlChar *cat)
{
	xmlNodePtr article = catner_get_article(cs, aid);
	if (article == NULL)
	{
		return NULL;
	}

	xmlNodePtr child = article->children;
	while (child != NULL)
	{
		// We are only interested in ARTICLE_REFERENCE nodes
		if (xmlStrcmp(child->name, BMECAT_NODE_ARTICLE_CATEGORY) == 0)
		{
			if (libcatner_get_child(child, BMECAT_NODE_ARTICLE_CATEGORY_ID, cat, 0) != NULL)
			{
				return child;
			}
		}
		
		// No match, check next node
		child = child->next;
	}

	// No such category present yet, let's add it
	xmlNodePtr category = xmlNewChild(article, NULL, BMECAT_NODE_ARTICLE_CATEGORY, NULL);
	xmlNewChild(category, NULL, BMECAT_NODE_ARTICLE_CATEGORY_ID, cat);

	return category;
}

catner_state_s *catner_init()
{
	catner_state_s *state = malloc(sizeof(catner_state_s));
	catner_state_s empty_state = { 0 };
	*state = empty_state;

	state->tree = xmlNewDoc(CATNER_XML_VERSION);
	state->root = xmlNewNode(NULL, BMECAT_NODE_ROOT);

	xmlNewProp(state->root, "version", BMECAT_VERSION);
	xmlNewProp(state->root, "xmlns",   BMECAT_NAMESPACE);

	state->header   = xmlNewChild(state->root,   NULL, BMECAT_NODE_HEADER,   NULL);
	state->articles = xmlNewChild(state->root,   NULL, BMECAT_NODE_ARTICLES, NULL);
	state->catalog  = xmlNewChild(state->header, NULL, BMECAT_NODE_CATALOG,  NULL);

	xmlDocSetRootElement(state->tree, state->root);

	return state;
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

	xmlSaveFormatFileEnc("-", cs->tree, CATNER_XML_ENCODING, 1);
}

