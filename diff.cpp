#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "diff.h"
#include "color.h"

// (+ (x) (- (3) (/ (x) (5))))


#ifdef DEBUG
    #define DBG(...) __VA_ARGS__
#else
    #define DBG(...)
#endif

#define CHECK_NODE(node, action)                                     \
    if ((node) == NULL)                                              \
    {                                                                \
        printf(RED_TEXT("[%d]%s(): ") "ERROR: node == NULL\n",       \
               __LINE__, __func__);                                  \
        action;                                                      \
    }

int GlobalX = 0;

int main (void) // TODO: сделать 
{
    struct Buffer buffer = {};

    FILE* test = fopen ("test.txt", "r");
    assert (test);

    struct Node* node = read_example (test, &buffer);

    printf ("\n\n\n%s(): END\n\n\n", __func__);

    // graph_dump (node, NULL);

    // GlobalX = 5;
    // printf ("\n""answer = %lg\n", eval (node));


    struct Node* diff_node = differentiator (node);
    
    print_in_tex (diff_node, "output.tex");

    //graph_dump (node, NULL);

    fclose (test);

    cleanup_buffer (&buffer);
    delete_sub_tree (node);

    return 0;
}

Node* new_node (int type, double value, struct Node* node_left, struct Node* node_right)
{
    Node* node = (Node*) calloc (1, sizeof (*node));
    CHECK_NODE (node, return NULL);
    // if (node == NULL)
    // {
    //     printf (RED_TEXT("[%d]%s():") "ERROR node == NULL", __LINE__, __func__);
    //     return NULL;
    // } 
    node->value = value;

    node->type  = type;

    node->left  = node_left;
    node->right = node_right;

    return node;
}

int graph_dump (struct Node* node, struct Node* selection)
{
    FILE* graph_dump = fopen ("graph_dump.dot", "w");
    if (graph_dump == NULL)
    {
        fprintf(stderr, "ERROR: Unable to open file graph_dump.dot\n");
        return -1;
    }

    fprintf (graph_dump, "digraph\n{\n");
    fprintf (graph_dump, "rankdir = \"TB\";\n");

    // fprintf(graph_dump, " %d ", node->data);
    fprintf (graph_dump, "\n");

    print_in_file_preorder (node, graph_dump, selection);

    fprintf (graph_dump, "}\n");

    fclose (graph_dump);

    system ("dot graph_dump.dot -T png -o graph_dump.png");

    return 0;
}

void print_in_file_preorder (struct Node* node, FILE* graph_dump, struct Node* selection) 
{
    CHECK_NODE (node, return);

    if (node->type == NUM)
        fprintf (graph_dump, "node%p [style = filled, shape=Mrecord, fillcolor = \"#%06x\" , label = \" { type = %d (NUM) | value = %g | { left = %3p | right = %3p } }\"]\n",
                              node, (selection == node ? 0xF08000 : 0xFFC0C0), node->type, node->value, node->left, node->right);

    else if (node->type == OP)
        fprintf (graph_dump, "node%p [style = filled, shape=Mrecord, fillcolor = \"#%06x\" , label = \" { type = %d (OP) | value = %c | { left = %3p | right = %3p } }\"]\n",
                              node, (selection == node ? 0xF08000 : 0xFFC0C0), node->type, (int) node->value, node->left, node->right);

    else if (node->type == VAR)
            fprintf (graph_dump, "node%p [style = filled, shape=Mrecord, fillcolor = \"#%06x\" , label = \" { type = %d (VAR) | value = %c | { left = %3p | right = %3p } }\"]\n",
                              node, (selection == node ? 0xF08000 : 0xFFC0C0), node->type, (int) node->value, node->left, node->right);

    if (node->left)
    {
        fprintf (graph_dump, "node%p -> node%p\n;", node, node->left);
        //print_in_file_preorder (node->left, graph_dump, selection);
    }

    if (node->right)
    {
        fprintf (graph_dump, "node%p -> node%p\n;", node, node->right);
        //print_in_file_preorder (node->right, graph_dump, selection);
    }

    if (node->left)
        print_in_file_preorder (node->left, graph_dump, selection);
    
    if (node->right)
        print_in_file_preorder (node->right, graph_dump, selection);

    printf (" ) ");
}

int print_in_tex (struct Node* node, const char* filename)
{
    FILE* file = fopen (filename, "wb");

    fprintf (file, "\n$");

    print_inorder (node, file);

    fprintf (file, "$\n");

    fclose (file);

    return 0;
}

