#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

using namespace std;

// Each Node represents a unique word from the articles
struct Node {
    
    string data;
    vector<string> id_list;
    // The count of this word in each article. The key is a paper id and the value is the count of this word in that paper id 
    // For relevancy ranking 
    //unordered_map<string, int> word_count_map;

    // The total count of this word in the whole dataset 
    // For finding the top 50 frequent words indexed
    int count = 1;

    Node* left;
    Node* right;
    int height;


    

    Node(string data, Node* left, Node* right, int height = 0) { 
    	this->data = data;
    	this->left = left;
    	this->right = right;
    	this->height = height;
    }

};



#endif