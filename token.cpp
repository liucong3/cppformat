
#include <string>
#include <vector>
using namespace std;

struct Loc
{
	int line;
	int ch;
};

struct Token
{
	string type;
	string text;
	Loc start;
	Loc end;
};

int nextChar(const vector<string> & lines, Loc & progress) {
	if (progress.line > lines.size()) {
		progress.ch = -1;
		return -1;
	}
	const string & line = lines[progress.line - 1];
	if (progress.ch > line.size()) {
		++ progress.line;
		progress.ch = 1;
		return '\n';
	}
	char c = line[progress.ch - 1];
	++ progress.ch;
	return c;
}

string nextChars(const vector<string> & lines, Loc & progress, int count) {
	string text;
	for (int i = 0; i < count; ++ i) {
		int c = nextChar(lines, progress);
		if (c == -1) return text;
		text += (char)c;
	}
	return text;
}

bool isInCharSet(int c, const string & set) {
	for (int i = 0; i < set.size(); ++ i) {
		if (c == set[i]) return true;
	}
	return false;
}

bool isDigit(char c) {
	return c >= '0' && c <= '9';
}

bool isLetter(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

string LETTER = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
string DIGIT = "0123456789";

bool match1(const vector<string> & lines, Loc & progress, Token & token, 
	const string & startCharSet, const string & nextCharSet, const string & type)
{
	Loc oldProgress = progress;
	token.start = progress;
	token.text = "";
	int next = nextChar(lines, progress);
	if (not isInCharSet(next, startCharSet)) {
		progress = oldProgress;
		return false;
	}
	while (true) {
		token.text = token.text + (char)next;
		token.end = progress;
		next = nextChar(lines, progress);
		if (not isInCharSet(next, nextCharSet)) {
			token.type = type;
			progress = token.end;
			return true;
		}
	}
}

bool match2(const vector<string> & lines, Loc & progress, Token & token, 
	const string & startChars, const string & endChars, const string & escape, const string & type)
{
	Loc oldProgress = progress;
	token.start = progress;
	string text = nextChars(lines, progress, startChars.size());
	token.text = text;
	if (text != startChars) {
		progress = oldProgress;
		return false;
	}
	//if (text == "/*") cout << "+";
	int size1 = escape.size();
	int size2 = endChars.size();
	while (true) {
		int c = nextChar(lines, progress);
		if (progress.ch == -1) {
			progress = oldProgress;
			return false;
		}
		token.end = progress;
		text += (char)c;
		token.text += (char)c;
		if (text.size() > size1 + size2) {
			text = text.substr(text.size() - (size1 + size2), size1 + size2);
		}
		//if (startChars == "/*") cout << " " << text; 
		if (text.size() < size2) continue;
		if (size1 > 0 && text.size() == size1 + size2) {
			string escape1 = text.substr(0, size1);
			if (escape1 == escape) continue;
		}
		string endChars1 = text.substr(text.size() - size2, size2);
		if (endChars1 == endChars) {
			token.type = type;
			//cout << token.text << endl;
			return true;
		}
	}
}

int find(const string matches[], int size, const string & toFind) {
	for (int i = 0; i < size; ++ i) {
		if (matches[i] == toFind) return i;
	}
	return -1;
}

bool match3(const vector<string> & lines, Loc & progress, Token & token, 
	const string & charSet, const string matches[], int size, const string & type)
{
	Loc oldProgress = progress;
	token.start = progress;
	token.text = "";
	int next = nextChar(lines, progress);
	if (not isInCharSet(next, charSet)) {
		progress = oldProgress;
		return false;
	}
	while (true) {
		token.text = token.text + (char)next;
		token.end = progress;
		next = nextChar(lines, progress);
		string text = token.text + (char)next;
		if (not isInCharSet(next, charSet) || find(matches, size, text) == -1) {
			token.type = type;
			progress = token.end;
			return true;
		}
	}	
}

bool matchAny(const vector<string> & lines, Loc & progress, Token & token, const string & type)
{
	token.start = progress;
	int next = nextChar(lines, progress);
	if (next == -1) return false;
	token.end = progress;
	token.text = string() + char(next);
	token.type = type;
	return true;
}

string KEYWORDS[] = {
	"and",
	"and_eq",
	"asm",
	"bitand",
	"bitor",
	"bool",
	"break",
	"case",
	"catch",
	"char",
	"class",
	"compl",
	"const",
	"const_cast",
	"continue",
	"default",
	"delete",
	"do",
	"double",
	"dynamic_cast",
	"else",
	"enum",
	"explicit",
	"export",
	"extern",
	"float",
	"for",
	"friend",
	"goto",
	"if",
	"inline",
	"int",
	"long",
	"mutable",
	"namespace",
	"new",
	"not",
	"not_eq",
	"operator",
	"or",
	"or_eq",
	"private",
	"protected",
	"public",
	"register",
	"reinterpret_cast",
	"return",
	"short",
	"signed",
	"sizeof",
	"static",
	"static_cast",
	"struct",
	"switch",
	"template",
	"this",
	"throw",
	"true",
	"try",
	"typedef",
	"typeid",
	"typename",
	"union",
	"unsigned",
	"using",
	"virtual",
	"void",
	"volatile",
	"wchar_t",
	"while",
	"xor",
	"xor_eq"
};

bool matchId(const vector<string> & lines, Loc & progress, Token & token) {
	bool succ = match1(lines, progress, token, LETTER+"_", LETTER+DIGIT+"_", "id");
	if (! succ) return false;
	if (token.text == "true" || token.text == "false") {
		token.type = "literal";
	}
	if (find(KEYWORDS, 72, token.text) != -1) {
		token.type = "keyword";
	}
	return true;
}

string OPERATOR_CHARS = "!%^&*()|/.,:;~-+=[]{}<>?";
string OPERATPRS[] = {
	"=",
	"+",
	"-",
	"*",
	"/",
	"%",
	"++",
	"--",
	"==",
	"!=",
	">",
	"<",
	">=",
	"<=",
	"!",
	"&&",
	"||",
	"~",
	"&",
	"|",
	"^",
	"<<",
	">>",
	"+=",
	"-=",
	"*=",
	"/=",
	"%=",
	"|=",
	"^=",
	"<<=",
	">>=",
	"[",
	"]",
	"->",
	".",
	"->*",
	".*",
	",",
	"?",
	":",
	"::"
};

bool matchOp(const vector<string> & lines, Loc & progress, Token & token) {
	return match3(lines, progress, token, OPERATOR_CHARS, OPERATPRS, 42, "operator");
}

bool matchNumber1(const vector<string> & lines, Loc & progress, Token & token) {
	return match1(lines, progress, token, DIGIT+".", DIGIT+".", "literal");
}

bool matchString(const vector<string> & lines, Loc & progress, Token & token) {
	return match2(lines, progress, token, "\"", "\"", "\\", "literal");
}

bool matchChar(const vector<string> & lines, Loc & progress, Token & token) {
	return match2(lines, progress, token, "'", "'", "\\", "literal");
}

bool matchComment1(const vector<string> & lines, Loc & progress, Token & token) {
	return match2(lines, progress, token, "/*", "*/", "", "comment");
}

bool matchComment2(const vector<string> & lines, Loc & progress, Token & token) {
	return match2(lines, progress, token, "//", "\n", "", "comment");
}

bool matchSpaces(const vector<string> & lines, Loc & progress, Token & token) {
	return match1(lines, progress, token, " ", " ", "spaces");
}

bool matchTabs(const vector<string> & lines, Loc & progress, Token & token) {
	return match1(lines, progress, token, "\t", "\t", "tabs");
}

bool matchLines(const vector<string> & lines, Loc & progress, Token & token) {
	return match1(lines, progress, token, "\n", "\n", "lines");
}

bool matchInclude1(const vector<string> & lines, Loc & progress, Token & token) {
	return match2(lines, progress, token, "#include", ">", "", "include");
}

bool matchInclude2(const vector<string> & lines, Loc & progress, Token & token) {
	return match2(lines, progress, token, "#include", "\"", "", "include");
}

vector<Token> getTokens(const vector<string> & lines) {
	vector<Token> tokens;
	Loc progress;
	progress.line = 1;
	progress.ch = 1;
	Token token;
	while (progress.ch != -1) {
		if (matchId(lines, progress, token) or
			matchString(lines, progress, token) or
			matchChar(lines, progress, token) or
			matchNumber1(lines, progress, token) or
			matchComment1(lines, progress, token) or
			matchComment2(lines, progress, token) or
			matchOp(lines, progress, token) or
			matchSpaces(lines, progress, token) or
			matchTabs(lines, progress, token) or
			matchLines(lines, progress, token) or
			matchInclude1(lines, progress, token) or
			matchInclude2(lines, progress, token) or
			matchAny(lines, progress, token, "lex_error")) {
			tokens.push_back(token);
		}
	}
	return tokens;
}

