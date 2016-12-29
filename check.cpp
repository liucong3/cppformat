
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;

#include "token.cpp"
#include "notes.cpp"

void writeFile(const char filename[], vector<string> & texts) {
	ofstream output(filename);
	for (int i = 0; i < texts.size(); ++ i) {
		output << texts[i];
	}
	output.close();
}

string toHTML(const string & text) {
	string result;
	for (int i = 0; i < text.size(); ++ i) {
		switch (text[i]) {
			case ' ': result += "_"; break;
			case '"': result += "&quot;"; break;
			case '&': result += "&amp;"; break;
			case '<': result += "&lt;"; break;
			case '>': result += "&gt;"; break;
			case '\t': result += "___."; break;
			case '\n': result += "&crarr;<br>\n"; break;
			default: result += text[i];
		}
	}
	return result;
}

string toHTML(const string & line, const string & color, bool change = true) {
	return "<font color=" + color + ">" + (change ? toHTML(line) : line) + "</font>";
}

string toHTML(const Token & token) {
	if (token.type == "id") return toHTML(token.text, "Black"); 
	if (token.type == "spaces") return toHTML(token.text, "Silver"); 
	if (token.type == "lines") return toHTML(token.text, "Silver"); 
	if (token.type == "tabs") return toHTML(token.text, "Silver"); 
	if (token.type == "keyword") return toHTML(token.text, "Purple"); 
	if (token.type == "literal") return toHTML(token.text, "DarkBlue"); 
	if (token.type == "operator") return toHTML(token.text, "Blue"); 
	if (token.type == "comment") return toHTML(token.text, "Gray"); 
	if (token.type == "include") return toHTML(token.text, "Green"); 
	if (token.type == "lex_error") return toHTML(token.text, "Red"); 
	if (token.type == "error") return toHTML(token.text, "Red", false); 
	return toHTML(token.text);
}

void processProgram2(int id, const string & problem, vector<string> & lines) {
	vector<string> content;
	content.push_back("<head><meta charset=\"UTF-8\"></head><body><font face=Courier>");
	// TODO
	stringstream header;
	header << "学号: " << id << " 题号:" << problem << "<br><br>";
	content.push_back(toHTML(header.str(), "red", false));
	vector<Token> tokens = getTokens(lines);
	tokens = addNotes(tokens);
	for (int i = 0; i < tokens.size(); ++ i) {
		content.push_back(toHTML(tokens[i]));
	}
	content.push_back("</font></body>");

	stringstream outfilename;
	outfilename << "output/" << id << '-' << problem;
	string filename = outfilename.str();
	writeFile((filename + ".html").c_str(), content);
	cout << "wkhtmltopdf --zoom 7 " << (filename + ".html") << " " <<  (filename + ".pdf") << endl;
}

void processProgram(int id, string problem, vector<string> & lines) {
	if (id == 0) return;
	int first = -1; int last = -1;
	for (int i = 0; i < lines.size(); ++ i) {
		if (lines[i] == "[example]") first = i;
		if (lines[i] == "[test]") last = i;		
	}
	stringstream outfilename;
	outfilename << "output/" << id << '-' << problem << ".html";
	vector<string> content;
	if (first == -1 || last == -1) {
		content.push_back("<head><meta charset=\"UTF-8\"></head><body>");
		content.push_back(toHTML("缺作业", "red"));
		content.push_back("</body>");
		writeFile(outfilename.str().c_str(), content);
		return;
	}
	else {
		vector<string> lines2;
		for (int i = first + 1; i < last; ++ i) {
			lines2.push_back(lines[i]);
		}
		processProgram2(id, problem, lines2);
	}
}

string trimBack(string & line) {
	int trim = 0;
	for (int i = line.size() - 1; i >= 0; -- i) {
		if (line[i] == ' ' || line[i] == '\r' || line[i] == '\t') {
			++ trim;
		}
		else break;
	}
	if (trim == 0) return line;
	if (line.size() == trim) return "";
	return line.substr(0, line.size() - trim);
}

void readFile(const char filename[]) {
	ifstream input(filename);
	if (input.fail()) {
		cout << "Cannot open file: " << filename << endl;
		return;
	}

	char hr[] = "******";
	vector<string> lines;
	int id = -1;
	string problem;

	while (! input.eof()) {
		string line;
		getline(input, line);
		line = trimBack(line);
		if (line.substr(0,6) == hr) {
			if (id != -1) {
				processProgram(id, problem, lines);
			} 
			lines.clear();
			getline(input, line);
			stringstream ss1(line);
			ss1 >> problem >> problem;
			getline(input, line);
			stringstream ss2(line);
			ss2 >> line >> id;
			//cout << id << endl;
			getline(input, line);
		}
		else {
			lines.push_back(line);
		}
	}
	if (id != -1) {
		processProgram(id, problem, lines);
	} 

	input.close();
}

int main() {
	readFile("9-11.txt");
	readFile("12-11.txt");
}