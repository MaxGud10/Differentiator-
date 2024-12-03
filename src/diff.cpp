#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "diff.h"
#include "read_rec_desc.h"
#include "color.h"

#define CHECK_NODE(node, action)                                     \
    if ((node) == NULL)                                              \
    {                                                                \
        printf(RED_TEXT("\n[%d]%s(): ") "ERROR: node == NULL\n",       \
               __LINE__, __func__);                                  \
        action;                                                      \
    }

int GlobalX = 0;
FILE* GlobalTex  = NULL;
struct Node* GlobalNode = NULL;

int main (void) // TODO: сделать 
{
    struct Buffer buffer = {};

    FILE* test = fopen ("test.txt", "r");
    assert (test);

    /*====================================================*/
    struct Node* node = get_g("2^(x*1+(1+4))$"); // 2^(1+x)+s(x+30)$
    printf ("\nThe reading is over\n");
    graph_dump (node, NULL);
    // print_in_tex (node, "output.tex");
    /*====================================================*/

    // struct Node* node = read_example (test, &buffer);

    // printf ("\n\n\n%s(): END\n\n\n", __func__);

    // graph_dump   (node, NULL);
    // print_in_tex (node, "output.tex");

    // GlobalX = 5;
    // printf ("\n""answer = %lg\n", eval (node));

    struct Node* diff_node = differentiator (node);
    struct Node* simplified_derivative = simplify (diff_node);
    double res = constant_folding (diff_node);

    // graph_dump (node, NULL);

    // struct Node* simplified_derivative = simplify (diff_node);
    // double res = constant_folding (diff_node);

    // fprintf (stderr, "\n%s(): MUL/LEFT_BY_1, RETURN. RESULT = %p | type = %d | value = %d(%c)\n", __func__, simplified_derivative, simplified_derivative->type, (int)simplified_derivative->value, (int)simplified_derivative->value);
    // double res = constant_folding (node);
    // fprintf (stderr, "\n%s:%d %s(): res = %lf\n", __FILE__, __LINE__, __func__, res);

    /*=========================================================================*/
    // struct Node* simplified_derivative = evaluate_and_simplify(node);
    // graph_dump (node, NULL);
    /*=========================================================================*/
    
    print_in_tex (simplified_derivative, "output.tex"); // diff_node

    fclose (test);                                                              

    cleanup_buffer (&buffer);                                       
    //fprintf (stderr, "\n%s:%d %s(): delete_tree\n", __FILE__, __LINE__, __func__);
    delete_tree (node);

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
        fprintf (graph_dump,    "node%p [style = filled, shape=Mrecord, fillcolor = \"#%06x\" , label = \" { %p| type = %d (NUM) | value = %g | { left = %3p | right = %3p } }\"]\n",
                                 node, (selection == node ? 0xF08000 : 0xC0FFFF), node, node->type, node->value, node->left, node->right);

    else if (node->type == OP)
        fprintf (graph_dump,    "node%p [style = filled, shape=Mrecord, fillcolor = \"#%06x\" , label = \" { %p | type = %d (OP)  | value = %c | { left = %3p | right = %3p } }\"]\n",
                                node, (selection == node ? 0xF08000 : 0xFFC0C0), node, node->type, (int) node->value, node->left, node->right);

    else if (node->type == VAR)
            fprintf (graph_dump, "node%p [style = filled, shape=Mrecord, fillcolor = \"#%06x\" , label = \" { %p | type = %d (VAR) | value = %c | { left = %3p | right = %3p } }\"]\n",
                                  node, (selection == node ? 0xF08000 : 0xA2F8a4), node, node->type, (int) node->value, node->left, node->right);

    else if (node->type == FUNC)
        fprintf (graph_dump,    "node%p [style = filled, shape=Mrecord, fillcolor = \"#%06x\" , label = \" { %p | type = %d (FUNC) | value = %c | { left = %3p | right = %3p } }\"]\n",
                                 node, (selection == node ? 0xF08000 : 0xECFD74), node, node->type, (int) node->value, node->left, node->right);

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
    if (!file) 
    {
        printf ("\n[%d]%s(): ERROR Failed to open file\n", __LINE__, __func__);
        return -1;
    }

    fprintf (file, "\n$$");
    DBG( printf("\nWriting to Tex file started. Calling print_inorder...\n"));

    int result = print_inorder (node, file);
    if (result != 0)
    {
        printf("\n[%d]%s(): Error while printing tree in print_inorder\n", __LINE__, __func__);
        fclose(file);
        return result;
    }

    fprintf (file, "$$\n");
    DBG (printf("\nWriting to Tex file finished. Closing file.\n"));

    fclose (file);

    return 0;
}

