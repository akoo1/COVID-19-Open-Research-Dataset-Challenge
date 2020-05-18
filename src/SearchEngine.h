#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <cctype>
#include <unordered_map>
#include <map>

#include "Article.h"   
#include "AVLTree.h"
#include "Node.h"
#include "HashTable.h"

#include "../utils/parser.hpp" 		   // csv parser
#include "../utils/json.hpp"    	   // json parser
#include "../utils/porter2_stemmer.h"  // word stemmer

#include <dirent.h>	   		  // for reading from multiple directories
#include <unistd.h>			  //	
#include <sys/stat.h>		  //
#include <sys/types.h>		  // 

using namespace std;
using namespace aria::csv;
using json = nlohmann::json;

void display_menu();
void restore_word_index(AVLTree& word_tree);
void restore_author_index(HashTable& author_table);
void display_statistics(int num_articles_indexed, int num_words_indexed, int num_stop_words, AVLTree& word_tree, HashTable& author_table);
bool way_to_sort(Node*& lhs, Node*& rhs);


// The Index processor
void index_processor(AVLTree& word_tree, HashTable& author_table, vector<Article>& articles, 
	unordered_map<string, string>& published_date_map, unordered_map<string, string>& publication_map,
	int& num_articles_indexed, int& num_words_indexed, int& num_stop_words);

// The Document processors
void parse_csv(string file_path, unordered_map<string, string>& published_date_map, unordered_map<string, string>& publication_map);
void parse_directory(string folder_path, vector<Article>& articles);
Article parse_json(string& file_path);

// Helper functions for the document processors 
void remove_punctuation(string& str);
void to_lower(string& str);
vector<string> tokenize(string& str);
void load_stop_words(AVLTree& stop_words_tree);
void stem_words(vector<string>& tokens);
void remove_duplicates(vector<string>& tokens);

// The Query processor and Search processor.
void perform_search(vector<string>& final_matches, string user_query, string& temp, AVLTree& word_tree, HashTable& author_table);

// Helper function for search processor
vector<string> intersection(vector<vector<string>>& vecs);

// The Ranking processor
void rank_results(vector<string>& final_matches, vector<Article>& articles, string& temp, vector<string>& top15_results);

void display_results(vector<string>& top15_results, vector<Article>& articles, 
	unordered_map<string, string>& published_date_map, unordered_map<string, string>& publication_map);




// The SearchEngine is responsible for declaring data structures, running the menu, and starting the search by calling other processors  
void SearchEngine() {

	AVLTree word_tree;
	HashTable author_table(98317);
	vector<Article> articles;
	unordered_map<string, string> published_date_map;
	unordered_map<string, string> publication_map;

	int num_articles_indexed = 0, num_words_indexed = 0, num_stop_words = 0;

	cout << "Parsing data..." << endl << endl;

	index_processor(word_tree, author_table, articles, published_date_map, publication_map, 
		num_articles_indexed, num_words_indexed, num_stop_words);


	display_menu();
	cout << "--------> Enter your choice: ";
	char user_choice;
	cin >> user_choice;
	cout << endl;

	while(true) {

		// Ask user to enter a search query
		if (user_choice == '1') {
			cout << "Please enter a properly formatted prefix Boolean query: ";
			cin.ignore(); // When getline() reads from the input, there is a newline character left in the input stream from the previous cin 
			string user_query;
			getline(cin, user_query);
			cout << endl;

			vector<string> final_matches;
			vector<string> top15_results;
			// temp stores the untokenized search terms
			string temp = "";

			cout << "Searching..." << endl << endl;

			perform_search(final_matches, user_query, temp, word_tree, author_table);

			rank_results(final_matches, articles, temp, top15_results);	

			display_results(top15_results, articles, published_date_map, publication_map); 
		}

		// Clear index
		else if (user_choice == '2') {
			word_tree.clear_tree();
			author_table.clear_table();
			cout << "Word index cleared" << endl;
			cout << "Author index cleared" << endl << endl;
			num_articles_indexed = 0, num_words_indexed = 0, num_stop_words = 0;

 		}

 		// Open index file
		else if (user_choice == '3') {
			ifstream index_ifs("word_index.txt");
			if (!index_ifs.is_open()) {
				cout << "Couldn't open file.." << endl;
			}
			string line;
			while(!index_ifs.eof()) {
				getline(index_ifs, line);
				cout << line << endl;
			}
			index_ifs.close();
		}

		// Restore the index and rebuild the AVLTree and HashTable by reading from the index file 
		else if (user_choice == '4') {
			cout << "Restoring the index..." << endl;
			restore_word_index(word_tree);
			restore_author_index(author_table);
		}

		else if (user_choice == '5') {
			display_statistics(num_articles_indexed, num_words_indexed, num_stop_words, word_tree, author_table); 
		}

		else if (user_choice == '9') {
			exit(1);
		}

		else {
			cout << "Invalid option." << endl;
		}

		display_menu();
		cout << "--------> Enter your choice: ";
		cin >> user_choice;
		cout << endl;
	}


}





