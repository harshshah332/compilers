/* File: parser.y
 * --------------
 * Bison input file to generate the parser for the compiler.
 *
 * pp2: your job is to write a parser that will construct the parse tree
 *      and if no parse errors were found, print it.  The parser should
 *      accept the language as described in specification, and as augmented
 *      in the pp2 handout.
 */

%{

/* Just like lex, the text within this first region delimited by %{ and %}
 * is assumed to be C/C++ code and will be copied verbatim to the y.tab.c
 * file ahead of the definitions of the yyparse() function. Add other header
 * file inclusions or C++ variable declarations/prototypes that are needed
 * by your code here.
 */
#include "scanner.h" // for yylex
#include "parser.h"
#include "errors.h"

void yyerror(const char *msg); // standard error-handling routine

%}

/* The section before the first %% is the Definitions section of the yacc
 * input file. Here is where you declare tokens and types, add precedence
 * and associativity options, and so on.
 */

/* yylval
 * ------
 * Here we define the type of the yylval global variable that is used by
 * the scanner to store attibute information about the token just scanned
 * and thus communicate that information to the parser.
 *
 * pp2: You will need to add new fields to this union as you add different
 *      attributes to your non-terminal symbols.
 */


%union {
    int integerConstant;
    bool boolConstant;
    float floatConstant;
    char identifier[MaxIdentLen+1]; // +1 for terminating null
    Decl *decl;
    List<Decl*> *declList;



    VarDecl *vardecl;
    List<VarDecl*> *vardecls;
    
    FnDecl  *fndecl;


    Expr *expr;
    Expr *emptyexpr;
    List<Expr*> *exprlist;
    Call *call;
    VarExpr *varexpr;

  
    SelectionExpr  *selectionexpr;


    
    LValue *lvalue;


    Program *program;

    Stmt *stmt;
    List<Stmt*> *stmts;
    StmtBlock *stmtblock;
    ConditionalStmt *conditionalstmt;
    LoopStmt *loopstmt;

    ForStmt *forstmt;
    WhileStmt *whilestmt;
    DoWhileStmt *dowhilestmt;
    IfStmt *ifstmt;
    ReturnStmt *returnstmt;
   
    SwitchStmt *switchstmt;
    SwitchLabel *switchlabel;
    Case *casestmt;
    List<Case*> *caselist;
    Default *defaultcase;

    Type *type;
    TypeQualifier *typequalifier;
    NamedType *namedtype;
    ArrayType *arraytype;


   
    ArithmeticExpr *arithmeticexpr;
    RelationalExpr *relationalexpr;
    EqualityExpr   *equalityexpr;
    LogicalExpr    *logicalexpr;
    AssignExpr     *assignexpr;
    PostfixExpr    *postfixexpr;





}


/* Tokens
 * ------
 * Here we tell yacc about all the token types that we are using.
 * Bison will assign unique numbers to these and export the #define
 * in the generated y.tab.h header file.
 */
%token   T_Void T_Bool T_Int T_Float
%token   T_LessEqual T_GreaterEqual T_EQ T_NE T_LeftAngle T_RightAngle
%token   T_And T_Or
%token   T_Equal T_MulAssign T_DivAssign T_AddAssign T_SubAssign
%token   T_While T_For T_If T_Else T_Return T_Break
%token   T_Const T_Uniform T_Layout T_Continue T_Do
%token   T_Inc T_Dec T_Switch T_Case T_Default
%token   T_In T_Out T_InOut
%token   T_Mat2 T_Mat3 T_Mat4 T_Vec2 T_Vec3 T_Vec4
%token   T_Ivec2 T_Ivec3 T_Ivec4 T_Bvec2 T_Bvec3 T_Bvec4
%token   T_Uint T_Uvec2 T_Uvec3 T_Uvec4 T_Struct
%token   T_Semicolon T_Dot T_Colon T_Question T_Comma
%token   T_Dash T_Plus T_Star T_Slash
%token   T_LeftParen T_RightParen T_LeftBracket T_RightBracket T_LeftBrace T_RightBrace

%token   <identifier> T_Identifier
%token   <integerConstant> T_IntConstant
%token   <floatConstant> T_FloatConstant
%token   <boolConstant> T_BoolConstant

