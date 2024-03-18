#include "gseen.h"


/*	this is the language loading main function
 */
void langs::load(const std::string &filename)
{
	xmlDocPtr doc;
	xmlNodePtr root_element;
	xmlNodePtr cur_node;

	/* 
	 * this initialize the library and check potential ABI mismatches 
	 * between the version it was compiled for and the actual shared 
	 * library used. 
	 */
	LIBXML_TEST_VERSION

	/*parse the file and get the DOM */
	doc = xmlReadFile(filename.c_str(), NULL, 0);

	if (doc == NULL ) {
		throw ModuleException("Can not read language file ('gseen.xml').");
		return;
	}
	// get the root element 
	root_element = xmlDocGetRootElement(doc);
	for (cur_node = root_element->children; cur_node; cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (!xmlStrcmp(cur_node->name, reinterpret_cast<const xmlChar *>("languages")))
				read_langnums(doc, cur_node);
			else if (!xmlStrcmp(cur_node->name, reinterpret_cast<const xmlChar *>("lang")))
				read_language_strings(doc, cur_node);
		}
	}
	// free the document
	xmlFreeDoc(doc);
	// Free the global variables that may
	// have been allocated by the parser.
	xmlCleanupParser();
	// free up some memory
	langnum.clear();
}


void langs::read_langnums(xmlDocPtr doc, xmlNodePtr a_node)
{
	xmlNodePtr cur_node;
	const char *buf;
	xmlChar *content = NULL;
	int value;
	// iterate through all childnodes 
	for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			// get the language number
			buf = reinterpret_cast<const char*>(xmlGetProp(cur_node, reinterpret_cast<const xmlChar*>("num")));
			if (!buf)
				return; // error 
			value = atoi(buf);
			delete [] buf;

			// get the language string from the childnode
			content = xmlNodeListGetString(doc, cur_node->children, 1);
			if (!content)
				return; // error
			if (debug == 2)
				alog("debug: adding language %s with value %i", content, value);
			// now add the language and the numbers to the list
			langnum.insert(std::make_pair(reinterpret_cast<const char*>(content), value));
			xmlFree(content);
		}
	}
}

void langs::read_language_strings(xmlDocPtr doc, xmlNodePtr a_node)
{
	xmlNodePtr cur_node;
	const char *index = NULL;
	const char *content = NULL;
	std::string buf;

	// get the INDEX of the current node
	index = reinterpret_cast<const char*>(xmlGetProp(a_node, reinterpret_cast<const xmlChar*>("index")));

	// iterate through all childnodes 
	for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			// get the language string from the childnode
			content = reinterpret_cast<const char*>(xmlNodeListGetString(doc, cur_node->children, 1));
			if (!content)
				return; // error  (we should crash here to punish the user for fucking up the lang file)
			this->AddLangString(index, reinterpret_cast<const char*>(cur_node->name), buf.assign(content));
			delete [] content;
		}
	}
	delete [] index;
}

void langs::AddLangString(const std::string &index, const std::string &language, std::string &langstring)
{
	size_t num = 0;
	std::map<std::string, int >::iterator iter = this->langnum.find(language);
	std::map<std::string, std::vector<std::string > >::iterator it = this->langlist.find(index);

	// get the language number from the list 
	if (iter != this->langnum.end())
		num = iter->second;
	else
		return; // error!

	std::vector<std::string> vectorthing = it->second;

	// make sure the vector can hold all languages 
	if (vectorthing.size() < (20))  // we have to increase this value if we 
		vectorthing.resize(20); // want to add more languages (use a #defined value instaed?)

	// now add the parsed string to the vector
	vectorthing[num] =  ParseLangstringOnLoad(langstring);

	// clear the old vector from the map, so we can add the new one
	if (it != langlist.end())
		langlist.erase(it);
	// add the vector to the map
	langlist.insert(std::make_pair(index, vectorthing));

	if (debug == 2)
		alog("debug: added INDEX \"%s\", language \"%s\" vectorthing[%i]: \"%s\"", 
				index.c_str(), language.c_str(), num, vectorthing[num].c_str());

}

std::string langs::ParseLangstringOnLoad(std::string &langstring)
{
	/*
		1. remove all tabs and spaces after \n
		2. if the first char after \n is a dot, replace it with a space
		3. replace all [b], [/b], [u], [/u] with the right color codes
			bold = \002
			underline = \037
			normal = \017 - return to normal text
			color = \003, followed by the color code
				
	*/

	std::string bbcode, colorcode;
	size_t i, startpos, endpos;

	for (startpos = langstring.find("\n"); startpos != std::string::npos; startpos = langstring.find("\n", startpos+1))
	{
		for (endpos = startpos+1; endpos < langstring.size(); endpos++)
		{	// exit the loop if we run out of tabs or spaces
			if ((langstring[endpos] != '\t') && (langstring[endpos] != ' '))
			{
				break;
			}
		}
		// lets replace the dot with a space
		if (langstring[endpos] == '.')
			langstring[endpos] = ' ';
		if (langstring[endpos] == '\0')
			startpos--;
		// now lets remove the unneeded tabs and spaces
		if (endpos-1 > startpos+1) // do we need this check?
			langstring.erase(startpos+1, endpos-startpos-1);
	}

	/* replace bbcodes with color codes */
	for (i = 0; i <= 3; i++)
	{
		switch (i)
		{
			case 0:
				bbcode = "[b]"; colorcode = "\002"; break;
			case 1:
				bbcode = "[/b]"; colorcode = "\002"; break;
			case 2:
				bbcode = "[u]"; colorcode = "\037"; break;
			case 3:
				bbcode = "[/u]"; colorcode = "\037"; break;
		}
		for (startpos = langstring.find(bbcode); startpos != std::string::npos; startpos = langstring.find(bbcode))
		{
			langstring.replace(startpos, bbcode.length(), colorcode);
		}
	}
	return langstring;
}


