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
    NUM  = 1,
    OP   = 2,
    VAR  = 3,
    FUNC = 4
};

enum Operations 
{
    ADD  = '+', 
    SUB  = '-',
    MUL  = '*',
    DIV  = '/',
    SIN  = 's',
    COS  = 'c',
    POW  = '^',
    EXP  = 'e',
    ROOT = 'r',
    LOG  = 'l',
    LN   = 'n', // ???
    TG   = 't'
};

#ifdef DEBUG
    #define DBG(...) __VA_ARGS__
#else
    #define DBG(...)
#endif

Node* new_node (int type, double value, struct Node* node_left, struct Node* node_right);

Node* read_example      (FILE* file, struct Buffer* buffer);
int   initialize_buffer (FILE* file, struct Buffer* buffer);
Node* read_node         (int level,  struct Buffer* buffer);
Node* read_node_double  (struct Node* node, int level, struct Buffer* buffer);
Node* read_node_func    (struct Node* node, int level, struct Buffer* buffer);
Node* read_node_symbol  (struct Node* node, int level, struct Buffer* buffer);

double eval (struct Node* node);

Node* differentiator (struct Node* node);
Node* copy           (struct Node* node);
Node* simplify       (struct Node* node);
int simplification(struct Node* root, struct Node* parent);
int delete_sub_tree (struct Node* node);
int is_value_equal(struct Node* node, int value);
void verificator(struct Node* node, const char* filename, int line);
int   simplifer      (struct Node* node, struct Node* parent);


int  graph_dump             (struct Node* node, struct Node* selection);
void print_in_file_preorder (struct Node* node, FILE* graph_dump, struct Node* selection);

int print_in_tex  (struct Node* node, const char* filename);
int print_inorder (struct Node* node, FILE* file);

int is_function_node (struct Node* node);
void print_func_in_tex (struct Node* node);
int tex_printf_tree_inorder (struct Node* node, struct Node* parent);
void tex_printf_expression (struct Node* node, struct Node* diff_node);
void tex_printf_tree (struct Node* node, struct Node* diff_node, const char* message);
int priority (int op);
void print_tree_preorder_for_file (struct Node* node, FILE* filename);
FILE* open_tex_file (const char* filename);
int tex_printf (const char* message, ...);
void close_tex_file (void);


void cleanup_buffer  (struct Buffer* buffer);
int  delete_tree     (struct Node* node); 

/*====ТЕСТЫ====*/
int test_graph_dump ();

/*
Node* read_node (int level, struct Buffer* buffer)
{
    CHECK_NODE (buffer, return NULL);

    int offset = -1;
    sscanf (buffer->current, " ( %n", &offset);
    if (offset < 0) 
    { 
        DBG ( PRINT ("No '(' found. Return NULL."); ) 
        return NULL; 
    }

    buffer->current += offset;

    DBG ( PRINT ("Got an '('. Creating a node."); )

    struct Node* node = new_node(0, 0, NULL, NULL); // = NULL

    node = new_node (0, 0, NULL, NULL); 

    offset    = -1;
    int start =  0, end = 0;
    sscanf (buffer->current, " %n%*[^()]%n %n", &start, &end, &offset);
    if (offset < 0) 
    { 
        PRINT ("No value found. Return NULL."); 
        return NULL; 
    }

    char   symbol = 0; 
    double number = 0;
    if (sscanf (buffer->current + start, " %lg ", &number) != 0)
    {
        node->value = number;
        node->type  = NUM;

        buffer->current += offset;
        DBG ( PRINT ("CURRENT_PTR was shifted."); )

        return read_node_double (node, level, buffer);
    }

    else if (sscanf (buffer->current + start, " %15s ", symbol) == 1)
    {
        if (strcmp (symbol, "cos") == 0)
        {
            node->value = COS;
            node->type  = FUNC;
        }
        else if (strcmp(symbol, "sin") == 0)
        {
            node->value = SIN;
            node->type = FUNC;
        }
        else if (strcmp(symbol, "sqrt") == 0)
        {
            node->value = SQRT;
            node->type = FUNC;
        }
        else if (strcmp(symbol, "pow") == 0)
        {
            node->value = POW;
            node->type = FUNC;
        }
        else
        {
            PRINT("Unknown function symbol: %s\n", symbol);
            return NULL;
        }

        buffer->current += offset;

        // Для функций (например, pow) ожидаем одну или две дочерние ноды
        node->left = read_node(level + 1, buffer);
        if (node->value == POW)
        {
            node->right = read_node(level + 1, buffer);
        }

        char next_char = '\0';
        sscanf(buffer->current, " %c %n", &next_char, &offset);
        if (next_char == ')')
        {
            buffer->current += offset;
            DBG(PRINT("Function node parsed successfully.\n");)
            return node;
        }

        DBG(PRINT("Missing closing parenthesis after function.\n");)
        return NULL;
    }

    else
    {
        sscanf (buffer->current + start, " %c ", &symbol);

        node->value = (int) symbol;

        node->type = (symbol == 'x') ? VAR : OP;

        buffer->current += offset;
        DBG( PRINT ("CURRENT_PTR was shifted."); )

        return read_node_symbol (node, level, buffer);
    }

    return NULL;
}

*/
