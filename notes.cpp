#include <vector>
#include <string>
#include <sstream>
using namespace std;

int indentation = 0;
int moreIndentation = 0;

void addErrorNote(vector<Token> & notes, int indents, const Token & token, const string & error) {
	Token errorToken;
	errorToken.type = "error";
	errorToken.text = "";
	int spaces = (token.start.ch - 1) + indents * 3;
	if (spaces > 0) {
		for (int i = 0; i < spaces; ++ i) errorToken.text += "&nbsp;";
	}
	errorToken.text += error + "<br>";
	notes.push_back(errorToken);
}

int getIndentationChanges1(vector<Token> & line) {
	int changes = 0;
	if (line[0].text == "}" || (line[0].type == "tabs" and line[1].text == "}")) -- changes;
	for (int i = 0; i < line.size(); ++ i) {
		if (line[i].text == "{") moreIndentation = 0;
	}
	return changes;
}

string LOOP_KEY[] = {
	"if", "else", "while", "do", "for" 
};

int getIndentationChanges2(vector<Token> & line) {
	int changes = 0;
	bool isLoop = false;
	for (int i = 0; i < line.size(); ++ i) {
		if (find(LOOP_KEY, 5, line[i].text) != -1) isLoop = true;
	}
	if (isLoop && line[line.size() - 2].text != ";") ++ moreIndentation;
	else moreIndentation = 0;
	for (int i = 0; i < line.size(); ++ i) {
		if (line[i].text == "}") -- changes;
		if (line[i].text == "{") {
			moreIndentation = 0;
			++ changes;
		}
	}
	return changes;
}

int getIndentation(vector<Token> & line) {
	if (line[0].type != "tabs") return 0;
	return line[0].text.length();
}

bool isFunction(vector<Token> & line) {
	if (line[0].type == "tabs" || line[0].text == "{" || line[0].text == "}") return false;
	if (line[0].type == "include" || line[0].text == "using") return false;

	bool hasSemicolon = false;
	bool hasId = false;
	bool hasParenthesis = false;
	for (int i = 0; i < line.size(); ++ i) {
		if (line[i].text == ";") hasSemicolon = true;
		if (line[i].type == "id") hasId = true;
		if (line[i].text == "(" || line[i].text == ")") hasParenthesis = true;
	}
	return (hasId && hasParenthesis && ! hasSemicolon);
}

int prevFuntionLine = 0;

string RIGHT[] = {
	",", ";", ")"
};

string LEFT[] = {
	"(", "-", "&", "~", "*"
};

string LEFT_RIGHT[] = {
	"=",
	"+",
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
	"?",
	":",
};

bool check(vector<string> & vec, const string & text) {
	bool has = false;
	for (int i = 0; i < vec.size(); ++ i) {
		if (vec[i] == text) {
			has = true;
			break;
		}
	}
	if (has) return true;
	vec.push_back(text);
	return false;
}

void processLine(vector<Token> & line, vector<Token> & prevLine, vector<Token> & notes) {
	// decomposition
	bool isFunc = isFunction(line);
	if (isFunc) prevFuntionLine = line[0].start.line;

	if (line.size() == 2 && line[0].text == "}") {
		int curLine = line[0].start.line;
		if (curLine - prevFuntionLine > 20) {
			addErrorNote(notes, 0, line[0], "函数太长，建议分解函数.");
		}
		//cout << curLine << "-" << prevFuntionLine << endl;
	}

	// indentatio
	int indentation2 = indentation + getIndentationChanges1(line) + moreIndentation;
	int indentation1 = getIndentation(line);
	if (indentation2 != indentation1) {
		stringstream ss;
		ss << "缩进错误，应该是" << indentation2 << "层";
		int first = (line[0].type == "tabs" ? 1 : 0);
		addErrorNote(notes, indentation1, line[first], ss.str());
	}
	indentation += getIndentationChanges2(line);
	if (indentation < 0) indentation = 0;

	// function space and comment
	if (isFunc && prevLine.size() > 0) {
		if (prevLine[0].type != "comment") {
			addErrorNote(notes, indentation1, line[0], "函数前建议写注释.");
			if (prevLine.size() > 1 && prevLine[prevLine.size() - 1].text.length() < 2) {
				addErrorNote(notes, indentation1, line[0], "函数要有空行.");
			}
		}
	}

	// spaces
	vector<string> opHas;
	for (int i = 0; i < line.size(); ++ i) {
		if (line[i].type != "operator") continue;
		bool leftRight = (find(LEFT_RIGHT, 30, line[i].text) != -1);
		bool right = (find(RIGHT, 4, line[i].text) != -1);
		bool left = (find(LEFT, 5, line[i].text) != -1);
		bool missLeft = false;
		bool missRight = false;
		if (left) { 
			if (i > 0 && line[i - 1].text != "(" && line[i - 1].text != "." && line[i - 1].type != "spaces" && line[i - 1].type != "tabs" && line[i - 1].type != "id") {
				missLeft = true;
			}
		}
		if (leftRight) { 
			if (i > 0 && line[i - 1].text != "(" && line[i - 1].type != "spaces" && line[i - 1].type != "tabs") {
				missLeft = true;
			}
		}
		if (leftRight || right) {
			if (i < line.size() - 1 && line[i + 1].text != ")" && line[i + 1].text != "." && line[i + 1].text != ";" && line[i + 1].text != "," && line[i + 1].type != "lines" && line[i + 1].type != "spaces") {
				missRight = true;
			}
		}
		if (missLeft && missRight) {
			if (! check(opHas, line[i].text))
				addErrorNote(notes, indentation1, line[i], line[i].text + " 左右应有空格.");
		}
		else {
			if (missLeft) {
				if (! check(opHas, line[i].text))
					addErrorNote(notes, indentation1, line[i], line[i].text + " 左边应有空格.");
			}
			else if (missRight) {
				if (! check(opHas, line[i].text))
					addErrorNote(notes, indentation1, line[i], line[i].text + " 右边应有空格.");
			}
		}
	}

	// variable names
	vector<string> idHas;
	for (int i = 0; i < line.size(); ++ i) {
		if (line[i].type == "id") {
			if (line[i].text.length() > 1) continue;
			if (line[i].text != "i" && line[i].text != "j" && line[i].text != "k") {
				bool isDef = false;
				for (int j = 0; j < i; ++ j) {
					if (line[j].text == "int" || line[j].text == "char" || line[j].text == "float" || line[j].text == "double") {
						isDef = true;
						break;
					}
				}
				if (isDef && ! check(idHas, line[i].text)) {
					addErrorNote(notes, indentation1, line[i], line[i].text + " 建议使用有意义的变量名.");
				}
			}
		}
	}

}

vector<Token> addNotes(vector<Token> & tokens) {
	indentation = 0;
	moreIndentation = 0;
	prevFuntionLine = 0;
	vector<Token> notes;
	vector<Token> prevLine;
	vector<Token> line;
	for (int i = 0; i < tokens.size(); ++ i) {
		notes.push_back(tokens[i]);
		line.push_back(tokens[i]);
		if (tokens[i].type == "lines") {
			processLine(line, prevLine, notes);
			prevLine = line;
			line.clear();
		}
	}
	return notes;
}