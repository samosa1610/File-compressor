#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <bitset>
#include <string>

using namespace std;

struct Node {
    unsigned char byte;
    int freq;
    Node* left;
    Node* right;

    Node(unsigned char b, int f) : byte(b), freq(f), left(nullptr), right(nullptr) {}
};

struct Compare {
    bool operator()(Node* a, Node* b) {
        return a->freq > b->freq;
    }
};

unordered_map<unsigned char, string> huffmanCodes;
unordered_map<string, unsigned char> reverseCodes;

void buildCodes(Node* root, string str) {
    if (!root) return;
    if (!root->left && !root->right) {
        huffmanCodes[root->byte] = str;
        reverseCodes[str] = root->byte;
    }
    buildCodes(root->left, str + "0");
    buildCodes(root->right, str + "1");
}

void writeTree(Node* root, ostream& out) {
    if (!root) return;
    if (!root->left && !root->right) {
        out.put('1');
        out.put(root->byte);
    } else {
        out.put('0');
        writeTree(root->left, out);
        writeTree(root->right, out);
    }
}

Node* readTree(istream& in) {
    char flag = in.get();
    if (flag == '1') {
        unsigned char byte = in.get();
        return new Node(byte, 0);
    } else {
        Node* node = new Node(0, 0);
        node->left = readTree(in);
        node->right = readTree(in);
        return node;
    }
}

void compressFile(const string& inFile, const string& outFile) {
    ifstream fin(inFile, ios::binary);
    ofstream fout(outFile, ios::binary);

    unordered_map<unsigned char, int> freq;
    vector<unsigned char> data((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
    for (unsigned char b : data) freq[b]++;

    priority_queue<Node*, vector<Node*>, Compare> pq;
    for (auto& p : freq) pq.push(new Node(p.first, p.second));

    while (pq.size() > 1) {
        Node* l = pq.top(); pq.pop();
        Node* r = pq.top(); pq.pop();
        Node* parent = new Node(0, l->freq + r->freq);
        parent->left = l; parent->right = r;
        pq.push(parent);
    }

    Node* root = pq.top();
    buildCodes(root, "");

    // write tree
    writeTree(root, fout);
    fout.put('|'); // delimiter for tree

    // write encoded data
    string encoded;
    for (unsigned char b : data) encoded += huffmanCodes[b];

    while (encoded.size() % 8 != 0) encoded += '0';

    for (size_t i = 0; i < encoded.size(); i += 8) {
        bitset<8> bits(encoded.substr(i, 8));
        fout.put((unsigned char)bits.to_ulong());
    }

    cout << "Compression complete: " << inFile << " -> " << outFile << endl;
}

void decompressFile(const string& inFile, const string& outFile) {
    ifstream fin(inFile, ios::binary);
    ofstream fout(outFile, ios::binary);

    Node* root = readTree(fin);
    if (fin.get() != '|') {
        cerr << "Invalid file format!\n";
        return;
    }

    // read remaining file as bits
    string bits;
    char byte;
    while (fin.get(byte)) {
        bitset<8> b((unsigned char)byte);
        bits += b.to_string();
    }

    Node* curr = root;
    for (char bit : bits) {
        if (bit == '0') curr = curr->left;
        else curr = curr->right;

        if (!curr->left && !curr->right) {
            fout.put(curr->byte);
            curr = root;
        }
    }

    cout << "Decompression complete: " << inFile << " -> " << outFile << endl;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cout << "Usage:\n";
        cout << "  compressor -c input.file output.huff\n";
        cout << "  compressor -d input.huff output.file\n";
        return 1;
    }

    string mode = argv[1], inFile = argv[2], outFile = argv[3];

    if (mode == "-c") compressFile(inFile, outFile);
    else if (mode == "-d") decompressFile(inFile, outFile);
    else cout << "Invalid option.\n";

    return 0;
}
