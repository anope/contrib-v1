#include "module.h"

class CommandText : public Command
{
	Configuration::Block *GetConfig(const Anope::string &cname)
	{
		for (int i = 0; i < Config->CountBlock("text"); ++i)
		{
			Configuration::Block *c = Config->GetBlock("text", i);

			if (c->Get<Anope::string>("name").equals_ci(cname))
				return c;
		}

		return NULL;
	}

 public:
	CommandText(Module *creator) : Command(creator, "text", 0, 0)
	{
	}

	void Execute(CommandSource &source, const std::vector<Anope::string> &params) anope_override
	{
		Configuration::Block *b = GetConfig(source.command);
		if (b == NULL)
			return;

		const Anope::string &message = b->Get<Anope::string>("message");
		if (message.empty())
			return;

		sepstream sep(message, '\n');
		for (Anope::string s; sep.GetToken(s);)
			source.Reply(s);
	}
};

class Text : public Module
{
	CommandText commandtext;

 public:
	Text(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator, THIRD),
		commandtext(this)
	{
		this->SetAuthor("Adam");
	}
};

MODULE_INIT(Text)
