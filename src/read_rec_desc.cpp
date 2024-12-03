//#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

//const char* str = "2^(1+x)+s(x+34)$"; // s(30)+c(45)-e(2)


#include "read_rec_desc.h"
#include "diff.h"
#include "color.h"


// TODO define NEED ('(').... сообщение пользователю что эти тфы не сделаны 

Node* get_n ();
Node* get_e ();
Node* get_t ();
Node* get_p ();
Node* get_v ();

Node* get_func          (); 


int   syntax_error (int line);

const char* str = NULL;
int p           = 0;

int mmain (void)
{
    Node* answer = get_g("log((x+2),(x+66))$"); // 2^(1+x)+s(x+23)$

    // printf ("answer = %d", answer);

    graph_dump (answer, NULL);
    print_in_tex (answer, "output.tex");

    return 0;
}

Node* get_n ()
{
    DBG( printf ("\nstarting %s() Cur = '%.10s' | symbol = '%c' (%d)\n", __func__, str, str[p], str[p]));
    int val   = 0;
    int old_p = p;
    while ('0' <= str[p] && str[p] <= '9') 
    {
        DBG( printf ("\nstr[p] = '%s' \n", str));
        val = val * 10 + str [p] - '0';
        DBG( printf ("\nval = %d\n", val));
        p++;
    }

    DBG( printf ("\nold_p = %d | p = %d\n", old_p, p));
    if (old_p == p)
    {
        syntax_error(__LINE__);
    }
    DBG( printf ("[%d] %s(): val = %d symbol = '%c'\n", __LINE__, __func__, val, str[p]));

    return new_node (NUM, val, NULL, NULL); 
}

Node* get_g (const char* input)
{
    str = input;

    Node* val = get_e ();
    DBG( printf ("\n%s():val = %d\n", __func__, val));
    if (str [p] != '$')
    {
        syntax_error (__LINE__);
    }

    p++;

    return val;
}

Node* get_e ()
{
    DBG( printf ("\nstarting %s() Cur = '%.10s' | symbol = '%c' (%d)\n", __func__, str, str[p], str[p]));
    Node* val = get_t ();
    while (str[p] == '+' || str[p] == '-')
    {
        int op = str[p];
        p++;
        Node* val2 = get_t();
        val = new_node(OP, op, val, val2);
        // Node* val2 = get_t ();
        // if (op == '+')
        //     val = new_node (OP, op, val, val2);
        //     //val += val2;
        // else
        //     val = new_node (OP, op, val, val2); 
            //val -= val2;
    }

    return val;
}

Node* get_t ()
{
    DBG( printf ("\nstarting %s() Cur = '%.10s' | symbol = '%c' (%d)\n", __func__, str, str[p], str[p]));
    Node* val = get_p ();
    while (str[p] == '*' || str[p] == '/' || str[p] == '^')
    {
        int op = str[p];
        p++;
        Node* val2 = get_p(); // Node* val2 = get_pow();
        val = new_node(OP, op, val, val2);
        // Node* val2 = get_p ();
        // if (op == '*')
        //     val = new_node (OP, op, val, val2); // ????
        // else
        //     val = new_node (OP, op, val, val2);
    }

    return val; 
}

Node* get_v() 
{
    DBG (printf ("\nstarting %s()\n", __func__));
    if (str[p] == 'x')  
    {
        DBG( printf(BLUE_TEXT("Detected variable: ") "x\n"));
        p++;

        return new_node(VAR, 'x', NULL, NULL);
    }

    return NULL;
}

Node* get_p() 
{
    DBG( printf ("\nstarting %s() Cur = '%.10s' | symbol = '%c' (%d)\n",
         __func__, str, str[p], str[p]));
    if (str[p] == '(') 
    {
        p++; 

        Node* val = get_e(); 

        if (str[p] != ')') 
        {
            DBG( printf("[%d] Error: Missing closing parenthesis\n", __LINE__));
            syntax_error(__LINE__);
        }

        p++; 
        return val; 
    } 
    // isalpha
    else if (isalpha(str[p])) // Если символ - это буква, значит это переменная или функция
    {
        if (str[p] == 'x') 
        {
            DBG( printf ("\n                 Cur = '%.10s' | symbol = '%c' (%d)\n", str, str[p], str[p]));
            return get_v();
        }

        else  
        {
            DBG( printf ("\n                 Cur = '%.10s' | symbol = '%c' (%d)\n", str, str[p], str[p]));
            return get_func(); 
        }
    }

    else 
    {
        DBG( printf ("\n                 Cur = '%.10s' | symbol = '%c' (%d)\n", str, str[p], str[p]));
        return get_n();
    }
}