int print_inorder(struct Node* node, FILE* file)
{
    CHECK_NODE(node, return -1);
    DBG( printf("\nnode->type = %d |  value = %lg(%c) | node->left = %p | node->right = %p\n", node->type, node->value, (int)node->value, node->left, node->right));

    bool brackets = (node->type == OP && node->left != NULL && node->right != NULL);

    if (brackets && node->type == OP && node->value == DIV)
        brackets = 0;

    if (brackets)
    {
        fprintf(file, " ( ");
        DBG( printf("\n Opened bracket\n"));
    }

    if (node->type == FUNC)
    {
        if (node->value == 's')
        {
            fprintf(file, " \\sin(");  
            DBG( printf("\nPrinted function: \\sin\n"));

            if (node->left)
            {
                print_inorder(node->left, file);
            }

            fprintf(file, ")");  
        }
        else if (node->value == 'c')
        {
            fprintf(file, " \\cos(");  
            DBG( printf("\nPrinted function: \\cos\n"));

            if (node->left)
            {
                print_inorder(node->left, file);
            }

            fprintf(file, ")");  
        }

        else if (node->value == 'n') // ln
        {
            fprintf (file, "\\ln");
            DBG( printf("\nPrinted function: ln\n"));

            if (node->left)
            {
                fprintf (file, "(");
                print_inorder(node->left, file); 
                fprintf(file, ")"); 
            }
        }

        else if (node->value == 'l')
        {
            fprintf(file, " log_{"); // Печатаем "log"
            DBG( printf("\nPrinted function: log\n"));

            if (node->left)
            {
                print_inorder(node->left, file); // Печатаем базу
            }

            fprintf(file, "}("); // Разделяем запятой
            DBG( printf("\nPrinted comma for log\n"));

            // Печатаем аргумент логарифма
            if (node->right)
            {
                print_inorder(node->right, file); // Печатаем аргумент
            }

            fprintf(file, ")"); // закрываем скобку логарифма      
        }

        else if (node->value == 't')
        {
            fprintf(file, " \\tan(");  
            DBG (printf("\nPrinted function: \\tan\n"));

            if (node->left)
            {
                print_inorder(node->left, file);
            }

            fprintf(file, ")");           
        }
        return 0;
    }

    if (node->left)
    {
        fprintf(file, " { ");
        DBG (printf("\nMoving to the left subtree...\n"));
        print_inorder(node->left, file);
        fprintf(file, " } ");
        DBG (printf("\nReturned from the left subtree\n"));
    }

    if (node->type == NUM)
    {
        fprintf(file, " %lg ", node->value);
        DBG (printf("\nPrinted number: %lg\n", node->value));
    }

    else
    {
        if (node->type == OP && node->value == DIV)
        {
            fprintf(file, " \\over ");
            DBG (printf("\nPrinted operation DIV (division)\n"));
        }

        else if (node->type == OP && node->value == MUL)
            fprintf(file, " \\cdot ");

        else if (node->type == OP && node->value == POW)
        {
            fprintf(file, " ^ ");
            DBG (printf("\nPrinted operation POW (power)\n"));
        }

        else
        {
            fprintf(file, " %c ", (int)node->value);
            DBG (printf("\nPrinted operator: %c\n", (int)node->value));
        }
    }

    if (node->right)
    {
        fprintf(file, " { ");
        DBG (printf("\nMoving to the right subtree...\n"));
        print_inorder(node->right, file);
        fprintf(file, " } ");
        DBG (printf("\nReturned from the right subtree\n"));
    }

    if (brackets)
    {
        fprintf(file, " ) ");
        DBG (printf("\nClosed bracket\n"));
    }

    return 0;
}


// int print_inorder (struct Node* node, FILE* file)
// {
//     CHECK_NODE (node, return -1);
//     printf ("\nnode->type = %d |  value = %lg | node->left = %p | node->right = %p\n", node->type, node->value, node->left, node->right);

//     bool brackets = (node->type == OP && node->left != NULL && node->right != NULL);

//     if (brackets && node->type == OP && node->value == DIV) 
//         brackets = 0; 

//     if (brackets)
//     {
//         fprintf (file, " ( ");
//         printf("\n Opened bracket\n");
//     }

//     if (node->left) 
//     {
//         fprintf (file, " { ");
//         printf("\nMoving to the left subtree...\n");
//         print_inorder (node->left, file);
//         fprintf (file, " } ");
//         printf("\nReturned from the left subtree\n");
//     }

//     if (node->type == NUM)
//     {
//         fprintf (file, " %lg ", node->value);
//         printf("\nPrinted number: %lg\n", node->value);
//     }