/* Non-terminal types
 * ------------------
 * In order for yacc to assign/access the correct field of $$, $1, we
 * must to declare which field is appropriate for the non-terminal.
 * As an example, this first type declaration establishes that the DeclList
 * non-terminal uses the field named "declList" in the yylval union. This
 * means that when we are setting $$ for a reduction for DeclList ore reading
 * $n which corresponds to a DeclList nonterminal we are accessing the field
 * of the union named "declList" which is of type List<Decl*>.
 * pp2: You'll need to add many of these of your own.
 */

%type <program>       Program
%type <declList>      DeclList
%type <decl>          Decl
%type <vardecl>       VarDecl Variable
%type <vardecls>      VarDecls Formals Variables
%type <type>          Type
%type <typequalifier> TypeQualifier
%type <fndecl>        FnDecl



//%type <namedtype>     NamedType
%type <arraytype>     ArrayType
%type <stmt>          Stmt
%type <stmtblock>     StmtBlock
%type <stmts>         Stmts
%type <ifstmt>        IfStmt
%type <whilestmt>     WhileStmt
%type <forstmt>       ForStmt
%type <returnstmt>    ReturnStmt


%type <switchstmt>    SwitchStmt
%type <casestmt>      CaseStmt
%type <caselist>  CaseList
%type <defaultcase>   DefaultCase



%type <expr>          Expr Constant
%type <exprlist>    Exprlist Actuals
%type <emptyexpr>     EmptyExpr
%type <varexpr> VarExpr


%type <arithmeticexpr> ArithmeticExpr
%type <relationalexpr> RelationalExpr
%type <equalityexpr>   EqualityExpr
%type <logicalexpr>    LogicalExpr
%type <assignexpr>     AssignExpr
%type <postfixexpr>    PostfixExpr


%type <call>          Call

%type <lvalue>        LValue



%nonassoc LOWER_ELSE
%nonassoc T_Else

/*
%nonassoc '='
%left     T_Or
%left     T_And 
%nonassoc T_Equal T_NotEqual
%nonassoc '<' T_LessEqual '>' T_GreaterEqual
%left     '+' '-' 
%left     '*' '/' '%'
%nonassoc '!'  T_Inc T_Dec
%nonassoc '[' '.'
*/

%%
/* Rules
 * -----
 * All productions and actions should be placed between the start and stop
 * %% markers which delimit the Rules section.
 */
Program   :    DeclList            {
                                      @1;
                                      /* pp2: The @1 is needed to convince
                                       * yacc to set up yylloc. You can remove
                                       * it once you have other uses of @n*/
                                      Program *program = new Program($1);
                                      // if no errors, advance to next phase
                                      if (ReportError::NumErrors() == 0)
                                          program->Print(0);
                                    }
          ;

DeclList  :    DeclList Decl        { ($$=$1)->Append($2); }
          |    Decl                 { ($$ = new List<Decl*>)->Append($1); }
          ;

Decl      :    VarDecl               {$$ =  $1;}
          |    FnDecl                 {$$ =  $1;}
          ;


VarDecl   : Variable T_Semicolon         


          ;



Variable    : Type T_Identifier       {
                                                 // replace it with your implementation
                                                 Identifier *id = new Identifier(@2, $2);
                                                 $$ = new VarDecl(id, $1);
                                                 }


            | Type T_Identifier T_Equal Expr         
                                                {
                                                 // replace it with your implementation
                                                 Identifier *id = new Identifier(@2, $2);
                                                 $$ = new VarDecl(id, $1, $4);
                                                 }


           | TypeQualifier Type T_Identifier           
                                              {
                                                 
                                                 Identifier *id = new Identifier(@3, $3);
                                                 $$ = new VarDecl(id, $2, $1);
                                              }      




           | TypeQualifier Type T_Identifier T_Equal Expr           
                                              {
                                                 
                                                 Identifier *id = new Identifier(@3, $3);
                                                 $$ = new VarDecl(id, $2, $1, $5);
                                              }  


           | TypeQualifier T_Identifier           
                                              {
                                                 
                                                 Identifier *id = new Identifier(@2, $2);
                                                 $$ = new VarDecl(id, $1);
                                              }    
           | TypeQualifier T_Identifier T_Equal Expr               
                                              {
                                                 
                                                 Identifier *id = new Identifier(@2, $2);
                                                 $$ = new VarDecl(id, $1, $4);
                                              } 
           | Type T_Identifier T_LeftBracket Constant T_RightBracket 
            {
             Identifier *id = new Identifier(@2, $2);
             ArrayType *t = new ArrayType(@1, $1);
             $$ = new VarDecl(id, t);}

          ;



