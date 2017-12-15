#include <iostream>
#include <iomanip>
#include "Parser.h"
#include <math.h>
using namespace std;

int maxHeight(std::shared_ptr<SyntaxTree>  node)
{
    return (node == nullptr) ? 0 : 1 + std::max(maxHeight(node->left), maxHeight(node->right));
}

void printBranches(int branchLen, int nodeSpaceLen, int startLen, int nodesInThisLevel, const deque<std::shared_ptr<SyntaxTree> >& nodesQueue, ostream& out) {
    deque<std::shared_ptr<SyntaxTree> >::const_iterator iter = nodesQueue.begin();
    for (int i = 0; i < nodesInThisLevel / 2; i++) {
        out << ((i == 0) ? setw(startLen-1) : setw(nodeSpaceLen-2)) << "" << ((*iter++) ? "/" : " ");
        out << setw(2*branchLen+2) << "" << ((*iter++) ? "\\" : " ");
    }
    out << endl;
}

// Print the branches and node (eg, ___10___ )
void printNodes(int branchLen, int nodeSpaceLen, int startLen, int nodesInThisLevel, const deque<std::shared_ptr<SyntaxTree> >& nodesQueue, ostream& out) {
    deque<std::shared_ptr<SyntaxTree> >::const_iterator iter = nodesQueue.begin();
    for (int i = 0; i < nodesInThisLevel; i++, iter++) {
        out << ((i == 0) ? setw(startLen) : setw(nodeSpaceLen)) << "" << ((*iter && (*iter)->left) ? setfill('_') : setfill(' '));
        out << setw(branchLen+2) << ((*iter) ? (*iter)->content : "");
        out << ((*iter && (*iter)->right) ? setfill('_') : setfill(' ')) << setw(branchLen) << "" << setfill(' ');
    }
    out << endl;
}

// Print the leaves only (just for the bottom row)
void printLeaves(int indentSpace, int level, int nodesInThisLevel, const deque<std::shared_ptr<SyntaxTree> >& nodesQueue, ostream& out) {
    deque<std::shared_ptr<SyntaxTree> >::const_iterator iter = nodesQueue.begin();
    for (int i = 0; i < nodesInThisLevel; i++, iter++) {
        out << ((i == 0) ? setw(indentSpace+2) : setw(2*level+2)) << ((*iter) ? (*iter)->content : "");
    }
    out << endl;
}

// Pretty formatting of a binary tree to the output stream
// @ param
// level  Control how wide you want the tree to sparse (eg, level 1 has the minimum space between nodes, while level 2 has a larger space between nodes)
// indentSpace  Change this to add some indent space to the left (eg, indentSpace of 0 means the lowest level of the left node will stick to the left margin)
void printPretty(std::shared_ptr<SyntaxTree>  root, int level, int indentSpace, ostream& out) {
    int h = maxHeight(root);
    int nodesInThisLevel = 1;

    int branchLen = 2*((int)pow(2.0,h)-1) - (3-level)*(int)pow(2.0,h-1);  // eq of the length of branch for each node of each level
    int nodeSpaceLen = 2 + (level+1)*(int)pow(2.0,h);  // distance between left neighbor node's right arm and right neighbor node's left arm
    int startLen = branchLen + (3-level) + indentSpace;  // starting space to the first node to print of each level (for the left most node of each level only)

    std::deque<std::shared_ptr<SyntaxTree> > nodesQueue;
    nodesQueue.push_back(root);
    for (int r = 1; r < h; r++) {
        printBranches(branchLen, nodeSpaceLen, startLen, nodesInThisLevel, nodesQueue, out);
        branchLen = branchLen/2 - 1;
        nodeSpaceLen = nodeSpaceLen/2 + 1;
        startLen = branchLen + (3-level) + indentSpace;
        printNodes(branchLen, nodeSpaceLen, startLen, nodesInThisLevel, nodesQueue, out);

        for (int i = 0; i < nodesInThisLevel; i++) {
            const std::shared_ptr<SyntaxTree> &currNode = nodesQueue.front();
            nodesQueue.pop_front();
            if (currNode) {
                nodesQueue.push_back(currNode->left);
                nodesQueue.push_back(currNode->right);
            } else {
                nodesQueue.push_back(NULL);
                nodesQueue.push_back(NULL);
            }
        }
        nodesInThisLevel *= 2;
    }
    printBranches(branchLen, nodeSpaceLen, startLen, nodesInThisLevel, nodesQueue, out);
    printLeaves(indentSpace, level, nodesInThisLevel, nodesQueue, out);
}


int main() {
    Parser p;
    std::shared_ptr<SyntaxTree> root = p.parse(" (a || b) && (b || c) || d");
    printPretty(root, 2, 4, std::cout);
    std::shared_ptr<SyntaxTree> r1 = p.parse("(a && b) < f || (c;d)");
    printPretty(r1, 2, 4, std::cout);

    std::shared_ptr<SyntaxTree> r2 = p.parse("a;b;c;d;e > file");
    printPretty(r2, 0, 4, std::cout);
    return 0;
}