std::string langs::GetLangStringIntern(int num, const std::string &index, va_list va)
{
	std::map<std::string, std::vector<std::string> >::iterator iter;
	std::vector<std::string> message;
	std::string langstring, value, mask;
	std::ostringstream valuestream;
	size_t i, startpos;
	void *p;
	char f;

	// search for the Index
	iter = langlist.find(index);
	if (iter == langlist.end())
	{
		alog("Error: NoticeLang() called with non existing Index: %s", index.c_str());
		return "An error occured. See your services.log for more details.";
	}
	message = iter->second;

	// get the langstring in the users language
	langstring = message[num];
	// language is not english AND langstring is emtpy
	if ((num > 0) && langstring.empty()) 
	{
		alog("Error: No message in language %i for Index %s, falling back to english", num, index.c_str());
		num = 0; // english 
		langstring = message[num];
		if (langstring.empty())
		{
			alog("Error: No english message for Index %s. giving up now.", index.c_str());
			return "An error occured. See your services.log for more details.";
		}
	}
	for (i = 0; i <= 9; i++)
	{
		// generate a mask like "%0" and "%1", so we can search for and replace it.
		valuestream << "%" << i;
		mask = valuestream.str();
		valuestream.str(""); // clear the buffer 
		// get the pointer of the next param
		p = va_arg(va, void*);
		for (startpos = langstring.find(mask); startpos != std::string::npos; startpos = langstring.find(mask))
		{
			f = langstring[startpos + mask.size()];
			if (p) 
			{
				switch (f)
				{
					case 's':	valuestream << static_cast<char *>(p); break;
					case 'i':	valuestream << reinterpret_cast<int>(reinterpret_cast<int*>(p)); break;
					case 'u':	valuestream << reinterpret_cast<size_t>(reinterpret_cast<size_t*>(p)); break;
				}
				value = valuestream.str();
				valuestream.str(""); // clear the buffer
				// if (value.empty())
				//	value = "(Error)";
				langstring.replace(startpos, mask.length()+1, value.c_str());
			}
			else
			{ 
				langstring.replace(startpos, mask.length()+1, "(null)");
			}
		}	
	}
//	va_end(va);

	return langstring;
}

int langs::GetLang(User *u)
{
	if (u && u->nc)
		return u->nc->language;
	return NSDefLanguage;
}

int langs::GetLang(NickCore *nc)
{
	return nc->language;
}

int langs::GetLang(ChannelInfo *ci)
{
	return ci->founder->language;
}

int langs::GetLang(Channel *c)
{
	if (c && c->ci)
		return c->ci->founder->language;
	return NSDefLanguage;
}

std::string langs::GetLangString(int num, const std::string &index, ...)
{
	va_list va;
	std::string langstring;
	va_start(va, index);
	langstring = GetLangStringIntern(num, index, va);
	va_end(va);
	return langstring;
}

void langs::NoticeLang(char *source, User *u, const std::string &index, ...)
{
	va_list va;
	std::string langstring, substr;
	size_t startpos, endpos;

	va_start(va, index);
	langstring = GetLangStringIntern(GetLang(u), index, va);
	va_end(va);

	// split the message into lines and send it line by line
	for (startpos = 0; startpos != langstring.size(); startpos = endpos)
	{
		if (langstring[startpos] == '\n')
			startpos++;
		endpos = langstring.find('\n', startpos);
		if (endpos == std::string::npos)
			endpos = langstring.size();
		substr = langstring.substr(startpos, endpos-startpos);
		u->SendMessage(source, "%s", substr.c_str());
	}
}


void langs::BotSpeakLang(ChannelInfo *ci, const std::string &index, ...)
{
	va_list va;
	std::string langstring, substr;
	size_t startpos, endpos;

	va_start(va, index);
	langstring = GetLangStringIntern(GetLang(ci), index, va);
	va_end(va);

	// split the message into lines and send it line by line
	for (startpos = 0; startpos != langstring.size(); startpos = endpos)
	{
		if (langstring[startpos] == '\n')
			startpos++;
		endpos = langstring.find('\n', startpos);
		if (endpos == std::string::npos)
			endpos = langstring.size();
		substr = langstring.substr(startpos, endpos-startpos);
		ircdproto->SendPrivmsg(ci->bi, ci->name, "%s", substr.c_str());
	}

}
