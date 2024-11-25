#pragma once

typedef double Node_t;

struct Node
{
    int    type;
    double value;
    struct Node* left;
    struct Node* right;
};

struct Buffer
{
    char* buffer;
    char* current;
};

enum Type
{
    NUM = 1,
    OP  = 2,
    VAR = 3,
};

enum Operations 
{
    ADD  = '+', 
    SUB  = '-',
    MUL  = '*',
    DIV  = '/',
    SQRT = 5, // ?????
    SIN  = 's',
    COS  = 'c',
    POW  = '^'
};

Node* new_node (int type, double value, struct Node* node_left, struct Node* node_right);

Node* read_example      (FILE* file, struct Buffer* buffer);
int   initialize_buffer (FILE* file, struct Buffer* buffer);
Node* read_node         (int level,  struct Buffer* buffer);
Node* read_node_double  (struct Node* node, int level, struct Buffer* buffer);
Node* read_node_symbol  (struct Node* node, int level, struct Buffer* buffer);

double eval (struct Node* node);

Node* differentiator (struct Node* node);
Node* copy           (struct Node* node);

int  graph_dump             (struct Node* node, struct Node* selection);
void print_in_file_preorder (struct Node* node, FILE* graph_dump, struct Node* selection);

int print_in_tex  (struct Node* node, const char* filename);
int print_inorder (struct Node* node, FILE* file);

void cleanup_buffer  (struct Buffer* buffer);
int  delete_sub_tree (struct Node* node);

/*====ТЕСТЫ====*/
int test_graph_dump ();
