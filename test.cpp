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
  static TestNode* makeCall() {
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

