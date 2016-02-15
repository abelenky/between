#include "targetver.h"

// TODO list:
// Get the Rules vector initialized inline, removing the ctor.
// Add LineCounter, Case Insensitive and Verbose.

#include "windows.h"
#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <map>
#include <unordered_map>
#include <exception>
#include "TextProcessor"

#include "atlstr.h"

// usage:
// between ==> Show help
// between help ==> Show help

// between ::CPP  input.file             Everything between two ::CPP lines
// between -S::CPP -E::END inputfile     Everything between ::CPP and ::END
// between -l25 -l30 inputfile           Everything on lines 26, 27, 28, 29
// between -l25 -c5 inputfile            Everything from line 25 for 5 lines (26, 27, 28, 29, 30)
// -I inclusive
// -C case-insensitive match.

// Consider ErrorLevel values:
// 0 = Success at least one matching pair of start-end was found.
// 1 = The target line was not found at all
// 2 = The target start was found, but was never closed with a matching end.

// List of options: -S -E -l -c

using std::vector;
using std::unordered_map;
using std::string;
using std::istream;
using std::ifstream;
using std::cout;
using std::endl;
using namespace Binky;


struct Option { enum Type { StartAt, EndAt, LineNumber, FreeText, NoCase };
	static string ToString(Type t)
	{
		unordered_map<Type, string> mapping
			({	{ StartAt,    "StartAt"     },
				{ EndAt,      "EndAt"       },
				{ LineNumber, "LineNumber"  },
				{ FreeText,   "FreeText"    },
				{ NoCase,     "NoCase"      }
			});
		return mapping.at(t);
	}
};

// CmdParam: Any commandline paramter: Search-Target, LineCounter, Case-sensitivity, 
// Delimiter:  Specific Search term: String (start and end) Line Number(and-or-count

class CmdParam
{
public:
	CmdParam(Option::Type t)             : type(t)            {}
	CmdParam(Option::Type t, string str) : type(t), text(str) {}
	CmdParam(Option::Type t, int n)      : type(t), number(n) {}

	__declspec(property(get = GetType))  Option::Type  Type;
	__declspec(property(get = GetText))  const string& Text;
	__declspec(property(get = GetValue)) int           Value;

	Option::Type  GetType()  const { return type;   }
	const string& GetText()  const { return text;   }
	int           GetValue() const { return number; }
	
	Option::Type type;
	string       text;
	int          number{ 0 };
};


class CmdLineOptions : public TextProcessor<CmdLineOptions>
{
public:
	CmdLineOptions();

	void AddStartPoint     (vector<string>&);
	void AddEndPoint       (vector<string>&);
	void AddLinePoint      (vector<string>&);
	void AddLineCounter    (vector<string>&);
	void AddCaseInsensitive(vector<string>&);
	void AddFreeText       (vector<string>&);
	const vector<Rule>& GetRules() const override { return rules; }

	vector<CmdParam>::iterator begin() { return options.begin(); }
	vector<CmdParam>::iterator end()   { return options.end();   }

	size_t CountOptions(Option::Type t) const
	{
		return std::count_if(options.begin(), options.end(), [=](const CmdParam& p){ return p.GetType() == t; });
	}

	const string& GetText(Option::Type t, int cnt = 1) const
	{
		for (auto it = options.begin(); it != options.end(); ++it)
		{
			if ((*it).GetType() == t)
			{
				cnt--;
				if (cnt == 0) return (*it).Text;
			}
		}
		throw std::exception("No Element found.");
	}

	int GetValue(Option::Type t, int cnt) const
	{
		for (auto it = options.begin(); it != options.end(); ++it)
		{
			if ((*it).Type == t)
			{
				cnt--;
				if (cnt == 0) return (*it).Value;
			}
		}
		throw std::exception("No Element found.");
	}

private:
	vector<CmdParam> options;