int syntax_error (int line)
{
    printf ("\n%s(): SYTAX_ERROR: '%s'\n", __func__, str + p);
        fprintf (stderr, "ERROR in read: syntax_error in %d line, CUR = '%.10s...', SYMBOL = '%c' (%d)\n\n",
                     line, str, str[p], str[p]);
    exit(1);
}

/*===============================================================================*/

Node* get_func() //TODO сделать логарифм  
{
    printf (BLUE_TEXT("Detected function: ") "%c\n", str[p]);

    char func_char = str[p];  
    int func_type  = 0;

    if (func_char == 's') 
        func_type = SIN;
    else if (func_char == 'c') 
        func_type = COS;
    else if (func_char == 't')
        func_type = TG;
    else if (func_char == 'e') 
        func_type = EXP;
    else if (func_char == 'p')
        func_type = POW;  
    else if (func_char == 'r') 
        func_type = ROOT; 
    else if  (func_char =='^')
        func_type = POW;
    else if (func_char == 'l')
    {
        if (str[p + 1] == 'n') 
        {
            func_type = LN; 
            p++; 
        }

        else if (str[p + 1] == 'o' && str[p + 2] == 'g') 
        {
            func_type = LOG; 
            p += 2; 
        }

        else
        {
            printf(RED_TEXT("\n[%d] Error: Invalid function starting with 'l'\n"), __LINE__);
            syntax_error(__LINE__);
        }
    }

    else 
    {
        printf("[%d] Error: Invalid function '%c'\n", __LINE__, func_char);
        syntax_error(__LINE__);
    }
    
    printf ("\n%s(): func_type = '%c' | %d\n", __func__, func_type, func_type);

    // int func_type = 0;
    // switch (str[p]) 
    // {
    //     case 's': func_type = SIN;  break; 
    //     case 'c': func_type = COS;  break; 
    //     case 'e': func_type = EXP;  break; 
    //     case 'l': func_type = LOG;  break; 
    //     case 'r': func_type = ROOT; break; 
        
    //     default:
    //         printf (RED_TEXT ("\nError: Unsupported function '%c' detected\n"), str[p]);
    //         syntax_error(__LINE__);
    // }

    p++; 

    Node* indicator = NULL;
    if (func_type == POW) 
    {
        if (str[p] == '^') 
        {
            printf ("[%d] %s(): str[p] = '%c'", __LINE__, __func__, str[p]);
            p++; 
        } 

        else 
        {
            printf (RED_TEXT ("\n[%d] %s(): Error: Expected '^' after base in exponentiation\n"), __LINE__, __func__);
            syntax_error(__LINE__);
        }

        Node* base = get_e();

        if (str[p] != '(') 
        {
            printf (RED_TEXT ("\n[%d] %s(): ") "Error: Expected '(' after '^' for exponentiation\n", __LINE__, __func__);
            syntax_error(__LINE__);
        }

        p++; 

        Node* indicator = get_e(); 

        if (str[p] != ')') 
        {
            printf(RED_TEXT ("\n[%d] %s(): ") "Error: Missing closing ')' in exponentiation\n",__LINE__, __func__);
            syntax_error(__LINE__);
        }

        p++; 

        return new_node(OP, func_type, base, indicator);
    }

    if (func_type == LOG) // Обработка log(a, b)
    {
        if (str[p] != '(')
        {
            printf (RED_TEXT("\n[%d] %s(): Expected '(' after 'log'\n"), __LINE__, __func__);
            syntax_error(__LINE__);
        }

        p++; 

        Node* base = get_e();

        if (str[p] != ',') 
        {
            printf (RED_TEXT("\n[%d] %s(): Expected ',' after base in 'log'\n"), __LINE__, __func__);
            syntax_error(__LINE__);
        }

        p++; 

        Node* argument = get_e();

        if (str[p] != ')') 
        {
            printf (RED_TEXT("\n[%d] %s(): Missing closing ')' in 'log(a, b)'\n"), __LINE__, __func__);
            syntax_error(__LINE__);
        }

        p++; 

        return new_node(FUNC, LOG, base, argument);
    }

    if (func_type == LN) // Обработка ln(x)
    {
        if (str[p] != '(') 
        {
            printf (RED_TEXT("\n[%d] %s(): Expected '(' after 'ln'\n"), __LINE__, __func__);
            syntax_error(__LINE__);
        }

        p++; 

        Node* argument = get_e();

        if (str[p] != ')') 
        {
            printf (RED_TEXT("\n[%d] %s(): Missing closing ')' in 'ln(x)'\n"), __LINE__, __func__);
            syntax_error(__LINE__);
        }

        p++; 

        return new_node(FUNC, LN, argument, NULL); 
    }    

    if (str[p] != '(') 
    {
        printf(RED_TEXT ("\n[%d] %s(): ") "Error: Expected '(' after function name '%c'\n", __LINE__, __func__, str[p - 1]);
        syntax_error(__LINE__);
    }

    p++; 

    Node* arg = get_e();

    if (str[p] != ')') 
    {
        printf(RED_TEXT ("\n[%d] %s(): ) Error: Missing closing ')' in function '%c'\n"), __LINE__, __func__, str[p - 1] );
        syntax_error(__LINE__);
    }

    p++; 

    return new_node(FUNC, func_type, arg, NULL); 
}

