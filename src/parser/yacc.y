%{
#include "ast.h"
#include "yacc.tab.h"
#include <iostream>
#include <memory>
#include "errors.h"

int yylex(YYSTYPE *yylval, YYLTYPE *yylloc);

void yyerror(YYLTYPE *locp, const char* s) {
    std::cerr << "Parser Error at line " << locp->first_line << " column " << locp->first_column << ": " << s << std::endl;
}

using namespace ast;
%}

// request a pure (reentrant) parser
%define api.pure full
// enable location in error handler
%locations
// enable verbose syntax error message
%define parse.error verbose

// keywords
%token SHOW LOAD TABLES CREATE TABLE DROP DESC INSERT INTO VALUES DELETE FROM ASC ORDER BY LIMIT
WHERE UPDATE SET SELECT INT CHAR FLOAT DATETIME INDEX AND JOIN EXIT HELP TXN_BEGIN TXN_COMMIT TXN_ABORT TXN_ROLLBACK ORDER_BY BIGINT 
COUNT MAX MIN SUM AS OUTPUT OFF
// non-keywords
%token LEQ NEQ GEQ T_EOF ADD MINUS

// type-specific tokens
%token <sv_str> IDENTIFIER VALUE_STRING
%token <sv_str> VALUE_INT_BIGINT
%token <sv_float> VALUE_FLOAT

// specify types for non-terminal symbol
%type <sv_node> stmt dbStmt ddl dml txnStmt loadDataStmt
%type <sv_field> field
%type <sv_fields> fieldList
%type <sv_type_len> type
%type <sv_comp_op> op
%type <sv_expr> expr
%type <sv_val> value
%type <sv_vals> valueList
%type <sv_str> tbName colName
%type <sv_sign> sign
%type <sv_strs> tableList colNameList
%type <sv_col> col aggregate_clause
%type <sv_cols> colList selector aggregate_list
%type <sv_set_clause> setClause
%type <sv_set_clauses> setClauses
%type <sv_cond> condition
%type <sv_conds> whereClause optWhereClause
%type <sv_orderby>  order_clause
%type <sv_orderbys> opt_order_clause  
%type <sv_orderby_list> order_clause_list
%type <sv_limit>  opt_limit_clause
%type <sv_aggregate_type>  aggregate_type
%type <sv_orderby_dir> opt_asc_desc

%%
start:
        stmt ';'
    {
        parse_tree = $1;
        YYACCEPT;
    }
    |   HELP
    {
        parse_tree = std::make_shared<Help>();
        YYACCEPT;
    }
    | SET OUTPUT OFF{
        parse_tree = std::make_shared<SetOutputOff>();
        YYACCEPT;
    }
    |   EXIT
    {
        parse_tree = nullptr;
        YYACCEPT;
    }
    |   T_EOF
    {
        parse_tree = nullptr;
        YYACCEPT;
    }
    ;

stmt:
        dbStmt
    |   ddl
    |   dml
    |   txnStmt
    |   loadDataStmt
    ;

txnStmt:
        TXN_BEGIN
    {
        $$ = std::make_shared<TxnBegin>();
    }
    |   TXN_COMMIT
    {
        $$ = std::make_shared<TxnCommit>();
    }
    |   TXN_ABORT
    {
        $$ = std::make_shared<TxnAbort>();
    }
    | TXN_ROLLBACK
    {
        $$ = std::make_shared<TxnRollback>();
    }
    ;

dbStmt:
        SHOW TABLES
    {
        $$ = std::make_shared<ShowTables>();
    }
    | SHOW INDEX  FROM tbName
    {
        $$ = std::make_shared<ShowIndexes>($4);
    }
    ;