int print_inorder (struct Node* node, FILE* file)
{
    CHECK_NODE (node, return -1);

    bool brackets = (node->type == OP && node->left != NULL && node->right != NULL);

    if (brackets && node->type == OP && node->value == DIV) 
        brackets = 0; 

    if (brackets)
        fprintf (file, " ( ");

    if (node->left) 
    {
        fprintf (file, " { ");
        print_inorder (node->left, file);
        fprintf (file, " } ");
    }

    if (node->type == NUM)
        fprintf (file, " %lg ", node->value);

    else 
    {
        if (node->type == OP && node->value == DIV)
            fprintf (file, " \\over "); 

        else if (node->type == OP && node->value == MUL)   
            fprintf (file, " \\cdot ");

        else if (node->type == OP && node->value == POW)
            fprintf (file, " ^ ");

        else
            fprintf(file, " %c ", (int) node->value);
    }

    if (node->right) 
    {
        fprintf (file, " { ");
        print_inorder (node->right, file);
        fprintf (file, " } ");
    }

    if (brackets)
        fprintf (file, " ) ");

    return 0;
}

int test_graph_dump ()
{

    // cos(sin(ln(x)))
    struct Node node3 = { VAR, 'x', NULL, NULL  };              
    struct Node node2 = { OP, 'l', &node3, NULL };               
    struct Node node1 = { OP, 's', &node2, NULL };               
    struct Node node0 = { OP, 'c', &node1, NULL };    
     

    graph_dump(&node0, &node1);

    // graph_dump (root, NULL);

    printf ("\n%s(): END\n", __func__);

    return 0;
}


Node* read_example (FILE* file, struct Buffer* buffer)
{
    if (file == NULL)
    {
        printf ("\n[%d]%s(): ERROR: fille == NULL\n", __LINE__, __func__);
        return NULL;
    }

    if (buffer == NULL)
    {
        printf ("\n[%d]%s(): ERROR: buffer == NULL\n", __LINE__, __func__);
        return NULL;
    }

    if (!initialize_buffer(file, buffer)) 
    {
        fprintf(stderr, "ERROR: Failed to initialize buffer.\n");
        return NULL;
    }

    buffer->current = buffer->buffer;

    return read_node (0, buffer);
}

int initialize_buffer(FILE* file, struct Buffer* buffer) 
{
    struct stat st = {};
    if (fstat(fileno(file), &st) != 0) 
    {
        fprintf(stderr, "ERROR: Could not retrieve file size.\n");
        return 0;
    }

    long file_size = st.st_size;

    buffer->buffer = (char*)calloc((size_t)file_size + 1, sizeof(char));  // 1 байт на '\0'
    if (!buffer->buffer) 
    {
        fprintf(stderr, "ERROR: Failed to allocate buffer memory.\n");
        return 0;
    }

    DBG( printf ("\n\n\n\nbuffer->buffer_ptr = [%p]\n\n\n\n", buffer->buffer); )

    size_t read_count = fread(buffer->buffer, sizeof(char), (size_t)file_size, file);
    if (read_count != (size_t)file_size) 
    {
        fprintf(stderr, "ERROR: File read incomplete. Read %zu out of %ld bytes.\n", read_count, file_size);
        free(buffer->buffer);
        return 0;
    }
    fclose(file);
    return 1;
}