Node* get_printf ()
{
        // printf ("\nstarting %s() Cur = '%.10s' | symbol = '%c' (%d)\n",__func__, str, str[p], str[p]);
        // if (str[p] == 'p') // printf
        // {
        //     p+=5;

        //     Node* 
        // }
}

// Node* get_if() 
// {
//     DBG(printf("\nStarting %s() Cur = '%.10s'\n", __func__, str + p));

//     Node* if_node = new_node (NODE_IF); 

//     if (strncmp(&str[p], "if", 2) == 0) 
//     {
//         p += 2; 
//     } 

//     else 
//     {
//         DBG(printf("[%d] Error: Expected 'if', found '%.10s'\n", __LINE__, str + p));
//         syntax_error(__LINE__);
//     }

//     if (str[p] != '(') 
//     {
//         DBG(printf("[%d] Error: Expected '(', found '%c'\n", __LINE__, str[p]));
//         syntax_error(__LINE__);
//     }
//     p++; 

//     if_node->left = get_e(); 

//     if (str[p] != ')') 
//     {
//         DBG(printf("[%d] Error: Expected ')', found '%c'\n", __LINE__, str[p]));
//         syntax_error(__LINE__);
//     }
//     p++; 

//     if (str[p] == 'x' && str[p + 1] == '=') 
//     {  
//         if_node->right = new_node (NODE_ASSIGN);  
//         if_node->right->left = new_node (NODE_VAR, 'x');  
//         p += 2;  // Пропуск 'x='
//         if_node->right->right = get_e();  
//     } 

//     else if (isalpha(str[p])) 
//     {  
//         if_node->right = get_func(); 
//     } 
//     else 
//     {
//         DBG(printf("[%d] Error: Expected assignment or function call, found '%c'\n", __LINE__, str[p]));
//         syntax_error (__LINE__);
//     }

//     return if_node;
// }


/*================================/разделение get_func(), не правильное/=========================================================*/
// Node* get_func() 
// {
//     printf(BLUE_TEXT("Detected function: ") "%c\n", str[p]); 

//     // Определение типа функции
//     int func_type = get_function_type();
    
//     // Обработка на основе типа функции
//     if (func_type == POW) 
//     {
//         return process_pow();
//     } 

//     else if (func_type == LOG) 
//     {
//         return process_log();
//     } 

//     else if (func_type == LN) 
//     {
//         return process_ln();
//     } 

//     else 
//     {
//         return process_basic_function(func_type);
//     }
// }

// int get_function_type () 
// {
//     char func_char = str[p];
    