ddl:
        CREATE TABLE tbName '(' fieldList ')'
    {
        $$ = std::make_shared<CreateTable>($3, $5);
    }
    |   DROP TABLE tbName
    {
        $$ = std::make_shared<DropTable>($3);
    }
    |   DESC tbName
    {
        $$ = std::make_shared<DescTable>($2);
    }
    |   CREATE INDEX tbName '(' colNameList ')'
    {
        $$ = std::make_shared<CreateIndex>($3, $5);
    }
    |   DROP INDEX tbName '(' colNameList ')'
    {
        $$ = std::make_shared<DropIndex>($3, $5);
    }
    ;

dml:
        INSERT INTO tbName VALUES '(' valueList ')'
    {
        $$ = std::make_shared<InsertStmt>($3, $6);
    }
    |   DELETE FROM tbName optWhereClause
    {
        $$ = std::make_shared<DeleteStmt>($3, $4);
    }
    |   UPDATE tbName SET setClauses optWhereClause
    {
        $$ = std::make_shared<UpdateStmt>($2, $4, $5);
    }
    |   SELECT selector FROM tableList optWhereClause opt_order_clause opt_limit_clause
    {
        $$ = std::make_shared<SelectStmt>($2, $4, $5, $6,$7);
    }
    ;

fieldList:
        field
    {
        $$ = std::vector<std::shared_ptr<Field>>{$1};
    }
    |   fieldList ',' field
    {
        $$.push_back($3);
    }
    ;

colNameList:
        colName
    {
        $$ = std::vector<std::string>{$1};
    }
    | colNameList ',' colName
    {
        $$.push_back($3);
    }
    ;

field:
        colName type
    {
        $$ = std::make_shared<ColDef>($1, $2);
    }
    ;

type:
        INT
    {
        $$ = std::make_shared<TypeLen>(SV_TYPE_INT, sizeof(int));
    }
    |   CHAR '(' VALUE_INT_BIGINT ')'
    {
        // 这里应该检查精度
        $$ = std::make_shared<TypeLen>(SV_TYPE_STRING, std::stoi($3.c_str()));
    }
    |   FLOAT
    {
        $$ = std::make_shared<TypeLen>(SV_TYPE_FLOAT, sizeof(float));
    }
    |   BIGINT
    {
        $$ = std::make_shared<TypeLen>(SV_TYPE_BIGINT, 8);
    }
    |   DATETIME
    {
        $$ = std::make_shared<TypeLen>(SV_TYPE_DATETIME, 8);
    }
    ;

valueList:
        value
    {
        $$ = std::vector<std::shared_ptr<Value>>{$1};
    }
    |   valueList ',' value
    {
        $$.push_back($3);
    }
    ;

value:
        VALUE_INT_BIGINT
    {
        $$ = std::make_shared<Int_Bint_Lit>($1);
    }
    |   VALUE_FLOAT
    {
        $$ = std::make_shared<FloatLit>($1);
    }
    |   VALUE_STRING
    {
        $$ = std::make_shared<StringLit>($1);
    }
    |   '+' VALUE_INT_BIGINT
    {
        $$ = std::make_shared<Int_Bint_Lit>($2);
    }
    |   '-' VALUE_INT_BIGINT
    {
        $$ = std::make_shared<Int_Bint_Lit>("-" + $2);
    }
    |   '+' VALUE_FLOAT
    {
        $$ = std::make_shared<FloatLit>($2);
    }
    |   '-' VALUE_FLOAT
    {
        $$ = std::make_shared<FloatLit>(-$2);
    }
    ;

condition:
        col op expr
    {
        $$ = std::make_shared<BinaryExpr>($1, $2, $3);
    }
    ;

optWhereClause:
        /* epsilon */ { /* ignore*/ }
    |   WHERE whereClause
    {
        $$ = $2;
    }
    ;

whereClause:
        condition 
    {
        $$ = std::vector<std::shared_ptr<BinaryExpr>>{$1};
    }
    |   whereClause AND condition
    {
        $$.push_back($3);
    }
    ;

col:
        tbName '.' colName
    {
        $$ = std::make_shared<Col>($1, $3);
    }
    |   colName
    {
        $$ = std::make_shared<Col>("", $1);
    }
    ;

