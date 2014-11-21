#include "lexer.h"
#include "parser.h"

struct TestNode {};

struct TestBuilder {
  static TestNode* makeToplevel() {
    printf("makeTopLevel\n");
    return nullptr;
  }
  static void appendStatement(TestNode* block, TestNode* statement) {
    printf("appendStatement\n");
  }
  static TestNode* makeEmpty() {
    printf("makeEmpty\n");
    return nullptr;
  }
  static TestNode* makeCall(TestNode *target, TestNode *args) {
    printf("makeCall\n");
    return nullptr;
  }
  static TestNode* makeList() {
    printf("makeList\n");
    return nullptr;
  }
  static int getSize(TestNode*) {
    printf("getSize\n");
    return 0;
  }
  static TestNode* makeIf(TestNode* cond, TestNode* ifTrue, TestNode* ifFalse) {
    printf("makeIf\n");
    return nullptr;
  }
  static void append(TestNode* node, TestNode* element) {
    printf("append\n");
  }
  static TestNode* makeUndefined() {
    printf("makeUndefined\n");
    return nullptr;
  }
  static TestNode* makeNull() {
    printf("makeNull\n");
    return nullptr;
  }
  static TestNode* makeWhile(TestNode* cond, TestNode* body) {
    printf("makeWhile\n");
    return nullptr;
  }
  static TestNode* makeDo(TestNode* cond, TestNode* body) {
    printf("makeDo\n");
    return nullptr;
  }
  static TestNode* makeFor(TestNode* init, TestNode* cond, TestNode* inc, TestNode* body) {
    printf("makeFor\n");
    return nullptr;
  }
  static TestNode* makeForIn(bool hasDecl, TestNode* var, TestNode* in, TestNode* body) {
    printf("makeFor\n");
    return nullptr;
  }
  static TestNode* makeSwitch(TestNode* cond) {
    printf("makeSwitch\n");
    return nullptr;
  }
  static TestNode* appendSwitchCase(TestNode* switch_, TestNode* case_) {
    printf("appendSwitchCase\n");
    return nullptr;
  }
  static TestNode* appendSwitchStatement(TestNode* switch_, TestNode* statement) {
    printf("appendSwitchStatement\n");
    return nullptr;
  }
  static TestNode* appendSwitchDefault(TestNode* switch_) {
    printf("appendSwitchDefault\n");
    return nullptr;
  }
  static bool isBinary(TestNode* node, std::string op) {
    printf("isBinary\n");
    return true;
  }
  static bool isName(TestNode* node) {
    printf("isName\n");
    return true;
  }
  static TestNode* makeBreak(std::string label) {
    printf("makeBreak\n");
    return nullptr;
  }
  static TestNode* makeContinue(std::string label) {
    printf("makeContinue\n");
    return nullptr;
  }
  static TestNode* makeReturn(TestNode* value) {
    printf("makeReturn\n");
    return nullptr;
  }
  static TestNode* makeThrow(TestNode* value) {
    printf("makeThrow\n");
    return nullptr;
  }
  static TestNode* makeTry(TestNode* tryStmt, TestNode* catchIdent, TestNode* catchStmt, TestNode* finallyStmt) {
    printf("makeTry\n");
    return nullptr;
  }
  static TestNode* makeVars() {
    printf("makeVars\n");
    return nullptr;
  }
  static TestNode* appendVar(TestNode* vars, std::string name, TestNode* value) {
    printf("appendVar\n");
    return nullptr;
  }
  static TestNode* makeLabel(std::string name, TestNode *body) {
    printf("makeLabel\n");
    return nullptr;
  }
  static TestNode* makeSub(TestNode *obj, TestNode *subee) {
    printf("makeSub\n");
    return nullptr;
  }
  static TestNode* makeIndex(TestNode *obj, std::string subee) {
    printf("makeIndex\n");
    return nullptr;
  }
  static TestNode* makeConditional(TestNode* cond, TestNode* ifTrue, TestNode* ifFalse) {
    printf("makeConditional\n");
    return nullptr;
  }
  static TestNode* makeBinary(std::string op, TestNode* left, TestNode* right) {
    printf("makeBinary\n");
    return nullptr;
  }
  static TestNode* makeUnary(std::string op, TestNode* inner) {
    printf("makeUnary\n");
    return nullptr;
  }
  static TestNode* makeArray(TestNode* vals) {
    printf("makeArray\n");
    return nullptr;
  }
  static TestNode* makeNew(TestNode* base, TestNode* args) {
    printf("makeNew\n");
    return nullptr;
  }
  static TestNode* makeFunction(std::string name, TestNode* params, TestNode* body) {
    printf("makeFunction\n");
    return nullptr;
  }
  static TestNode* makeName(std::string name) {
    printf("makeName\n");
    return nullptr;
  }
  static TestNode* makeNum(double num) {
    printf("makeNum\n");
    return nullptr;
  }
  static TestNode* makeString(std::string str) {
    printf("makeString\n");
    return nullptr;
  }
  static TestNode* makeBool(bool b) {
    printf("makeBool\n");
    return nullptr;
  }
};

almond::Parser<TestNode, TestBuilder> tb;

int main() {
  almond::init();

  tb.parseFile("test.js", "print('hello world');");
}

