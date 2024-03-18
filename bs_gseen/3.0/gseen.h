#ifndef GSEEN_H
#define GSEEN_H

#define MYSQLPP_MYSQL_HEADERS_BURIED 
#include "module.h"
#include <mysql++/mysql++.h> 
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#define DBNAME "bs_gseen"


class langs
{
 private: 
	// langs.lang["INDEX"][langnum][line]
	std::map<std::string, std::vector<std::string> > langlist;
	// langnum["english"]
	std::map<std::string, int> langnum;

	// get the language numbers from xml
	void read_langnums(xmlDocPtr doc, xmlNodePtr node);

	// load the language strings from xml
	void read_language_strings(xmlDocPtr doc, xmlNodePtr a_node);

	// add the language strings to the list
	void AddLangString(const std::string &index, const std::string &language,std::string &langstring);

	// parses the language string on file load, removes tabs and spaces 
	// and replaces all non-dynamic values like colorcodes
	std::string ParseLangstringOnLoad(std::string &langstring);

	std::string GetLangStringIntern(int num, const std::string &index, va_list va);


 public: 
	// open and read the xml file
	void load(const std::string &filename);

	// returns the language number
	int GetLang(User *u);
	int GetLang(NickCore *nc);
	int GetLang(Channel *c);
	int GetLang(ChannelInfo *ci);

	// return the requested langstring
	std::string GetLangString(int num, const std::string &index, ...);

	// notice the language string to the user
	void NoticeLang(char *source, User *u, const std::string &index, ...);

	// makes botserv send a message to the channel
	void BotSpeakLang(ChannelInfo *ci, const std::string &index, ...);
};

class CommandSEEN : public Command
{
 public:
	CommandSEEN() : Command("SEEN", 1, 3)
	{
	}

	CommandReturn Execute(User *u, std::vector<ci::string> &params);
};



extern mysqlpp::Connection con;
extern mysqlpp::Query query;
extern langs lng;


#endif	/* GSEEN_H */