colList:
        col
    {
        $$ = std::vector<std::shared_ptr<Col>>{$1};
    }
    |   colList ',' col
    {
        $$.push_back($3);
    }
    ;

op:
        '='
    {
        $$ = SV_OP_EQ;
    }
    |   '<'
    {
        $$ = SV_OP_LT;
    }
    |   '>'
    {
        $$ = SV_OP_GT;
    }
    |   NEQ
    {
        $$ = SV_OP_NE;
    }
    |   LEQ
    {
        $$ = SV_OP_LE;
    }
    |   GEQ
    {
        $$ = SV_OP_GE;
    }
    ;

sign:
        '+'
    {
        $$ = '+';
    }
    |   '-'
    {
        $$ = '-';
    }
    ;

expr:
        value
    {
        $$ = std::static_pointer_cast<Expr>($1);
    }
    |   col
    {
        $$ = std::static_pointer_cast<Expr>($1);
    }
    ;

setClauses:
        setClause
    {
        $$ = std::vector<std::shared_ptr<SetClause>>{$1};
    }
    |   setClauses ',' setClause
    {
        $$.push_back($3);
    }
    ;

setClause:
    colName '=' value
    {
        $$ = std::make_shared<SetClause>($1, '\0', $3);
    }
    |    colName '=' colName sign value
    {
        $$ = std::make_shared<SetClause>($1, $4, $5);
    }
    ;

selector:
        '*'
    {
        $$ = {};
    }
    |   colList
    |   aggregate_list
    ;

tableList:
        tbName
    {
        $$ = std::vector<std::string>{$1};
    }
    |   tableList ',' tbName
    {
        $$.push_back($3);
    }
    |   tableList JOIN tbName
    {
        $$.push_back($3);
    }
    ;

aggregate_list:
    aggregate_clause
    {
        $$  = std::vector<std::shared_ptr<Col>>{$1};
    }
    | aggregate_list ',' aggregate_clause
    {
        $$.push_back($3);
    }

aggregate_clause:
    aggregate_type '(' col ')'
    {
        $$ = $3;
        $$->aggregate_type = $1;
    }
    | aggregate_type '(' '*' ')'
    {
        $$ = std::make_shared<Col>("", "");
        $$->aggregate_type = $1;;
    }
    | aggregate_type '(' col ')' AS colName
    {
        $$ = $3;
        $$->aggregate_type = $1;
        $$->another_name = $6;
    }
    | aggregate_type '(' '*' ')' AS colName
    {
        $$ = std::make_shared<Col>("", "");
        $$->aggregate_type = $1;
        $$->another_name = $6;
    }

aggregate_type:
    COUNT
    {
        $$ = ast::COUNT;
    }
    | MAX
    {
        $$ = ast::MAX;
    }
    | MIN
    {
        $$ = ast::MIN;
    }
    | SUM
    {
        $$ = ast::SUM;
    }

opt_order_clause:
    ORDER BY order_clause_list      
    { 
        $$ = std::make_shared<OrderBys>($3);
    }
    |   /* epsilon */ { /* ignore*/ }
    ;
opt_limit_clause:
    LIMIT  value
    { 
        $$ = std::make_shared<Limit>($2); 
    }
    |   /* epsilon */ { /* ignore*/ }
    ;

order_clause_list:
    order_clause
    { 
        $$ =std::vector< std::shared_ptr<OrderBy>>{$1};
    }

    |order_clause_list ',' order_clause{

        $$.push_back($3);
    }
    ;   

order_clause:
           col  opt_asc_desc 
           {$$ = std::make_shared<OrderBy>($1,$2);}
           ;
opt_asc_desc:
    ASC          { $$ = OrderBy_ASC;     }
    |  DESC      { $$ = OrderBy_DESC;    }
    |       { $$ = OrderBy_DEFAULT; }
    ;    

loadDataStmt :
    LOAD VALUE_STRING INTO tbName {$$ = std::make_shared<LoadData>($2,$4);}

tbName: IDENTIFIER;

colName: IDENTIFIER;
%%