//===================================================================================================================================
//===================================================================================================================================

void display_menu() {
	cout << endl;
	cout << "Welcome to Mustang COIVD-19 Research Article Search Engine" << endl;
	cout << " 1. search for articles" << endl;
	cout << " 2. clear the index" << endl;
	cout << " 3. open a persistence file" << endl;
	cout << " 4. parse the corpus and populate index" << endl;
	cout << " 5. print basic statistics of the search engine" << endl;
	cout << " 9. quit" << endl;
}


// The Index processor
// This function is responsible for building Article objects, and inverted file index using data structures such as AVLTree for 
// storing unique words and HashTable for storing unique authors by parsing the dataset (json files)
void index_processor(AVLTree& word_tree, HashTable& author_table, vector<Article>& articles, 
	unordered_map<string, string>& published_date_map, unordered_map<string, string>& publication_map,
	int& num_articles_indexed, int& num_words_indexed, int& num_stop_words) {

	// Parse the metadata.csv, and create two maps, one maps "paper_id" to "published date", the other maps "paper_id" to "publication"
	parse_csv("../dataset_small/metadata-cs2341.csv", published_date_map, publication_map);

	// Parse all the .json files in the cs2341_data folder to build a vector of Article objects
	parse_directory("../dataset_small", articles);


	int num_article_parsed = articles.size();



	// Retrieve information from the Articles objects for each node to build the AVLTree and the HashTable
	//  - paper_id 
	//  - text =>  1.remove punctuations  2.lowercase  3.tokenize  4.remove stop words  5.stem  6. remove duplicates 
	string paper_id;
	string text;
	vector<string> tokens;  // tokens of text
	vector<string> authors_last;


	// Inserting stop words into an AVLTree
	AVLTree stop_words_tree;
	load_stop_words(stop_words_tree);

	// Iterate over each article
	for (int i = 0; i < articles.size(); i += 1) {

		paper_id = articles.at(i).get_id();
		text = articles.at(i).get_text();
		authors_last = articles.at(i).get_authors_last();


		// The whole text processing happens here
		remove_punctuation(text);

		to_lower(text);

		// tokenize the body text
		tokens = tokenize(text);

		// stop words removal
		vector<string> temp;  // A vector that stores words that are not stop words 
		for (int i = 0; i < tokens.size(); i += 1) {
			if (stop_words_tree.contain(tokens.at(i))) {
				num_stop_words += 1;
			}
			else {
				temp.push_back(tokens.at(i));
			}
		}

		stem_words(temp);

		remove_duplicates(temp);
		
		// Inserting words for one article into the AVLTree
		for (int j = 0; j < temp.size(); j += 1) {
	        word_tree.insert(temp.at(j), paper_id); 
	        num_words_indexed += 1;
		}


		// Inserting authors for one article into the HashTable
		for (int j = 0; j < authors_last.size(); j += 1) {
			author_table.insert(authors_last.at(j), paper_id);
		}

		num_articles_indexed += 1;
	}


	// Writing word_index to a text file
	ofstream word_index_ofs("word_index.txt");
	word_tree.write_to_file(word_index_ofs);
	word_index_ofs.close();

	// Writing author_index to a text file
	ofstream author_index_ofs("author_index.txt");
	author_table.write_to_file(author_index_ofs);
	author_index_ofs.close();

}