//     else 
//     {
//         if (node->type == OP && node->value == DIV)
//         {
//             fprintf (file, " \\over "); 
//             printf("\nPrinted operation DIV (division)\n");
//         }

//         else if (node->type == OP && node->value == MUL)   
//             fprintf (file, " \\cdot ");

//         // else if (node->type == OP && node->value == POW)
//         // {
//         //     fprintf(file, " \\cdot ");
//         //     printf("\nPrinted operation MUL (multiplication)\n");
//         // }

//         else if (node->type == OP && node->value == POW)
//         {
//             fprintf(file, " ^ ");
//             printf("\nPrinted operation POW (power)\n");
//         }

//         else if (node->type == FUNC)
//         {
//             if (node->value == 's')
//             {
//                 fprintf(file, "\\sin ");
//                 printf("\nPrinted function: \\sin\n");
//             }

//             else if (node->value == 'c')
//             {
//                 fprintf(file, "\\cos ");
//                 printf("\nPrinted function: \\cos\n");
//             }
//         }

//         else
//         {
//             fprintf(file, " %c ", (int) node->value);
//             printf("\nPrinted operator: %c\n", (int)node->value);
//         }
//     }

//     if (node->right) 
//     {
//         fprintf (file, " { ");
//         printf("\nMoving to the right subtree...\n");
//         print_inorder (node->right, file);
//         fprintf (file, " } ");
//         printf("\nReturned from the right subtree\n");
//     }

//     if (brackets)
//     {
//         fprintf (file, " ) ");
//         printf("\nClosed bracket\n");
//     }

//     return 0;
// }

// int print_in_tex (struct Node* node, const char* filename)
// {
//     FILE* file = fopen (filename, "wb");

//     fprintf (file, "\n$$");

//     print_inorder (node, file);

//     fprintf (file, "$$\n");

//     fclose (file);

//     return 0;
// }

// int print_inorder (struct Node* node, FILE* file)
// {
//     CHECK_NODE (node, return -1);

//     bool brackets = (node->type == OP && node->left != NULL && node->right != NULL);
//     DBG( printf ("\nnode->type = %d, node->left = %p, node->right = %p\n", node->type, node->left, node->right));

//     if (brackets && node->type == OP && node->value == DIV) 
//         brackets = 0; 

//     DBG( printf ("\nbrackets = %d\n", brackets));
//     if (brackets)
//         fprintf (file, " ( ");

//     if (node->left) 
//     {
//         fprintf (file, " { ");
//         printf ("\n(())\n");
//         print_inorder (node->left, file);
//         fprintf (file, " } ");
//     }

//     if (node->type == NUM)
//         fprintf (file, " %lg ", node->value);

//     else 
//     {
//         if (node->type == OP && node->value == DIV)
//             fprintf (file, " \\over "); 

//         else if (node->type == OP && node->value == MUL)   
//             fprintf (file, " \\cdot ");

//         else if (node->type == OP && node->value == POW)
//             fprintf (file, " ^ ");

//         else
//             fprintf(file, " %c ", (int) node->value);
//     }

//     if (node->right) 
//     {
//         fprintf (file, " { ");
//         print_inorder (node->right, file);
//         fprintf (file, " } ");
//     }

//     if (brackets)
//         fprintf (file, " ) ");

//     return 0;
// }

int is_function_node (struct Node* node)
{
    CHECK_NODE (node, return -1);

    if (node->type == OP && (int) node->value == 's')
        return 1;

    if (node->type == OP && (int) node->value == 'c')
        return 1;

    if (node->type == OP && (int) node->value == 'l')
        return 1;

    return 0;
}

void print_func_in_tex (struct Node* node)
{
    assert (node && "node is NULL in print_func");

    if ( (int) node->value == 's')
        tex_printf ("sin ");

    if ( (int) node->value == 'c')
        tex_printf ("cos ");

   if (node->left)
    {
        tex_printf (" { ");
        tex_printf_tree_inorder (node->left, node);
        tex_printf (" } ");
    }
}