Type      :    T_Int                 { $$ = Type::intType; }
          |    T_Float               { $$ = Type::floatType; }
          |    T_Bool                { $$ = Type::boolType; }
          |    T_Void                { $$ = Type::voidType; }
          |    T_Mat2                { $$ = Type::mat2Type; }
          |    T_Mat3                { $$ = Type::mat3Type; }
          |    T_Mat4                { $$ = Type::mat4Type; }
          |    T_Vec2                { $$ = Type::vec2Type; }
          |    T_Vec3                { $$ = Type::vec3Type; }
          |    T_Vec4                { $$ = Type::vec4Type; }
          |    T_Ivec2               { $$ = Type::ivec2Type; }
          |    T_Ivec3               { $$ = Type::ivec3Type; }
          |    T_Ivec4               { $$ = Type::ivec4Type; }
          |    T_Bvec2               { $$ = Type::bvec2Type; }
          |    T_Bvec3               { $$ = Type::bvec3Type; }
          |    T_Bvec4               { $$ = Type::bvec4Type; }
          |    T_Uint                { $$ = Type::uintType; }
          |    T_Uvec2               { $$ = Type::uvec2Type; }
          |    T_Uvec3               { $$ = Type::uvec3Type; }
          |    T_Uvec4               { $$ = Type::uvec4Type; }
    //       |    NamedType
          |    ArrayType             { $$ = $1; } 
          ;


TypeQualifier   : T_In               { $$ = TypeQualifier::inTypeQualifier; }
                | T_Out              { $$ = TypeQualifier::outTypeQualifier; }
                | T_Const            { $$ = TypeQualifier::constTypeQualifier; }
                | T_Uniform          { $$ = TypeQualifier::uniformTypeQualifier; }

                ;


FnDecl    :    Type T_Identifier T_LeftParen Formals T_RightParen StmtBlock
                                     { $$ = new FnDecl(new Identifier(@2, $2), $1, $4); 
                                       $$->SetFunctionBody($6); }
          |    T_Void T_Identifier T_LeftParen Formals T_RightParen StmtBlock
                                     { $$ = new FnDecl(new Identifier(@2, $2), Type::voidType, $4); 
                                       $$->SetFunctionBody($6); }
          ;

Formals   :    Variables             { $$ = $1; }
          |                          { $$ = new List<VarDecl*>; }
          ;

Variables :    Variables T_Comma Variable            { ($$ = $1)->Append($3 ); }
          |    Variable                      { ($$ = new List<VarDecl*>)->Append($1); }
          ;

ArrayType :    Type T_Identifier T_LeftBracket Constant T_RightBracket   
                 { $$ = new ArrayType(@1, $1); } 
     
Expr       : 
             Call                        { $$ =  $1;} 
           | Constant                    { $$ =  $1;} 
           |  T_LeftParen Expr T_RightParen           { $$ = $2; }
           | VarExpr                     { $$ =  $1;} 
           | LValue                      { $$ =  $1;}
           | AssignExpr                  { $$ =  $1;}
           | ArithmeticExpr                      { $$ =  $1;}
           | PostfixExpr                  { $$ =  $1;}
           | EqualityExpr                      { $$ =  $1;}
           | RelationalExpr                  { $$ =  $1;}
           | LogicalExpr                      { $$ =  $1;}
           