// The Document processor
// This function parses the metadata.csv and creates two maps, one maps "paper_id" to "published date", the other maps "paper_id" to "publication"
void parse_csv(string file_path, unordered_map<string, string>& published_date_map, unordered_map<string, string>& publication_map) {
	
	ifstream csv_inFS(file_path);  	 
	if (!csv_inFS.is_open()) {
		cout << "couldn't open the .csv file..." << endl;
	}

	CsvParser parser(csv_inFS);

	// Each row of the CSV is represented as a vector<string>
	for (auto& row : parser) {
		// Inserting key-value pairs into the maps by using [] operator 
		published_date_map[row.at(1)] = row.at(9);
		publication_map[row.at(1)] = row.at(11);

	}

}



// The Document processor
// This function parse every json file in the "cs2341_data" folder, and return a vector of Article objects 
// (11995 json files + 1 the first file is alwasy .DS_Store file + 1 csv file)
void parse_directory(string folder_path, vector<Article>& articles) {
    
  Article article;

  int filepath_count = 0;

  string dir, filepath;
  int num = 1;
  DIR *dp;
  struct dirent *dirp;
  struct stat filestat;


  dir = folder_path;

  dp = opendir(dir.c_str());

  if (dp == NULL) {
    cout << "Error(" << errno << ") opening " << dir << endl;
  }


  // To limit the numner of files to pares
  int counter = 0;

  while ((dirp = readdir( dp ))) {
    filepath = dir + "/" + dirp->d_name;

    // If the file is a directory (or is in some way invalid) we'll skip it 
    if (stat( filepath.c_str(), &filestat )) continue;
    if (S_ISDIR( filestat.st_mode ))         continue;


    // Where we acutually open the json files and do stuff 
    if (filepath[filepath.size() - 1] == 'n') {

      // parsing magic goes here...
      article = parse_json(filepath);
      articles.push_back(article);

    }
  }

  closedir( dp );
}


// The Document porcessor
// This function parses one json file, creates an Article object out of it and returns it
Article parse_json(string& file_path) {
  
	ifstream json_ifs(file_path);

	// create a json object
	json j;
	// read everything into the json object from the json file 
	json_ifs >> j;

	string paper_id;
 	string title;

	string full_name = "";
	string first_name;
	string last_name;
	vector<string> authors;
	vector<string> authors_last;

	string text;
	string whole_text = "";


	paper_id = j["paper_id"];

	if (j["metadata"]["title"] == "") {
		title = "N/A";
	}
	else {
		title = j["metadata"]["title"];
	}

	if (j["metadata"]["authors"].size() == 0) {
		full_name = "N/A";
		last_name = "N/A";
		authors.push_back(full_name);
		authors_last.push_back(last_name);
	}
	else {
		for (int i = 0; i < j["metadata"]["authors"].size(); i += 1) {
			first_name = j["metadata"]["authors"][i]["first"];
			last_name = j["metadata"]["authors"][i]["last"];  
			full_name = first_name + " " + last_name;

			authors.push_back(full_name);
			authors_last.push_back(last_name);
		}
	}
	

	for (int i = 0; i < j["body_text"].size(); i += 1) {
	  text = j["body_text"][i]["text"];
	  whole_text += text;
	  whole_text += " ";   // put a space between two strings when concatenating 
	}


	Article article(paper_id, title, authors, authors_last, whole_text);

	json_ifs.close();

	return article;
}


// referecne: https://stackoverflow.com/questions/19138983/c-remove-punctuation-from-string
void remove_punctuation(string& str) {

    for (int i = 0, len = str.size(); i < len; i += 1) { 
        // check whether parsing character is punctuation or not 
        if (ispunct(str[i]) || isdigit(str[i])) { 
            str.erase(i--, 1); 
            len = str.size(); 
        } 
    } 
}