	vector<Rule> rules;
	/*vector<Rule> rules {
		Rule(regex("(-S)(.*)"),           &CmdLineOptions::AddStartPoint      ),
		Rule(regex("(-E)(.*)"),           &CmdLineOptions::AddEndPoint        ),
		Rule(regex("(-l)([[:digit:]]*)"), &CmdLineOptions::AddLinePoint       ),
		Rule(regex("(-c)([[:digit:]]*)"), &CmdLineOptions::AddLineCounter     ),
		Rule(regex("(-I)"),               &CmdLineOptions::AddCaseInsensitive ),
		Rule(regex("(.*)"),               &CmdLineOptions::AddFreeText        ),*/
	};
};

class Between
{
public:
	Between(const CmdLineOptions& options);

private:
	void File_StartToEnd (const CmdLineOptions& options);
	void StdIn_StartToEnd(const CmdLineOptions& options);
	void File_LineToLine (const CmdLineOptions& options); 
	void StdIn_LineToLine(const CmdLineOptions& options);
	void StdIn_TextToText(const CmdLineOptions& options);
	void File_TextToText(const CmdLineOptions& options);
	
	void read_StartToEnd(const string& start, const string& end, istream& input = std::cin);
	void read_LineToLine(int lineA, int lineB, istream& input = std::cin);
};


int main(int argc, char* argv[])
{
	CmdLineOptions options;

	try
	{
		for (int i = 1; i < argc; ++i)
		{
			options.DispatchText(string(argv[i]));
		}

		//if (options.Count() == 0)
		//{
		//	// Show Help
		//}

		//// Verbose Output of raw options.
		for (auto p = options.begin(); p != options.end(); ++p)
		{
			string msg;
			msg = Option::ToString((*p).GetType()) + "\n";
			OutputDebugString(msg.c_str());
		}

		Between between(options);
	}
	catch (std::exception& e)
	{
		cout << e.what();
	}

	return 0;
}


CmdLineOptions::CmdLineOptions()
{
	rules = {
		{ regex("(-S)(.*)"               ), &CmdLineOptions::AddStartPoint      },
		{ regex("(-E)(.*)"               ), &CmdLineOptions::AddEndPoint        },
		{ regex("(-l)([+-]?[[:digit:]]*)"), &CmdLineOptions::AddLinePoint       },
		{ regex("(-c)([[:digit:]]*)"     ), &CmdLineOptions::AddLineCounter     },
		{ regex("(-I)"                   ), &CmdLineOptions::AddCaseInsensitive },
		{ regex("(.*)"                   ), &CmdLineOptions::AddFreeText        },
	};
}

// -S<Text>
void CmdLineOptions::AddStartPoint(vector<string>& paramData)
{
	string data(paramData[0] + paramData[1]);
	OutputDebugString(string(data + ": Add Start Point\n").c_str());

	options.push_back(CmdParam(Option::StartAt, paramData[1]));
}

// -E<Text>
void CmdLineOptions::AddEndPoint(vector<string>& paramData)
{
	string data(paramData[0] + paramData[1]);
	OutputDebugString(string(data + ": Add End Point\n").c_str());
	options.push_back(CmdParam(Option::EndAt, paramData[1]));
}

// -l<line number>
void CmdLineOptions::AddLinePoint(vector<string>& paramData)
{
	string data(paramData[0] + paramData[1]);
	OutputDebugString(string(data + ": Add Line Point\n").c_str());

	int lineNum = std::stoi(paramData[1], NULL, 10);
	if (lineNum < 0)
	{
		throw std::exception("Line Number cannot be negative.");
	}

	options.push_back(CmdParam(Option::LineNumber, lineNum));
}

// -c<line count>
void CmdLineOptions::AddLineCounter(vector<string>&)
{}

// -I
void CmdLineOptions::AddCaseInsensitive(vector<string>&)
{}

void CmdLineOptions::AddFreeText(vector<string>& paramData)
{
	string data(paramData[0]);
	OutputDebugString(string(data + ": Add FreeText\n").c_str());

	options.push_back(CmdParam(Option::FreeText, paramData[0]));
}


