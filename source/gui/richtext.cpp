







#include "richtext.h"
#include "icon.h"
#include "../utils.h"


//#define USTR_DEBUG

RichPart::RichPart()
{
	m_type = RICH_TEXT;

#ifdef USTR_DEBUG
	Log("RichPart::RichPart()");
	
#endif
}

RichPart::RichPart(const RichPart& original)
{

#ifdef USTR_DEBUG
	Log("RichPart::RichPart(const RichPart& original)");
	
#endif

	*this = original;
}

RichPart::RichPart(const char* cstr)
{
	m_type = RICH_TEXT;
	m_text = UStr(cstr);

#ifdef USTR_DEBUG
	Log("RichPart::RichPart(const char* cstr) end '"<<m_text.rawstr()<<"'");
	
#endif
}

RichPart::RichPart(UStr ustr)
{
	m_type = RICH_TEXT;
	m_text = ustr;
}


RichPart::RichPart(int type, int subtype)
{
	if(type == RICH_ICON)
	{
		m_type = RICH_ICON;
		m_icon = subtype;
		return;
	}

	m_type = RICH_TEXT;
}

RichPart& RichPart::operator=(const RichPart &original)
{
#ifdef USTR_DEBUG
	Log("RichPart& RichPart::operator=(const RichPart &original)");
	
#endif

	m_type = original.m_type;
	m_text = original.m_text;
	m_icon = original.m_icon;

	return *this;
}

int RichPart::texlen() const	// icons count as 1 glyph
{
	if(m_type == RICH_TEXT)
		return m_text.m_length;
	else if(m_type == RICH_ICON)
		//return g_icon[m_icon].m_tag.m_length;
		return 1;

	return 0;
}

int RichPart::rawlen() const	// icon tag length is counted
{
	if(m_type == RICH_TEXT)
		return m_text.m_length;
	else if(m_type == RICH_ICON)
		return g_icon[m_icon].m_tag.m_length;

	return 0;
}

std::string RichPart::texval() const
{
	if(m_type == RICH_TEXT)
	{
#if 0
//#ifdef USTR_DEBUG
		Log("\tstring RichPart::texval() const...");
		
#endif

		return m_text.rawstr();

#if 0
//#ifdef USTR_DEBUG
		Log("\tstring RichPart::texval() const = "<<m_text.rawstr());
		
#endif
	}
	else if(m_type == RICH_ICON)
	{
		Icon* icon = &g_icon[m_icon];
		return icon->m_tag.rawstr();
	}

	return m_text.rawstr();
}

RichPart RichPart::substr(int start, int length) const
{
	if(m_type == RICH_ICON)
		return *this;
	else if(m_type == RICH_TEXT)
	{
		RichPart subp(m_text.substr(start, length));
		return subp;
	}

	return *this;
}

RichText::RichText(const RichPart& part)
{
	m_part.clear();
	m_part.push_back(part);
}

RichText::RichText(const RichText& original)
{
#ifdef USTR_DEBUG
	Log("RichText::RichText(const RichText& original) = "<<original.rawstr());
	
#endif
	
	*this = original;
}

RichText::RichText(const char* cstr)
{
#ifdef USTR_DEBUG
	Log("RichText::RichText(const char* cstr)");
	
#endif

	m_part.clear();
	m_part.push_back( RichPart(cstr) );
}