/*
           | Expr T_Equal Expr           { $$ = new AssignExpr($1, new Operator(@2, "="), $3); } 
           | Expr T_MulAssign Expr       { $$ = new AssignExpr($1, new Operator(@2, "*="), $3); } 
           | Expr T_DivAssign Expr       { $$ = new AssignExpr($1, new Operator(@2, "/="), $3); } 
           | Expr T_AddAssign Expr       { $$ = new AssignExpr($1, new Operator(@2, "+="), $3); } 
           | Expr T_SubAssign Expr       { $$ = new AssignExpr($1, new Operator(@2, "-="), $3); }

           | Expr T_Plus Expr            { $$ = new ArithmeticExpr($1, new Operator(@2, "+"), $3); } 
           | Expr T_Dash Expr            { $$ = new ArithmeticExpr($1, new Operator(@2, "-"), $3); } 
           | Expr T_Star Expr            { $$ = new ArithmeticExpr($1, new Operator(@2, "*"), $3); } 
           | Expr T_Slash Expr           { $$ = new ArithmeticExpr($1, new Operator(@2, "/"), $3); }
           | Expr '%' Expr               { $$ = new ArithmeticExpr($1, new Operator(@2, "%"), $3); }
           | T_Inc Expr                  { $$ = new ArithmeticExpr( new Operator(@2, "++"), $2); }
           | T_Dec Expr                  { $$ = new ArithmeticExpr( new Operator(@2, "--"), $2); }

           | VarExpr T_Inc                { $$ = new PostfixExpr( $1, new Operator(@2, "++")); }
           | VarExpr T_Dec                { $$ = new PostfixExpr( $1, new Operator(@2, "--")); }


           | Expr T_EQ Expr              { $$ = new EqualityExpr($1, new Operator(@2, "=="), $3); }
           | Expr T_NE Expr              { $$ = new EqualityExpr($1, new Operator(@2, "!="), $3); }  

           | Expr T_LeftAngle Expr       { $$ = new RelationalExpr($1, new Operator(@2, "<"), $3); }
           | Expr T_RightAngle Expr      { $$ = new RelationalExpr($1, new Operator(@2, ">"), $3); } 
           | Expr T_LessEqual Expr       { $$ = new RelationalExpr($1, new Operator(@2, "<="), $3); }   
           | Expr T_GreaterEqual Expr    { $$ = new RelationalExpr($1, new Operator(@2, ">="), $3); }

           | Expr T_And Expr             { $$ = new LogicalExpr($1, new Operator(@2, "&&"), $3); }
           | Expr T_Or Expr              { $$ = new LogicalExpr($1, new Operator(@2, "||"), $3); }

*/
           ;

AssignExpr     : Expr T_Equal Expr           { $$ = new AssignExpr($1, new Operator(@2, "="), $3); } 
               | Expr T_MulAssign Expr       { $$ = new AssignExpr($1, new Operator(@2, "*="), $3); } 
               | Expr T_DivAssign Expr       { $$ = new AssignExpr($1, new Operator(@2, "/="), $3); } 
               | Expr T_AddAssign Expr       { $$ = new AssignExpr($1, new Operator(@2, "+="), $3); } 
               | Expr T_SubAssign Expr       { $$ = new AssignExpr($1, new Operator(@2, "-="), $3); }

                ;



ArithmeticExpr : Expr T_Plus Expr       { $$ = new ArithmeticExpr($1, new Operator(@2, "+"), $3); }
               | Expr T_Dash Expr       { $$ = new ArithmeticExpr($1, new Operator(@2, "-"), $3); } 
               | Expr T_Star Expr       { $$ = new ArithmeticExpr($1, new Operator(@2, "*"), $3); }
               | Expr T_Slash Expr      { $$ = new ArithmeticExpr($1, new Operator(@2, "/"), $3); }
               | Expr '%' Expr          { $$ = new ArithmeticExpr($1, new Operator(@2, "%"), $3); }
               | T_Inc Expr             { $$ = new ArithmeticExpr( new Operator(@2, "++"), $2); }
               | T_Dec Expr             { $$ = new ArithmeticExpr( new Operator(@2, "--"), $2); }
               ;



PostfixExpr    : VarExpr T_Inc                { $$ = new PostfixExpr( $1, new Operator(@2, "++")); }
               | VarExpr T_Dec                { $$ = new PostfixExpr( $1, new Operator(@2, "--")); }

               ;


EqualityExpr   : Expr T_EQ Expr              { $$ = new EqualityExpr($1, new Operator(@2, "=="), $3); }
               | Expr T_NE Expr              { $$ = new EqualityExpr($1, new Operator(@2, "!="), $3); } 

               ;


RelationalExpr : Expr T_LeftAngle Expr       { $$ = new RelationalExpr($1, new Operator(@2, "<"), $3); }
               | Expr T_RightAngle Expr      { $$ = new RelationalExpr($1, new Operator(@2, ">"), $3); } 
               | Expr T_LessEqual Expr       { $$ = new RelationalExpr($1, new Operator(@2, "<="), $3); }   
               | Expr T_GreaterEqual Expr    { $$ = new RelationalExpr($1, new Operator(@2, ">="), $3); }

               ;