void to_lower(string& str) {
	transform(str.begin(), str.end(), str.begin(), ::tolower);
}


vector<string> tokenize(string& str) {
	// Vector of strings to save tokens 
    vector<string> tokens; 
      
    // stringstream class ss. To treat strings as ifstream, useful for tokenizing
    stringstream ss(str); 
      
    string intermediate; 
      
    // Tokenizing delimeter ' ' 
    while(getline(ss, intermediate, ' ')) { 
		if (intermediate != "") {                // to avoid pushing back whitespaces to the vector 
			tokens.push_back(intermediate);
		} 
    } 

    return tokens;
}


void load_stop_words(AVLTree& stop_words_tree) {

	ifstream stop_word_list_inFS("stop-words-list.txt");
    string stop_word;

    while (!stop_word_list_inFS.eof()) {
    	getline(stop_word_list_inFS, stop_word);
	    stop_words_tree.insert(stop_word);
	}
	stop_word_list_inFS.close();
}



// This functions uses the C++ porter2_stemmer to stem words 
void stem_words(vector<string>& tokens) {

	for (int i = 0; i < tokens.size(); i += 1) {
		Porter2Stemmer::trim(tokens.at(i));
        Porter2Stemmer::stem(tokens.at(i));
	}
}


// unique() removes all consecutive duplicate elements
// constant space, nlog(n) run time
// reference: https://www.techiedelight.com/remove-duplicates-vector-cpp/
void remove_duplicates(vector<string>& tokens) {

	sort(tokens.begin(), tokens.end());
	tokens.erase( unique(tokens.begin(), tokens.end()) , tokens.end());
}


void display_statistics(int num_articles_indexed, int num_words_indexed, int num_stop_words, AVLTree& word_tree, HashTable& author_table) {

	cout << "Total number of articles indexed:            " << num_articles_indexed << endl;
	cout << "Total numer of words indexed:                " << num_words_indexed << endl;
	cout << "Total number of unique words indexed:        " << word_tree.get_num_unique_words() << endl;
	cout << "Total number of unique authors indexed:      " << author_table.get_num_unique_authors() << endl;
	if (num_articles_indexed != 0) {
	cout << "Average number of words indexed per article: " << num_words_indexed / num_articles_indexed << endl;
	cout << "Average number of stop words in per article: " << num_stop_words / num_articles_indexed << endl;
	}
	else {
	cout << "Average number of words indexed per article: " << 0 << endl;
	cout << "Average number of stop words in per article: " << 0 << endl;
 	}

	cout << endl << "Top 50 most frequent words => " << endl;
	// Traverse the AVLTree and store all nodes in a vector, and sort the vector by the node's data member count
	vector<Node*> words;
	words = word_tree.get_words();
	sort(words.begin(), words.end(), way_to_sort);

	for (int i = 0; i < words.size(); i += 1) {
		cout << words.at(i)->data << ": " << words.at(i)->count << endl;
		if (i == 50) {
			break;
		}
	}

}

// This function defines how to sort a vector of custom type objects. This function is passed into the
// third parameter of sort() and it will tell it to sort descendingly.
bool way_to_sort(Node*& lhs, Node*& rhs) {
    return lhs->count > rhs->count;
}


// reference: https://stackoverflow.com/questions/25505868/the-intersection-of-multiple-sorted-arrays
vector<string> intersection(vector<vector<string>>& vecs) {

    auto last_intersection = vecs[0];
    vector<string> curr_intersection;

    for (int i = 1; i < vecs.size(); ++i) {
        set_intersection(last_intersection.begin(), last_intersection.end(),
            vecs[i].begin(), vecs[i].end(),
            back_inserter(curr_intersection));
        swap(last_intersection, curr_intersection);
        curr_intersection.clear();
    }
    return last_intersection;
}


vector<string> get_union(vector<string> v) {
	remove_duplicates(v);
	return v;
}