#define PRINT(...)  { int n =  fprintf (stderr, "%s:%d: ", __FILE__, __LINE__);                  \
                          n += fprintf (stderr, "%*s", level*2, "");                             \
                          n += fprintf (stderr, __VA_ARGS__);                                    \
                          n += fprintf (stderr, "%*s" "Cur = '%.25s'..., [%p].\n",               \
                                                85 - n, "", buffer->current, buffer->current); }

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

    node = new_node (0, 0, NULL, NULL); // TODO - ???? правильно ли 

    offset = -1;
    int start = 0, end = 0;
    sscanf (buffer->current, " %n%*[^()]%n %n", &start, &end, &offset);
    if (offset < 0) 
    { 
        PRINT ("No value found. Return NULL."); 
        return NULL; 
    }

    char symbol = 0;
    double number = 0;

    if (sscanf (buffer->current + start, " %lg ", &number) != 0)
    {
        node->value = number;
        node->type = NUM;

        buffer->current += offset;
        DBG ( PRINT ("CURRENT_PTR was shifted."); )

        return read_node_double (node, level, buffer);
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

Node* read_node_double (struct Node* node, int level, struct Buffer* buffer)
{
    int offset = -1;
    char next_char = '\0';
    sscanf (buffer->current, " %c %n", &next_char, &offset);
    if (offset < 0) 
    { 
        DBG( PRINT ("No ending symbol (1) found. Return NULL."); ) 
        return NULL; 
    }

    if (next_char == ')')
    {
        buffer->current += offset;

        DBG( printf ("\nthe node is completed\n") );

        return node;
    }

    node->left = read_node (level + 1, buffer);

    DBG( PRINT ("LEFT subtree read. value of left root = '%lg'", node->left->value); )

    node->right = read_node (level + 1, buffer);

    DBG( PRINT ("RIGHT subtree read. value of right root = '%lg'", node->right->value); )

    //next_char = '\0';
    sscanf (buffer->current, " %c %n", &next_char, &offset);
    if (offset < 0) 
    { 
        DBG( PRINT ("No ending symbol (2) found. Return NULL."); ) 
        return NULL; 
    }

    if (next_char == ')')
    {
        buffer->current += offset;

        DBG( PRINT ("Got a ')', FULL Node END (value = '%lg'). Return node.", node->value); )

        return node;
    }

    DBG( PRINT ("Does NOT get ')'. Syntax error. Return NULL."); )

    return NULL;
}

Node* read_node_symbol (struct Node* node, int level, struct Buffer* buffer)
{
    int n = -1;
    char next_char = '\0';
    sscanf (buffer->current, " %c %n", &next_char, &n);
    if (n < 0) 
    { 
        DBG( PRINT ("No ending symbol (1) found. Return NULL."); ) 
        return NULL; 
    }

    if (next_char == ')')
    {
        buffer->current += n;

        DBG ( PRINT ("Got a ')', SHORT Node END (value = '%c'). Return node.", (int) node->value); )

        return node;
    }

    DBG( PRINT ("')' NOT found. Supposing a left/right subtree. Reading left node."); )

    node->left = read_node (level + 1, buffer);

    DBG( PRINT ("LEFT subtree read. value of left root = '%c' (%lg), TYPE %d", (int) node->left->value, node->left->value, node->left->type); )

    DBG( PRINT ("Reading right node."); )

    node->right = read_node (level + 1, buffer);

    DBG( PRINT ("RIGHT subtree read. value of right root = '%c' (%lg), TYPE %d", (int) node->right->value, node->right->value, node->right->type); )

    //next_char = '\0';
    sscanf (buffer->current, " %c %n", &next_char, &n);
    if (n < 0) 
    { 
        DBG( PRINT ("No ending symbol (2) found. Return NULL."); ) 
        return NULL; 
    }

    if (next_char == ')')
    {
        buffer->current += n;

        DBG( PRINT ("Got a ')', FULL Node END (value = '%c'). Return node.", (int) node->value); )

        return node;
    }

    DBG( PRINT ("Does NOT get ')'. Syntax error. Return NULL."); )

    return NULL;
}

double eval (struct Node* node)
{
    DBG( printf ("\nnode->type = %d\n", node->type); )

    if (node->type == NUM)
        return node->value;

    if (node->type == VAR)
        return GlobalX; // !!!!!!!!!

    if (node->type == OP)
    {
        graph_dump (node, NULL);

        fprintf (stderr, "\nnode->type = %.2lf | %c\nnode->value = %d   | ADD = %d\n", node->value, (int) node->value, (int) node->value, ADD);
        switch ((int) node->value)
        {
            case ADD:  {
                double e_left  = eval (node->left );
                double e_right = eval (node->right);
                DBG( fprintf (stderr ,"\ne_left = %f | e_right = %f\n", e_left, e_right); )
                return eval (node->left) + eval (node->right); }

            case SUB:  { return eval (node->left) - eval (node->right); }

            case MUL:  { return eval (node->left) * eval (node->right); }

            case DIV:  { return eval (node->left) / eval (node->right); }

            // case SQRT: { return sqrt (eval (node->left));               }

            // case COS:  { return cos  (eval(node->left));                }

            // case SIN:  { return sin  (eval(node->left));                }

            default:
            {
                printf ("Unknown operation. returned 1");
                return 1;
            }
       }
    }

    printf ("\n\nERROR return govno\n\n");
    return -1;
}

Node* differentiator (struct Node* node)
{
    CHECK_NODE (node, return NULL);

    if (node->type == NUM) return new_node (NUM, 0, NULL, NULL);

    if (node->type == VAR) return new_node (NUM, 1, NULL, NULL);

    if (node->type == OP)
        switch ((int) node->value)
        {
            case ADD: 
            {
                graph_dump (node, NULL);

                DBG( printf ("\nnode->value = %lf | %c\n", node->value, (int) node->value));

                struct Node* n_left  = differentiator (node->left );
                struct Node* n_right = differentiator (node->right);

                struct Node* node_1 = new_node (OP, ADD, differentiator (node->left), differentiator (node->right));

                graph_dump (n_left,  NULL); // 1
                graph_dump (n_right, NULL); // 0

                //graph_dump (node_1, NULL);

                return node_1;
            }
            case SUB: 
            {
                return new_node (OP, SUB, differentiator (node->left), differentiator (node->right));
            }
            //case MUL: return new_node (OP, MUL, differentiator (node->left), copy (node->right), new_node (OP, ADD, copy (node->left), differentiator (node->right))); // return new_node (OP, MUL, differentiator (node->left), differentiator (node->right)); 
            case MUL: 
            {
                //f'g + fg'
                struct Node* dL = differentiator (node->left);  // df
                struct Node* dR = differentiator (node->right); // dg

                struct Node* cL = new_node (node->left->type,  node->left->value,  node->left->left,  node->left->right ); 
                struct Node* cR = new_node (node->right->type, node->right->value, node->right->left, node->right->right); 
                
                struct Node* left_mul  = new_node (OP, MUL, dL, cR); // f'g
                struct Node* right_mul = new_node (OP, MUL, cL, dR); // fg'
                return new_node (OP, ADD, left_mul, right_mul);
            }

            case DIV: 
            {
                // (f'g - fg') / g ^ 2 
                struct Node* dL = differentiator (node->left);  // df
                struct Node* dR = differentiator (node->right); // dg
                struct Node* cL = new_node (node->left->type,  node->left->value,  node->left->left,  node->left->right ); 
                struct Node* cR = new_node (node->right->type, node->right->value, node->right->left, node->right->right); 
                
                struct Node* left_mul  = new_node (OP, MUL, dL, cR); // f'g
                struct Node* right_mul = new_node (OP, MUL, cL, dR); // fg'
                struct Node* numerator = new_node (OP, SUB, left_mul, right_mul); // f'g - fg'
                
                struct Node* denominator = new_node (OP, MUL, cR, cR); // g²

                return new_node (OP, DIV, numerator, denominator);
            }

            case POW: 
            {
                // (f^g): g * (f^(g-1)) * f'
                struct Node* cL = new_node (node->left->type,  node->left->value,  node->left->left,  node->left->right ); 
                struct Node* cR = new_node (node->right->type, node->right->value, node->right->left, node->right->right); 

                struct Node* dL = differentiator (node->left);  // f'
                struct Node* g_minus_1 = new_node (OP, SUB, cR, new_node (NUM, 1.0, NULL, NULL)); // g - 1
                struct Node* power     = new_node (OP, POW, cL, g_minus_1); // f^(g-1)
                
                return new_node (OP, MUL, cR, new_node (OP, MUL, power, dL)); // g * (f^(g-1)) * f'
            }

            case SIN: 
            {
                // cos(f) * f'
                struct Node* cL = new_node (node->left->type, node->left->value, node->left->left, node->left->right); 
                struct Node* dL =  (node->left);  // f'
                
                struct Node* cos_f = new_node (OP, COS, cL, NULL); // cos(f)
                return new_node (OP, MUL, cos_f, dL); // cos(f) * f'
            }

            case COS: 
            {
                // -sin(f) * f'
                struct Node* cL = new_node (node->left->type, node->left->value, node->left->left, node->left->right); 
                struct Node* dL = differentiator (node->left);  // f'

                struct Node* sin_f = new_node (OP, SIN, cL, NULL); // sin(f)
                struct Node* neg_sin_f = new_node (OP, MUL, new_node(NUM, -1, NULL, NULL), sin_f); // -sin(f)
                return new_node(2, MUL, neg_sin_f, dL); // -sin(f) * f'
            }

            default: 
            {
                fprintf(stderr, "ERROR: Unknown operation '%c' (%d).\n", (char)node->value, (int)node->value);
                return NULL;
            }
        }

    fprintf(stderr, "ERROR: Unknown node type.\n");
    return NULL;
}

Node* copy (struct Node* node)
{
    CHECK_NODE(node, return NULL);

    struct Node* copy_left  = node->left  ? copy (node->left)  : NULL;
    struct Node* copy_right = node->right ? copy (node->right) : NULL;

    return new_node (node->type, node->value, copy_left, copy_right);
}

void cleanup_buffer(struct Buffer* buffer) 
{
    if (buffer->buffer) 
    {
        free(buffer->buffer);
        buffer->buffer = NULL;
        buffer->current = NULL;
    }
}

int delete_sub_tree (struct Node* node)
{
    node->value = 0;
    node->type = 0;

    if (node->left)  delete_sub_tree (node->left);

    if (node->right) delete_sub_tree (node->right);

    free (node);

    return 0;
}