int tex_printf_tree_inorder (struct Node* node, struct Node* parent)
{
    assert (node && "node is NULL in tex_printf_tree_inorder");

    if (is_function_node (node))
    {
        print_func_in_tex (node);
        return 0;
    }

    bool brackets = (node->left != NULL && node->right != NULL);

    if (node && parent && node->type == OP && parent->type == OP)
        if (priority (node->value) >= priority (parent->value))
            brackets = 0;

    if (parent == NULL)
        brackets = 0;

    if (node->type == OP)
        if (node->value == DIV)
            brackets = 0;

    if (node->type == OP)
        if (node->value == POW)
            brackets = 0;

    if (node->type == OP)
        if (node->value == POW)
            if (node->left->value == COS)
                brackets = 1;

    if (node->type == OP)
        if (node->value == POW)
            if (node->left->value == SIN)
                brackets = 1;

    if (node->type == OP)
        if (node->value == SUB)
            brackets = 0;

    if (node->type == NUM)
        if (node->value == -1)
            brackets = 1;

    if (brackets)
        tex_printf (" ( ");

    if (node->left)
    {
        tex_printf (" { ");
        tex_printf_tree_inorder (node->left, node);
        tex_printf (" } ");
    }

    if (node->type == NUM)
        tex_printf (" %lg ", node->value);
    else
    {
        if (node->type == OP && node->value == MUL)
            tex_printf (" \\cdot ");
        else
        if (node->type == OP && node->value == DIV)
            tex_printf (" \\over ");
        else
            tex_printf (" %c ", (int) node->value);
    }

    if (node->right)
    {
        tex_printf (" { ");
        tex_printf_tree_inorder (node->right, node);
        tex_printf (" } ");
    }

    if (brackets)
        tex_printf (" ) ");

    return 0;
}

void tex_printf_expression (struct Node* node, struct Node* diff_node)
{
    assert (node);
    assert (diff_node);

    tex_printf ("$$ ({");
    tex_printf_tree_inorder (node, NULL);

    tex_printf ("})' = {");

    tex_printf_tree_inorder (diff_node, NULL);
    tex_printf ("} $$\n");
}


void tex_printf_tree (struct Node* node, struct Node* diff_node, const char* message)
{
    assert (node);
    assert (diff_node);
    assert (message);

    if (GlobalNode != diff_node)
    {
        tex_printf ("%s", message);
        tex_printf_expression (node, diff_node);
    }
    else
        tex_printf ("%s... wait wait its the same, see above bro\\newline \\newline ", message);

    GlobalNode = diff_node;
}

int priority (int op)
{
    switch (op)
    {
        case ADD:
        case SUB: return 3;

        case MUL: return 4;
        case DIV: return 0;

        case POW: return 2;

        case SIN: return 1;

        case COS: return 1;

        default:
            printf ("priority of operation %d (%c) dont exists\n", op, op);
            return -1;
    }
}

void print_tree_preorder_for_file (struct Node* node, FILE* filename)
{
    assert (node);
    assert (filename);

    if (node->type == NUM)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { [%p] | type = %d (NUM) | value = %g | { left = [%p] | right = [%p] } }\"; style = filled; fillcolor = \"#FFD700\"];\n",
             node, node, node->type, node->value, node->left, node->right);
    else if (node->type == OP)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { [%p] | type = %d (OP) | value = '%c' | { left = [%p] | right = [%p] } }\"; style = filled; fillcolor = \"#20B2AA\"];\n",
             node, node, node->type, (int) node->value, node->left, node->right);
    else if (node->type == VAR)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { [%p] | type = %d (VAR) | value = '%c' | { left = [%p] | right = [%p] } }\"; style = filled; fillcolor = \"#F00000\"];\n",
             node, node, node->type, (int) node->value, node->left, node->right);

    if (node->left)
        fprintf (filename, "node%p -> node%p\n;", node, node->left);

    if (node->right)
        fprintf (filename, "node%p -> node%p\n;", node, node->right);

    if (node->left)  print_tree_preorder_for_file (node->left , filename);

    if (node->right) print_tree_preorder_for_file (node->right, filename);
}

FILE* open_tex_file (const char* filename)
{
    GlobalTex = fopen (filename, "wb");
    if (GlobalTex == NULL)
    {
        fprintf(stderr, "ERROR open tex_file\n");
        return NULL;
    }

    tex_printf ("\\documentclass{article}\n");
    tex_printf ("\\begin{document}\n");
    tex_printf ("\\section{Differentiator}\n");
    tex_printf ("wazzzuuuup, shut up and take my money.\\newline ");

    return GlobalTex;
}

int tex_printf (const char* message, ...)
{
    va_list args;
    va_start (args, message);

    vfprintf (GlobalTex, message, args);

    va_end (args);

    return 0;
}