LogicalExpr    : Expr T_And Expr             { $$ = new LogicalExpr($1, new Operator(@2, "&&"), $3); }
               | Expr T_Or Expr              { $$ = new LogicalExpr($1, new Operator(@2, "||"), $3); }

               ;

VarExpr    : T_Identifier         {  Identifier *id = new Identifier(@1, $1);
                                     $$ = new VarExpr(@1, id);
                                  }

Exprlist   : Exprlist T_Comma Expr          { ($$ = $1)->Append($3); }
           | Expr                    { ($$ = new List<Expr*>)->Append($1); }
           ; 

EmptyExpr  : Expr                    {$$ =  $1;}
           |                         { $$ = new EmptyExpr(); }
           ;
 
            
LValue     : 
          //   T_Identifier                                { $$ = new FieldAccess(NULL, new Identifier(@1, $1)); }  
             Expr T_Dot T_Identifier                     { $$ = new FieldAccess($1, new Identifier(@3, $3)); }
           | Expr T_LeftBracket Expr T_RightBracket      { $$ = new ArrayAccess(Join(@1, @4), $1, $3); }
           ; 


        
Actuals    : Exprlist                { $$ = $1; }
           |                         { $$ = new List<Expr*>; }
           ;
           
Constant   : T_IntConstant        { $$ = new IntConstant(@1,$1); }    
           | T_FloatConstant       { $$ = new FloatConstant(@1, $1); }
           | T_BoolConstant      { $$ = new BoolConstant(@1, $1); }
          
           ;
              
Stmt       : EmptyExpr T_Semicolon   { $$ = $1; }
           | IfStmt  {$$ =  $1;}
           | WhileStmt  {$$ =  $1;}
           | ForStmt  {$$ =  $1;}
           | T_Break T_Semicolon             { $$ = new BreakStmt(@1); }     
           | ReturnStmt  {$$ =  $1;}
           | SwitchStmt   {$$ =  $1;}
           | StmtBlock  {$$ =  $1;}
           ;



Stmts      : Stmts Stmt              { ($$ = $1)->Append($2); }
           | Stmt                    { ($$ = new List<Stmt*>)->Append($1);  }
           ;

StmtBlock  : T_LeftBrace VarDecls Stmts T_RightBrace  { $$ = new StmtBlock($2, $3); }
           | T_LeftBrace VarDecls T_RightBrace         { $$ = new StmtBlock($2, new List<Stmt*>); }
           ;

IfStmt     : T_If T_LeftParen Expr T_RightParen Stmt       %prec LOWER_ELSE        { $$ = new IfStmt($3, $5, NULL); }
           | T_If T_LeftParen Expr T_RightParen Stmt T_Else Stmt  { $$ = new IfStmt($3, $5, $7); }
           ;
                                     
           
WhileStmt  : T_While T_LeftParen Expr T_RightParen Stmt
                                     { $$ = new WhileStmt($3, $5); }
           ;
           
ForStmt    : T_For T_LeftParen EmptyExpr T_Semicolon Expr T_Semicolon EmptyExpr T_RightParen Stmt
                                     { $$ = new ForStmt($3, $5, $7, $9); }
           ;
           
ReturnStmt : T_Return EmptyExpr T_Semicolon    { $$ = new ReturnStmt(@2, $2); }
           ;
       
        
SwitchStmt : T_Switch T_LeftParen Expr T_RightParen T_LeftBrace CaseList DefaultCase T_RightBrace
                                     { $$ = new SwitchStmt($3, $6, $7); } 
           ;

CaseList   : CaseList CaseStmt           { ($$ = $1)->Append($2); }
           | CaseStmt                    { ($$ = new List<Case*>)->Append($1); }
           ;

CaseStmt   : T_Case Constant T_Colon Stmts        { $$ = new Case($2, $4); }
                                              
           | T_Case Constant T_Colon              { $$ = new Case($2, new List<Stmt*>); }
           ;

DefaultCase  : T_Default T_Colon Stmts         { $$ = new Default($3); }
             |                                     { $$ = NULL; }
             ;


           

VarDecls   : VarDecls VarDecl        { ($$ = $1)->Append($2);    }
           |                         { $$ = new List<VarDecl*>;  }