Between::Between(const CmdLineOptions& options)
{
	// Post Process Rules:
	struct ExecutionRule
	{
		int cntStart;
		int cntEnd;
		int cntLines;
		int cntFreeText;
		
		bool operator<(const ExecutionRule& b) const
		{
			const ExecutionRule& a = *this;
			
			if (a.cntStart    != b.cntStart    ) return a.cntStart    < b.cntStart;
			if (a.cntEnd      != b.cntEnd      ) return a.cntEnd      < b.cntEnd;
			if (a.cntLines    != b.cntLines    ) return a.cntLines    < b.cntLines;
			if (a.cntFreeText != b.cntFreeText ) return a.cntFreeText < b.cntFreeText;
			return false;	
		}
	};

	typedef void(Between::*Method)(const CmdLineOptions&);
		
	std::map<ExecutionRule, Method> execRules{
		// -S  -E  -l  FT  Input  GetStart GetEnd:
		{{  1,  1,  0,  1 }, &Between::File_StartToEnd  }, // Filename for Start-End
		{{  1,  1,  0,  0 }, &Between::StdIn_StartToEnd }, // StdIn for Start-End
		{{  0,  0,  2,  1 }, &Between::File_LineToLine  }, // Filename for l1 - l2
		{{  0,  0,  2,  0 }, &Between::StdIn_LineToLine }, // StdIn for l1 - l2
		{{  0,  0,  0,  2 }, &Between::File_TextToText  }, // Filename for same start/end point (Filename is second param)
		{{  0,  0,  0,  1 }, &Between::StdIn_TextToText }, // StdIn for same start/end point.
	};

	// Figure out which ExecutionRule applies:
	ExecutionRule target{
		options.CountOptions(Option::StartAt),
		options.CountOptions(Option::EndAt),
		options.CountOptions(Option::LineNumber),
		options.CountOptions(Option::FreeText)
	};

	(this->*execRules[target])(options);
}


void Between::File_StartToEnd(const CmdLineOptions& options)
{
	read_StartToEnd(options.GetText(Option::StartAt),
					options.GetText(Option::EndAt),
					ifstream(options.GetText(Option::FreeText)));
}

void Between::StdIn_StartToEnd(const CmdLineOptions& options)
{
	read_StartToEnd(options.GetText(Option::StartAt),
					options.GetText(Option::EndAt));
}

void Between::read_StartToEnd(const string& start, const string& end, istream& input /* = std::cin */)
{
	do
	{
		string line;
		do
		{
			getline(input, line);
		} while (start != line && !input.eof());

		if (input.eof())
		{
			break;
		}

		do
		{
			getline(input, line); // line is currently the Start-Marker; move to the first line to output.
			if (line == end)
			{
				break;
			}
			cout << line << endl;
		} while (!input.eof());
	} while (!input.eof());

}

void Between::File_LineToLine (const CmdLineOptions& options)// Filename for l1 - l2
{
	read_LineToLine(options.GetValue(Option::LineNumber, 1),
					options.GetValue(Option::LineNumber, 2),
					ifstream(options.GetText(Option::FreeText)));

} 

void Between::StdIn_LineToLine(const CmdLineOptions& options) // StdIn for l1 - l2 
{
	read_LineToLine(options.GetValue(Option::LineNumber, 1),
					options.GetValue(Option::LineNumber, 2));
}

void Between::read_LineToLine(int lineA, int lineB, istream& input /* = std::cin */)
{
	int minLine = min(lineA, lineB);
	int maxLine = max(lineA, lineB);
	int lineNum = 0;

	string line;
	while (lineNum != minLine && !input.eof())
	{
		getline(input, line);
		lineNum++;
	};

	do
	{
		getline(input, line);
		lineNum++;
		if (lineNum == maxLine)
		{
			break;
		}
		cout << line << endl;
	} while (!input.eof());
}

void Between::File_TextToText (const CmdLineOptions& options)
{
	read_StartToEnd(options.GetText(Option::FreeText, 1),
					options.GetText(Option::FreeText, 1),
					ifstream(options.GetText(Option::FreeText, 2)));

}

void Between::StdIn_TextToText(const CmdLineOptions& options)
{
	const string& str = options.GetText(Option::FreeText);
	read_StartToEnd(str, str);
}

