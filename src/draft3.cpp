#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

const char* str = "2*(5+3)$";
int p           = 0;

int get_n ();
int get_g ();
int get_e ();
int get_t ();
int get_p ();
int syntax_error ();

int main (void)
{
   int answer = get_g();

    printf ("answer = %d", answer);

    return 0;
}

int get_n ()
{
    int val = 0;
    int old_p = p;
    while ('0' <= str[p] && str[p] <= '9') 
    {
        printf ("\nstr[p] = '%s' \n", str);
        val = val * 10 + str [p] - '0';
        printf ("\nval = %d\n", val);
        p++;
    }

    printf ("\nold_p = %d | p = %d\n", old_p, p);
    if (old_p == p)
    {
        printf ("[%d]", __LINE__);
        syntax_error();
    }
    printf ("[%d] val = %d", __LINE__, val);

    return val;
}

int get_g ()
{
    int val = get_e ();
    printf ("\nval = %d\n", val);
    if (str [p] != '$')
    {
        printf ("[%d]", __LINE__);
        syntax_error();
    }

    p++;
    
    // printf ("answer = %d", val);

    return val;
}

int get_e ()
{
    int val = get_t ();
    while (str[p] == '+' || str[p] == '-')
    {
        int op = str[p];
        p++;
        int val2 = get_t ();
        if (op == '+')
            val += val2;
        else
            val -= val2;
    }

    return val;
}

int get_t ()
{
    int val = get_p ();
    while (str[p] == '*' || str[p] == '/')
    {
        int op = str[p];
        p++;
        int val2 = get_p ();
        if (op == '*')
            val *= val2;
        else
            val /= val2;
    }

    return val; 
}

int get_p ()
{
    if (str[p] == '(')
    {
        p++;

        int val = get_e ();
        if (str[p] != ')')
        {
            printf ("[%d]", __LINE__);
            syntax_error ();
        }

        p++;

        return val;
    }

    else 
        return get_n();
}

int syntax_error ()
{
    printf ("\n%s(): SYTAX_ERROR\n", __func__);
    exit(1);
}


// #include <stdio.h>
// #include <stdlib.h>

// const char* String   = "2*(5+3)$";
// int         Position =   0;

// int GetG (void);
// int GetN (void);
// int GetE (void);
// int GetT (void);
// int GetP (void);
// void SyntaxError (int line);

// int main (void)
// {
//     int answer = GetG ();
//     fprintf (stderr, "\n\nanswer = %d\n\n", answer);

//     return 0;
// }

// int GetG (void)
// {
//     int val = GetE ();

//     if ( String [Position] != '$')
//         SyntaxError (__LINE__);

//     Position++;

//     return val;
// }

// int GetN (void)
// {
//     int val = 0;
//     int old_p = Position;

//     while ('0' <= String[Position] && String[Position] <= '9')
//     {
//         fprintf (stderr, "pos = %d, str = %s, val = %d\n", Position, String, val);
//         val = val * 10 + String[Position] - '0';
//         Position++;
//         fprintf (stderr, "pos = %d, str = %s, val = %d\n", Position, String, val);
//     }

//     if (old_p == Position) SyntaxError (__LINE__);

//     return val;
// }

// int GetE (void)
// {
//     int val = GetT ();

//     while ( String[Position] == '+' || String[Position] == '-' )
//     {
//         int op = String[Position];
//         Position++;
//         int val2 = GetT ();

//         if (op == '+')
//             val += val2;
//         else
//             val -= val2;
//     }

//     return val;
// }

// int GetT (void)
// {
//     int val = GetP ();
//     while ( String[Position] == '*' || String[Position] == '/' )
//     {
//         int op = String[Position];
//         Position++;
//         int val2 = GetP ();

//         if (op == '*')
//             val *= val2;
//         else
//             val /= val2;
//     }

//     return val;
// }

// int GetP (void)
// {
//     if ( String[Position] == '(' )
//     {
//         Position++;

//         int val = GetE ();
//         if (String[Position] != ')')
//             SyntaxError (__LINE__);

//         Position++;
//         return val;
//     }
//     else
//         return GetN ();
// }

// void SyntaxError (int line)
// {
//     fprintf (stderr, "ERROR in read: SyntaxError in %d line\n\n", line);
//     exit (1);
// }