RichText& RichText::operator=(const RichText &original)
{
#ifdef USTR_DEBUG
//#if 1
	Log("richtext::= ";
	
	Log("from: ";
	
	Log(rawstr());
	
	Log("to: ";
	
	Log(original.rawstr());
	
#endif

	m_part.clear();

	for(std::list<RichPart>::const_iterator i=original.m_part.begin(); i!=original.m_part.end(); i++)
		m_part.push_back( *i );

	return *this;
}

RichText RichText::operator+(const RichText &other)
{
	RichPart twopart;
	bool havecombomid = false;

	RichPart* last1 = NULL;
	RichPart* first2 = NULL;

	//if there's something in text 1 and text 2...
	if(m_part.size() > 0 && other.m_part.size() > 0)
	{
		//get the last RichPart in text1 that has something in it
		for(std::list<RichPart>::reverse_iterator i=m_part.rbegin(); i!=m_part.rend(); i++)
		{
			if(i->texlen() > 0)
			{
				last1 = &*i;
				break;
			}
		}

		//get the first RichPart in text2 that has something in it
		for(std::list<RichPart>::const_iterator i=other.m_part.begin(); i!=other.m_part.end(); i++)
		{
			if(i->texlen() > 0)
			{
				first2 = (RichPart*)&*i;
				break;
			}
		}

		/*
		if the last RichPart of text1 and the first RichPart of text2
		are text (and not icons), they can be joined into one RichPart.
		*/
		if(last1 && 
			first2 &&
			last1->m_type == RICH_TEXT && 
			first2->m_type == RICH_TEXT)
		{
			twopart.m_type = RICH_TEXT;
			twopart.m_text = last1->m_text + first2->m_text;
			havecombomid = true;
		}
		//otherwise, set these to NULL to remember not to combine them
		else
		{
			last1 = NULL;
			first2 = NULL;
		}
	}

	RichText combined;

	for(std::list<RichPart>::iterator i=m_part.begin(); i!=m_part.end(); i++)
	{
		//if this is the part that we've combined, don't add it
		if(&*i == last1)
			break;

		//don't copy if empty
		if(i->texlen() <= 0)
			continue;

		combined.m_part.push_back(*i);

		//Log("combined1 rawstr = "<<combined.rawstr());
	}

	//if we have a combined middle, add that
	if(twopart.texlen() > 0 && havecombomid)
		combined.m_part.push_back(twopart);

	//Log("combined2 rawstr = "<<combined.rawstr());

	for(std::list<RichPart>::const_iterator i=other.m_part.begin(); i!=other.m_part.end(); i++)
	{
		//if this is the part that we've combined, don't add it
		if(&*i == first2)
			//break;
			continue;	//corpc fix
		
		//don't copy if empty
		if(i->texlen() <= 0)
			continue;

		combined.m_part.push_back(*i);

		//Log("combined3 rawstr = "<<combined.rawstr());
	}

	return combined;
}

RichText RichText::substr(int start, int length) const
{
	RichText retval;

	int totallen = texlen();
	int currplace = 0;
	std::list<RichPart>::const_iterator currp = m_part.begin();

	while(currplace < totallen && currplace < start+length && currp != m_part.end())
	{
		int currlen = currp->texlen();

		if(currlen <= 0)
			continue;

		int startplace = start - currplace;
		int endplace = (start+length) - currplace;

		//if(currplace < start+length && currplace+currlen >= start)
		if(startplace < currlen && endplace >= 0)
		{
			RichPart addp;

			if(startplace < 0)
				startplace = 0;

			if(endplace > currlen)
				endplace = currlen;

			int addlen = endplace - startplace;

			addp = currp->substr(startplace, addlen);

			retval = retval + addp;
		}

		currplace += currlen;
		currp++;
	}

	return retval;
}

std::string RichText::rawstr() const
{
	std::string raws;

#ifdef USTR_DEBUG
	//int parti = 0;
	//Log("std::string RichText::rawstr() const before loop..."<<parti);
	//
#endif

	for(std::list<RichPart>::const_iterator i=m_part.begin(); i!=m_part.end(); i++)
	{
#ifdef USTR_DEBUG
		//Log("std::string RichText::rawstr() const parti="<<parti);
		//
		//Log("\tstring RichText::rawstr() const = "<<i->texval());
		//
		//parti++;
#endif

		raws += i->texval();
	}

	return raws;
}

int RichText::texlen() const	// icons count as 1 glyph
{
	int runl = 0;

	for(std::list<RichPart>::const_iterator i=m_part.begin(); i!=m_part.end(); i++)
		runl += i->texlen();

	return runl;
}

int RichText::rawlen() const	// icon tag length is counted
{
	int runl = 0;

	for(std::list<RichPart>::const_iterator i=m_part.begin(); i!=m_part.end(); i++)
		runl += i->rawlen();

	return runl;
}

RichText::RichText()
{
}

RichText RichText::pwver() const	//asterisk-mask password std::string
{
	int len = 0;

	for(std::list<RichPart>::const_iterator i=m_part.begin(); i!=m_part.end(); i++)
		len += i->texlen();

	char* pwcstr = new char[len+1];

	for(int i=0; i<len; i++)
		pwcstr[i] = '*';

	pwcstr[len] = '\0';

	RichPart pwstrp(pwcstr);
	delete pwcstr;

	RichText pwstr;
	pwstr.m_part.push_back(pwstrp);

	return pwstr;
}

#ifdef USTR_DEBUG
int parsedepth = 0;
#endif

RichText ParseTags(RichText original, int* caret)
{
	RichText parsed;
	int currplace = 0;

	bool changed = false;

#ifdef USTR_DEBUG
	parsedepth ++;

	Log("ParseTags #"<<parsedepth);
	

	//if(parsedepth > 10)
	//	return original;
#endif
	
	for(std::list<RichPart>::iterator i=original.m_part.begin(); i!=original.m_part.end(); i++)
	{
		if(i->m_type == RICH_TEXT)
		{
			bool foundtag = false;

			std::string num;
			int firstof = -1;
			int hashoff = -1;
			int lastof = -1;

			RichPart* p = &*i;
			UStr* s = &p->m_text;
			unsigned int* u = s->m_data;

			for(int j=0; j<s->m_length; j++)
			{
				if(u[j] == '&' &&
					!foundtag)
				{
					firstof = j;
					lastof = j;
					num.clear();
				}
				else if(u[j] == '#' &&
					!foundtag &&
					firstof == j-1 &&
					firstof >= 0)
				{
					hashoff = j;
					lastof = j;
					num.clear();
				}
				else if(u[j] == ';' && 
					firstof >= 0 && 
					hashoff == firstof+1 &&
					!foundtag && 
					lastof > firstof &&
					num.length() > 0)
				{
					lastof = j;
					foundtag = true;
				}
				else if(u[j] >= '0' &&
					u[j] <= '9' &&
					firstof >= 0 &&
					hashoff == firstof+1 &&
					!foundtag)
				{
					num += (char)u[j];
				}
				else if(!foundtag)
				{
					num.clear();
					firstof = -1;
					hashoff = -1;
					lastof = -1;
				}
			}

			if(!foundtag)
			{
				parsed = parsed + *i;
				continue;
			}

#ifdef USTR_DEBUG
			Log("ParseTags found tag \""<<icon->m_tag.rawstr()<<"\" in \""<<i->m_text.rawstr()<<"\"");
			
#endif

			if(firstof > 0)
			{
				RichPart before = i->substr(0, firstof);

#ifdef USTR_DEBUG
				Log("ParseTags before str at "<<firstof<<" \""<<before.m_text.rawstr()<<"\"");
				
#endif

				parsed = parsed + RichText(before);

#ifdef USTR_DEBUG
				Log("\tparsed now = \""<<parsed.rawstr()<<"\"");
				
#endif
			}

			unsigned int addi = StrToInt(num.c_str());
			RichPart addp(addi);
			parsed = parsed + RichText(addp);

			int taglen = lastof - firstof + 1;
			int partlen =  i->m_text.m_length;

#ifdef USTR_DEBUG
			Log("\tparsed now = \""<<parsed.rawstr()<<"\"");
			
#endif

			if(firstof+taglen < partlen)
			{
				RichPart after = i->substr(firstof+taglen, partlen-(firstof+taglen));

#ifdef USTR_DEBUG
				Log("ParseTags after str at "<<(firstof+taglen)<<" \""<<after.m_text.rawstr()<<"\"");
				
#endif

				parsed = parsed + RichText(after);

#ifdef USTR_DEBUG
				Log("\tparsed now = \""<<parsed.rawstr()<<"\"");
				
#endif
			}

			if(caret != NULL)
			{
				if(currplace+firstof < *caret)
				{
					*caret -= taglen-1;
					currplace += partlen-taglen+1;
				}
			}

			foundtag = true;
			changed = true;
		}
		else
		{
			parsed = parsed + *i;
		}

		if(!changed && caret != NULL)
			currplace += i->texlen();
	}

	if(changed)
		return ParseTags(parsed, caret);

	//reset
	parsed = RichText("");

	for(std::list<RichPart>::iterator i=original.m_part.begin(); i!=original.m_part.end(); i++)
	{
		if(i->m_type == RICH_TEXT)
		{
			bool foundtag = false;

			for(int j=0; j<ICONS; j++)
			{
				Icon* icon = &g_icon[j];
				int firstof = i->m_text.firstof(icon->m_tag);

				if(firstof < 0)
					continue;

#ifdef USTR_DEBUG
				Log("ParseTags found tag \""<<icon->m_tag.rawstr()<<"\" in \""<<i->m_text.rawstr()<<"\"");
				
#endif

				if(firstof > 0)
				{
					RichPart before = i->substr(0, firstof);

#ifdef USTR_DEBUG
					Log("ParseTags before str at "<<firstof<<" \""<<before.m_text.rawstr()<<"\"");
					
#endif

					parsed = parsed + RichText(before);

#ifdef USTR_DEBUG
					Log("\tparsed now = \""<<parsed.rawstr()<<"\"");
					
#endif
				}

				RichPart iconp(RICH_ICON, j);
				parsed = parsed + RichText(iconp);

				int taglen = icon->m_tag.m_length;
				int partlen =  i->m_text.m_length;

#ifdef USTR_DEBUG
				Log("\tparsed now = \""<<parsed.rawstr()<<"\"");
				
#endif

				if(firstof+taglen < partlen)
				{
					RichPart after = i->substr(firstof+taglen, partlen-(firstof+taglen));

#ifdef USTR_DEBUG
					Log("ParseTags after str at "<<(firstof+taglen)<<" \""<<after.m_text.rawstr()<<"\"");
					
#endif

					parsed = parsed + RichText(after);

#ifdef USTR_DEBUG
					Log("\tparsed now = \""<<parsed.rawstr()<<"\"");
					
#endif
				}

				if(caret != NULL)
				{
					if(currplace+firstof < *caret)
					{
						*caret -= taglen-1;
						currplace += partlen-taglen+1;
					}
				}

				foundtag = true;
				changed = true;

				break;
			}

			if(!foundtag)
			{
				parsed = parsed + *i;
			}
		}
		else
		{
			parsed = parsed + *i;
		}

		if(!changed && caret != NULL)
			currplace += i->texlen();
	}
	
	if(changed)
		return ParseTags(parsed, caret);

#ifdef USTR_DEBUG
	Log("ParseTags final = "<<original.rawstr());
	
#endif

	return original;
}