//     if (func_char == 's') 
//         return SIN;
//     else if (func_char == 'c') 
//         return COS;
//     else if (func_char == 't') 
//         return TG;
//     else if (func_char == 'e') 
//         return EXP;
//     else if (func_char == 'p') 
//         return POW;  
//     else if (func_char == 'r') 
//         return ROOT; 
//     else if (func_char == '^')
//         return POW;
//     else if (func_char == 'l') 
//     {
//         // Разделение между LN и LOG
//         if (str[p + 1] == 'n') 
//         {
//             p++;
//             return LN; 
//         } 

//         else if (str[p + 1] == 'o' && str[p + 2] == 'g') 
//         {
//             p += 2;
//             return LOG; 
//         } 

//         else 
//         {
//             printf(RED_TEXT("\n[%d] Error: Invalid function starting with 'l'\n"), __LINE__);
//             syntax_error(__LINE__);
//         }
//     }
     
//     else 
//     {
//         printf(RED_TEXT("\n[%d] Error: Invalid function '%c'\n"), __LINE__, func_char);
//         syntax_error(__LINE__);
//     }

//     return -1; 
// }

// Node* process_ln ()
// {
//     if (str[p] != '(') 
//     {
//         printf(RED_TEXT("\n[%d] process_ln(): Expected '(' after 'ln'\n"), __LINE__);
//         syntax_error(__LINE__);
//     }

//     p++; 

//     Node* argument = get_e(); // Считываем аргумент

//     if (str[p] != ')') {
//         printf(RED_TEXT("\n[%d] process_ln(): Missing closing ')' in 'ln(x)'\n"), __LINE__);
//         syntax_error(__LINE__);
//     }

//     p++; // Пропускаем ')'

//     return new_node(FUNC, LN, argument, NULL); 
// }

// Node* process_log () 
// {
//     if (str[p] != '(') 
//     {
//         printf(RED_TEXT("\n[%d] process_log(): Expected '(' after 'log'\n"), __LINE__);
//         syntax_error(__LINE__);
//     }

//     p++; // Пропускаем '('

//     Node* base = get_e(); // Считываем основание логарифма

//     if (str[p] != ',') 
//     {
//         printf(RED_TEXT("\n[%d] process_log(): Expected ',' after base in 'log'\n"), __LINE__);
//         syntax_error(__LINE__);
//     }

//     p++; // Пропускаем ','

//     Node* argument = get_e(); // Считываем аргумент логарифма

//     if (str[p] != ')') 
//     {
//         printf(RED_TEXT("\n[%d] process_log(): Missing closing ')' in 'log(a, b)'\n"), __LINE__);
//         syntax_error(__LINE__);
//     }

//     p++; 

//     return new_node(FUNC, LOG, base, argument);
// }

// Node* process_pow() 
// {
//     if (str[p] == '^')
//     {
//         p++; // Пропускаем '^'
//     } 
    
//     else 
//     {
//         printf(RED_TEXT("\n[%d] process_pow(): Expected '^' for power operation\n"), __LINE__);
//         syntax_error(__LINE__);
//     }

//     Node* base = get_e(); // Получаем базу

//     if (str[p] != '(') 
//     {

//         printf(RED_TEXT("\n[%d] process_pow(): Expected '(' after '^' for power operation\n"), __LINE__);
//         syntax_error(__LINE__);
//     }

//     p++; // Пропускаем '('

//     Node* exponent = get_e(); // Получаем показатель степени

//     if (str[p] != ')') 
//     {
//         printf(RED_TEXT("\n[%d] process_pow(): Missing closing ')' in power operation\n"), __LINE__);
//         syntax_error(__LINE__);
//     }

//     p++; // Пропускаем ')'

//     return new_node(OP, POW, base, exponent);
// }

// Node* process_basic_function (int func_type)
// {
//     if (str[p] != '(') 
//     {
//         printf(RED_TEXT("\n[%d] process_basic_function(): Expected '(' after function name\n"), __LINE__);
//         syntax_error(__LINE__);
//     }

//     p++; // Пропускаем '('

//     Node* argument = get_e(); // Считываем аргумент функции

//     if (str[p] != ')') 
//     {
//         printf(RED_TEXT("\n[%d] process_basic_function(): Missing closing ')' in function\n"), __LINE__);
//         syntax_error(__LINE__);
//     }

//     p++; // Пропускаем ')'

//     return new_node(FUNC, func_type, argument, NULL);
// }