// The Query processor and Search processor. 
// This function parses the prefix boolean query enterd by the user and find the final matches of paper ids.
void perform_search(vector<string>& final_matches, string user_query, string& temp, AVLTree& word_tree, HashTable& author_table) {

	vector<string> possible_matches; // stores final possible matches for either AND or OR (either intersection or union) 
	vector<string> exclusions;       // stores the paper ids of the search term followed by NOT
	vector<string> authors_matches;	 // stores the paper ids of the search term followed by AUTHOR


	// If an AND operator is in the query, extract the search terms, get their paper ids, and find the intersection of them
	// AND could be followed by a NOT or AUTHOR
	if (user_query.find("AND") != string::npos) {       
		
		vector<vector<string>> vecs;     // stores vectors of ids of the search terms followed by AND

		for (int i = 4; i < user_query.size(); i += 1) {
			if (user_query[i] == 'N' | user_query[i] == 'A') {
				break;
			}
			temp += user_query[i];
		}
	
		vector<string> search_terms = tokenize(temp);

		// Get the vector of paper_ids for each search term from the index, store them in vecs 
		for (int i = 0; i < search_terms.size(); i += 1) {
			vector<string> id_list;
			word_tree.get_paper_ids(search_terms.at(i), id_list);
			sort(id_list.begin(), id_list.end());  // intersection fucntion only works for sorted arrays 
			vecs.push_back(id_list);
		}

		possible_matches = intersection(vecs);
		
	}

	// If an OR operator is in the query, extract the search terms, get their paper ids, and find the union of them
	// OR could be followed by a NOT or AUTHOR. Make sure the fins() doesn't mistake the OR in AUTHOR for OR
	if (user_query.find("OR") == 0) {

		vector<string> temp_union; 		 // stores all paper ids of the search terms followed by OR

		for (int i = 3; i < user_query.size(); i += 1) {
			if (user_query[i] == 'N' | user_query[i] == 'A') {
				break;
			}
			temp += user_query[i];
		}

		vector<string> search_terms = tokenize(temp);

		// Get the all paper_ids for each search term from the index, store them in temp_union 
		for (int i = 0; i < search_terms.size(); i += 1) {
			vector<string> id_list;
			word_tree.get_paper_ids(search_terms.at(i), id_list);

			// Insert all the paper ids into one vector of string and remove duplicates to find the union the paper ids
			temp_union.insert(temp_union.end(), id_list.begin(), id_list.end());
		}
		// removing duplicates to make it a union
		remove_duplicates(temp_union); 
		possible_matches = temp_union;

	}

	// There is no AND or OR operator in the query, the one search term could be followed by a NOT or AUTHOR
	if (possible_matches.empty()) {

		for (int i = 0; i < user_query.size(); i += 1) {
			if (user_query[i] == ' ') {
				break;
			}
			temp += user_query[i];
		}

		word_tree.get_paper_ids(temp, possible_matches);

	}

	// NOT could be followed by a AUTHOR.
	if (user_query.find("NOT") != string::npos) {     
		int NOT_pos = user_query.find("NOT");
		string not_term = "";
		for (int i = NOT_pos + 4; i < user_query.size(); i += 1) {
			if (user_query[i] == ' ') {  // Only one search term, so break if it his a white space
				break;
			}
			not_term += user_query[i];
		}

		word_tree.get_paper_ids(not_term, exclusions);
	}

	// AUTHOR could only be followed by a last name
	if (user_query.find("AUTHOR") != string::npos) {  
		int AUTHOR_pos = user_query.find("AUTHOR");
		string author_term = "";
		for (int i = AUTHOR_pos + 7; i < user_query.size(); i += 1) {
			author_term += user_query[i];
		}

		// Look up the search term in the hash table
		authors_matches = author_table.get_paper_ids(author_term);
	}



	// Filter out the paper ids in exlusions from possible matches
	for (int i = 0; i < possible_matches.size(); i += 1) {
		// If the exclusions is empty, the inner for loop will just never exceute
		for (int j = 0; j < exclusions.size(); j += 1) {
			if (possible_matches.at(i) == exclusions.at(j)) {
				possible_matches.erase(possible_matches.begin() + i);
				// decrement i by 1 so the next element won't be skipped since erase will move all remaining elements down by 1 index to fill the hole
				i -= 1;
				break;
			}
		}
	}

	
	// Find the intersection of possible_matches and authors_matches, only if the authors_matches is not empty
	if (!authors_matches.empty()) {
		vector<vector<string>> vecs; 
		vecs.push_back(possible_matches);
		vecs.push_back(authors_matches);
		final_matches = intersection(vecs);
	}
	else {
		final_matches = possible_matches;
	}

}