void close_tex_file (void)
{
    tex_printf ("\\end{document}\n");

    fflush (GlobalTex);
    fclose (GlobalTex);
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

    buffer->buffer = (char*) calloc ((size_t)file_size + 1, sizeof(char));  // 1 байт на '\0'
    if (!buffer->buffer) 
    {
        fprintf(stderr, "ERROR: Failed to allocate buffer memory.\n");
        return 0;
    }

    DBG( printf ("\n\n\n\nbuffer->buffer_ptr = [%p]\n\n\n\n", buffer->buffer); )

    size_t read_count = fread (buffer->buffer, sizeof(char), (size_t)file_size, file);
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


// стестовый вариант 
Node* read_node_func (struct Node* node, int level, struct Buffer* buffer) 
{
    int offset = -1;
    char next_char = '\0';

    sscanf(buffer->current, " %c %n", &next_char, &offset);
    if (offset < 0) 
    {
        DBG( PRINT ("No ending symbol (1) found. Return NULL."); )
        return NULL;
    }

    if (next_char == ')') 
    {
        buffer->current += offset;
        DBG( PRINT ("Got a ')'. Function Node END (value = '%lg'). Return node.", node->value); )
        return node;
    }

    node->left = read_node (level + 1, buffer);
    DBG( PRINT ("LEFT subtree read for func node. Value of left root = '%lg'.", node->left->value); )

    sscanf(buffer->current, " %c %n", &next_char, &offset);
    if (offset < 0) 
    {
        DBG( PRINT ("No ending symbol (2) found. Return NULL."); )
        return NULL;
    }

    if (next_char == ')') 
    {
        buffer->current += offset;
        DBG( PRINT ("Got a ')'. Function Node END (with one argument). Return node."); )
        return node;
    }

    node->right = read_node(level + 1, buffer);
    DBG( PRINT ("RIGHT subtree read for func node. Value of right root = '%lg'.", node->right->value); )

    sscanf(buffer->current, " %c %n", &next_char, &offset);
    if (offset < 0 || next_char != ')') 
    {
        DBG( PRINT ("No closing ')' after right subtree. Syntax error. Return NULL."); )
        return NULL;
    }

    buffer->current += offset;
    DBG( PRINT ("Function Node END. Subtrees complete. Return node."); )
    return node;
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
    DBG( fprintf (stderr, "\nnode->type = %d\n", node->type); )

    if (node->type == NUM)
    {
        double value = node->value;
        fprintf (stderr, "node->type = NUM >>> node->value = %lg\n\n", node->value);

        delete_node (node);

        return value;
    }

    if (node->type == VAR)
    {
        fprintf (stderr, "node->type = VAR >>> node->value = %c\n\n", (int) node->value);
        return NAN; // !!!!!!!!!
    }

    if (node->type == OP)
    {

        fprintf (stderr, "\nnode->type = %.2lf | %c\nnode->value = %d   | ADD = %d\n", node->value, (int) node->value, (int) node->value, ADD);
        switch ((int) node->value)
        {
            case ADD:  
            {
                double e_left  = eval (node->left );
                double e_right = eval (node->right);

                DBG( fprintf (stderr ,"\ne_left = %f | e_right = %f\n", e_left, e_right); )

                double res = e_left + e_right;

                delete_node (node);

                return res;
            }
            case SUB:  
            {
                double res = eval (node->left) -
                             eval (node->right);
                printf ("case SUB: result = %lg\n\n", res);

                delete_node (node);

                return res;
            }

            case MUL:  
            {
                double res = eval (node->left) *
                             eval (node->right);
                printf ("case MUL: result = %lg\n\n", res);

                delete_node (node);

                return res;
            }

            case DIV:  
            {
                double res = eval (node->left) /
                             eval (node->right);
                printf ("case DIV: result = %lg\n\n", res);

                delete_node (node);

                return res;
            }

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
    printf ("\n%s(): node->type = %d | node->value = %.1lf(%c)\n", __func__, node->type, node->value, (int)node->value);

    //graph_dump (node, NULL);

    if (node->type == NUM) return new_node (NUM, 0, NULL, NULL);

    if (node->type == VAR) return new_node (NUM, 1, NULL, NULL);

    if (node->type == OP)
        switch ((int) node->value)
        {
            case ADD: 
            {
                //graph_dump (node, NULL);

                DBG( printf ("\nnode->value = %.2lf | %c\n", node->value, (int) node->value));

                struct Node* n_left  = differentiator (node->left );
                struct Node* n_right = differentiator (node->right);

                struct Node* node_1 = new_node (OP, ADD, differentiator (node->left), differentiator (node->right));
                //tex_printf_tree (node, node_1, "the derivative of the sum can be represented as follows: ");

                //graph_dump (n_left,  NULL); // 1
                //graph_dump (n_right, NULL); // 0

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
                // Дифференцирование f(x)^g(x) с учетом правил производной степени
                if (node->left->type == NUM) // Частный случай: a^g(x), где a — константа
                {
                    // Формула: (a^(g(x)))' = a^(g(x)) * ln(a) * g'(x)

                    // Левое поддерево (основание степени)
                    struct Node* cL = new_node(node->left->type, node->left->value, NULL, NULL);

                    // Правое поддерево (показатель степени)
                    struct Node* cR = new_node(node->right->type, node->right->value, node->right->left, node->right->right);

                    // Первая часть: a^(g(x))
                    struct Node* pow_part = new_node(OP, POW, cL, cR);

                    // Вторая часть: ln(a)
                    struct Node* ln_part = new_node(FUNC, 'n', copy(cL), NULL); // ln(a)

                    // Третья часть: g'(x)
                    struct Node* deriv_part = differentiator(node->right);

                    // Итог: a^(g(x)) * ln(a) * g'(x)
                    struct Node* mul_part1 = new_node(OP, MUL, pow_part, ln_part);
                    return new_node(OP, MUL, mul_part1, deriv_part);
                }

                else // Общий случай: f(x)^g(x)
                {
                    // Формула: (f(x)^g(x))' = f(x)^g(x) * (g'(x) * ln(f(x)) + g(x) * f'(x)/f(x))
                    struct Node* cL = new_node (node->left->type,  node->left->value,  node->left->left,  node->left->right ); 
                    struct Node* cR = new_node (node->right->type, node->right->value, node->right->left, node->right->right); 

                    struct Node* dL = differentiator  (node->left);  // f' // node->left
                    struct Node* g_minus_1 = new_node (OP, SUB, cR, new_node (NUM, 1.0, NULL, NULL)); // g - 1
                    struct Node* power     = new_node (OP, POW, cL, g_minus_1); // f^(g-1)
                    
                    return new_node (OP, MUL, cR, new_node (OP, MUL, power, dL)); // g * (f^(g-1)) * f'
                }
            }

            default: 
            {
                fprintf(stderr, "ERROR: Unknown operation '%c' (%d).\n", (char)node->value, (int)node->value);
                return NULL;
            }
        }
    
    if (node->type == FUNC)
        switch ((int) node->value)
        {
            case SIN: 
            {      
                // struct Node* diif_node = new_node (OP, MUL, new_node (OP, COS, copy(node->left), NULL), differentiator(node->left));
                // graph_dump(diif_node, NULL);
                // return diif_node;
                /*==================================*/         
                // Формула: (sin(f(x)))' = cos(f(x)) * f'(x)
                struct Node* cosine = new_node(FUNC, COS, copy(node->left), NULL); // cos(f(x))
                struct Node* derivative = differentiator(node->left);                  // f'(x)
                struct Node* dsin = new_node(OP, MUL, cosine, derivative);

                // return new_node(OP, MUL, cosine, derivative); // cos(f(x)) * f'(x)
                //graph_dump(dsin, NULL);

                return dsin;
                /*============================================*/
                // // cos(f) * f'
                // struct Node* cL = new_node (node->left->type, node->left->value, node->left->left, node->left->right); 
                // struct Node* dL =  (node->left);  // f'
                
                // struct Node* cos_f = new_node (OP, COS, cL, NULL); // cos(f)

                // struct Node* dsin = new_node (OP, MUL, cos_f, dL);

                // graph_dump(dsin, NULL);
                // // return new_node (OP, MUL, cos_f, dL); // cos(f) * f'
                // return dsin;
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

            case TG: // как сделать квадрат а не cos * cos
            {
                struct Node* arg_deriv = differentiator(node->left);

                struct Node* cos_x = new_node(FUNC, 'c', copy(node->left), NULL);

                // cos^2(x)
                struct Node* cos_squared = new_node(OP, MUL, cos_x, copy(cos_x)); 

                // 1 / cos^2(x)
                struct Node* one_over_cos_squared = 
                    new_node(OP, DIV, new_node(NUM, 1, NULL, NULL), cos_squared);

                // (1 / cos^2(x)) * d(arg)/dx
                return new_node(OP, MUL, one_over_cos_squared, arg_deriv);              
            }

            default: 
            {
                fprintf(stderr, "ERROR: Unknown FUNCTION '%c' (%d).\n", (char)node->value, (int)node->value);
                return NULL;
            }
        }

    fprintf(stderr, "ERROR: Unknown node type \n");
    return NULL;
}

Node* copy (struct Node* node)
{
    CHECK_NODE(node, return NULL);

    struct Node* copy_left  = node->left  ? copy (node->left)  : NULL;
    struct Node* copy_right = node->right ? copy (node->right) : NULL;

    return new_node (node->type, node->value, copy_left, copy_right);
}

Node* simplify (struct Node* node) 
{
    CHECK_NODE (node, return NULL);

    DBG (printf("\n%s(): Processing Node [%p], type = %d, value = %.1lf | '%c'\n", __func__, node, node->type, node->value, (int)node->value));

    if (node->left) 
    {
        DBG (fprintf(stderr, "\nGoing to simplify left child of Node...\n"));
        DBG (fprintf(stderr, "BEFORE %s (left): Node->left = [%p], type = %d, value = %.1lf | '%c'\n", __func__, node->left, node->left->type, node->left->value, (int)node->left->value));                       
        
        Node* ret = simplify(node->left);
        
        if (ret != NULL) DBG( fprintf (stderr, "\nAFTER %s (left): RET = [%p] | type = %d, value = %.1lf | '%c'\n", __func__, ret, ret->type, ret->value, (int)ret->value)); // virtual space
        else             DBG (fprintf (stderr, "[%d]%s(): RET == NULL\n", __LINE__, __func__));
            // node->left = simplify(node->left);
        node->left = ret;

        DBG (fprintf (stderr, "REPLACED node->left.\n\n"));
    }

    if (node->right) 
    {
        DBG (printf("\n%s(): Going to simplify right child of Node [%p] | type = %d, value = %.1lf | '%c'\n", __func__, node, node->type, node->value, (int)node->value));
        Node* ret = simplify(node->right);
        if (ret != NULL) DBG (fprintf (stderr, "\nAFTER simplify left child of RET [%p] | type = %d, value = %.1lf | '%c | '\n", ret, ret->type, ret->value, (int)ret->value));
        else             DBG (fprintf (stderr, "[%d]%s(): ret == NULL\n", __LINE__, __func__));
        
        node->right = ret;

        //node->right = simplify(node->right);
    }
     
    DBG (fprintf (stderr, "\nNode: %p | type  = %d | value %d (%c)\n", node, node->type, (int)node->value, (int)node->value));
    
    if (node->type == OP) 
    {
        switch ((int)node->value)
        {
            case ADD: // a + 0 = a, 0 + a = a
                DBG (fprintf (stderr, "\n%s:%d: !!! <case ADD>, node %p, node->left->type = %d, node->left->value = %d'%c'\n", __FILE__, __LINE__, node, node->left->type, (int)node->left->value, (int)node->left->value));

                if (node->left->type == NUM && node->left->value == 0) 
                {
                    DBG (fprintf(stderr, "\n%s:%d %s(): ADD: Applying rule a + 0 = a on Node [%p]\n", __FILE__, __LINE__, __func__, node));
                    struct Node* temp = node->right;

                    DBG (fprintf (stderr, "\n%s:%d %s(): node->left = %p\n", __FILE__, __LINE__, __func__, node->left));
                    //free(node->left);

                    DBG (fprintf (stderr, "\n%s:%d %s(): node = %p\n", __FILE__, __LINE__, __func__, node));
                    //free(node);

                    DBG (fprintf (stderr, "%s(): MUL/LEFT_BY_0: Finished. Node = %p | type = %d | value = %d(%c)", __func__, temp, temp->type, (int)temp->value, (int)temp->value));
                    return temp;
                }

                if (node->right->type == NUM && node->right->value == 0) 
                {
                    DBG (printf("\n%s:%d %s(): Applying rule 0 + a = a on Node [%p]\n", __FILE__, __LINE__, __func__, node));
                    struct Node* temp = node->left;

                    DBG (fprintf (stderr, "\n%s:%d %s(): node->right = %p\n", __FILE__, __LINE__, __func__, node->right));
                    //free(node->right);

                    DBG (fprintf (stderr, "\n%s:%d %s(): node = %p\n", __FILE__, __LINE__, __func__, node));
                    //free(node);

                    DBG (fprintf (stderr, "%s(): MUL/LEFT_BY_0: Finished. Node = %p | type = %d | value = %d(%c)", __func__, temp, temp->type, (int)temp->value, (int)temp->value));
                    return temp;
                }
                break;

            case SUB: // a - 0 = a
                if (node->right->type == NUM && node->right->value == 0) 
                {
                    DBG (printf("\n%s:%d %s(): Applying rule a - 0 = a on Node [%p]\n", __FILE__, __LINE__, __func__, node));
                    struct Node* temp = node->left;
                    //free(node->right);
                    //free(node);
                    return temp;
                }
                break;

            case MUL: // a * 1 = a, 1 * a = a, a * 0 = 0, 0 * a = 0
                DBG (fprintf (stderr, "\n%s:%d: !!! <case MUL>, node %p, node->left->type = %d, node->left->value = %d'%c'\n", __FILE__, __LINE__, node, node->left->type, (int)node->left->value, (int)node->left->value));
                

                if (node->left->type == NUM && node->left->value == 0) 
                {
                    DBG (printf("\n%s:%d %s(): MUL/LEFT: Applying rule a * 0 = 0 on Node [%p]\n", __FILE__, __LINE__,__func__, node));
                    
                    node->type  = NUM;
                    node->value = 0;

                    //free(node->left);
                    //free(node->right); // !!!!

                    node->left = node->right = NULL;
                    
                    DBG (fprintf (stderr, "%s(): MUL/LEFT_BY_0: Finished. Node = %p | type = %d | value = %d(%c)", __func__, node, node->type, (int)node->value, (int)node->value));
                } 
                
                else if (node->right->type == NUM && node->right->value == 0)
                {
                    DBG (printf("\n%s(): MUL/RIGHT: Applying rule 0 * a = 0 on TEMP [%p]\n", __func__, node));
                    node->type  = NUM;
                    node->value = 0;

                    //free(node->left); //!!!
                    //free(node->right);

                    node->left = node->right = NULL;

                    DBG (fprintf (stderr, "%s(): MUL/RIGHT_BY_0, finished. TEMP = %p | type = %d | value = %d(%c)", __func__, node, node->type, (int)node->value, (int)node->value));
                }
                
                else if (node->left->type == NUM && node->left->value == 1) 
                {
                    DBG (printf("\n%s:%d %s(): Applying rule a * 1 = a on Node [%p]\n", __FILE__, __LINE__, __func__, node));
                    struct Node* temp = node->right; // TODO переименовать temp
                    //free(node->left);
                    //(node);

                    DBG (fprintf (stderr, "%s(): MUL/LEFT_BY_1, finished. RESULT = %p | type = %d | value = %d(%c)", __func__, temp, temp->type, (int)temp->value, (int)temp->value));
                    return temp;
                }

                else if (node->right->type == NUM && node->right->value == 1) 
                {
                    DBG (printf("\n%s:%d %s(): Applying rule 1 * a = a on Node [%p]\n", __FILE__, __LINE__, __func__, node));
                    struct Node* temp = node->left;
                    //free(node->right);
                    //free(node);

                    DBG (fprintf (stderr, "%s(): MUL/LEFT_BY_1, finished. RESULT = %p | type = %d | value = %d(%c)", __func__, temp, temp->type, (int)temp->value, (int)temp->value)); 
                    return temp;
                }
                break;

            case DIV: // a / 1 = a
                if (node->right->type == NUM && node->right->value == 1) 
                {
                    DBG (printf("\n%s(): Applying rule a / 1 = a on Node [%p]\n", __func__, node));
                    struct Node* temp = node->left;
                    //free(node->right);
                    //free(node);
                    return temp;
                }
                break;

            default:
                printf("\n%s(): Unknown operation in Node [%p] with value = %d | '%c'\n", __func__, node, (int)node->value, (int)node->value);
        }
    }

    printf ("\n%s(): END\n", __func__);

    return node;
}

double constant_folding (struct Node* node)
{
    assert (node);

    int count_changes = 0;

    if (node->left)
        count_changes += constant_folding (node->left);

    if (node->right)
        count_changes += constant_folding (node->right);

    if (node->type == OP)
        if (node->left->type == NUM && node->right->type == NUM)
        {
            double answer = eval (node);
            fprintf (stderr, "\n%s:%d %s(): node [%p]: answer = %lg\n\n",__FILE__, __LINE__, __func__,  node, answer);

            node->type  = NUM;
            node->value = answer;

            count_changes++;
        }

    return count_changes;
}

struct Node* evaluate_and_simplify (struct Node* node)
{
    for (int i = 0; i < 100; i++)
    {
        int changes = 0;

        struct Node* new_node = simplify(node);
        if (new_node != node) 
        {
            changes++;
            node = new_node; // Если simplify вернул другой узел, обновляем дерево
        }

        if (constant_folding(node) != 0)
            changes++;

        fprintf(stderr, "\nchanges = %d\n\n", changes);

        if (changes == 0)
            break;
    }

    return node;
}

void cleanup_buffer(struct Buffer* buffer) 
{
    if (buffer->buffer) 
    {
        free(buffer->buffer);
        buffer->buffer  = NULL;
        buffer->current = NULL;
    }
}

int delete_node (struct Node* node)
{
    if (node == NULL)
        fprintf (stderr, "node = NULL\n");

    fprintf (stderr, "node [%p], node->left = [%p], node->right = [%p]\n", node, node->left, node->right);

    node->type  = 666;
    node->value = 0;

    node->left  = NULL;
    node->right = NULL;

    //free (node);

    return 0;
}

int delete_tree (struct Node* node)
{
    CHECK_NODE (node, return -1);

    node->value = 0;
    node->type  = 0;

    if (node->left)  delete_tree (node->left);

    if (node->right) delete_tree (node->right);

    //free (node); // ???

    return 0;
}
