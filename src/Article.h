#ifndef ARTICLE_H
#define ARTICLE_H

#include <iostream>
#include <vector>
#include <string>

using namespace std;


// Each Article represents a research article (json file) and contains its metadata 
class Article {

private:
	string id;
	string title;
	vector<string> authors;
	vector<string> authors_last;
	string body_text;

public:
	Article() {

	}

	Article(string id, string title, vector<string> authors, vector<string> authors_last, string body_text) { 
		this->id = id;
		this->title = title;
		this->authors = authors;
		this->authors_last = authors_last;
		this->body_text = body_text;
	}


	string get_id() { return id; };
	string get_title() { return title; };
	vector<string> get_authors() { return authors; };
	vector<string> get_authors_last() { return authors_last; };
	string get_text() { return body_text; };

};


#endif