// The Ranking processor
// This function ranks the final matches by their relevancy scores, and finds the top 15 ranked results
void rank_results(vector<string>& final_matches, vector<Article>& articles, string& temp, vector<string>& top15_results) {

	// There will be one map for each search term. Each map will store all the final matches (paper ids) as the keys, and the number of times 
	// that particular search term appeared in those paper id as the values

	// In terms of ranking, a search term in each of the final matches (articles) will have a relevancy score, the score is calculated by dividing the 
	// number of times that search term appeared in the article by the size of the article's body text. 

	// To make sure that when a search term appeared more frequently in a long article, doesn't mean that article is more important 

	vector< unordered_map<string, double> > maps;
	vector<string> search_terms = tokenize(temp);

	AVLTree stop_words_tree;
	load_stop_words(stop_words_tree);


	// Initialize maps. One map for each search term
	for (int i = 0; i < search_terms.size(); i += 1) {
		unordered_map<string, double> word_count_map;

		for (int j = 0; j < final_matches.size(); j += 1) {

			for (int k = 0; k < articles.size(); k += 1) {

				// Get the text of one final match, find out the how many times the search term apppeared in the text
				// Insert the final match paper id as the key and its relevancy_score as the value into the map for that particular search term
				// 
				if (final_matches.at(j) == articles.at(k).get_id()) {
					// Get the body text from the right Article object and process it
					string text = articles.at(k).get_text(); 
					//to_lower(text);
					vector<string> temp = tokenize(text);
					// stop words removal
					vector<string> tokens;  // A vector that stores words that are not stop words 
					for (int i = 0; i < temp.size(); i += 1) {
						if (!stop_words_tree.contain(temp.at(i))) {
							tokens.push_back(temp.at(i));
						}
					}
					//stem_words(temp);

					// Get the count of the search term in that body text
					double word_count = count(tokens.begin(), tokens.end(), search_terms.at(i));
					double relev_score = (word_count / tokens.size());
					// Insert the key-value pair, map[paper_id] = relev_score 
					word_count_map[final_matches.at(j)] = relev_score;
					break;
				}
			}
		}

		maps.push_back(word_count_map);
	}


	// Add up the relevancy scores in all the map for the same paper_id, store the sums in the final map as the values, and the corresponding 
	// paper ids as the keys 
	unordered_map<string, double> final_map;

	for (int i = 0; i < final_matches.size(); i += 1) {
		for (int j = 0; j < maps.size(); j += 1) {
			final_map[final_matches.at(i)] += maps.at(j)[final_matches.at(i)];
		}
	}


	// To find the top 15 largest values in the final map, iterate through the map 15 times 
	// For each iteration, find the max value, store its key in a vector, delete the key, find the next max value
	// The max value is pushed back to the vector first, and the second max value, and so on, therefore, the 15 paper ids will be  
	// sorted in decreasing order by their values in the vector 
	for (int i = 0; i < final_map.size(); i += 1) {

		double curr_max = 0;
		string curr_key;
		for (auto x : final_map) {
			if (x.second > curr_max) {
				curr_max = x.second;
				curr_key = x.first;
			}
		}

		top15_results.push_back(curr_key);
		final_map.erase(curr_key);

		if (i == 14) {
			break;
		}
	}
}





