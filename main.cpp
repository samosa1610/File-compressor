#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <bitset>
#include <sys/stat.h>
using namespace std;

struct Node {
    char ch;
    int freq;
    Node *left, *right;

    Node(char c, int f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
};

struct Compare {
    bool operator()(Node* a, Node* b) {
        return a->freq > b->freq;
    }
};

void buildCodes(Node* root, string str, unordered_map<char, string> &huffmanCode) {
    if (!root) return;
    if (!root->left && !root->right)
        huffmanCode[root->ch] = str;
    buildCodes(root->left, str + "0", huffmanCode);
    buildCodes(root->right, str + "1", huffmanCode);
}

// stores the hoffman tree in preorder traversal (needed during decompression)
void serializeTree(Node* root, string &output) {
    if (!root) return;
    if (!root->left && !root->right) {
        output += "1";
        output += root->ch;
        return;
    }
    output += "0";
    serializeTree(root->left, output);
    serializeTree(root->right, output);
}

Node* deserializeTree(string &data, int &index) {
    if (index >= data.size()) return nullptr;
    if (data[index] == '1') {
        char ch = data[++index];
        ++index;
        return new Node(ch, 0);
    }
    ++index;
    Node* left = deserializeTree(data, index);
    Node* right = deserializeTree(data, index);
    Node* node = new Node('\0', 0);
    node->left = left;
    node->right = right;
    return node;
}

string compressText(const string &text, unordered_map<char, string> &huffmanCode) {
    string encoded = "";
    for (char ch : text)
        encoded += huffmanCode[ch];
    return encoded;
}

void writeBinaryFile(const string &binaryStr, const string &filename) {
    ofstream out(filename, ios::binary);
    for (size_t i = 0; i < binaryStr.size(); i += 8) {
        bitset<8> bits(binaryStr.substr(i, 8).append(8 - (binaryStr.size() - i >= 8 ? 0 : binaryStr.size() - i), '0'));
        out.put((unsigned char)bits.to_ulong());
    }
    out.close();
}

string readBinaryFile(const string &filename, size_t bitLength) {
    ifstream in(filename, ios::binary);
    string result;
    char byte;
    while (in.get(byte)) {
        bitset<8> bits(byte);
        result += bits.to_string();
    }
    in.close();
    return result.substr(0, bitLength); // cut to original bit length
}

string decompress(const string &encodedStr, Node* root) {
    string decoded = "";
    Node* curr = root;
    for (char bit : encodedStr) {
        curr = (bit == '0') ? curr->left : curr->right;
        if (!curr->left && !curr->right) {
            decoded += curr->ch;
            curr = root;
        }
    }
    return decoded;
}

long long fileSize(const string &filename) {
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

int main() {
    ifstream inFile("input.txt");
    ofstream treeFile("tree.txt");
    ofstream outDecoded("decompressed.txt");

    if (!inFile.is_open()) {
        cerr << "Error: input.txt not found.\n";
        return 1;
    }

    string text((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    unordered_map<char, int> freq;
    for (char ch : text) freq[ch]++;

    priority_queue<Node*, vector<Node*>, Compare> pq;
    for (auto &pair : freq)
        pq.push(new Node(pair.first, pair.second));

    while (pq.size() > 1) {
        Node* l = pq.top(); pq.pop();
        Node* r = pq.top(); pq.pop();
        Node* m = new Node('\0', l->freq + r->freq);
        m->left = l; m->right = r;
        pq.push(m);
    }

    Node* root = pq.top();
    unordered_map<char, string> huffmanCode;
    buildCodes(root, "", huffmanCode);

    string encoded = compressText(text, huffmanCode);
    writeBinaryFile(encoded, "compressed.bin");

    string serializedTree;
    serializeTree(root, serializedTree);
    treeFile << serializedTree;

    int idx = 0;
    Node* restoredRoot = deserializeTree(serializedTree, idx);
    string decoded = decompress(readBinaryFile("compressed.bin", encoded.size()), restoredRoot);
    outDecoded << decoded;

    // Output stats
    cout << "Original file size: " << fileSize("input.txt") << " bytes\n";
    cout << "Compressed file size: " << fileSize("compressed.bin") << " bytes\n";
    cout << "Compression Ratio: " << (double(fileSize("compressed.bin")) / fileSize("input.txt")) << "\n";
    cout << "Decompression successful: " << boolalpha << (text == decoded) << endl;

    return 0;
}
