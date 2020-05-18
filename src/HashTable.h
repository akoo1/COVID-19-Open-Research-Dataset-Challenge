#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <iostream>
#include <vector>
#include <string> 
#include <functional>

using namespace std;


class HashTable { 
  
private:

	struct HashNode{
		string author;
		vector<string> id_list;

		HashNode(string key) {
			author = key;
		}
	};


    // A vector of buckets  
    vector<vector<HashNode>> hash_table; 
    // Number of buckets
    int size;
    int num_unique_authors = 0;


public: 

    HashTable(int size) { 
        hash_table = vector<vector<HashNode>>(size); 
        this->size = size; 
    }


    int get_hash_index(string key) {
    	hash<string> h;
    	int hash_value = h(key) % size;

        return hash_value; 
    } 
  
    void insert(string author, string paper_id) { 
    
        // inserting the element according to hash index 
    	int idx = get_hash_index(author);
  
    	// Find the bucket with same index (hash value), scan for the same key (author name)
    	// if found, just push back the paper_id, if not found, create a new HashNode for the key (author name) and push back to the current bucket
        for (int i = 0; i < hash_table.at(idx).size(); i += 1) {
    
        	if (author == hash_table.at(idx).at(i).author) {
        		hash_table.at(idx).at(i).id_list.push_back(paper_id);
        		return;
        	}
        }

        HashNode n(author);
        hash_table.at(idx).push_back(n);
        num_unique_authors += 1;
    } 


    vector<string> get_paper_ids(string author) {

    	// Find the bucket with the same index (hash value)
        int idx = get_hash_index(author); 

        // Scan the bucket (vector) to find the HashNode with the same author name, and return its id_list  
        for (int i = 0; i < hash_table.at(idx).size(); i += 1) { 
            if (author == hash_table.at(idx).at(i).author) { 
            	return hash_table.at(idx).at(i).id_list;
            }
        }
        cout << "author not found..." << endl;
        vector<string> v;
        return v;
    }
  

    void remove(string author) { 
		// Find the bucket with the same index (hash value)
        int idx = get_hash_index(author); 

        for (int i = 0; i < hash_table.at(idx).size(); i += 1) { 
            if (author == hash_table.at(idx).at(i).author) { 
            	hash_table[idx].erase(hash_table[idx].begin() + i);
            }
        }

        cout << "author key not found..." << endl;
    }

    void clear_table() {
    	
        for (int i = 0; i < hash_table.size(); i += 1) {
            // The clear_table() only removes the elements from the vector
            hash_table.at(i).clear();
            // This frees up the memory that was allocated to the elements 
            hash_table.shrink_to_fit();
        }

        num_unique_authors = 0; // reset the counter also
    }
 


    void print_table() { 
   
        for (int i = 0; i < hash_table.size(); i += 1) { 
            cout << i; 
            for (int j = 0; j < hash_table.at(i).size(); j += 1) 
                cout << " -> " << hash_table[i][j].author; 
            cout << endl; 
        } 
    }

    int get_num_unique_authors() {
        return num_unique_authors;
    }


    void write_to_file(ofstream& index_ofs) {

        for (int i = 0; i < hash_table.size(); i += 1) { 
            for (int j = 0; j < hash_table.at(i).size(); j += 1) {
                // write the author to the file
                index_ofs << hash_table.at(i).at(j).author << endl;
                /// write the id_list to the file right after author
                for (int k = 0; k < hash_table.at(i).at(j).id_list.size(); k += 1) {
                    index_ofs << hash_table.at(i).at(j).id_list.at(k) << endl;
                }
            }
        }
    }


}; 


#endif