// This function formats nd displays the top 15 ranked articles and lets the user open an article
void display_results(vector<string>& top15_results, vector<Article>& articles, 
	unordered_map<string, string>& published_date_map, unordered_map<string, string>& publication_map) {


	// The text_map stores the texts of the top 15 ranked articles, in case the user wishes to open one of the 15 articles
	unordered_map<int, string> text_map; 
	int text_num = 1;

	for (int i = 0; i < top15_results.size(); i += 1) {
		for (int j = 0; j < articles.size(); j += 1) {
			if (top15_results.at(i) == articles.at(j).get_id()) {
				cout << text_num << "." << endl;                    			  // label each article with its ranking number
				cout << "Title:          " << articles.at(j).get_title() << endl;			      
				// Display the first 3 authors
				cout << "Author:         ";
				if (articles.at(j).get_authors().at(0) == "N/A") {
					cout << "N/A" << endl;
				}
				else {
					for (int k = 0; k < articles.at(j).get_authors().size(); k += 1) {
						if (k == 2) {
							cout << articles.at(j).get_authors().at(k) << "..." << endl;
							break;
						}
						cout << articles.at(j).get_authors().at(k) << ", ";
					}
				}

				cout << "Date published: " << published_date_map[top15_results.at(i)] << endl;            
				cout << "Publication:    " << publication_map[top15_results.at(i)] << endl << endl;       
				// store the text for each of the 15 articles in a map as the values, and its ranking number (1~15) as the keys 
				text_map[text_num] = articles.at(j).get_text();  
				text_num += 1;  
				break;
			}
		}

	}

	// Allow user to choose an article to display the first 300 words of the text
	cout << "Enter a number between 1 and 15 if you wish to open an article (enter 0 to go back to menu): ";
	int user_choice;
	cin >> user_choice;

	while (true) {
		if (user_choice >= 1 && user_choice <= 15) {
			// Tokenize the full text and append the first 300 words to an empty string 
			string short_text = "";
			// Get the right text from the text_map according to the user choice 
			string full_text = text_map[user_choice];
			vector<string> tokens;
			tokens = tokenize(full_text);
			for (int i = 0; i < tokens.size(); i += 1) {
				short_text += tokens[i] + " ";
				if (i == 300) {break; }
			}
			cout << endl << short_text << endl << endl;
		}

		else if (user_choice == 0) {
			break;
		}

		else {
			cout << "Invalid input." << endl;
		}

		cout << "Enter a number 1 ~ 15 if you wish to open an article (enter 0 to go back to menu): ";
		cin >> user_choice;
	}

}


void restore_word_index(AVLTree& word_tree) {

	ifstream index_ifs("word_index.txt");
	if (!index_ifs.is_open()) {
		cout << "Couldn't open file.." << endl;
	}

	// In the index file, every word is followed by a list of paper ids
	// Read in the first line, which is a word, and read in the second line, which is its first paper id
	// While the next line read in has size 40 (paper id size), keep inserting the same word with different ids into the AVLTree
	// until the next word is read in, which we can assume it will not be size 40
	string word;
	string paper_id;

	getline(index_ifs, word);

	while (!index_ifs.eof()) {
		getline(index_ifs, paper_id);

		while (paper_id.size() == 40) {
			word_tree.insert(word, paper_id);
			getline(index_ifs, paper_id);
			if (paper_id.size() != 40) {
				word = paper_id;
				break;
			}
		}
	}
}


void restore_author_index(HashTable& author_table) {

	ifstream index_ifs("author_index.txt");
	if (!index_ifs.is_open()) {
		cout << "Couldn't open file.." << endl;
	}

	// Same approach as for restoring word_index
	string author;
	string paper_id;

	getline(index_ifs, author);

	while (!index_ifs.eof()) {
		getline(index_ifs, paper_id);

		if (paper_id.size() != 40) {
			author = paper_id;
		}
		else {
			while (paper_id.size() == 40) {
				author_table.insert(author, paper_id);
				getline(index_ifs, paper_id);
				if (paper_id.size() != 40) {
					author = paper_id;
					break;
				}
			}
		}
	}
}


#endif

