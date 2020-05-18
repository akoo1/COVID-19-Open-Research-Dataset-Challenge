#ifndef AVLTREE_H
#define AVLTREE_H

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>

#include "Node.h"

using namespace std;


class AVLTree {

private:
	Node* root = nullptr;
	int num_unique_words = 0;

	// This vector stores all the nodes in the AVLTree
	vector<Node*> words;


	// A helper function to access the height to avoid seg fault because the node might be a nullptr
	int get_height(Node* curr) {
		if (curr == nullptr) {
			return -1;
		}
		else {
			return curr->height;
		}
	}

	// For building a words index
	void insert(string& new_data, string paper_id, Node*& curr) {

		if (curr == nullptr) {
			curr = new Node(new_data, nullptr, nullptr);
			curr->id_list.push_back(paper_id);
			words.push_back(curr);
			// Only increment word count when creating a new word to avoid double counting for duplicates
			num_unique_words += 1; 
		}

		else if (new_data < curr->data) {
			insert(new_data, paper_id, curr->left);    // recurrsive call
			if (get_height(curr->left) - get_height(curr->right) == 2) {
				if (new_data < curr->left->data) {
					rotate_with_left_child(curr);      // Case 1 rotation (LeftLeft rotation)
				}
				else {
					double_with_left_child(curr);	   // Case 2 rotation (RightLeft rotation)
				}
			}
		}

		else if (new_data > curr->data) {
			insert(new_data, paper_id, curr->right);	   // recurrsive call
			if (get_height(curr->right) - get_height(curr->left) == 2) {
				if (new_data > curr->right->data) {
					rotate_with_right_child(curr);   // Case 4 rotation (RightRight rotation)
				}
				else {
					double_with_right_child(curr);   // Case 3 rotation (LeftRight rotation)
				}
			}
		}

		// The same word already exists, only push_back its paper id
		else if (new_data == curr->data) {
			curr->id_list.push_back(paper_id);
			curr->count += 1;
			return;
		}

		// Finally update the height for every node
		curr->height = max(get_height(curr->left), get_height(curr->right)) + 1;
	}


	// For building a stop word tree
	void insert(string& new_data, Node*& curr) {

		if (curr == nullptr) {
			curr = new Node(new_data, nullptr, nullptr);
		}

		else if (new_data < curr->data) {
			insert(new_data, curr->left);   	// recurrsive call
			if (get_height(curr->left) - get_height(curr->right) == 2) {
				if (new_data < curr->left->data) {
					rotate_with_left_child(curr);     // Case 1 rotation (LeftLeft rotation)
				}
				else {
					double_with_left_child(curr);	   // Case 2 rotation (RightLeft rotation)
				}
			}
		}

		else if (new_data > curr->data) {
			insert(new_data, curr->right);	   // recurrsive call
			if (get_height(curr->right) - get_height(curr->left) == 2) {
				if (new_data > curr->right->data) {
					rotate_with_right_child(curr);   // Case 4 rotation (RightRight rotation)
				}
				else {
					double_with_right_child(curr);   // Case 3 rotation (LeftRight rotation)
				}
			}
		}

		else if (new_data == curr->data) {
			return;
		}

		curr->height = max(get_height(curr->left), get_height(curr->right)) + 1;
	}



	// Case 1 (LL)
	void rotate_with_left_child(Node*& k2) {
		Node* k1 = k2->left;
		k2->left = k1->right;
		k1->right = k2;
		k2->height = max(get_height(k2->left), get_height(k2->right)) + 1;
		k1->height = max(get_height(k1->left), get_height(k1->right)) + 1;
		k2 = k1;
	}
	// Case 4 (RR)
	void rotate_with_right_child(Node*& k2) {
		Node* k1 = k2->right;
		k2->right = k1->left;
		k1->left = k2;
		k2->height = max(get_height(k2->left), get_height(k2->right)) + 1;
		k1->height = max(get_height(k1->left), get_height(k1->right)) + 1;
		k2 = k1;
	}



	// Case 2 (RL)
	void double_with_left_child(Node*& k3) {
		rotate_with_right_child(k3->left);
		rotate_with_left_child(k3);
	}

	// Case 3 (LR)
	void double_with_right_child(Node*& k3) {
		rotate_with_left_child(k3->right);
		rotate_with_right_child(k3);
	}


	// Print all the words and the papper_ids each word appeared in
	void inorderTraversal(Node* curr) {
		if (curr != nullptr) {
			inorderTraversal(curr->left);

			cout << curr->data << endl;
			cout << "paper_ids: " << endl;
			for (int i = 0; i < curr->id_list.size(); i += 1) {
				cout << curr->id_list.at(i) << endl;
			}

			inorderTraversal(curr->right);
		}
	}


	void get_paper_ids(string search_term, vector<string>& paper_ids, Node* curr) {
		if (curr == nullptr) {
			cout << "search term not found." << endl << endl;
		}

		else if (search_term < curr->data) {
			get_paper_ids(search_term, paper_ids, curr->left);
		}
		else if (search_term > curr->data) {
			get_paper_ids(search_term, paper_ids, curr->right);
		}
		else if (search_term == curr->data) {
			for (int i = 0; i < curr->id_list.size(); i += 1) {
				paper_ids.push_back(curr->id_list.at(i));

			}
		}
	}


	bool contain(string& word, Node* curr) {
		if (curr == nullptr) {
			return false;
		}
		else if (word < curr->data) {
			return contain(word, curr->left);
		}
		else if (word > curr->data) {
			return contain(word, curr->right);
		}
		else if (word == curr->data) {
			return true;
		}
		else {
			return false;
		}
	}


	// This function uses Post-order Traversal to clear the tree (before deleting the parent node we should delete its children nodes first)
	void clear_tree(Node* curr) {
		if (curr != nullptr) {
			clear_tree(curr->left);
			clear_tree(curr->right);
			//cout << "deleting node: " << curr->data << endl;
			delete curr;
			curr->left = nullptr;
			curr->right = nullptr;
		}
		root = nullptr;
		words.clear();
		words.shrink_to_fit();

		num_unique_words = 0; // clear the counter too
	}


	// This function uses In-order Traversal to wirte the AVLTree to a textfile 
	void write_to_file(Node* curr, ofstream& index_ofs) {
		
		if (curr != nullptr) {

			write_to_file(curr->left, index_ofs);

			if (curr->data != "") {
				index_ofs << curr->data << endl;

				for (int i = 0; i < curr->id_list.size(); i += 1) {
					index_ofs << curr->id_list.at(i) << endl;
				}
			}

			write_to_file(curr->right, index_ofs);
		}
	}




public:

	AVLTree() {
		root = nullptr;
	}

	void insert(string data, string paper_id) {
		insert(data, paper_id, root); 				// calls the private version of the insert function, restrict the public interface to the user
	}

	// For stop words
	void insert(string data) {
		insert(data, root);
	}


	void inorderTraversal() {
		inorderTraversal(root);			  		    // calls the private version of the traversal function
	}


	void get_paper_ids(string search_term, vector<string>& paper_ids) {
		get_paper_ids(search_term, paper_ids, root); 			// calls the private version of the get_paper_ids function
	}


	int get_num_unique_words() {
		return num_unique_words;
	}


	// This function calls set_words() to initialize data member vector<Node*> words, and return it 
	vector<Node*> get_words() {
		return words;
	}


	bool contain(string word) {
		return contain(word, root);
	}


	void clear_tree() {
		clear_tree(root);
	}

	void write_to_file(ofstream& index_ofs) {
		write_to_file(root, index_ofs);
	}

};











#endif


