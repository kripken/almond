#include "lexer.h"
#include "parser.h"

struct TestNode {};

struct TestBuilder {
  static TestNode* makeToplevel() {
    printf("makeTopLevel");
    return nullptr;
  }
  static void appendStatement(TestNode* block, TestNode* statement) {
    printf("appendStatement");
  }
  static TestNode* makeEmpty() {
    printf("makeEmpty");
    return nullptr;
  }
  static TestNode* makeCall(TestNode *target, TestNode *args) {
    printf("makeCall");
    return nullptr;
  }
  static TestNode* makeList() {
    printf("makeList");
    return nullptr;
  }
  static int getSize(TestNode*) {
    printf("getSize");
    return 0;
  }
  static TestNode* makeIf(TestNode* cond, TestNode* ifTrue, TestNode* ifFalse) {
    printf("makeIf");
    return nullptr;
  }
  static void append(TestNode* node, TestNode* element) {
    printf("append");
  }
  static TestNode* makeUndefined() {
    printf("makeUndefined");
    return nullptr;
  }
  static TestNode* makeNull() {
    printf("makeNull");
    return nullptr;
  }
  static TestNode* makeWhile(TestNode* cond, TestNode* body) {
    printf("makeWhile");
    return nullptr;
  }
  static TestNode* makeDo(TestNode* cond, TestNode* body) {
    printf("makeDo");
    return nullptr;
  }
  static TestNode* makeSwitch(TestNode* cond) {
    printf("makeSwitch");
    return nullptr;
  }
  static TestNode* appendSwitchCase(TestNode* switch_, TestNode* case_) {
    printf("appendSwitchCase");
    return nullptr;
  }
  static TestNode* appendSwitchStatement(TestNode* switch_, TestNode* statement) {
    printf("appendSwitchStatement");
    return nullptr;
  }
  static TestNode* appendSwitchDefault(TestNode* switch_) {
    printf("appendSwitchDefault");
    return nullptr;
  }
  static bool isBinary(TestNode* node) {
    printf("isBinary");
    return true;
  }
  static TestNode* makeBreak(std::string label) {
    printf("makeBreak");
    return nullptr;
  }
  static TestNode* makeContinue(std::string label) {
    printf("makeContinue");
    return nullptr;
  }
  static TestNode* makeReturn(TestNode* value) {
    printf("makeReturn");
    return nullptr;
  }
  static TestNode* makeThrow(TestNode* value) {
    printf("makeThrow");
    return nullptr;
  }
  static TestNode* makeTry(TestNode* tryStmt, TestNode* catchIdent, TestNode* catchStmt, TestNode* finallyStmt) {
    printf("makeTry");
    return nullptr;
  }
  static TestNode* makeVars() {
    printf("makeVars");
    return nullptr;
  }
  static TestNode* appendVar(TestNode* vars, std::string name, TestNode* value) {
    printf("appendVar");
    return nullptr;
  }
  static TestNode* makeLabel(std::string name, TestNode *body) {
    printf("makeLabel");
    return nullptr;
  }
  static TestNode* makeSub(TestNode *obj, TestNode *subee) {
    printf("makeSub");
    return nullptr;
  }
  static TestNode* makeIndex(TestNode *obj, std::string subee) {
    printf("makeIndex");
    return nullptr;
  }
  static TestNode* makeConditional(TestNode* cond, TestNode* ifTrue, TestNode* ifFalse) {
    printf("makeConditional");
    return nullptr;
  }
  static TestNode* makeBinary(std::string op, TestNode* left, TestNode* right) {
    printf("makeBinary");
    return nullptr;
  }
  static TestNode* makeUnary(std::string op, TestNode* inner) {
    printf("makeUnary");
    return nullptr;
  }
/*
  static TestNode* XXX() {
    printf("XXX");
    return nullptr;
  }
  static TestNode* XXX() {
    printf("XXX");
    return nullptr;
  }
  static TestNode* XXX() {
    printf("XXX");
    return nullptr;
  }
  static TestNode* XXX() {
    printf("XXX");
    return nullptr;
  }
  static TestNode* XXX() {
    printf("XXX");
    return nullptr;
  }
  static TestNode* XXX() {
    printf("XXX");
    return nullptr;
  }
  static TestNode* XXX() {
    printf("XXX");
    return nullptr;
  }
  static TestNode* XXX() {
    printf("XXX");
    return nullptr;
  }
  static TestNode* XXX() {
    printf("XXX");
    return nullptr;
  }
  static TestNode* XXX() {
    printf("XXX");
    return nullptr;
  }
  static TestNode* XXX() {
    printf("XXX");
    return nullptr;
  }
  static TestNode* XXX() {
    printf("XXX");
    return nullptr;
  }
  static TestNode* XXX() {
    printf("XXX");
    return nullptr;
  }
  static TestNode* XXX() {
    printf("XXX");
    return nullptr;
  }
  static TestNode* XXX() {
    printf("XXX");
    return nullptr;
  }
  static TestNode* XXX() {
    printf("XXX");
    return nullptr;
  }
  static TestNode* XXX() {
    printf("XXX");
    return nullptr;
  }
  static TestNode* XXX() {
    printf("XXX");
    return nullptr;
  }
*/
};

almond::Parser<TestNode, TestBuilder> tb;

int main() {
  almond::init();

  tb.parseFile("test.js", "print('hello world');");
}