Call       : T_Identifier T_LeftParen Actuals T_RightParen 
                                     { $$ = new Call(Join(@1, @4), NULL, new Identifier(@1, $1), $3); }  

           | Expr T_Dot T_Identifier T_LeftParen Actuals T_RightParen
                                     { $$ = new Call(Join(@1, @6), $1, new Identifier(@3, $3), $5); }


// Do we need these hard coded types for calls. ex) int x = bvec(2.0); 
// Do we need them for ints/bool/float? Do having them mess up other code in the parse tree?

           | T_Int   T_LeftParen Actuals T_RightParen 
                                     { $$ = new Call(Join(@1, @4), NULL, new Identifier(@1, "int"), $3); }  
           | T_Float T_LeftParen Actuals T_RightParen 
                                     { $$ = new Call(Join(@1, @4), NULL, new Identifier(@1, "float"), $3); }   
           | T_Bool  T_LeftParen Actuals T_RightParen 
                                     { $$ = new Call(Join(@1, @4), NULL, new Identifier(@1, "bool"), $3); }   



           | T_Vec2 T_LeftParen Actuals T_RightParen 
                                     { $$ = new Call(Join(@1, @4), NULL, new Identifier(@1, "vec2"), $3); }  
           | T_Vec3 T_LeftParen Actuals T_RightParen 
                                     { $$ = new Call(Join(@1, @4), NULL, new Identifier(@1, "vec3"), $3); }   
           | T_Vec4 T_LeftParen Actuals T_RightParen 
                                     { $$ = new Call(Join(@1, @4), NULL, new Identifier(@1, "vec4"), $3); }    

           | T_Ivec2 T_LeftParen Actuals T_RightParen 
                                     { $$ = new Call(Join(@1, @4), NULL, new Identifier(@1, "ivec2"), $3); }   
           | T_Ivec3 T_LeftParen Actuals T_RightParen 
                                     { $$ = new Call(Join(@1, @4), NULL, new Identifier(@1, "ivec3"), $3); }    
           | T_Ivec4 T_LeftParen Actuals T_RightParen 
                                     { $$ = new Call(Join(@1, @4), NULL, new Identifier(@1, "ivec4"), $3); }    

           | T_Bvec2 T_LeftParen Actuals T_RightParen 
                                     { $$ = new Call(Join(@1, @4), NULL, new Identifier(@1, "bvec2"), $3); }   
           | T_Bvec3 T_LeftParen Actuals T_RightParen 
                                     { $$ = new Call(Join(@1, @4), NULL, new Identifier(@1, "bvec2"), $3); }    
           | T_Bvec4 T_LeftParen Actuals T_RightParen 
                                     { $$ = new Call(Join(@1, @4), NULL, new Identifier(@1, "bvec2"), $3); }   

           | T_Uint  T_LeftParen Actuals T_RightParen 
                                     { $$ = new Call(Join(@1, @4), NULL, new Identifier(@1, "uint"), $3); } 
           | T_Uvec2 T_LeftParen Actuals T_RightParen 
                                     { $$ = new Call(Join(@1, @4), NULL, new Identifier(@1, "uvec2"), $3); }   
           | T_Uvec3 T_LeftParen Actuals T_RightParen 
                                     { $$ = new Call(Join(@1, @4), NULL, new Identifier(@1, "uvec2"), $3); }    
           | T_Uvec4 T_LeftParen Actuals T_RightParen 
                                     { $$ = new Call(Join(@1, @4), NULL, new Identifier(@1, "uvec2"), $3); }   
           ;


   


%%

/* The closing %% above marks the end of the Rules section and the beginning
 * of the User Subroutines section. All text from here to the end of the
 * file is copied verbatim to the end of the generated y.tab.c file.
 * This section is where you put definitions of helper functions.
 */

/* Function: InitParser
 * --------------------
 * This function will be called before any calls to yyparse().  It is designed
 * to give you an opportunity to do anything that must be done to initialize
 * the parser (set global variables, configure starting state, etc.). One
 * thing it already does for you is assign the value of the global variable
 * yydebug that controls whether yacc prints debugging information about
 * parser actions (shift/reduce) and contents of state stack during parser.
 * If set to false, no information is printed. Setting it to true will give
 * you a running trail that might be helpful when debugging your parser.
 * Please be sure the variable is set to false when submitting your final
 * version.
 */
void InitParser()
{
   PrintDebug("parser", "Initializing parser");
   yydebug = false;
}